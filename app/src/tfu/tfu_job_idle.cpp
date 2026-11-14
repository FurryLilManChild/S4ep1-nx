// TFU-nx (2026-11-12): what TFU's job workers do when their queue is empty (tools/tfu_spin_idle.py puts the call into
// the recompiled worker loop sub_826B1398, on its "queue empty" branch).
//
// The Switch CPU profile at stock clocks had 16.7 % of all samples, 25 % of the game's worker threads, inside that
// loop's own code: an idle worker reads the queue counters again at once, without waiting. The 360 ran it on six
// hardware threads; the Switch gives the game three cores, which the idle workers then share with the main and render
// threads. Here an idle worker first yields its core (to a thread of its priority that is ready) a few times, then
// sleeps a short while between looks. A new item is picked up at the next look: at most tfu_job_idle_sleep_us late.
//
// Not exact in timing only: the work, its order and the queue's counters are the game's own.
//
// TFU-nx (2026-11-13, build 4): with four cores (a forwarder launch, banjo_system_core 1) the game threads stay on cores
// 0-2 and core 3 holds the audio threads and the XMA Decoder. Test 6 at stock clocks: cores 0-2 full in the heavy
// scenes (19.6-25 fps) while core 3 was about half idle. The job workers are the only threads that run this hook, so
// here each worker adds core 3 to its own cores (tfu_job_core3), the first time it finds the queue empty and again
// every so often (a guest affinity call can narrow it back). The main and render threads keep cores 0-2. The audio
// threads on core 3 run above the workers' priority (0x2B against 0x3B), so a worker there waits for them, not the
// other way round.
#include <rex/cvar.h>
#include <rex/logging.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

#if defined(__SWITCH__)
#include <switch.h>
// sdk/src/core/threading_switch.cpp: the system core (3 when four cores are granted and banjo_system_core is 1 or 2,
// else -1), and libnx's own svcSetThreadCoreMask behind the SDK's --wrap (which takes the system core out of every
// mask of more than one core).
extern "C" int RexSwitchSystemCore(void);
extern "C" Result __real_svcSetThreadCoreMask(Handle handle, s32 preferred_core, u32 affinity_mask);
#endif

REXCVAR_DEFINE_BOOL(tfu_job_idle, true, "TFU",
                    "TFU's job workers (sub_826B1398): when the queue is empty, yield the core, then sleep "
                    "between looks (tfu_job_idle_spins, tfu_job_idle_sleep_us). false: the game's spinning as it was");
REXCVAR_DEFINE_INT32(tfu_job_idle_spins, 16, "TFU",
                     "tfu_job_idle: looks at an empty queue that only yield the core before the worker sleeps")
    .range(0, 100000);
REXCVAR_DEFINE_INT32(tfu_job_idle_sleep_us, 100, "TFU",
                     "tfu_job_idle: the sleep between looks after tfu_job_idle_spins (microseconds; 0 = always only "
                     "yield). On the PC only yields: Windows sleeps are a millisecond or more")
    .range(0, 5000);
REXCVAR_DEFINE_BOOL(tfu_job_core3, true, "TFU",
                    "Four-core mode (a forwarder launch): TFU's job workers may also run on core 3, the system core "
                    "(main and render threads stay on cores 0-2; the audio threads there have priority). No effect "
                    "with three cores or on the PC");

namespace {

std::atomic<uint64_t> g_looks{0}, g_yields{0}, g_sleeps{0};
std::atomic<uint32_t> g_core3_added{0}, g_core3_failed{0};  // tfu_job_core3: mask changes since the start
std::atomic<int64_t> g_last_report{0};

int64_t NowUs() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

void YieldCore() {
#if defined(__SWITCH__)
  svcSleepThread(0);  // to a ready thread of the same priority on this core, if any
#else
  std::this_thread::yield();
#endif
}

void SleepUs(int us) {
#if defined(__SWITCH__)
  svcSleepThread(static_cast<int64_t>(us) * 1000);
#else
  (void)us;
  std::this_thread::yield();
#endif
}

// tfu_job_core3: the calling worker's cores plus the system core. Cheap enough for every 4096th look: one svc.
void CheckCore3() {
#if defined(__SWITCH__)
  thread_local uint32_t looks = 0;
  if ((looks++ & 4095) != 0 || !REXCVAR_GET(tfu_job_core3))
    return;
  const int sys = RexSwitchSystemCore();
  if (sys < 0)
    return;
  s32 preferred = 0;
  u64 mask = 0;
  if (R_FAILED(svcGetThreadCoreMask(&preferred, &mask, CUR_THREAD_HANDLE)) || (mask & (1ull << sys)) != 0)
    return;
  if (R_SUCCEEDED(__real_svcSetThreadCoreMask(CUR_THREAD_HANDLE, preferred, u32(mask | (1ull << sys)))))
    g_core3_added.fetch_add(1, std::memory_order_relaxed);
  else
    g_core3_failed.fetch_add(1, std::memory_order_relaxed);
#endif
}

void MaybeReport(int64_t now) {
  int64_t last = g_last_report.load(std::memory_order_relaxed);
  if (last == 0) {
    g_last_report.compare_exchange_strong(last, now, std::memory_order_relaxed);
    return;
  }
  if (now - last < 10'000'000 || !g_last_report.compare_exchange_strong(last, now, std::memory_order_relaxed))
    return;
  const uint64_t looks = g_looks.exchange(0, std::memory_order_relaxed);
  const uint64_t yields = g_yields.exchange(0, std::memory_order_relaxed);
  const uint64_t sleeps = g_sleeps.exchange(0, std::memory_order_relaxed);
  REXLOG_INFO("[tfu] job workers, {:.1f} s: {} looks at an empty queue ({} yielded the core, {} slept {} us); "
              "tfu_job_idle {}; core 3 added to a worker {} times ({} failed), tfu_job_core3 {}",
              (now - last) / 1e6, looks, yields, sleeps, REXCVAR_GET(tfu_job_idle_sleep_us),
              REXCVAR_GET(tfu_job_idle) ? "on" : "off", g_core3_added.load(std::memory_order_relaxed),
              g_core3_failed.load(std::memory_order_relaxed), REXCVAR_GET(tfu_job_core3) ? "on" : "off");
}

}  // namespace

extern "C" void tfu_job_idle(void) {
  const int64_t now = NowUs();
  g_looks.fetch_add(1, std::memory_order_relaxed);
  MaybeReport(now);
  CheckCore3();
  if (!REXCVAR_GET(tfu_job_idle))
    return;
  // Looks in a row: a gap longer than a sleep and then some means the worker ran an item in between.
  thread_local uint32_t in_a_row = 0;
  thread_local int64_t last_look = 0;
  const int sleep_us = REXCVAR_GET(tfu_job_idle_sleep_us);
  if (now - last_look > 2 * sleep_us + 200)
    in_a_row = 0;
  last_look = now;
  if (++in_a_row <= static_cast<uint32_t>(REXCVAR_GET(tfu_job_idle_spins)) || sleep_us == 0) {
    g_yields.fetch_add(1, std::memory_order_relaxed);
    YieldCore();
  } else {
    g_sleeps.fetch_add(1, std::memory_order_relaxed);
    SleepUs(sleep_us);
    last_look = NowUs();
  }
}
