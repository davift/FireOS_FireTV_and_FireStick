/*
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * To match the U-Boot user interface on ARM platforms to the U-Boot
 * standard (as on PPC platforms), some messages with debug character
 * are removed from the default U-Boot build.
 *
 * Define DEBUG here if you want additional info as shown below
 * printed upon startup:
 *
 * U-Boot code: 00F00000 -> 00F3C774  BSS: -> 00FC3274
 * IRQ Stack: 00ebff7c
 * FIQ Stack: 00ebef7c
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <timestamp.h>
#include <version.h>
#include <net.h>
#include <serial.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <mmc.h>
#include <stdio.h>
#include <linux/ctype.h>
// MSTAR start
#include <MsInit.h>
#include <ShareType.h>
#include <MsSystem.h>
#include <CusUpgrade.h>

// MSTAR end

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

#ifdef CONFIG_DRIVER_SMC91111
#include "../drivers/net/smc91111.h"
#endif
#ifdef CONFIG_DRIVER_LAN91C96
#include "../drivers/net/lan91c96.h"
#endif

#ifdef UFBL_FEATURE_IDME
#include <idme.h>
#define DEFAULT_IP_ADDR "192.168.1.101"
#endif

#include <mstarstr.h>

/* initialize gd to make sure it in data section*/
gd_t *gd =(gd_t *)0xbabebabe;

ulong monitor_flash_len;
#ifdef CONFIG_HAS_DATAFLASH
extern int  AT91F_DataflashInit(void);
extern void dataflash_print_info(void);
#endif

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

const char version_string[] =
	U_BOOT_VERSION" (" U_BOOT_DATE " - " U_BOOT_TIME ")"CONFIG_IDENT_STRING;

#ifdef CONFIG_DRIVER_RTL8019
extern void rtl8019_get_enetaddr (uchar * addr);
#endif

#if defined(CONFIG_HARD_I2C) || \
    defined(CONFIG_SOFT_I2C)
#include <i2c.h>
#endif

#ifdef CONFIG_MSTAR_UFS
extern int ufs_init(void);
#endif

//#define DYNAMIC_RELOCATE_BIST 1

struct rpmb_fs_partition* rpmbData;

/************************************************************************
 * Coloured LED functionality
 ************************************************************************
 * May be supplied by boards if desired
 */
void inline __coloured_LED_init (void) {};
void coloured_LED_init (void) __attribute__((weak, alias("__coloured_LED_init")));
void inline __red_LED_on (void) {}
void red_LED_on (void) __attribute__((weak, alias("__red_LED_on")));
void inline __red_LED_off(void) {}
void red_LED_off(void) __attribute__((weak, alias("__red_LED_off")));
void inline __green_LED_on(void) {}
void green_LED_on(void) __attribute__((weak, alias("__green_LED_on")));
void inline __green_LED_off(void) {}
void green_LED_off(void) __attribute__((weak, alias("__green_LED_off")));
void inline __yellow_LED_on(void) {}
void yellow_LED_on(void) __attribute__((weak, alias("__yellow_LED_on")));
void inline __yellow_LED_off(void) {}
void yellow_LED_off(void) __attribute__((weak, alias("__yellow_LED_off")));
void inline __blue_LED_on(void) {}
void blue_LED_on(void) __attribute__((weak, alias("__blue_LED_on")));
void inline __blue_LED_off(void) {}
void blue_LED_off(void) __attribute__((weak, alias("__blue_LED_off")));

#if defined (CONFIG_POINTS_HANDLER_ENABLE)
extern int snprintf(char *str, size_t size, const char *fmt, ...);
#endif

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * or dropped completely,
 * but let's get it working (again) first...
 */

