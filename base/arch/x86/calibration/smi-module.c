/*
 *   SMI workaround for x86.
 *   
 *   Copyright (C) Vitor Angelo
 *   Copyright (C) Gilles Chanteperdrix
 *   Copyright (C) Alberto Sechi
 *
 *   Cut/Pasted from Vitor Angelo "smi" module.
 *   Adapted by Gilles Chanteperdrix <gilles.chanteperdrix@laposte.net>.
 *   Further adaptation by Alberto Sechi <sechi@aero.polimi.it>.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, 
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/reboot.h>

#include <rtai_wrappers.h>

/* set these as you need */
#define CONFIG_RTAI_HW_SMI_ALL		0
#define CONFIG_RTAI_HW_SMI_INTEL_USB2	0
#define CONFIG_RTAI_HW_SMI_LEGACY_USB2	0
#define CONFIG_RTAI_HW_SMI_PERIODIC	0
#define CONFIG_RTAI_HW_SMI_TCO		1
#define CONFIG_RTAI_HW_SMI_MC		0
#define CONFIG_RTAI_HW_SMI_APMC		0
#define CONFIG_RTAI_HW_SMI_LEGACY_USB	0
#define CONFIG_RTAI_HW_SMI_BIOS		0

#ifndef PCI_DEVICE_ID_INTEL_ICH7_0
#define PCI_DEVICE_ID_INTEL_ICH7_0  0x27b8
#endif
#ifndef PCI_DEVICE_ID_INTEL_ICH7_1
#define PCI_DEVICE_ID_INTEL_ICH7_1  0x27b9
#endif
#ifndef PCI_DEVICE_ID_INTEL_ICH8_4
#define PCI_DEVICE_ID_INTEL_ICH8_4  0x2815
#endif
#ifndef PCI_DEVICE_ID_INTEL_ICH10_1
#define #define PCI_DEVICE_ID_INTEL_ICH10_1  0x3a16 
#endif

#ifndef PCI_DEVICE_ID_INTEL_ICH9_1
#define PCI_DEVICE_ID_INTEL_ICH9_1 0x2917
#endif
#ifndef PCI_DEVICE_ID_INTEL_ICH9_5
#define PCI_DEVICE_ID_INTEL_ICH9_5 0x2919
#endif
#ifndef PCI_DEVICE_ID_INTEL_PCH_LPC_MIN
#define PCI_DEVICE_ID_INTEL_PCH_LPC_MIN 0x3b00
#endif
#ifndef PCI_DEVICE_ID_INTEL_ESB2_0
#define PCI_DEVICE_ID_INTEL_ESB2_0 0x2670
#endif

#ifndef PCI_DEVICE_ID_INTEL_H77_EXPRESS_CHIPSET_LPC
#define PCI_DEVICE_ID_INTEL_H77_EXPRESS_CHIPSET_LPC 0x1e4a
#endif
#ifndef PCI_DEVICE_ID_INTEL_SUNRISE_POINT_H_LPC_0
#define PCI_DEVICE_ID_INTEL_SUNRISE_POINT_H_LPC_0 0xa145
#endif
#ifndef PCI_DEVICE_ID_INTEL_SUNRISE_POINT_H_LPC_1
#define PCI_DEVICE_ID_INTEL_SUNRISE_POINT_H_LPC_1 0xa149
#endif
#ifndef PCI_DEVICE_ID_INTEL_CELERON_J1900
#define PCI_DEVICE_ID_INTEL_CELERON_J1900 0x0f1c
#endif

static struct pci_device_id hal_smi_pci_tbl[] = {
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801AA_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801AB_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801BA_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801BA_10) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801E_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801CA_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801CA_12) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801DB_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801DB_12) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801EB_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH6_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH6_1) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH6_2) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH7_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH7_1) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH8_4) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH10_1) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_1) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_5) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_PCH_LPC_MIN) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ESB2_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_H77_EXPRESS_CHIPSET_LPC) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SUNRISE_POINT_H_LPC_0) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SUNRISE_POINT_H_LPC_1) },
{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_CELERON_J1900) },
{ 0, },
{ 0, },
};

#define DEVFN        0xf8 /* device 31, function 0 */
    
#define PMBASE_B0    0x40
#define PMBASE_B1    0x41

#define SMI_CTRL_ADDR    0x30
#define SMI_STATUS_ADDR  0x34
#define SMI_MON_ADDR     0x40

/* SMI_EN register: ICH[0](16 bits), ICH[2-5](32 bits) */
#define INTEL_USB2_EN_BIT   (0x01 << 18) /* ICH4, ... */
#define LEGACY_USB2_EN_BIT  (0x01 << 17) /* ICH4, ... */
#define PERIODIC_EN_BIT     (0x01 << 14) /* called 1MIN_ in ICH0 */
#define TCO_EN_BIT          (0x01 << 13)
#define MCSMI_EN_BIT        (0x01 << 11)
#define SWSMI_TMR_EN_BIT    (0x01 << 6)
#define APMC_EN_BIT         (0x01 << 5)
#define SLP_EN_BIT          (0x01 << 4)
#define LEGACY_USB_EN_BIT   (0x01 << 3)
#define BIOS_EN_BIT         (0x01 << 2)
#define GBL_SMI_EN_BIT      (0x01 << 0)  /* This is reset by a PCI reset event! */

