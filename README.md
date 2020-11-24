## RTAI 5.2.3 for LinuxCNC

IMPORTANT NOTICE: STOP COPYING MY WORK FROM THIS TREE WITHOUT CREDITING ME!

### Only AMD64 CPUs are supported!

Xenomai has dropped 32-bit IPIPE support since kernel 4.14.71. The earliest kernel series
supported in this tree is 4.19.

IA32 emulation and X32 ABI have been disabled in Kconfig due to various build errors.
32-bit binaries will not work.