#if defined(CONFIG_ARM_DCC) && !defined(CONFIG_BAUDRATE)
#define CONFIG_BAUDRATE 115200
#endif
static int init_baudrate (void)
{
	char tmp[64];	/* long enough for environment variables */
	int i = getenv_f("baudrate", tmp, sizeof (tmp));

	gd->baudrate = (i > 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;

	return (0);
}

// this code is more prior, so we can't use array to extern. it will be cleared
#if defined (CONFIG_POINTS_HANDLER_ENABLE)
#define CMD_BUF                 128
char aPrePoints[CMD_BUF] = "MB_start_ub";
unsigned int start_ub_time = 9;   // value: only in data section
#endif
static int display_banner (void)
{
    /* checkpoint for autotest boottime, plz dont remove it */
    extern unsigned long G_MS_BOOTTIME_SBOOT; // global variable for storing the boot time used in sboot (ms)
    unsigned int PiuTick = *(volatile unsigned int *)(0x1f006090);
    PiuTick |= (*(volatile unsigned int *)(0x1f006094)) << 16;
    unsigned int PiuTime = PiuTick / 12000;
    G_MS_BOOTTIME_SBOOT = (unsigned long)PiuTime;
    printf("[AT][MB][start ub][%u]", PiuTime);
    #if defined (CONFIG_POINTS_HANDLER_ENABLE)
    start_ub_time = PiuTime;
    #endif

//	printf ("\n\n%s  %s\n\n", version_string,MBOOT_CL);
    printf ("\n\n%s \n\n", version_string);

#ifdef CONFIG_MSTAR_STR_MINISIZE
    debug ("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	       _TEXT_BASE,
	       _bss_start_ofs+uboot2_ofs+_TEXT_BASE, _bss_end_ofs+uboot2_ofs+_TEXT_BASE);
#else
	debug ("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	       _TEXT_BASE,
	       _bss_start_ofs+_TEXT_BASE, _bss_end_ofs+_TEXT_BASE);
#endif

#ifdef CONFIG_MODEM_SUPPORT
	debug ("Modem Support enabled\n");
#endif
#ifdef CONFIG_USE_IRQ
	debug ("IRQ Stack: %08lx\n", IRQ_STACK_START);
	debug ("FIQ Stack: %08lx\n", FIQ_STACK_START);
#endif

	return (0);
}

/*
 * WARNING: this code looks "cleaner" than the PowerPC version, but
 * has the disadvantage that you either get nothing, or everything.
 * On PowerPC, you might see "DRAM: " before the system hangs - which
 * gives a simple yet clear indication which part of the
 * initialization if failing.
 */
static int display_dram_config (void)
{
	int i;

#ifdef DEBUG
	puts ("RAM Configuration:\n");

	for(i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		printf ("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size (gd->bd->bi_dram[i].size, "\n");
	}
#else
	ulong size = 0;

	for (i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
	}
	puts("DRAM:  ");
	print_size(size, "\n");
#endif

	return (0);
}

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
static int init_func_i2c (void)
{
	puts ("I2C:   ");
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	puts ("ready\n");
	return (0);
}
#endif

#if defined(CONFIG_CMD_PCI) || defined (CONFIG_PCI)
#include <pci.h>
static int arm_pci_init(void)
{
	pci_init();
	return 0;
}
#endif /* CONFIG_CMD_PCI || CONFIG_PCI */

/*
 * Breathe some life into the board...
 *
 * Initialize a serial port as console, and carry out some hardware
 * tests.
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependent #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

int print_cpuinfo (void);

void __dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =  gd->ram_size;
}
void dram_init_banksize(void)
	__attribute__((weak, alias("__dram_init_banksize")));

init_fnc_t *init_sequence[] = {
#if defined(CONFIG_ARCH_CPU_INIT)
	arch_cpu_init,		/* basic arch cpu dependent setup */
#endif
#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
	timer_init,		/* initialize timer */
#ifdef CONFIG_FSL_ESDHC
	get_clocks,
#endif
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	init_func_i2c,
#endif
	dram_init,		/* configure available RAM banks */
	NULL,
};

void board_init_f (ulong bootflag)
{
	bd_t *bd;
	init_fnc_t **init_fnc_ptr;
	gd_t *id;
	ulong addr, addr_sp, relocaddr;
    extern void set_irq_sp(ulong);

	/* Pointer is writable since we allocated a register for it */
#ifdef CONFIG_DYNAMIC_RELOCATE
	ulong sp_addr;
    u32 *dram_size = (u32 *)RELOCATION_INFO_ADDRESS;
    sp_addr = CONFIG_SYS_SDRAM_BASE + (*dram_size -1)*0x100000;
	gd = (gd_t *) (sp_addr);
#else
    gd = (gd_t *) ((CONFIG_SYS_INIT_SP_ADDR) & ~0x07);
#endif

	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	memset ((void*)gd, 0, sizeof (gd_t));

#ifdef CONFIG_MSTAR_STR_MINISIZE
	gd->mon_len = uboot2_ofs+_bss_end_ofs;
#else
    gd->mon_len = _bss_end_ofs;
#endif

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}

        //printf("\n\rHello U-Boot\n");
	debug ("monitor len: %08lX\n", gd->mon_len);

	/*
	 * Ram is setup, size stored in gd !!
	 */
	debug ("ramsize: %08lX\n", gd->ram_size);
#if defined(CONFIG_SYS_MEM_TOP_HIDE)
	/*
	 * Subtract specified amount of memory to hide so that it won't
	 * get "touched" at all by U-Boot. By fixing up gd->ram_size
	 * the Linux kernel should now get passed the now "corrected"
	 * memory size and won't touch it either. This should work
	 * for arch/ppc and arch/powerpc. Only Linux board ports in
	 * arch/powerpc with bootwrapper support, that recalculate the
	 * memory size from the SDRAM controller setup will have to
	 * get fixed.
	 */
	gd->ram_size -= CONFIG_SYS_MEM_TOP_HIDE;
#endif

	addr = CONFIG_SYS_SDRAM_BASE + gd->ram_size;

#ifdef CONFIG_LOGBUFFER
#ifndef CONFIG_ALT_LB_ADDR
	/* reserve kernel log buffer */
	addr -= (LOGBUFF_RESERVE);
	debug ("Reserving %dk for kernel logbuffer at %08lx\n", LOGBUFF_LEN, addr);
#endif
#endif

#ifdef CONFIG_PRAM
	/*
	 * reserve protected RAM
	 */
	i = getenv_r ("pram", (char *)tmp, sizeof (tmp));
	reg = (i > 0) ? simple_strtoul ((const char *)tmp, NULL, 10) : CONFIG_PRAM;
	addr -= (reg << 10);		/* size is in kB */
	debug ("Reserving %ldk for protected RAM at %08lx\n", reg, addr);
#endif /* CONFIG_PRAM */

#if !(defined(CONFIG_SYS_NO_ICACHE) && defined(CONFIG_SYS_NO_DCACHE))
	/* reserve TLB table */
	addr -= (4096 * 4);

	/* round down to next 64 kB limit */
	addr &= ~(0x10000 - 1);

	gd->tlb_addr = addr;
	debug ("TLB table at: %08lx\n", addr);
	/*Setup page table, and enable MMU*/
	relocaddr = addr;
	relocaddr &= ~(4096 - 1);
	relocaddr -= gd->mon_len;
	relocaddr &= ~(0x100000 - 1);
	enable_mmu(gd->tlb_addr, relocaddr);
#endif

	/* round down to next 4 kB limit */
	addr &= ~(4096 - 1);
	debug ("Top of RAM usable for U-Boot at: %08lx\n", addr);

#ifdef CONFIG_LCD
#ifdef CONFIG_FB_ADDR
	gd->fb_base = CONFIG_FB_ADDR;
#else
	/* reserve memory for LCD display (always full pages) */
	addr = lcd_setmem (addr);
	gd->fb_base = addr;
#endif /* CONFIG_FB_ADDR */
#endif /* CONFIG_LCD */

	/*
	 * reserve memory for U-Boot code, data & bss
	 * round down to next 1 MB limit
	 */
	addr -= gd->mon_len;
	addr &= ~(0x100000 - 1);

	debug ("Reserving %ldk for U-Boot at: %08lx\n", gd->mon_len >> 10, addr);

#ifndef CONFIG_PRELOADER
	/*
	 * reserve memory for malloc() arena
	 */
	addr_sp = addr - TOTAL_MALLOC_LEN;
	debug ("Reserving %dk for malloc() at: %08lx\n",
			TOTAL_MALLOC_LEN >> 10, addr_sp);
	/*
	 * (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	addr_sp -= sizeof (bd_t);
	bd = (bd_t *) addr_sp;
	gd->bd = bd;
	debug ("Reserving %zu Bytes for Board Info at: %08lx\n",
			sizeof (bd_t), addr_sp);
	addr_sp -= sizeof (gd_t);
	id = (gd_t *) addr_sp;
	debug ("Reserving %zu Bytes for Global Data at: %08lx\n",
			sizeof (gd_t), addr_sp);

	/* setup stackpointer for exeptions */
	gd->irq_sp = addr_sp;
	set_irq_sp(gd->irq_sp);
#ifdef CONFIG_USE_IRQ
	addr_sp -= (CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ);
	debug ("Reserving %zu Bytes for IRQ stack at: %08lx\n",
		CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ, addr_sp);
#endif
	/* leave 3 words for abort-stack    */
	addr_sp -= 12;

	/* 8-byte alignment for ABI compliance */
	addr_sp &= ~0x07;
#else
	addr_sp += 128;	/* leave 32 words for abort-stack   */
	gd->irq_sp = addr_sp;
#endif

	debug ("New Stack Pointer is: %08lx\n", addr_sp);

#ifdef CONFIG_POST
	post_bootmode_init();
	post_run (NULL, POST_ROM | post_bootmode_get(0));
#endif

	gd->bd->bi_baudrate = gd->baudrate;
	gd->relocaddr = addr;
	gd->start_addr_sp = addr_sp;
	gd->reloc_off = addr - _TEXT_BASE;
	printf ("\nRelocation Offset is: %08lx\n", gd->reloc_off);
	memcpy (id, (void *)gd, sizeof (gd_t));
#ifdef CONFIG_MSTAR_STR_MINISIZE
    relocate_code (addr_sp, id, addr, (ulong)board_init_r);
#else
	relocate_code (addr_sp, id, addr);
#endif
	/* NOTREACHED - relocate_code() does not return */
}