unsigned long hal_smi_masked_bits = 0x1
#if 0
#if CONFIG_RTAI_HW_SMI_ALL
    | GBL_SMI_EN_BIT
#else
#if CONFIG_RTAI_HW_SMI_INTEL_USB2
    | INTEL_USB2_EN_BIT
#endif
#if CONFIG_RTAI_HW_SMI_LEGACY_USB2
    | LEGACY_USB2_EN_BIT
#endif
#if CONFIG_RTAI_HW_SMI_PERIODIC
    | PERIODIC_EN_BIT
#endif
#if CONFIG_RTAI_HW_SMI_TCO
    | TCO_EN_BIT
#endif
#if CONFIG_RTAI_HW_SMI_MC
    | MCSMI_EN_BIT
#endif
#if CONFIG_RTAI_HW_SMI_APMC
    | APMC_EN_BIT
#endif
#if CONFIG_RTAI_HW_SMI_LEGACY_USB
    | LEGACY_USB_EN_BIT
#endif
#if CONFIG_RTAI_HW_SMI_BIOS
    | BIOS_EN_BIT
#endif
#endif
#endif
;
RTAI_MODULE_PARM(hal_smi_masked_bits, ulong);

static int user_smi_device;
RTAI_MODULE_PARM(user_smi_device, int);

static int disp_smi_count;
RTAI_MODULE_PARM(disp_smi_count, int);

static unsigned long original_smi_value;
static unsigned long hal_smi_saved_bits;
static unsigned short hal_smi_en_addr;
static struct pci_dev *smi_dev;

#define mask_bits(v, p)  outl(inl(p) & ~(v), (p))
#define  set_bits(v, p)  outl(inl(p) |  (v), (p))

static int rtai_smi_notify_reboot(struct notifier_block *nb, unsigned long event, void *p)
{
	switch (event) {
		case SYS_DOWN:
		case SYS_HALT:
		case SYS_POWER_OFF:
		if (hal_smi_en_addr) {
			set_bits(hal_smi_saved_bits, hal_smi_en_addr);
		}
	}
	return NOTIFY_DONE;
}

static struct notifier_block rtai_smi_reboot_notifier = {
        .notifier_call  = &rtai_smi_notify_reboot,
        .next           = NULL,
        .priority       = 0
};

static void hal_smi_restore(void)
{
	if (hal_smi_en_addr) {
		set_bits(hal_smi_saved_bits, hal_smi_en_addr);
		pci_dev_put(smi_dev);
		unregister_reboot_notifier(&rtai_smi_reboot_notifier);
	}
}

static void hal_smi_disable(void)
{
 	if (hal_smi_en_addr) {
 		original_smi_value = inl(hal_smi_en_addr);
		hal_smi_saved_bits = original_smi_value & hal_smi_masked_bits;
		mask_bits(hal_smi_masked_bits, hal_smi_en_addr);
		register_reboot_notifier(&rtai_smi_reboot_notifier);
	}
}

static unsigned short get_smi_en_addr(struct pci_dev *dev)
{
	u_int8_t byte0, byte1;

	pci_read_config_byte(dev, PMBASE_B0, &byte0);
	pci_read_config_byte(dev, PMBASE_B1, &byte1);
	return SMI_CTRL_ADDR + (((byte1 << 1) | (byte0 >> 7)) << 7); //bits 7-15
}

static int hal_smi_init(void)
{
	struct pci_dev *dev = NULL;
	struct pci_device_id *id;

/*
 * Do not use pci_register_driver, pci_enable_device, ...
 * Just register the used ports.
 */
	if (user_smi_device) {
		id = &hal_smi_pci_tbl[sizeof(hal_smi_pci_tbl)/sizeof(struct pci_device_id) - 1];
		id->vendor = PCI_VENDOR_ID_INTEL;
		id->device = user_smi_device;
		dev = pci_get_device(id->vendor, user_smi_device, NULL);
		printk("RTAI: User assigned Intel SMI chipset (%0x:%0x).\n", id->vendor, id->device);
	} else {
		for (id = &hal_smi_pci_tbl[0]; dev == NULL && id->vendor != 0; id++) {
		        dev = pci_get_device(id->vendor, id->device, NULL);
		}
		id--;
	}

	if (dev == NULL || dev->bus->number || dev->devfn != DEVFN) {
		pci_dev_put(dev);
		printk("RTAI: Intel SMI chipset not found.\n");
  		return -ENODEV;
        }

	printk("RTAI: Intel SMI chipset found (%0x:%0x), enabling SMI workaround.\n", id->vendor, id->device);
	hal_smi_en_addr = get_smi_en_addr(dev);
	smi_dev = dev;
	hal_smi_disable();
  	return 0;
}

