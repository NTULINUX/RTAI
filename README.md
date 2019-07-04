## RTAI 5.2.1 for LinuxCNC

IMPORTANT NOTICE: STOP COPYING MY WORK FROM THIS TREE WITHOUT CREDITING ME!

### Only AMD64 CPUs are supported!

Starting with IPIPE 4.14.71 from Xenomai, native 32-bit support has been disabled.
This is unlikely to change in the future, but 32-bit support will remain in RTAI for the time being.
IA32 emulation and X32 ABI has been disabled due to build errors.

If you are willing to test 32-bit support with IPIPE, patch the kernel source with all of the
available patches for that kernel version, then inside the kernel source directory, run:

`sed -i 's/depends on X86_64/depends on X86/g' kernel/ipipe/Kconfig`

Please note that 32-bit support is not even compile tested!
