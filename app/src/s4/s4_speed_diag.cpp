// S4ep1-nx (2026-11-15): the slow-motion check. On Renut the game runs 60 frames a second, the guest clock matches real
// time and every thread is mostly idle (PC profile), yet the game moves at about half speed; on the emulated GPU it is
// right. This counts, every 10 s of real time and on either renderer, how often the game's own frame pieces run, so the
// two renderers can be compared line by line:
//  - sub_821647A0: called twice in each pass of the main loop (sub_821644F8);
//  - sub_82488958: the frame pacer's per-frame step (chooses a wait by the display's refresh against 60 fps);
//  - sub_82488698: the wait for a pacer's deadline;
//  - sub_821C65E0: the 360 D3D's GPU interrupt handler (vertical blank and swap), registered by D3D's device setup;
//  - sub_821C0770: D3D's swap-done callback (the GPU's swap interrupt).
// And the values the pacer reads: the display refresh at 0x824F1390, and each pacer's period (+32) at 0x824F0FA8 and
// 0x824F1330.
#include <rex/hook.h>
#include <rex/logging.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>

#include <fmt/format.h>

REX_EXTERN(__imp__sub_821647A0);
REX_EXTERN(__imp__sub_82488958);
REX_EXTERN(__imp__sub_82488698);
REX_EXTERN(__imp__sub_821C65E0);
REX_EXTERN(__imp__sub_821C0770);
REX_EXTERN(__imp__sub_8249BAB0);
REX_EXTERN(__imp__sub_824891C0);
REX_EXTERN(__imp__sub_82219A00);
REX_EXTERN(__imp__sub_82496AF0);
REX_EXTERN(__imp__sub_824960B0);
REX_EXTERN(__imp__sub_8249ECA8);

namespace {
std::atomic<uint32_t> g_update{0}, g_pacer{0}, g_wait{0}, g_interrupt{0}, g_swap_done{0};
// 2026-11-15, run 2: the frame thread (sub_82488B10's loop) stepped 88 times in 10 s... a second on the emulated GPU
// and 15-28 on Renut, with the main loop the same (120 pieces a second) on both. Wall time spent in each piece of that
// loop, per 10 s, says which one holds it up.
enum Piece { kPacer, kBAB0, k91C0, k9A00, k6AF0, k60B0, kECA8, kPieces };
const char *const kPieceNames[kPieces] = {"pacer 82488958", "8249BAB0", "824891C0", "82219A00", "82496AF0",
                                          "824960B0", "8249ECA8"};
std::atomic<uint64_t> g_piece_us[kPieces];
std::atomic<uint32_t> g_piece_calls[kPieces];
uint64_t NowUs() {
  return uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count());
}
struct PieceTimer {
  Piece p;
  uint64_t t0;
  explicit PieceTimer(Piece piece) : p(piece), t0(NowUs()) {}
  ~PieceTimer() {
    g_piece_us[p].fetch_add(NowUs() - t0, std::memory_order_relaxed);
    g_piece_calls[p].fetch_add(1, std::memory_order_relaxed);
  }
};

uint32_t Rd32(const uint8_t *base, uint32_t va) {
  uint32_t v;
  std::memcpy(&v, base + va, 4);
  return __builtin_bswap32(v);
}
uint64_t Rd64(const uint8_t *base, uint32_t va) {
  uint64_t v;
  std::memcpy(&v, base + va, 8);
  return __builtin_bswap64(v);
}
float RdF32(const uint8_t *base, uint32_t va) {
  const uint32_t u = Rd32(base, va);
  float f;
  std::memcpy(&f, &u, 4);
  return f;
}

void MaybeReport(const uint8_t *base) {
  static std::atomic<int64_t> next{0};
  const int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now().time_since_epoch())
                          .count();
  int64_t due = next.load(std::memory_order_relaxed);
  if (due == 0) {
    next.compare_exchange_strong(due, now + 10000);
    return;
  }
  if (now < due || !next.compare_exchange_strong(due, now + 10000))
    return;
  REXLOG_INFO("[s4 speed] 10 s: main-loop pieces {}, pacer steps {}, deadline waits {}, D3D GPU interrupts {}, "
              "D3D swap-done callbacks {}; display refresh {}, pacer periods {} and {} ticks",
              g_update.exchange(0), g_pacer.exchange(0), g_wait.exchange(0), g_interrupt.exchange(0),
              g_swap_done.exchange(0), RdF32(base, 0x824F1390), Rd64(base, 0x824F0FA8 + 32),
              Rd64(base, 0x824F1330 + 32));
  std::string pieces;
  for (int i = 0; i < kPieces; ++i)
    pieces += fmt::format("{}{} {}x {:.1f} ms", i ? ", " : "", kPieceNames[i], g_piece_calls[i].exchange(0),
                          double(g_piece_us[i].exchange(0)) / 1000.0);
  REXLOG_INFO("[s4 speed] frame thread pieces in 10 s (calls, wall time): {}", pieces);
}
}  // namespace

REX_HOOK_RAW(sub_821647A0) {
  g_update.fetch_add(1, std::memory_order_relaxed);
  MaybeReport(base);
  __imp__sub_821647A0(ctx, base);
}
REX_HOOK_RAW(sub_82488958) {
  PieceTimer timer(kPacer);
  g_pacer.fetch_add(1, std::memory_order_relaxed);
  __imp__sub_82488958(ctx, base);
}
REX_HOOK_RAW(sub_82488698) {
  g_wait.fetch_add(1, std::memory_order_relaxed);
  __imp__sub_82488698(ctx, base);
}
REX_HOOK_RAW(sub_821C65E0) {
  g_interrupt.fetch_add(1, std::memory_order_relaxed);
  __imp__sub_821C65E0(ctx, base);
}
REX_HOOK_RAW(sub_821C0770) {
  g_swap_done.fetch_add(1, std::memory_order_relaxed);
  __imp__sub_821C0770(ctx, base);
}
REX_HOOK_RAW(sub_8249BAB0) {
  PieceTimer timer(kBAB0);
  __imp__sub_8249BAB0(ctx, base);
}
REX_HOOK_RAW(sub_824891C0) {
  PieceTimer timer(k91C0);
  __imp__sub_824891C0(ctx, base);
}
REX_HOOK_RAW(sub_82219A00) {
  PieceTimer timer(k9A00);
  __imp__sub_82219A00(ctx, base);
}
REX_HOOK_RAW(sub_82496AF0) {
  PieceTimer timer(k6AF0);
  __imp__sub_82496AF0(ctx, base);
}
REX_HOOK_RAW(sub_824960B0) {
  PieceTimer timer(k60B0);
  __imp__sub_824960B0(ctx, base);
}
REX_HOOK_RAW(sub_8249ECA8) {
  PieceTimer timer(kECA8);
  __imp__sub_8249ECA8(ctx, base);
}