#if !defined(CONFIG_SYS_NO_FLASH)
static char *failed = "*** failed ***\n";
#endif

#ifdef CONFIG_MSTAR_STR_MINISIZE
void init_cmd_tbl(void)
{
    u_boot_cmd_tbl_start=&__u_boot_cmd_start;
    u_boot_cmd_tbl_end=&__u_boot_cmd_end;
}
#endif
/************************************************************************
 *
 * This is the next part if the initialization sequence: we are now
 * running from RAM and have a "normal" C environment, i. e. global
 * data can be written, BSS has been cleared, the stack size in not
 * that critical any more, etc.
 *
 ************************************************************************
 */
#if CONFIG_FTL_AC_BURN_TEST
extern int ftl_BlkDev_Init(void);
extern U32 FtlTest_PwrCutTestCheck(void);
extern U32 FtlTest_PwrCutTestRun(U32 LoopCnt);
#endif

unsigned int u32UbootStart = 0;
unsigned int u32UbootEnd = 0;

static int valid_part(char* s)
{
    int n = strlen(s);
    int i;

    // length of string should not be more than 3
    if (n > 3)
        return 0;

    // check if the string only contains digits
    for (i=0; i<n; i++)
        if ((s[i] >= '0' && s[i] <= '9') == false)
            return 0;

    int v = atoi(s);
    // check if the number is between 0 to 255
    return (v >= 0 && v <= 255);
}

