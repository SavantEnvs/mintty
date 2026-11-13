# OSC command number overflows `int`

`osc-number-overflow.bin` is `ESC ] 1111111111 1`. In `term_do_write` (`src/termout.c`, `OSC_NUM` state)
`term.cmd_num * 10 + c - '0'` is computed in `int` and only checked for a negative result afterwards, so
a long run of digits is signed integer overflow (UBSan: `1111111111 * 10 cannot be represented in type 'int'`).

Fix: clamp before multiplying, e.g. `if (term.cmd_num > (INT_MAX - 9) / 10) term.cmd_num = -99; else ...`.

A second member of the same family: `CSI 2147483647 e` (VPR) adds `arg0_def1` to `curs->y` unchecked
(`src/termout.c`, `case 'e'`), which overflows `int`.