/************************************************************************/

#include <asm/cpufeature.h>
#include <asm/msr.h>
static int check_smi_count(void)
{
	int cpuid, ret;
	unsigned long long q;
	struct cpuinfo_x86 p;

	if (!cpu_has(&p, X86_FEATURE_MSR)) {
		printk("MSR NOT SUPPORTED\n");
	} else {
		for (cpuid = 0; cpuid < num_active_cpus(); cpuid++) {
			ret = rdmsrl_safe_on_cpu(cpuid, MSR_SMI_COUNT, &q);
			if (!ret) {
				printk("- CPU %d (RETVAL %d), SMI COUNT %llu.\n", cpuid, ret, q);
			} else {
				printk("- CPU %d (RETVAL %d), CANNOT READ SMI COUNT REGISTER 0x%x.\n", cpuid, ret, MSR_SMI_COUNT);
			}
		}
	}

	return disp_smi_count;
}

static char *hint[] = { " GBL_SMI,", " EOS (special),", " BIOS,", " LEGACY_USB,", " SLP,", " APMC,", " SWSMI_TMR,", "", "", "", "", " MCSMI,", "", " TCO,", " PERIODIC,", "", "", " LEGACY_USB2,", " INTEL_USB2,"};

static void reminders(unsigned long smi_value)
{
	char echo[200] = "Bits enabled:";
	int i;
	for (i = 0; i < sizeof(hint)/sizeof(char *); i++) {
		if (smi_value & (1 << i)) {
			strcat(echo, hint[i]);
		}
	}
	printk("%s\b.\n", echo);
	return;
}

int init_module(void)
{
	int retval;
	if (check_smi_count()) return 0;
	if (!(retval = hal_smi_init())) {
		unsigned long smi_value;
		smi_value = inl(hal_smi_en_addr);
		reminders(original_smi_value);
		printk("Original SMI configuration value 0x%lx has been cleared with mask = 0x%lx (saved mask setting 0x%lx), new value 0x%lx.\n", original_smi_value, hal_smi_masked_bits, hal_smi_saved_bits, smi_value);
		reminders(smi_value);
	}
	return retval;
}

void cleanup_module(void)         
{
	unsigned long smival;
	if (disp_smi_count) return;
	smival = inl(hal_smi_en_addr);
	hal_smi_restore();
	printk("SMI configuration value 0x%lx has been reset to its original value 0x%x, saved mask used = 0x%lx.\n", smival, inl(hal_smi_en_addr), hal_smi_saved_bits);
	return;
}

MODULE_LICENSE("GPL");

/**************************************************************************/
/*

   FIXME: there are many more SMI sources than those of the SMI_EN
   register. From http://www.intel.com/design/chipsets/datashts/252516.htm
   there are at least the following other sources :

   pages 377, 386, 388, 389; Power management
       register GEN_PMCON1, bit SMI_LOCK, locks GLB_SMI_EN
       bits PER_SMI_SEL, allow selection of the periodic SMI
       registers PM1_STS, PM1_EN, PM1_CNT bit SCI_EN, if cleared generates SMI
       for power management events.

   pages 173, 381, 400; GPIOs
       register GPI[0-15]_ROUT allow routing each GPIO to SMI or SCI
       register ALT_GP_SMI_EN, ALT_GP_SMI_STS, allow masking SMIs for GPIOs

   pages 184, 188, 402; legacy devices emulation (ATA, floppy, parallel, UARTs,
       keyboard). I/O to specified ports may cause events, which can generate an
       SMI, depending on registers configuration :
       register DEVTRAP_EN, DEVTRAP_STS
       BIG FAT WARNING : globally disabling SMI on a box with SATA disks and
           SATA controller in "legacy" mode, probably prevents disks from
           working.

   pages 382, 383, 400; Monitors ?
       seem to be a generic legacy device emulation (like previous), registers
       MON[4-7]_FWD_EN, enables forwarding of I/O to LPC
       MON[4-7]_TRP_RNG, address of the emulated devices
       MON[4-7]_TRP_MSK and MON_SMI (registers MON[4-7]_TRAP_EN and
                                     MON[4-7]_TRAP_STS)

   page 407: TCO
       register TCO1_CNT, bit NMI2SMI_EN, enables TCO to use SMI instead of NMI,
       bit TCO_TMR_HLT, should be cleared to avoid being rebooted when the TCO
       timer expires. Dangerous bit: TCO_LOCK locks the TCO timer until reboot.
       register used by Linux drivers/char/watchdog/i8xx_tco.c

   page 492, 493: USB EHCI legacy support and SPECIAL SMI, i.e Intel Specific
       USB 2.0 SMI register.
       
   page 520, SMBus
       may be disabled by clearing register HOSTC, bit SMB_SMI_EN
       register used by Linux driver drivers/i2c/busses/i2c-i801.c

*/