static int is_valid_ipaddr(char* pAddr)
{
    int counter = 0;
    char pStrIP[32] = {0};

    strncpy(pStrIP, pAddr, strlen(pAddr));

    char* p = strtok(pStrIP, ".");
    while (p)
    {
        if (valid_part(p))
        {
            p = strtok(NULL, ".");
            if (p != NULL)
                ++counter;
        }
        else
        {
            return 0;
        }
    }

    // valid IP string must contain 3 dots
    if (counter != 3)
        return 0;
    return 1;
}

static int is_valid_macaddr(char* str)
{
    int i;

    if (str == NULL)
        return 0;

    // check input string is hexdecimal digits
    for(i=0; i<12; i++)
        if (isxdigit(str[i]) == 0)
            return 0;

    return 1;
}

#if defined(CONFIG_UBOOT_RPMB_RW) && defined(CONFIG_MSTAR_AUTHEN_ENABLE)
// Magic flag indicating anti-rollback fields in RPMB have been initialized
#define FLAG_ANTIROLLBACK_INITIALIZED (0x42525468)

    struct rpmb_fs_partition {
        U32 rpmb_fs_magic;
        U32 fs_version;
        U32 write_counter;
        U32 fat_start_address;
        /* Do not use reserved[] for other purpose than partition data. */
        U8 reserved[40];
        U8  unique_itvid[24];
        U8  magicString[16];
        U32 uboot_version;
        U32 hash1_version;
        U32 teekeybank_version;
        U32 teeloader_version;
        U32 reeloader_version;
        U32 optee_version;
        U32 armfw_version;
        U32 anti_rollback_init_flag;
    };
