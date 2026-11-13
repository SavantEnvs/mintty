# Out-of-bounds read of `types[count]` in rule W5

`et-run-oob.bin` is a bidi flags byte followed by the code points U+0025 and U+0000. In `do_bidi`
(`src/minibidi.c`, rule W5) the loop `while (j < count && types[j] == ET) j++;` can leave `j == count`
when a run of European terminators reaches the end of the line, and the next statement reads
`types[j]` (UBSan: `index 2 out of bounds for type 'uchar [*]'`; ASan: a heap/stack over-read).

Fix: `if (j < count && types[j] == EN) types[i] = EN;`.
