#!/usr/bin/env python3
"""TFU-nx (2026-11-12): TFU's job workers wait for work without spinning flat out.

The Switch CPU profile of 2026-11-12 (stock clocks, in the first level): 16.7 % of all samples, 25 % of the game's
worker threads, inside sub_826B1398 itself and not in the work it hands out. sub_826B1398 is a job-queue worker: it
reads a queue's next and end counters with lwarx / stwcx. pairs, takes an item with a compare-and-swap, runs it
(sub_826B0D40), and when the queue is empty goes back to read a state word and the counters again, with no wait,
for as long as that word stays below 1024. On the 360's six hardware threads an idle worker spinning cost little;
on the Switch's three cores the idle workers take CPU from the main and render threads (a theory: build 2 tests it).

This step adds one call, tfu_job_idle() (app/src/tfu/tfu_job_idle.cpp), on the "queue empty" branch of the loop
(the `bge cr6,0x826b1578` after the next / end compare): the worker gives its core away for a moment there, and
does nothing different otherwise. The work, its order and the counters are untouched: only the timing of an idle
worker's next look changes. Switched off at run time with tfu_job_idle = false (renut.toml).

The generated code is game-derived and not in git, so the change is made to app/generated at build time: by
tools/post_codegen.py (last step) and by the build scripts before every build (idempotent; the file is written only
when it changes, so an unchanged build recompiles nothing).

  python tools/tfu_spin_idle.py [--gen app/generated] [--check]
"""
import argparse
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FUNC = 'sub_826B1398'
MARK = 'tfu_job_idle();'
DECL = 'extern "C" void tfu_job_idle(void);  // TFU-nx: tools/tfu_spin_idle.py, app/src/tfu/tfu_job_idle.cpp'
# The queue-empty branch: `cmplw cr6,r11,r10` then `bge cr6,0x826b1578` (the other branches to 0x826B1578 are a bne
# and a beq, after an item was taken).
# After the post-codegen steps the branch may go to a loop-exit label (loc_F0000001: loop_locals.py writes the loop's
# locals back there first), so the label is taken as it stands.
BRANCH = re.compile(r'^(\s*)// bge cr6,0x826b1578\n(\s*)if \(!((?:ctx\.)?cr6)\.lt\) goto (loc_\w+);$', re.M)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--gen', default=os.path.join(ROOT, 'app', 'generated'))
    ap.add_argument('--check', action='store_true', help='only say whether the change is there')
    a = ap.parse_args()
    target = None
    for path in sorted(glob.glob(os.path.join(a.gen, '*.cpp'))):
        raw = open(path, 'rb').read()
        if ('DEFINE_REX_FUNC(%s)' % FUNC).encode() in raw:
            target = path
            break
    if not target:
        print('tfu_spin_idle: %s not found in %s' % (FUNC, a.gen))
        return 1
    raw = open(target, encoding='utf-8', newline='').read()
    nl = '\r\n' if '\r\n' in raw else '\n'
    text = raw.replace('\r\n', '\n')
    start = text.index('DEFINE_REX_FUNC(%s)' % FUNC)
    end = text.index('\n}\n', start) + 3
    body = text[start:end]
    if MARK in body:
        print('tfu_spin_idle: already in %s (%s)' % (os.path.basename(target), FUNC))
        return 0
    if a.check:
        print('tfu_spin_idle: NOT in %s (%s)' % (os.path.basename(target), FUNC))
        return 1
    found = BRANCH.findall(body)
    if len(found) != 1:
        print('tfu_spin_idle: expected one queue-empty branch in %s, found %d: nothing changed' % (FUNC, len(found)))
        return 1
    body = BRANCH.sub(lambda m: '%s// bge cr6,0x826b1578 (TFU-nx: the queue is empty: give the core away a moment)\n'
                                '%sif (!%s.lt) { %s goto %s; }' % (m.group(1), m.group(2), m.group(3), MARK, m.group(4)),
                      body)
    text = text[:start] + DECL + '\n' + body + text[end:]
    open(target, 'w', encoding='utf-8', newline='').write(text.replace('\n', nl))
    print('tfu_spin_idle: added to %s in %s' % (FUNC, os.path.basename(target)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