#endif
void board_init_r (gd_t *id, ulong dest_addr)
{
	char *s;
	bd_t *bd;
	ulong malloc_start;
#if !defined(CONFIG_SYS_NO_FLASH)
	ulong flash_size;
#endif
#if defined(UFBL_FEATURE_IDME)
        char serialno[18] = {0};
#endif
    gd = id;
	bd = gd->bd;

	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */
#ifdef CONFIG_MSTAR_STR_MINISIZE
    init_cmd_tbl();
#endif

	/* The Malloc area is immediately below the monitor copy in DRAM */
	malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_init (malloc_start, TOTAL_MALLOC_LEN);

    amzn_init_log_buf();

    display_banner();		/* say that we are here */

    /*MSTAR start*/
    MstarSysInit();
    u32UbootStart = id->start_addr_sp;
    u32UbootEnd = CONFIG_SYS_SDRAM_BASE+id->ram_size;
    printf("\nuboot held at [%08x~%08x],size=%08x\n",
            u32UbootStart,
            u32UbootEnd,
            u32UbootEnd-u32UbootStart);
    /* MSTAR end*/
#ifdef CONFIG_MSTAR_STR_MINISIZE
    monitor_flash_len = uboot2_ofs+_end_ofs;
#else
	monitor_flash_len = _end_ofs;
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
	//print_cpuinfo();		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	//checkboard();		/* display board info */
#endif

    /* Ram ist board specific, so move it to board code ... */
	dram_init_banksize();
	display_dram_config();	/* and display it */
	debug ("monitor flash len: %08lX\n", monitor_flash_len);
	board_init();	/* Setup chipselects */

#ifdef CONFIG_SERIAL_MULTI
	serial_initialize();
#endif

	printf ("Now running in RAM - U-Boot at: %08lx\n", dest_addr);

#ifdef CONFIG_LOGBUFFER
	logbuff_init_ptrs ();
#endif
#ifdef CONFIG_POST
	post_output_backlog ();
#endif

#if !defined(CONFIG_SYS_NO_FLASH)
	puts ("Flash: ");

	if ((flash_size = flash_init ()) > 0) {
# ifdef CONFIG_SYS_FLASH_CHECKSUM
		print_size (flash_size, "");
		/*
		 * Compute and print flash CRC if flashchecksum is set to 'y'
		 *
		 * NOTE: Maybe we should add some WATCHDOG_RESET()? XXX
		 */
		s = getenv ("flashchecksum");
		if (s && (*s == 'y')) {
			printf ("  CRC: %08X",
				crc32 (0, (const unsigned char *) CONFIG_SYS_FLASH_BASE, flash_size)
			);
		}
		putc ('\n');
# else	/* !CONFIG_SYS_FLASH_CHECKSUM */
		print_size (flash_size, "\n");
# endif /* CONFIG_SYS_FLASH_CHECKSUM */
	} else {
		puts (failed);
		hang ();
	}
#endif

#if defined(CONFIG_CMD_NAND)
	puts ("NAND:  ");
	nand_init();		/* go init the NAND */
#endif

#if defined(CONFIG_CMD_ONENAND)
	onenand_init();
#endif

#ifdef CONFIG_HAS_DATAFLASH
	AT91F_DataflashInit();
	dataflash_print_info();
#endif
#if defined(CONFIG_GENERIC_MMC) && defined(CONFIG_ENV_IS_IN_MMC)
	//puts("MMC:	");
	mmc_initialize(bd);
#endif

#ifdef CONFIG_MSTAR_UFS
	puts ("UFS:\n");
	ufs_init();
#endif

	/* initialize environment */
	env_relocate ();

	/* Reduce boottime */
	close_console_log();

	/* check and save the first bootup status */
	save_first_boot_status();

#if (ENABLE_MODULE_NAND_FLASH == 1)
	nand_CheckSAR5ON();
#endif

#if defined(CONFIG_GENERIC_MMC) && defined(CONFIG_ENV_IS_IN_SPI_FLASH)
	//puts("MMC:	");
	mmc_initialize(bd);
#endif

#if defined(CONFIG_GENERIC_MMC)
	eMMC_Check_SAR5ON();
#endif

#if defined(CONFIG_SD)
	printf("sd_initialize [ start ]\n");
	sd_initialize(bd);
	printf("sd_initialize [  end  ]\n");
#endif

#if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI)
	arm_pci_init();
#endif

#ifdef UFBL_FEATURE_IDME
        idme_initialize();

   // updating idme/env
   if (!is_str_resume()) { // no need to do this if str resume
        int bNeedToSave = 0;

        //bootmode set for diag/normal boot separation
        char bootmode_buf[6] = {0};
        char bootmode_cmd[64] = {0};
        int bootmode = 1;
        char *bootmodeInEnv = NULL;
        if (!idme_get_var_external("bootmode", bootmode_buf, sizeof(bootmode_buf)-1)) {
                bootmode = atoi(bootmode_buf);
                printf("idme bootmode read [%d]\n",bootmode);
        }else {
                printf("idme bootmode failed");
                bootmode = 1;
        }
        if(bootmode < 0 || bootmode > IDME_BOOTMODE_MAX)
                bootmode = 1;

        if (bootmode == IDME_BOOTMODE_TRANSITION)
               run_command("amzn_boot transition", 0);

	if (bootmode == IDME_BOOTMODE_STANDBY_LOGO_POST_SHIPPING_SW_SWITCH)
	       run_command("amzn_boot reset", 0);

        bootmodeInEnv = getenv("bootmode");
        /* Need to update env variabel bootmode? */
        if ((!bootmodeInEnv) || (atoi(bootmodeInEnv) != bootmode)) {
           snprintf(bootmode_cmd, sizeof(bootmode_cmd), "setenv bootmode %d", bootmode);
           run_command(bootmode_cmd, 0);
           bNeedToSave = 1;
        }

	// update product_name and productid in IDME based on HW ID
	const char* prod_name = amzn_target_device_name();
	unsigned int name_len = strlen(prod_name);
	if (idme_update_var_ex("product_name", prod_name, name_len))
	{
	   printf("error updating product_name\n");
	}
	if (idme_update_var_ex("productid", prod_name, name_len))
	{
	   printf("error updating productid\n");
	}

        char idmeipaddr[32] = {0};
        char ipaddrcmd[64] = {0};
        char *ipToUse = NULL;
        char *ipInEnv = NULL;
        if (!idme_get_var_external("eth_ip_addr", idmeipaddr, sizeof(idmeipaddr)-1) &&
            strchr((const char*)idmeipaddr, '.')) {
           if (is_valid_ipaddr(idmeipaddr)) {
               ipToUse = idmeipaddr;
           }
           else {
               ipToUse = DEFAULT_IP_ADDR; // 192.168.1.101
               printf("idmeipaddr is invalid, use default ip addr:%s \n", ipToUse);
           }
        } else {
           ipToUse = DEFAULT_IP_ADDR; // 192.168.1.101
        }
        ipInEnv = getenv("ipaddr");
        /* Need to update env variable ipaddr? */
        if ((!ipInEnv) || (strcmp(ipToUse, ipInEnv))) {
           snprintf(ipaddrcmd, sizeof(ipaddrcmd), "setenv ipaddr %s", ipToUse);
           run_command(ipaddrcmd, 0);
           bNeedToSave = 1;
        }

        char idmemacaddr[32] = {0};
        char addrwithsep[48] = {0};
        char macaddrcmd[64] = {0};
        char *macInEnv = NULL;
        if (!idme_get_var_external("eth_mac_addr", idmemacaddr, sizeof(idmemacaddr)-1) &&
            strlen((const char*)idmemacaddr) == 12) {
           if (!is_valid_macaddr(idmemacaddr))
           {
               printf("idmeipaddr is invalid, mac:%s \n", idmemacaddr);
               memset(idmemacaddr, 0, sizeof(idmemacaddr));
           }
           snprintf(addrwithsep, sizeof(addrwithsep), "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
                          idmemacaddr[0], idmemacaddr[1], idmemacaddr[2], idmemacaddr[3],
                          idmemacaddr[4], idmemacaddr[5], idmemacaddr[6], idmemacaddr[7],
                          idmemacaddr[8], idmemacaddr[9], idmemacaddr[10], idmemacaddr[11]);
           macInEnv = getenv("macaddr");
           /* Need to update env variable macaddr? */
           if ((!macInEnv) || (strcmp(addrwithsep, macInEnv))) {
              snprintf(macaddrcmd, sizeof(macaddrcmd), "setenv macaddr %s", addrwithsep);
              run_command(macaddrcmd, 0);
              bNeedToSave = 1;
           }
        }

        char dev_flags_buf[8] = {0};
        char dev_flags_cmd[64] = {0};
        unsigned dev_flags = 0;
        char *devflagsInEnv = NULL;
        /* treat dev_flags as an unsigned integer in hex */
        if (!idme_get_var_external("dev_flags", dev_flags_buf, sizeof(dev_flags_buf)-1)) {
                dev_flags = simple_strtoul(dev_flags_buf, NULL, 16);
                printf("idme dev_flags read [0x%x]\n",dev_flags);
        }else {
                printf("idme dev_flags read failed");
        }
        devflagsInEnv = getenv("dev_flags");
        /* Need to update env variabel dev_flags? */
        if ((!devflagsInEnv) || (simple_strtoul(getenv("dev_flags"), NULL, 16) != dev_flags)) {
           snprintf(dev_flags_cmd, sizeof(dev_flags_cmd), "setenv dev_flags %x",dev_flags);
           run_command(dev_flags_cmd, 0);
           bNeedToSave = 1;
        }

        char usr_flags_buf[8] = {0};
        char usr_flags_cmd[64] = {0};
        unsigned usr_flags = 0;
        char *usrflagsInEnv = NULL;
        /* treat usr_flags as an unsigned integer in hex */
        if (!idme_get_var_external("usr_flags", usr_flags_buf, sizeof(usr_flags_buf)-1)) {
                usr_flags = simple_strtoul(usr_flags_buf, NULL, 16);
                printf("idme usr_flags read [0x%x]\n",usr_flags);
        }else {
                printf("idme usr_flags read failed");
        }
        usrflagsInEnv = getenv("usr_flags");
        /* Need to update env variabel usr_flags? */
        if ((!usrflagsInEnv) || (simple_strtoul(getenv("usr_flags"), NULL, 16) != usr_flags)) {
           snprintf(usr_flags_cmd, sizeof(usr_flags_cmd), "setenv usr_flags %x",usr_flags);
           run_command(usr_flags_cmd, 0);
           bNeedToSave = 1;
        }

	char key_layout_buf[32] = {0};
	char key_layout_cmd[64] = {0};
	unsigned int key_layout = 0;
	char *key_layoutInEnv = NULL;
	if (!idme_get_var_external("key_layout", key_layout_buf, sizeof(key_layout_buf)-1)) {
		key_layout = simple_strtoul(key_layout_buf, NULL, 10);
		printf("idme key_layout read %d \n", key_layout);
	} else {
		key_layout = 0;
		printf("idme key_layout read failed, use default value 0 \n");
	}

       if ((!key_layoutInEnv) || (simple_strtoul(getenv("key_layout"), NULL, 10) != key_layout)) {
           snprintf(key_layout_cmd, sizeof(key_layout_cmd), "setenv key_layout %d", key_layout);
           run_command(key_layout_cmd, 0);
           bNeedToSave = 1;
        }

        if (!idme_get_var_external("serial", serialno, sizeof(serialno)-1)) {
                printf("idme serial read [%s]\n", serialno);
        }else {
                printf("idme serial read failed\n");
        }

        /* saveenv after done loading ip and mac addr from idme */
        if (bNeedToSave)
           run_command("saveenv", 0);
   } // !is_str_resume()
#endif

	/* IP Address */
	gd->bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

	stdio_init ();	/* get the devices list going. */

	jumptable_init ();

#if defined(CONFIG_API)
	/* Initialize API */
	api_init ();
#endif

#if (CONFIG_Second_UART_Mode_1 || CONFIG_Second_UART_Mode_2  || CONFIG_Second_UART_Mode_3 ||\
	CONFIG_Third_UART_Mode_1 || CONFIG_Third_UART_Mode_2  || CONFIG_Third_UART_Mode_3 ||\
	CONFIG_Fourth_UART_Mode_1 || CONFIG_Fourth_UART_Mode_2  || CONFIG_Fourth_UART_Mode_3)
//Do nothing
#else
	console_init_r ();	/* fully init console as a device */
#endif

#if defined(CONFIG_ARCH_MISC_INIT)
	/* miscellaneous arch dependent initialisations */
	arch_misc_init ();
#endif
#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
#endif

	/* set up exceptions */
	interrupt_init ();
	/* enable exceptions */
	enable_interrupts ();

	/* Perform network card initialisation if necessary */
#if defined(CONFIG_DRIVER_SMC91111) || defined (CONFIG_DRIVER_LAN91C96)
	/* XXX: this needs to be moved to board init */
	if (getenv ("ethaddr")) {
		uchar enetaddr[6];
		eth_getenv_enetaddr("ethaddr", enetaddr);
		smc_set_mac_addr(enetaddr);
	}
#endif /* CONFIG_DRIVER_SMC91111 || CONFIG_DRIVER_LAN91C96 */

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if defined(CONFIG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif

#ifdef BOARD_LATE_INIT
	board_late_init ();
#endif

#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif
#if defined(CONFIG_CMD_NET)
#if defined(CONFIG_NET_MULTI)
	puts ("Net:   ");
#endif
	eth_initialize(gd->bd);
#if defined(CONFIG_RESET_PHY_R)
	debug ("Reset Ethernet PHY\n");
	reset_phy();
#endif
#endif

#ifdef CONFIG_POST
	post_run (NULL, POST_RAM | post_bootmode_get(0));
#endif

#if defined(CONFIG_PRAM) || defined(CONFIG_LOGBUFFER)
	/*
	 * Export available size of memory for Linux,
	 * taking into account the protected RAM at top of memory
	 */
	{
		ulong pram;
		uchar memsz[32];
#ifdef CONFIG_PRAM
		char *s;

		if ((s = getenv ("pram")) != NULL) {
			pram = simple_strtoul (s, NULL, 10);
		} else {
			pram = CONFIG_PRAM;
		}
#else
		pram=0;
#endif
#ifdef CONFIG_LOGBUFFER
#ifndef CONFIG_ALT_LB_ADDR
		/* Also take the logbuffer into account (pram is in kB) */
		pram += (LOGBUFF_LEN+LOGBUFF_OVERHEAD)/1024;
#endif
#endif
		sprintf ((char *)memsz, "%ldk", (bd->bi_memsize / 1024) - pram);
		setenv ("mem", (char *)memsz);
	}
#endif

    if(-1==MstarDrvInit())
    {
        printf("Error: MstarInit() \n");
    }
    printf("[AT][MBoot][Driver Init][%lu]\n", MsSystemGetBootTime());

	/* main_loop() can return to retry autoboot, if so just run it again. */

#ifdef CONFIG_DYNAMIC_RELOCATE
    #ifdef DYNAMIC_RELOCATE_BIST
        u32 *dram_size = (u32 *)(RELOCATION_INFO_ADDRESS + 0x20000000);
        *dram_size = 151;
        Chip_Flush_Memory();
    #endif
#endif

#if (ENABLE_MODULE_NAND_FLASH == 1)
	extern U32 nand_MTDMarkBad(U32 u32_GoodBlkIdx);
	extern int nand_ShiftBlock_Recover(U32 (*nand_markbad)(U32));
	nand_ShiftBlock_Recover(nand_MTDMarkBad);
#endif

#if CONFIG_FTL_AC_BURN_TEST
    ftl_BlkDev_Init();
    FtlTest_PwrCutTestCheck();
    FtlTest_PwrCutTestRun(0x3697);
#endif

#if defined(CONFIG_UBOOT_RPMB_RW) && defined(CONFIG_MSTAR_AUTHEN_ENABLE)

    char data[256] = { 0 };
    int32_t ret = 0;
    int i = 0 ;

    ret = uboot_rpmb_read_data(data, 256, 0, 0);
    if(ret == -1){
        printf("uboot_rpmb_read_data fail\n");
        goto rpmb_fail;
    }

    //uboot_version offset in rpmb partion (blk0): 96th byte
    //the toltal length of the uboot_version is 4 bytes
    rpmbData = (struct rpmb_fs_partition*)data;
    if (rpmbData->anti_rollback_init_flag == FLAG_ANTIROLLBACK_INITIALIZED)
    {
        printf("Read uboot_version from RPMB = %x\n", rpmbData->uboot_version);

        if( rpmbData->uboot_version < CONFIG_UBOOT_VERSION_NUMBER ){
            rpmbData->uboot_version  = CONFIG_UBOOT_VERSION_NUMBER;

            printf("Read uboot_version from mboot.bin = %x\n", rpmbData->uboot_version);
            ret = uboot_rpmb_write_data(rpmbData, 256, 0, 0);
            if(ret == -1){
                    printf("uboot_rpmb_write_data fail\n");
                    goto rpmb_fail;
            }
        }
        goto rpmb_ok;
    }
    else
    {
        printf("\nu-boot Anti-rollback not enabled\n\n");
        goto rpmb_ok;
    }

rpmb_fail:
    for(i=0; i<16; i++)
        printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        data[16*i+0],data[16*i+1],data[16*i+2],data[16*i+3],
        data[16*i+4],data[16*i+5],data[16*i+6],data[16*i+7],
        data[16*i+8],data[16*i+9],data[16*i+10],data[16*i+11],
        data[16*i+12],data[16*i+13],data[16*i+14],data[16*i+15]);
rpmb_ok:
    ret = uboot_disable_smc();
    if(ret == 0)
        printf("disable smc success\n");
    else
        printf("disable smc failed\n");
#endif //defined(CONFIG_UBOOT_RPMB_RW) && defined(CONFIG_MSTAR_AUTHEN_ENABLE)
	for (;;) {
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
