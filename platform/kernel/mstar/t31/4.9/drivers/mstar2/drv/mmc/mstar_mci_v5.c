/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#include "mstar_mci.h"
#include "linux/mmc/mmc.h"
#include <linux/of.h>
#include <linux/of_irq.h>

/******************************************************************************
 * Defines
 ******************************************************************************/
#define MCI_RETRY_CNT_CMD_TO        100
#define MCI_RETRY_CNT_CRC_ERR       200 // avoid stack overflow
#define MCI_RETRY_CNT_OK_CLK_UP     10

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
static void mstar_mci_send_command(struct mstar_mci_host *pMStarHost_st, struct mmc_command *pCmd_st);
static void mstar_mci_completed_command(struct mstar_mci_host *pMStarHost_st);
#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
static void mstar_mci_completed_command_FromRAM(struct mstar_mci_host *pMStarHost_st);
#endif

#if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
static int mstar_mci_pre_dma_transfer(struct mstar_mci_host *pMStarHost_st, struct mmc_data *data, struct mstar_mci_host_next *next);
#endif

u32 mstar_mci_WaitD0High(u32 u32_us);

#if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
static U32 mstar_mci_miu_chksum(struct mmc_data *data);
#endif
DEFINE_SPINLOCK(fcie_lock);

/*****************************************************************************
 * Define Static Global Variables
 ******************************************************************************/
static U32 u32_ok_cnt = 0;
static struct task_struct *sgp_eMMCThread_st = NULL;
#if 0
static ulong wr_seg_size = 0;
static ulong wr_split_threshold = 0;
#endif
u8 u8_enable_sar5 = 1;

/* dedicated workqueue */
static struct workqueue_struct *mci_workqueue;

// ===============================
// for /sys files
#if 0
U32 gu32_pwrsvr_gpio_enable = 0;
U32 gu32_pwrsvr_gpio_addr = 0;
U32 gu32_pwrsvr_gpio_bit = 0;
U32 gu32_pwrsvr_gpio_trigger = 0;
U32 gu32_emmc_sanitize = 0;
#endif
U32 gu32_eMMC_read_log_enable =0;
U32 gu32_eMMC_write_log_enable =0;
U32 gu32_eMMC_monitor_enable=0;

U16 u16_OldPLLClkParam=0xFFFF;
U16 u16_OldPLLDLLClkParam=0xFFFF;
U8  u8_tune_overtone = 1;
#if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
static U32 u32_nbytes =0;
#endif


#define FCIE_ADMA_DESC_COUNT    512
struct _AdmaDescriptor eMMC_ALIGN0 gAdmaDesc_st[FCIE_ADMA_DESC_COUNT] eMMC_ALIGN1;
#if defined(CONFIG_OF)
static const struct of_device_id mstar_emmc_dt_match[] = {
    { .compatible = DRIVER_NAME, },
    {}
};
MODULE_DEVICE_TABLE(of, mstar_emmc_dt_match);
#endif

static ktime_t starttime;
struct mstar_rw_speed emmc_rw_speed;


/*****************************************************************************
 * for profiling
 ******************************************************************************/
#if defined(CONFIG_MMC_MSTAR_MMC_EMMC_LIFETEST)
static struct proc_dir_entry * writefile;
const char procfs_name[] = "StorageBytes";

int procfile_read(char* buffer, char ** buffer_location, off_t offset,
                  int buffer_length, int *eof, void *data)
{
    int ret;

    if(offset > 0)
        ret = 0;
    else
    {
        ret = scnprintf(buffer, PAGE_SIZE,
                      "TotalWriteBytes %llu GB %llu MB\nTotalReadBytes %llu GB %llu MB\n",
                      g_eMMCDrv.u64_CNT_TotalWBlk/1024/1024/2,
                      (g_eMMCDrv.u64_CNT_TotalWBlk/1024/2) % 1024,
                      g_eMMCDrv.u64_CNT_TotalRBlk/1024/1024/2,
                      (g_eMMCDrv.u64_CNT_TotalRBlk/1024/2) % 1024);
    }

    return ret;
}
#endif

/******************************************************************************
 * Functions
 ******************************************************************************/
#if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM

static U32 mstar_mci_miu_chksum(struct mmc_data *data)
{
    struct scatterlist  *pSG_st = data->sg;
    int i;
    u32 dmalen                  = 0;
    u16 checksum[8];
    u16 old_checksum[8];
    u16 data_in[8];
    u16 *u16_pBuf;
    struct sg_mapping_iter miter;
    int nents;

    memset(checksum, 0, 8*sizeof(u16));
    memset(old_checksum, 0, 8*sizeof(u16));
    memset(data_in, 0, 8*sizeof(u16));
    
    
    nents = sg_nents_for_len(pSG_st, u32_nbytes);
    if (nents < 0)
        return -EINVAL;
    
    sg_miter_start(&miter, pSG_st, nents, SG_MITER_ATOMIC | SG_MITER_FROM_SG);
    sg_miter_next(&miter);
    
    while(u32_nbytes > 0) {
        dmalen = (miter.length > u32_nbytes) ? u32_nbytes:miter.length;
        u16_pBuf = (u16*)miter.addr;
        if (u16_pBuf == NULL)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,"miter addr=NULL\n");
        }
    
        for (i = 0; i < dmalen / sizeof(u16); i++) {
    
            data_in[i%8] = u16_pBuf[i];
    
            old_checksum[i%8] = checksum[i%8];
    
            if (i % 8 != 0)
                checksum[i%8] = data_in[i%8] ^ (((old_checksum[i%8] << 1) & 0xFFFF) | (old_checksum[(i-1)%8] >> 15));
            else
                checksum[i%8] = data_in[i%8] ^ (((old_checksum[i%8] << 1) & 0xFFFF) | (checksum[7] >> 15));
        }
    
        u32_nbytes -= dmalen;
        sg_miter_next(&miter);
    }
    sg_miter_stop(&miter);

    for (i = 0; i < 8; i++) {
        REG_FCIE_CLRBIT(GET_REG_ADDR(FCIE1_BASE, 0x51), BIT12|BIT11|BIT10|BIT9|BIT8);
        REG_FCIE_SETBIT(GET_REG_ADDR(FCIE1_BASE, 0x51), (0x10+i)<<8);
    
        if (checksum[i] != REG_FCIE(FCIE_EMMC_DEBUG_BUS0))
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "\n\nFCIE's MIU checksum is not the same with CPU!\n\n");
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "\n\nFCIE's MIU checksum:0x%x, CPU checksum:0x%x, I:%d\n\n",
                 REG_FCIE(FCIE_EMMC_DEBUG_BUS0) , checksum[i], i);
        }
    }

    return eMMC_ST_SUCCESS;
}

#endif
static int mstar_mci_get_dma_dir(struct mmc_data *data)
{
    #if defined(CONFIG_ENABLE_EMMC_ACP) && CONFIG_ENABLE_EMMC_ACP

    return DMA_ACP;

    #else

    if (data->flags & MMC_DATA_WRITE)
        return DMA_TO_DEVICE;
    else
        return DMA_FROM_DEVICE;

    #endif
}

#if !(defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM)
static int mstar_mci_config_ecsd(struct mmc_data *pData_st, struct mmc_host *pMMCHost_st)
{
    struct scatterlist *pSG_st  = 0;
    dma_addr_t dmaaddr          = 0;
    int err                     = 0;
    u8 *pBuf;

    if( !pData_st )
    {
        return -EINVAL;
    }

    if(0 ==(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_INIT_DONE))
    {
        pSG_st = &pData_st->sg[0];
        dmaaddr = sg_dma_address(pSG_st);

        pBuf = (u8*)phys_to_virt(dmaaddr);

        if ( pBuf[177] !=0x2 && (pBuf[179]>>3) !=0x1) {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: eMMC boot bus condition=%xh, eMMC boot config=%xh\n",
                pBuf[177], pBuf[179]);
            eMMC_die("\n");
        }

        //--------------------------------
        if(0 == g_eMMCDrv.u32_SEC_COUNT)
            g_eMMCDrv.u32_SEC_COUNT = ((pBuf[215]<<24)|
                                       (pBuf[214]<<16)|
                                       (pBuf[213]<< 8)|
                                       (pBuf[212])) - 8; //-8: Toshiba CMD18 access the last block report out of range error

        //-------------------------------
        if(0 == g_eMMCDrv.u32_BOOT_SEC_COUNT)
            g_eMMCDrv.u32_BOOT_SEC_COUNT = pBuf[226] * 128 * 2;

        //--------------------------------
        if(!g_eMMCDrv.u8_BUS_WIDTH)
        {
            g_eMMCDrv.u8_BUS_WIDTH = pBuf[183];

            switch(g_eMMCDrv.u8_BUS_WIDTH)
            {
                case 0:  g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_1;  break;
                case 1:  g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_4;  break;
                case 2:  g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_8;  break;
                default: eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: eMMC BUS_WIDTH not support \n");
                while(1);
            }
        }

        //--------------------------------
        if(pBuf[231]&BIT4) // TRIM
            g_eMMCDrv.u32_eMMCFlag |= eMMC_FLAG_TRIM;
        else
            g_eMMCDrv.u32_eMMCFlag &= ~eMMC_FLAG_TRIM;

        //--------------------------------
        if(pBuf[503]&BIT0) // HPI
        {
            if(pBuf[503]&BIT1)
                g_eMMCDrv.u32_eMMCFlag |= eMMC_FLAG_HPI_CMD12;
            else
                g_eMMCDrv.u32_eMMCFlag |= eMMC_FLAG_HPI_CMD13;
        }
        else
            g_eMMCDrv.u32_eMMCFlag &= ~(eMMC_FLAG_HPI_CMD12|eMMC_FLAG_HPI_CMD13);

        //--------------------------------
        if(pBuf[166]&BIT2) // Reliable Write
            g_eMMCDrv.u16_ReliableWBlkCnt = BIT_SD_JOB_BLK_CNT_MASK;
        else
        {
            #if 0
            g_eMMCDrv.u16_ReliableWBlkCnt = pBuf[222];
            #else
            if((pBuf[503]&BIT0) && 1==pBuf[222])
                g_eMMCDrv.u16_ReliableWBlkCnt = 1;
            else if(0==(pBuf[503]&BIT0))
                g_eMMCDrv.u16_ReliableWBlkCnt = pBuf[222];
            else
            {
                //eMMC_debug(0,1,"eMMC Warn: not support dynamic  Reliable-W\n");
                g_eMMCDrv.u16_ReliableWBlkCnt = 0; // can not support Reliable Write
            }
            #endif
        }

        //--------------------------------
        g_eMMCDrv.u8_ErasedMemContent = pBuf[181];

        //--------------------------------
        g_eMMCDrv.u8_ECSD184_Stroe_Support = pBuf[184];
        g_eMMCDrv.u8_ECSD185_HsTiming = pBuf[185];
        g_eMMCDrv.u8_ECSD192_Ver = pBuf[192];
        g_eMMCDrv.u8_ECSD196_DevType = pBuf[196];
        g_eMMCDrv.u8_ECSD197_DriverStrength = pBuf[197];
        g_eMMCDrv.u8_ECSD248_CMD6TO = pBuf[248];
        g_eMMCDrv.u8_ECSD247_PwrOffLongTO = pBuf[247];
        g_eMMCDrv.u8_ECSD34_PwrOffCtrl = pBuf[34];

        //for GP Partition
        g_eMMCDrv.u8_ECSD160_PartSupField = pBuf[160];
        g_eMMCDrv.u8_ECSD224_HCEraseGRPSize= pBuf[224];
        g_eMMCDrv.u8_ECSD221_HCWpGRPSize= pBuf[221];

        g_eMMCDrv.GP_Part[0].u32_PartSize = ((pBuf[145] << 16) |
                                             (pBuf[144] << 8) |
                                             (pBuf[143])) *
                                             (g_eMMCDrv.u8_ECSD224_HCEraseGRPSize  * g_eMMCDrv.u8_ECSD221_HCWpGRPSize * 0x80000);

        g_eMMCDrv.GP_Part[1].u32_PartSize = ((pBuf[148] << 16) |
                                             (pBuf[147] << 8) |
                                             (pBuf[146])) *
                                             (g_eMMCDrv.u8_ECSD224_HCEraseGRPSize  * g_eMMCDrv.u8_ECSD221_HCWpGRPSize * 0x80000);

        g_eMMCDrv.GP_Part[2].u32_PartSize = ((pBuf[151] << 16) |
                                             (pBuf[150] << 8) |
                                             (pBuf[149])) *
                                             (g_eMMCDrv.u8_ECSD224_HCEraseGRPSize  * g_eMMCDrv.u8_ECSD221_HCWpGRPSize * 0x80000);

        g_eMMCDrv.GP_Part[3].u32_PartSize = ((pBuf[154] << 16) |
                                             (pBuf[153] << 8) |
                                             (pBuf[152])) *
                                             (g_eMMCDrv.u8_ECSD224_HCEraseGRPSize  * g_eMMCDrv.u8_ECSD221_HCWpGRPSize * 0x80000);

        //for Max Enhance Size
        g_eMMCDrv.u8_ECSD157_MaxEnhSize_0= pBuf[157];
        g_eMMCDrv.u8_ECSD158_MaxEnhSize_1= pBuf[158];
        g_eMMCDrv.u8_ECSD159_MaxEnhSize_2= pBuf[159];

        g_eMMCDrv.u8_u8_ECSD155_PartSetComplete = pBuf[155];
        g_eMMCDrv.u8_ECSD166_WrRelParam = pBuf[166];

        //for boot bus condidtion
        g_eMMCDrv.u8_bootbus_condition = pBuf[177];

        g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_INIT_DONE;

		#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)

		#if (defined(ENABLE_eMMC_HS400_5_1) && ENABLE_eMMC_HS400_5_1) ||\
			(defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400)
		if(g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_HS400_1_8V &&
			((pMMCHost_st->caps2 & MMC_CAP2_HS400_1_8V)||(pMMCHost_st->caps2 & MMC_CAP2_HS400_ES))){

			err = eMMC_LoadTimingTable(FCIE_eMMC_HS400);
			if(eMMC_ST_SUCCESS != err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: HS400 no Timing Table, %Xh\n", err);
				pMMCHost_st->caps2 &=~ MMC_CAP2_HS400_1_8V;
				pMMCHost_st->caps2 &=~ MMC_CAP2_HS400_ES;
			}
			else return err;
		}
        #endif

		#if defined(ENABLE_eMMC_HS200) && ENABLE_eMMC_HS200
		if(g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_HS200_1_8V &&
			pMMCHost_st->caps2 & MMC_CAP2_HS200_1_8V_SDR){
			err = eMMC_LoadTimingTable(FCIE_eMMC_HS200);
			if(eMMC_ST_SUCCESS != err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: HS200 no Timing Table, %Xh\n", err);
				pMMCHost_st->caps2 &=~ MMC_CAP2_HS200_1_8V_SDR;
			}
			else return err;
		}
        #endif
		
		#if defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP
		if(g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_DDR &&
			(pMMCHost_st->caps & (MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50)) ){
			err = eMMC_LoadTimingTable(FCIE_eMMC_DDR);
			if(eMMC_ST_SUCCESS != err)
			{
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: DDR no Timing Table, %Xh\n", err);
				pMMCHost_st->caps &=~ (MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50);
			}
			else return err;
		}
		#endif

		#endif
    }

    return err;
}
#endif



#if defined(ENABLE_FCIE_ADMA) && ENABLE_FCIE_ADMA

static void mstar_mci_pre_adma_read(struct mstar_mci_host *pMStarHost_st)
{
    struct mmc_command *pCmd_st = pMStarHost_st->cmd;
    struct mmc_data  *pData_st  = pCmd_st->data;
    struct scatterlist	*pSG_st = pData_st->sg;
    u32 dmalen                  = 0;
    dma_addr_t dmaaddr          = 0;
    #if (defined(ENABLE_UMA) && ENABLE_UMA) && defined(CONFIG_ARM64)
    uintptr_t uint_dma_addr     = 0;
    #else
    U32 u32_dma_addr            = 0;
    #endif
    int i;
    #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
	u32_nbytes =0;
    #endif

    if( pData_st->sg_len > FCIE_ADMA_DESC_COUNT )
    {
        eMMC_die("mstar_mci_pre_adma_read: sglist has more than FCIE_ADMA_DESC_COUNT items. Must change 512 to larger value.\n");
    }

    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    mstar_mci_pre_dma_transfer(pMStarHost_st, pData_st, NULL);
    #else
    dma_map_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    REG_FCIE_W(FCIE_BLK_SIZE, eMMC_SECTOR_512BYTE);
    for(i=0; i<pData_st->sg_len; i++)
    {
        gAdmaDesc_st[i].u32_End = 0;     
        dmaaddr = sg_dma_address(pSG_st);
        dmalen = sg_dma_len(pSG_st);



		#if !(defined (ENABLE_UMA) && ENABLE_UMA)

        #ifdef MSTAR_MIU2_BUS_BASE
        if( dmaaddr >= MSTAR_MIU2_BUS_BASE) // MIU2
        {
            dmaaddr -= MSTAR_MIU2_BUS_BASE;
            gAdmaDesc_st[i].u32_MiuSel = 2;
        }
        else
        #endif
        if( dmaaddr >= MSTAR_MIU1_BUS_BASE) // MIU1
        {
            dmaaddr -= MSTAR_MIU1_BUS_BASE;
            gAdmaDesc_st[i].u32_MiuSel = 1;
        }
        else // MIU0
        {
            dmaaddr -= MSTAR_MIU0_BUS_BASE;
            gAdmaDesc_st[i].u32_MiuSel = 0;
        }

        #else // ENABLE_UMA

        #ifdef CONFIG_MSTAR_M7821

        if( dmaaddr >= MSTAR_MIU1_BUS_BASE) // MIU1
        {
            dmaaddr -= MSTAR_MIU1_BUS_BASE;
            dmaaddr += 0x100000000;
        }
        else // MIU0
        {
            dmaaddr -= MSTAR_MIU0_BUS_BASE;
        }
        gAdmaDesc_st[i].u32_MiuSel = 0;

        #else // Murphy

		#if defined(MSTAR_MIU1_BUS_BASE_H) && defined(ARM_MIU1_BASE_ADDR)		// for 64 bit address beyond 0x1_0000_0000
		if(dmaaddr > MSTAR_MIU1_BUS_BASE_H)
		{
			dmaaddr -= MSTAR_MIU1_BUS_BASE_H;
			dmaaddr += ARM_MIU1_BASE_ADDR;
		}
		else
			dmaaddr -= MSTAR_MIU0_BUS_BASE;
		#else
        dmaaddr -= MSTAR_MIU0_BUS_BASE;
		#endif
        gAdmaDesc_st[i].u32_MiuSel = 0;

        #endif

        #endif

        #if (defined(ENABLE_UMA) && ENABLE_UMA) && defined(CONFIG_ARM64)
        gAdmaDesc_st[i].u32_Address = (u32)dmaaddr;
        gAdmaDesc_st[i].u32_Address2 = (u32)(dmaaddr >> 32);
        #else
        gAdmaDesc_st[i].u32_Address = (u32)dmaaddr;
        #endif
        gAdmaDesc_st[i].u32_DmaLen = dmalen;

        if(dmalen >= 0x200)
        {
            gAdmaDesc_st[i].u32_JobCnt = (dmalen >> 9);
            //eMMC_debug(0,0,"  %Xh JobCnt\n", (dmalen >> 9));
        }
        else
        {   // should be only one sg element
            gAdmaDesc_st[i].u32_JobCnt = 1;
            REG_FCIE_W(FCIE_BLK_SIZE, dmalen);
            //eMMC_debug(0,0,"  %Xh bytes\n", dmalen);
        }

        pSG_st = sg_next(pSG_st);


        #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
        u32_nbytes += dmalen;
        #endif   
    }

    gAdmaDesc_st[pData_st->sg_len-1].u32_End = 1;

    Chip_Clean_Cache_Range_VA_PA((uintptr_t)gAdmaDesc_st,
                                 (uintptr_t)virt_to_phys(gAdmaDesc_st),
                                 sizeof(struct _AdmaDescriptor)*pData_st->sg_len);

    //eMMC_FCIE_ClearEvents();

    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);

    #if (defined(ENABLE_UMA) && ENABLE_UMA) && defined(CONFIG_ARM64)
    uint_dma_addr = eMMC_translate_DMA_address_Ex(virt_to_phys(gAdmaDesc_st), 0);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, uint_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, (uint_dma_addr >> 16) & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, uint_dma_addr >> 32);
    #else
    u32_dma_addr = eMMC_translate_DMA_address_Ex(virt_to_phys(gAdmaDesc_st), 0);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_dma_addr >> 16);
    #endif

    REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 0x0010);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16,0x0000);

}

static U32 mstar_mci_post_adma_read(struct mstar_mci_host *pMStarHost_st)
{
    /* Define Local Variables */
    struct mmc_command *pCmd_st = pMStarHost_st->cmd;
    struct mmc_data *pData_st   = pCmd_st->data;


    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
      #if !(defined(eMMC_ONLY_W_INTERRUPT) && eMMC_ONLY_W_INTERRUPT)    
    unsigned long flags;

    spin_lock_irqsave(pMStarHost_st->lock, flags);
    REG_FCIE_SETBIT(FCIE_MIE_INT_EN, (BIT_DMA_END|BIT_ERR_STS));
    spin_unlock_irqrestore(pMStarHost_st->lock, flags);
      #endif
    #endif

    if((eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, (BIT_DMA_END|BIT_ERR_STS), eMMC_GENERIC_WAIT_TIME) != eMMC_ST_SUCCESS) ||
       (REG_FCIE(FCIE_SD_STATUS)&(BIT_SD_R_CRC_ERR|BIT_DAT_RD_TOUT)))
    {
        if(REG_FCIE(FCIE_SD_STATUS)&BIT_SD_R_CRC_ERR)
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CRC STS 0x%X \n", REG_FCIE(FCIE_SD_STATUS) );
        else
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: r timeout, MIE EVENT 0x%X\n", REG_FCIE(FCIE_MIE_EVENT));
        return eMMC_ST_ERR_TIMEOUT_MIULASTDONE;
    }
    #if defined(eMMC_EMULATE_WR_FAIL) &&eMMC_EMULATE_WR_FAIL
    if(!(prandom_u32() % 1000))
    {
        return eMMC_ST_ERR_TIMEOUT_CARDDMAEND;
    }
    #endif
    pData_st->bytes_xfered = pData_st->blocks * pData_st->blksz;



    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    if(!pData_st->host_cookie)
    {
        dma_unmap_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, (int)pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    }
    #else
    dma_unmap_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM

    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    if (pData_st->host_cookie)
        dma_unmap_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    mstar_mci_miu_chksum(pData_st);

    #endif

    if(gu32_eMMC_read_log_enable)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\n");
        if(pCmd_st->opcode==17)
        {
           eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"cmd:%u ,arg:%xh\n",pCmd_st->opcode, pCmd_st->arg);
        }
        else if(pCmd_st->opcode==18)
        {
           eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"cmd:%u ,arg:%xh, blk cnt:%xh\n",pCmd_st->opcode, pCmd_st->arg ,pData_st->blocks);
        }
    }

    mstar_mci_completed_command(pMStarHost_st); // copy back rsp for cmd with data

    if(
        pMStarHost_st->request->stop
        #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
        && !pMStarHost_st->request->sbc
        #endif
    )
    {
        mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->stop);
    }
    else
    {
        if(MCI_RETRY_CNT_OK_CLK_UP == u32_ok_cnt++)
        {
            //eMMC_debug(0,1,"eMMC: restore IF\n");
            eMMC_FCIE_ErrHandler_RestoreClk();
        }
    }

    return eMMC_ST_SUCCESS;
}

#else

static void mstar_mci_pre_dma_read(struct mstar_mci_host *pMStarHost_st)
{
    /* Define Local Variables */
    struct mmc_command *pCmd_st = pMStarHost_st->cmd;
    struct mmc_data  *pData_st  = pCmd_st->data;
    struct scatterlist *pSG_st  = &pData_st->sg[0];
    u32 dmalen                  = 0;
    dma_addr_t dmaaddr          = 0;
    #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
    u32_nbytes =0;
    #endif

    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    mstar_mci_pre_dma_transfer(pMStarHost_st, pData_st, NULL);
    #else
    dma_map_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    dmaaddr = sg_dma_address(pSG_st);
    dmalen = sg_dma_len(pSG_st);

	#if !(defined(ENABLE_UMA) && ENABLE_UMA)

    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_MIU_SELECT_MASK);

    #ifdef MSTAR_MIU2_BUS_BASE
    if( dmaaddr >= MSTAR_MIU2_BUS_BASE) // MIU2
    {
        REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_MIU2_SELECT);
        dmaaddr -= MSTAR_MIU2_BUS_BASE;
    }
    else
    #endif
    if( dmaaddr >= MSTAR_MIU1_BUS_BASE) // MIU1
    {
        REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_MIU1_SELECT);
        dmaaddr -= MSTAR_MIU1_BUS_BASE;
    }
    else // MIU0
    {
        dmaaddr -= MSTAR_MIU0_BUS_BASE;
    }

	#else

    #ifdef CONFIG_MSTAR_M7821

    if (dmaaddr >= MSTAR_MIU1_BUS_BASE) // MIU1
    {
        dmaaddr -= MSTAR_MIU1_BUS_BASE;
        dmaaddr += 0x100000000;
    }
    else // MIU0
    {
        dmaaddr -= MSTAR_MIU0_BUS_BASE;
    }

    #else // Murphy
	#if defined(MSTAR_MIU1_BUS_BASE_H) && defined(ARM_MIU1_BASE_ADDR)		// for 64 bit address beyond 0x1_0000_0000
	if(dmaaddr > MSTAR_MIU1_BUS_BASE_H)
	{
		dmaaddr -= MSTAR_MIU1_BUS_BASE_H;
		dmaaddr += ARM_MIU1_BASE_ADDR;
	}
	else
		dmaaddr -= MSTAR_MIU0_BUS_BASE;
	#else
    dmaaddr -= MSTAR_MIU0_BUS_BASE;
	#endif

    #endif

	#endif


    REG_FCIE_W(FCIE_JOB_BL_CNT, dmalen>>eMMC_SECTOR_512BYTE_BITS);

    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dmaaddr & 0xFFFF);

    #if (defined(ENABLE_UMA) && ENABLE_UMA) && defined(CONFIG_ARM64)
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, (dmaaddr >> 16) & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dmaaddr >> 32);
    #else
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dmaaddr >> 16);
    #endif
    REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, dmalen & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, dmalen >> 16);

    #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
	u32_nbytes += dmalen;
    #endif
}

static U32 mstar_mci_post_dma_read(struct mstar_mci_host *pMStarHost_st)
{
    /* Define Local Variables */
    struct mmc_command *pCmd_st = pMStarHost_st->cmd;
    struct mmc_data *pData_st   = pCmd_st->data;
    struct scatterlist *pSG_st  = &(pData_st->sg[0]);
    u32 dmalen                  = 0;
    dma_addr_t dmaaddr          = 0;
    U32 u32_err                 = eMMC_ST_SUCCESS;
    int i;


    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
      #if !(defined(eMMC_ONLY_W_INTERRUPT) && eMMC_ONLY_W_INTERRUPT)    
    unsigned long flags;

    spin_lock_irqsave(pMStarHost_st->lock, flags);  
    REG_FCIE_SETBIT(FCIE_MIE_INT_EN, (BIT_DMA_END|BIT_ERR_STS));
    spin_unlock_irqrestore(pMStarHost_st->lock, flags);
      #endif
    #endif

    if(eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, (BIT_DMA_END|BIT_ERR_STS), eMMC_GENERIC_WAIT_TIME) != eMMC_ST_SUCCESS ||
       (REG_FCIE(FCIE_SD_STATUS)& (BIT_SD_R_CRC_ERR|BIT_DAT_RD_TOUT)))
    {
        if(REG_FCIE(FCIE_SD_STATUS)&BIT_SD_R_CRC_ERR)
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CRC STS 0x%X \n", REG_FCIE(FCIE_SD_STATUS) );
        else
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: r timeout, MIE EVENT 0x%X\n", REG_FCIE(FCIE_MIE_EVENT));
        return eMMC_ST_ERR_TIMEOUT_MIULASTDONE;
    }

    pData_st->bytes_xfered += pSG_st->length;


    for(i=1; i<pData_st->sg_len; i++)
    {
        eMMC_FCIE_ClearEvents_Reg0();
        pSG_st = sg_next(pSG_st);

        dmaaddr = sg_dma_address(pSG_st);
        dmalen = sg_dma_len(pSG_st);



		#if !(defined(ENABLE_UMA) && ENABLE_UMA)

        REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_MIU_SELECT_MASK);

        #ifdef MSTAR_MIU2_BUS_BASE
        if( dmaaddr >= MSTAR_MIU2_BUS_BASE) // MIU2
        {
            REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_MIU2_SELECT);
            dmaaddr -= MSTAR_MIU2_BUS_BASE;
        }
        else
        #endif
        if( dmaaddr >= MSTAR_MIU1_BUS_BASE) // MIU1
        {
            REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_MIU1_SELECT);
            dmaaddr -= MSTAR_MIU1_BUS_BASE;
        }
        else // MIU0
        {
            dmaaddr -= MSTAR_MIU0_BUS_BASE;
        }

        #else

        #ifdef CONFIG_MSTAR_M7821

        if (dmaaddr >= MSTAR_MIU1_BUS_BASE) // MIU1
        {
            dmaaddr -= MSTAR_MIU1_BUS_BASE;
            dmaaddr += 0x100000000;
        }
        else // MIU0
        {
            dmaaddr -= MSTAR_MIU0_BUS_BASE;
        }

        #else // Murphy
		#if defined(MSTAR_MIU1_BUS_BASE_H) && defined(ARM_MIU1_BASE_ADDR)		// for 64 bit address beyond 0x1_0000_0000
		if(dmaaddr > MSTAR_MIU1_BUS_BASE_H)
		{
			dmaaddr -= MSTAR_MIU1_BUS_BASE_H;
			dmaaddr += ARM_MIU1_BASE_ADDR;
		}
		else
			dmaaddr -= MSTAR_MIU0_BUS_BASE;
		#else
        dmaaddr -= MSTAR_MIU0_BUS_BASE;
		#endif

        #endif

        #endif

        REG_FCIE_W(FCIE_JOB_BL_CNT, dmalen>>eMMC_SECTOR_512BYTE_BITS);

        REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dmaaddr & 0xFFFF);
        #if (defined(ENABLE_UMA) && ENABLE_UMA) && defined(CONFIG_ARM64)
        REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, (dmaaddr >> 16) & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dmaaddr >> 32);
        #else
        REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dmaaddr >> 16);
        #endif
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, dmalen & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, dmalen >> 16);

        #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
          #if !(defined(eMMC_ONLY_W_INTERRUPT) && eMMC_ONLY_W_INTERRUPT)        
        spin_lock_irqsave(pMStarHost_st->lock, flags);
        REG_FCIE_SETBIT(FCIE_MIE_INT_EN, (BIT_DMA_END|BIT_ERR_STS));
        spin_unlock_irqrestore(pMStarHost_st->lock, flags);
          #endif
        #endif

        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_ERR_DET_ON);
        REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

        if((eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, (BIT_DMA_END|BIT_ERR_STS), eMMC_GENERIC_WAIT_TIME) != eMMC_ST_SUCCESS)||
        (REG_FCIE(FCIE_SD_STATUS)&(BIT_SD_R_CRC_ERR|BIT_DAT_RD_TOUT)))
        {
            if(REG_FCIE(FCIE_SD_STATUS)&BIT_SD_R_CRC_ERR)
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CRC STS 0x%X \n", REG_FCIE(FCIE_SD_STATUS));
            else
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: r timeout \n");
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_ERROR_RETRY;
            u32_err = eMMC_ST_ERR_TIMEOUT_MIULASTDONE;
            goto dma_read_end;
        }

        pData_st->bytes_xfered += pSG_st->length;

        // -----------------------------------
        #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
        u32_nbytes += dmalen;
        #endif
    }

    dma_read_end:

    // -----------------------------------
    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    if(!pData_st->host_cookie)
    {
        dma_unmap_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, (int)pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    }
    #else
    dma_unmap_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM

    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    if (pData_st->host_cookie)
        dma_unmap_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    if( !u32_err )
        mstar_mci_miu_chksum(pData_st);

    #endif

    if(gu32_eMMC_read_log_enable)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\n");
        if(pCmd_st->opcode==17)
        {
           eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"cmd:%u ,arg:%xh\n",pCmd_st->opcode, pCmd_st->arg);
        }
        else if(pCmd_st->opcode==18)
        {
           eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"cmd:%u ,arg:%xh, blk cnt:%xh\n",pCmd_st->opcode, pCmd_st->arg ,pData_st->blocks);
        }
    }

    if( !u32_err ) // success
    {
        mstar_mci_completed_command(pMStarHost_st); // copy back rsp for cmd with data

        if(
            pMStarHost_st->request->stop
            #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
            && !pMStarHost_st->request->sbc
            #endif
        )
        {
            mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->stop);
        }
        else
        {
            if(MCI_RETRY_CNT_OK_CLK_UP == u32_ok_cnt++)
            {
                //eMMC_debug(0,1,"eMMC: restore IF\n");
                eMMC_FCIE_ErrHandler_RestoreClk();
            }
        }
    }

    return u32_err;
}

#endif

static U32 mstar_mci_dma_write(struct mstar_mci_host *pMStarHost_st)
{
    struct mmc_command *pCmd_st = pMStarHost_st->cmd;
    struct mmc_data *pData_st   = pCmd_st->data;
    struct scatterlist  *pSG_st = 0;
    u32 dmalen                  = 0;
    dma_addr_t dmaaddr          = 0;
    U32 err                     = eMMC_ST_SUCCESS;
    #if defined(ENABLE_FCIE_ADMA) && ENABLE_FCIE_ADMA
    #if (defined(ENABLE_UMA) && ENABLE_UMA) && defined(CONFIG_ARM64)
    uintptr_t uint_dma_addr     = 0;
    #else
    U32 u32_dma_addr            = 0;
    #endif
    #endif
    int i;
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    unsigned long flags;
    #endif

    #if defined(ENABLE_FCIE_ADMA) && ENABLE_FCIE_ADMA
    if( pData_st->sg_len > FCIE_ADMA_DESC_COUNT )
    {
        eMMC_die("mstar_mci_pre_adma_read: sglist has more than FCIE_ADMA_DESC_COUNT items. Must change 512 to larger value.\n");
    }

    #endif

    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    mstar_mci_pre_dma_transfer(pMStarHost_st, pData_st, NULL);
    #else
    dma_map_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    #if defined(ENABLE_FCIE_ADMA) && ENABLE_FCIE_ADMA
    pSG_st = pData_st->sg;
    for(i=0; i<pData_st->sg_len; i++)
    {
        gAdmaDesc_st[i].u32_End = 0;
        dmaaddr = sg_dma_address(pSG_st);
        dmalen = sg_dma_len(pSG_st);



		#if !(defined(ENABLE_UMA) && ENABLE_UMA)

        #ifdef MSTAR_MIU2_BUS_BASE
        if( dmaaddr >= MSTAR_MIU2_BUS_BASE) // MIU2
        {
            dmaaddr -= MSTAR_MIU2_BUS_BASE;
            gAdmaDesc_st[i].u32_MiuSel = 2;
        }
        else
        #endif
        if( dmaaddr >= MSTAR_MIU1_BUS_BASE) // MIU1
        {
            dmaaddr -= MSTAR_MIU1_BUS_BASE;
            gAdmaDesc_st[i].u32_MiuSel = 1;
        }
        else // MIU0
        {
            dmaaddr -= MSTAR_MIU0_BUS_BASE;
            gAdmaDesc_st[i].u32_MiuSel = 0;
        }

		#else

        #ifdef CONFIG_MSTAR_M7821

        if( dmaaddr >= MSTAR_MIU1_BUS_BASE) // MIU1
        {
            dmaaddr -= MSTAR_MIU1_BUS_BASE;
            dmaaddr += 0x100000000;
        }
        else // MIU0
        {
            dmaaddr -= MSTAR_MIU0_BUS_BASE;
        }
        gAdmaDesc_st[i].u32_MiuSel = 0;

        #else // Murphy
		#if defined(MSTAR_MIU1_BUS_BASE_H) && defined(ARM_MIU1_BASE_ADDR)		// for 64 bit address beyond 0x1_0000_0000
		if(dmaaddr > MSTAR_MIU1_BUS_BASE_H)
		{
			dmaaddr -= MSTAR_MIU1_BUS_BASE_H;
			dmaaddr += ARM_MIU1_BASE_ADDR;
		}
		else
			dmaaddr -= MSTAR_MIU0_BUS_BASE;
		#else
        dmaaddr -= MSTAR_MIU0_BUS_BASE;
		#endif
        gAdmaDesc_st[i].u32_MiuSel = 0;

        #endif

		#endif

        #if (defined(ENABLE_UMA) && ENABLE_UMA) && defined(CONFIG_ARM64)
        gAdmaDesc_st[i].u32_Address = (u32)dmaaddr;
        gAdmaDesc_st[i].u32_Address2 = (u32)(dmaaddr >> 32);
        #else
        gAdmaDesc_st[i].u32_Address = dmaaddr;
        #endif
        gAdmaDesc_st[i].u32_DmaLen = dmalen;
        gAdmaDesc_st[i].u32_JobCnt = (dmalen >> 9);

        pSG_st = sg_next(pSG_st);
    }

    gAdmaDesc_st[pData_st->sg_len-1].u32_End = 1;

    Chip_Clean_Cache_Range_VA_PA((uintptr_t)gAdmaDesc_st,
                                 (uintptr_t)virt_to_phys(gAdmaDesc_st),
                                 sizeof(struct _AdmaDescriptor)*pData_st->sg_len);

    //eMMC_FCIE_ClearEvents();

    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    REG_FCIE_W(FCIE_BLK_SIZE, eMMC_SECTOR_512BYTE);

    #if (defined(ENABLE_UMA) && ENABLE_UMA) && defined(CONFIG_ARM64)
    uint_dma_addr = eMMC_translate_DMA_address_Ex(virt_to_phys(gAdmaDesc_st), 0);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, uint_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, (uint_dma_addr >> 16) & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, uint_dma_addr >> 32);
    #else
    u32_dma_addr = eMMC_translate_DMA_address_Ex(virt_to_phys(gAdmaDesc_st), 0);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_dma_addr >> 16);
    #endif

    REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 0x0010);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16,0x0000);

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    spin_lock_irqsave(pMStarHost_st->lock, flags);
    REG_FCIE_SETBIT(FCIE_MIE_INT_EN, (BIT_DMA_END|BIT_ERR_STS));
    spin_unlock_irqrestore(pMStarHost_st->lock, flags);
    #endif

    REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DTRX_EN|BIT_SD_DAT_DIR_W|BIT_ADMA_EN|BIT_ERR_DET_ON);
    REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

    if((eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, (BIT_DMA_END|BIT_ERR_STS), eMMC_GENERIC_WAIT_TIME) != eMMC_ST_SUCCESS)||
        (REG_FCIE(FCIE_SD_STATUS)&(BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
    {
        if((REG_FCIE(FCIE_SD_STATUS)&BIT_SD_W_FAIL))
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CRC STS 0x%X \n", REG_FCIE(FCIE_SD_STATUS) );
        else
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: w timeout \n");
        err = eMMC_ST_ERR_TIMEOUT_CARDDMAEND;
        goto dma_write_end;
    }

    #if defined(eMMC_EMULATE_WR_FAIL) &&eMMC_EMULATE_WR_FAIL
    if(!(prandom_u32() % 500))
    {
        err = eMMC_ST_ERR_TIMEOUT_CARDDMAEND;
        goto dma_write_end;
    }
    #endif
    pData_st->bytes_xfered = pData_st->blocks * pData_st->blksz;

    #else

    pSG_st = &(pData_st->sg[0]);
    for(i=0; i<pData_st->sg_len; i++)
    {
        dmaaddr = sg_dma_address(pSG_st);
        dmalen = sg_dma_len(pSG_st);



        #if !(defined(ENABLE_UMA) && ENABLE_UMA)

        REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_MIU_SELECT_MASK);
        #ifdef MSTAR_MIU2_BUS_BASE
        if (dmaaddr >= MSTAR_MIU2_BUS_BASE) // MIU2
        {
            REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_MIU2_SELECT);
            dmaaddr -= MSTAR_MIU2_BUS_BASE;
        }
        else
        #endif
        if (dmaaddr >= MSTAR_MIU1_BUS_BASE) // MIU1
        {
            REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_MIU1_SELECT);
            dmaaddr -= MSTAR_MIU1_BUS_BASE;
        }
        else // MIU0
        {
            dmaaddr -= MSTAR_MIU0_BUS_BASE;
        }

		#else

        #ifdef CONFIG_MSTAR_M7821

        if (dmaaddr >= MSTAR_MIU1_BUS_BASE) // MIU1
        {
            dmaaddr -= MSTAR_MIU1_BUS_BASE;
            dmaaddr += 0x100000000;
        }
        else // MIU0
        {
            dmaaddr -= MSTAR_MIU0_BUS_BASE;
        }

        #else // Murphy
        #if defined(MSTAR_MIU1_BUS_BASE_H) && defined(ARM_MIU1_BASE_ADDR)		// for 64 bit address beyond 0x1_0000_0000
        if(dmaaddr > MSTAR_MIU1_BUS_BASE_H)
        {
            dmaaddr -= MSTAR_MIU1_BUS_BASE_H;
            dmaaddr += ARM_MIU1_BASE_ADDR;
        }
        else
            dmaaddr -= MSTAR_MIU0_BUS_BASE;
        #else
        dmaaddr -= MSTAR_MIU0_BUS_BASE;
        #endif

        #endif

        #endif

        eMMC_FCIE_ClearEvents_Reg0();

        REG_FCIE_W(FCIE_BLK_SIZE, 512);
        REG_FCIE_W(FCIE_JOB_BL_CNT, dmalen>>eMMC_SECTOR_BYTECNT_BITS);

        REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dmaaddr & 0xFFFF);
        #if (defined(ENABLE_UMA) && ENABLE_UMA) && defined(CONFIG_ARM64)
        REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, (dmaaddr >> 16) & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dmaaddr >> 32);
        #else
        REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dmaaddr >> 16);
        #endif
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, dmalen & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, dmalen >> 16);

        if(eMMC_ST_SUCCESS != mstar_mci_WaitD0High(TIME_WAIT_DAT0_HIGH))
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: wait D0 H TO\n");
            eMMC_FCIE_ErrHandler_Stop();
        }

        #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
		spin_lock_irqsave(pMStarHost_st->lock, flags);
        REG_FCIE_SETBIT(FCIE_MIE_INT_EN, (BIT_DMA_END|BIT_ERR_STS));
		spin_unlock_irqrestore(pMStarHost_st->lock, flags);
        #endif

        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W|BIT_ERR_DET_ON);
        REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

        if((eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, (BIT_DMA_END|BIT_ERR_STS), eMMC_GENERIC_WAIT_TIME) != eMMC_ST_SUCCESS)||
           (REG_FCIE(FCIE_SD_STATUS)&(BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
        {
            if((REG_FCIE(FCIE_SD_STATUS)&BIT_SD_W_FAIL))
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CRC STS 0x%X \n", REG_FCIE(FCIE_SD_STATUS));
            else
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: w timeout \n");
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_ERROR_RETRY;
            err = eMMC_ST_ERR_TIMEOUT_CARDDMAEND;
            goto dma_write_end;
        }

        // -----------------------------------

        pData_st->bytes_xfered += pSG_st->length;
        pSG_st = sg_next(pSG_st);

    }

    #endif

    dma_write_end:



    // -----------------------------------
    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    if(!pData_st->host_cookie)
    {
        dma_unmap_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, (int)pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    }
    #else
    dma_unmap_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif

    #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    if (pData_st->host_cookie)
        dma_unmap_sg(mmc_dev(pMStarHost_st->mmc), pData_st->sg, pData_st->sg_len, mstar_mci_get_dma_dir(pData_st));
    #endif
    #endif
    if(gu32_eMMC_write_log_enable)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\n");
        if(pCmd_st->opcode==24)
        {
           eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"cmd:%u, arg:%xh\n",pCmd_st->opcode,pCmd_st->arg);
        }
        else if(pCmd_st->opcode==25)
        {
           eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"cmd:%u, arg:%xh, blk cnt:%xh\n",pCmd_st->opcode,pCmd_st->arg,pData_st->blocks);
        }
    }

    if( !err )
    {
        mstar_mci_completed_command(pMStarHost_st); // copy back rsp for cmd with data

        if(
            pMStarHost_st->request->stop
            #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
            && !pMStarHost_st->request->sbc
            #endif
        )
        {
            mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->stop);
        }
        else
        {
            if(MCI_RETRY_CNT_OK_CLK_UP == u32_ok_cnt++)
            {
                //eMMC_debug(0,1,"eMMC: restore IF\n");
                eMMC_FCIE_ErrHandler_RestoreClk();
            }
        }
    }

    return err;
}

static void mstar_mci_completed_command(struct mstar_mci_host *pMStarHost_st)
{
    /* Define Local Variables */
	struct mmc_command *pCmd_st = pMStarHost_st->cmd;
	static u32 u32_retry_cnt    = 0;
    u16 u16_st;
    u16 u16_i;
    u8 *pTemp;

    // ----------------------------------
    // retrun response from FCIE to mmc driver
    pTemp = (u8*)&(pCmd_st->resp[0]);
    for(u16_i=0; u16_i < 15; u16_i++)
    {
        pTemp[(3 - (u16_i % 4)) + (4 * (u16_i / 4))] =
            (u8)(REG_FCIE(FCIE_CMDFIFO_BASE_ADDR+(((u16_i+1)/2)*4)) >> (8*((u16_i+1)%2)));
    }
    #if 0
    eMMC_debug(0,0,"------------------\n");
    eMMC_debug(0,1,"resp[0]: %08Xh\n", pCmd_st->resp[0]);
    eMMC_debug(0,1,"CIFC: %04Xh %04Xh %04Xh \n", REG_FCIE(FCIE_CMDFIFO_BASE_ADDR),
        REG_FCIE(FCIE_CMDFIFO_BASE_ADDR+4), REG_FCIE(FCIE_CMDFIFO_BASE_ADDR+8));
    //eMMC_dump_mem(pTemp, 0x10);
    eMMC_debug(0,0,"------------------\n");
    #endif

    // ----------------------------------
    u16_st = REG_FCIE(FCIE_SD_STATUS);
    if((u16_st & BIT_SD_FCIE_ERR_FLAGS) || (g_eMMCDrv.u32_DrvFlag & DRV_FLAG_ERROR_RETRY))
    {
        g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_ERROR_RETRY;

        if((u16_st & BIT_SD_RSP_CRC_ERR) && !(mmc_resp_type(pCmd_st) & MMC_RSP_CRC))
        {
            pCmd_st->error = 0;
            u32_retry_cnt = 0;
            if((pCmd_st->opcode == 1) &&((pCmd_st->resp[0]>>31) & BIT0))
                g_eMMCDrv.u8_IfSectorMode= (pCmd_st->resp[0] >>30) & BIT0;
        }
        else
        {
            #if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
                "eMMC Err: ST:%Xh, CMD:%u, retry: %u, flag: %Xh, R1 err: %Xh\n",
                u16_st, pCmd_st->opcode, u32_retry_cnt, g_eMMCDrv.u32_DrvFlag,
                pCmd_st->resp[0]&eMMC_ERR_R1_NEED_RETRY);

            u32_ok_cnt = 0;

            if(u32_retry_cnt++ >= MCI_RETRY_CNT_CRC_ERR)
                eMMC_FCIE_ErrHandler_Stop(); // fatal error

            if(12==pCmd_st->opcode)
                eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);

            eMMC_hw_timer_sleep(1);
            eMMC_FCIE_ErrHandler_ReInit();

            if(0 != pCmd_st->data)
            {
                eMMC_FCIE_ErrHandler_Retry(); // slow dwon clock

                #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
                if( pMStarHost_st->request->sbc)
                    mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->sbc);
                else
                #endif
                mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->cmd);
            }
            else // for CMD12
            {
                eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: no data, just reset eMMC. \n");
                REG_FCIE_W(FCIE_SD_STATUS, BIT_SD_FCIE_ERR_FLAGS);
                mstar_mci_completed_command_FromRAM(pMStarHost_st);
                return;
            }

            if(0==u32_retry_cnt)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: retry ok \n");
                return;
            }

            //-----------------------------------------
            #else
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: ST:%Xh, CMD:%u, arg:%xh, retry:%u \n",
                u16_st, pCmd_st->opcode, pCmd_st->arg , pCmd_st->retries);
            #endif

            // should be trivial
            if(u16_st & (BIT_SD_RSP_TIMEOUT))
                pCmd_st->error = -ETIMEDOUT;
            else if(u16_st & (BIT_SD_RSP_CRC_ERR | BIT_SD_R_CRC_ERR | BIT_SD_W_CRC_ERR))
                pCmd_st->error = -EILSEQ;
            else
                pCmd_st->error = -EIO;
        }
    }
    else
    {
        #if 0
        u16 u16_Tmp;

        // To prevent res bit shift
        u16_Tmp = REG_FCIE(FCIE_CMDFIFO_BASE_ADDR);

        if( (mmc_resp_type(pCmd_st) == MMC_RSP_R2) ||
            (mmc_resp_type(pCmd_st) == MMC_RSP_R3) )
        {
            if( (u16_Tmp & 0x3F) != 0x3F )
            {
                pCmd_st->error = -EILSEQ;
                eMMC_debug(0, 1, "CMD%d response buffer error\n", pCmd_st->opcode);
                eMMC_FCIE_ErrHandler_Stop();
            }
        }
        else
        {
            if( (u16_Tmp & 0xFF) != pCmd_st->opcode )
            {
                pCmd_st->error = -EILSEQ;
                eMMC_debug(0, 1, "CMD%d response buffer error\n", pCmd_st->opcode);
                eMMC_FCIE_ErrHandler_Stop();
            }
        }
        #endif

        pCmd_st->error = 0;
        u32_retry_cnt = 0;
    }


    if((pCmd_st->opcode == 17) || (pCmd_st->opcode == 18) ||
       (pCmd_st->opcode == 24) || (pCmd_st->opcode == 25) ||
       (pCmd_st->opcode == 23) || (pCmd_st->opcode == 12) ||
       (pCmd_st->opcode == 6))
    {
        if(pCmd_st->resp[0] & eMMC_ERR_R1_31_0)
        {
            pCmd_st->error |= -EIO;

            if((pCmd_st->opcode == 6) || (pCmd_st->opcode == 12) || (pCmd_st->opcode == 23))
				eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0, "eMMC Warn: CMD%u R1 error: %Xh, arg %08x\n",
				   pCmd_st->opcode, pCmd_st->resp[0], pCmd_st->arg);
            else
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0, "eMMC Warn: CMD%u R1 error: %Xh, arg %08x, blocks %08x\n",
                    pCmd_st->opcode, pCmd_st->resp[0], pCmd_st->arg, pCmd_st->data->blocks);

            eMMC_DumpPadClk();
            eMMC_FCIE_DumpRegisters();
            eMMC_FCIE_DumpDebugBus();
        }
    }

    #if 0
    if(MCI_RETRY_CNT_OK_CLK_UP == u32_ok_cnt)
    {
        eMMC_debug(0,1,"eMMC: restore IF\n");
        eMMC_FCIE_ErrHandler_RestoreClk();
    }
    #endif

    REG_FCIE_W(FCIE_SD_STATUS, BIT_SD_FCIE_ERR_FLAGS);
}

#define WAIT_D0H_POLLING_TIME   HW_TIMER_DELAY_100us
u32 mstar_mci_WaitD0High(u32 u32_us)
{
    #if defined(ENABLE_FCIE_HW_BUSY_CHECK)&&ENABLE_FCIE_HW_BUSY_CHECK

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    unsigned long flags;
    #endif

    REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_BUSY_DET_ON);

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
	spin_lock_irqsave(&fcie_lock, flags);
    // enable busy int
    REG_FCIE_SETBIT(FCIE_MIE_INT_EN, BIT_BUSY_END_INT);
	spin_unlock_irqrestore(&fcie_lock, flags);
    #endif

    if(eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_BUSY_END_INT, u32_us) != eMMC_ST_SUCCESS)
    {
        REG_FCIE_CLRBIT(FCIE_SD_CTRL, BIT_BUSY_DET_ON);
        return eMMC_ST_ERR_TIMEOUT_WAITD0HIGH;
    }
    REG_FCIE_CLRBIT(FCIE_SD_CTRL, BIT_BUSY_DET_ON);
    return eMMC_ST_SUCCESS;

    #else

    u32 u32_cnt, u32_wait;

    for(u32_cnt=0; u32_cnt<u32_us; u32_cnt+=WAIT_D0H_POLLING_TIME)
    {
        u32_wait = eMMC_FCIE_WaitD0High_Ex(WAIT_D0H_POLLING_TIME);

        if(u32_wait < WAIT_D0H_POLLING_TIME)
        {
            #if 0
            if(u32_cnt + u32_wait)
                printk("eMMC wait d0: %u us\n", u32_cnt+u32_wait);
            #endif

            return eMMC_ST_SUCCESS;
        }

        //if(u32_cnt > HW_TIMER_DELAY_1ms)
        {
            //msleep(1);
            //schedule_hrtimeout is more precise and can reduce idle time of emmc

            {
                ktime_t expires = ktime_add_ns(ktime_get(), 1000 * 1000);
                set_current_state(TASK_UNINTERRUPTIBLE);
                schedule_hrtimeout(&expires, HRTIMER_MODE_ABS);
            }

            u32_cnt += HW_TIMER_DELAY_1ms;
        }
    }

    return eMMC_ST_ERR_TIMEOUT_WAITD0HIGH;

    #endif
}

#if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
static void mstar_mci_send_data(struct work_struct *work)
{
    struct mstar_mci_host *pMStarHost_st    = container_of(work, struct mstar_mci_host, async_work);
    struct mmc_command *pCmd_st             = pMStarHost_st->cmd;
    struct mmc_data *pData_st               = pCmd_st->data;
    static u8 u8_retry_data                 = 0;
    U32 err                                 = eMMC_ST_SUCCESS;
    U32 u32_per_read_time =0, u32_per_write_time =0;


    if(pData_st->flags & MMC_DATA_WRITE)
    {
        err = mstar_mci_dma_write(pMStarHost_st);
    }
    else if(pData_st->flags & MMC_DATA_READ)
    {
        #if defined(ENABLE_FCIE_ADMA) && ENABLE_FCIE_ADMA
        err = mstar_mci_post_adma_read(pMStarHost_st);
        #else
        err = mstar_mci_post_dma_read(pMStarHost_st);
        #endif
    }

    if( err )
    {
        u32_ok_cnt = 0;

        if(pData_st->flags & MMC_DATA_WRITE)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: w, cmd.%u arg.%Xh, ST: %Xh or TO\n",
                pCmd_st->opcode, pCmd_st->arg, REG_FCIE(FCIE_SD_STATUS));
        }
        else if(pData_st->flags & MMC_DATA_READ)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: r, cmd.%u arg.%Xh, ST: %Xh or TO \n",
                pCmd_st->opcode, pCmd_st->arg, REG_FCIE(FCIE_SD_STATUS));
        }

        if(u8_retry_data < MCI_RETRY_CNT_CMD_TO)
        {
            u8_retry_data++;
            eMMC_FCIE_ErrHandler_ReInit();
            if((g_eMMCDrv.u8_partition_config & 0x7) == 0x3)	//Disable RPMB retry
            {
                u8_retry_data = 0;
                pMStarHost_st->request->cmd->data->error =-EIO;
                eMMC_UnlockFCIE((U8*)__FUNCTION__);
                mmc_request_done(pMStarHost_st->mmc, pMStarHost_st->request);
            }
            else
            {
                eMMC_FCIE_ErrHandler_Retry();
                #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
                if( pMStarHost_st->request->sbc)
                    mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->sbc);
                else
                #endif
                    mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->cmd);
            }
        }
        else
        {
            #if defined(ENABLE_FCIE_ADMA) && ENABLE_FCIE_ADMA
            eMMC_dump_mem((U8*)gAdmaDesc_st, (U32)(sizeof(struct _AdmaDescriptor)*(pData_st->sg_len+1)));
            #endif
            eMMC_FCIE_ErrHandler_Stop();
        }
    }
    else
    {
        if(u8_retry_data)
            eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC: cmd.%u arg.%Xh blocks:%u: data retry ok \n",
            pCmd_st->opcode, pCmd_st->arg, pData_st->blocks);

        if(gu32_eMMC_monitor_enable)
        {
           if((pCmd_st->opcode==17)||(pCmd_st->opcode==18)) {
               u32_per_read_time = (U32)(ktime_to_ns(ktime_sub(ktime_get(), starttime))/1000);
               if ((emmc_rw_speed.total_read_bytes + pData_st->blocks*eMMC_SECTOR_512BYTE) > 0 &&
                   (emmc_rw_speed.total_read_time_usec + u32_per_read_time) > 0) {
                   emmc_rw_speed.total_read_bytes += pData_st->blocks*eMMC_SECTOR_512BYTE;
                   emmc_rw_speed.total_read_time_usec += u32_per_read_time;
               }
           }
           else if((pCmd_st->opcode==24)||(pCmd_st->opcode==25)) {
               u32_per_write_time = (U32)(ktime_to_ns(ktime_sub(ktime_get(), starttime))/1000);
               if ((emmc_rw_speed.total_write_bytes + pData_st->blocks*eMMC_SECTOR_512BYTE) > 0 &&
                   (emmc_rw_speed.total_write_time_usec + u32_per_write_time) > 0) {
                   emmc_rw_speed.total_write_bytes += pData_st->blocks*eMMC_SECTOR_512BYTE;
                   emmc_rw_speed.total_write_time_usec += u32_per_write_time;
               }
           }
        }

        #if !(defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM)
        if( pCmd_st->opcode == 8 )
        {
            mstar_mci_config_ecsd(pData_st, pMStarHost_st->mmc);
        }
        #endif

        u8_retry_data =0;


        eMMC_UnlockFCIE((U8*)__FUNCTION__);
        mmc_request_done(pMStarHost_st->mmc, pMStarHost_st->request);
    }
}
#endif

static void mstar_check_BootMode_config(struct mmc_command *pCmd_st)
{
   u8 u8_patition_config = g_eMMCDrv.u8_partition_config;
   u8 u8_bootbus_condition = g_eMMCDrv.u8_bootbus_condition;

   if(pCmd_st->opcode  == 0x6) {
       if(((pCmd_st->arg & 0xff0000)>>16) == 0xB3) {
           switch((pCmd_st->arg>>24)&3)
           {
                case eMMC_ExtCSD_SetBit:
                u8_patition_config |=  (pCmd_st->arg&0xFF00)>>8;
                if ((u8_patition_config >>3) !=0x1) {
                    pCmd_st->arg &= ~(pCmd_st->arg&0x7800);//Clear ecsd[179] bit6~bit3
                    pCmd_st->arg |= 0x800; //ECSD set boot partiton 1 enable for boot               
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: mmc switch to wrong ecsd[179]= %Xh\n", 
                        u8_patition_config);       
                }
                break;
                case eMMC_ExtCSD_ClrBit:
                u8_patition_config &= ~((pCmd_st->arg&0xFF00)>>8);
                if ((u8_patition_config >>3) !=0x1) {
                    pCmd_st->arg &= ~(pCmd_st->arg&0x7800);//Do not set ecsd[179] bit6~bit3
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: mmc switch to wrong ecsd[179]= %Xh\n", 
                        u8_patition_config);
                }
                break;
                case eMMC_ExtCSD_WByte:
                u8_patition_config = (pCmd_st->arg&0xFF00)>>8;
                if ((u8_patition_config >>3) !=0x1) {
                    pCmd_st->arg &= ~(pCmd_st->arg&0x7800);//Clear ecsd[179] bit6~bit3
                    pCmd_st->arg |= 0x800; //ECSD set boot partiton 1 enable for boot   
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: mmc switch to wrong ecsd[179]= %Xh\n", 
                        u8_patition_config);
                }
                break;
           }
       }
       else if(((pCmd_st->arg & 0xff0000)>>16) == 0xB1) {
           switch((pCmd_st->arg>>24)&3)
           {
                case eMMC_ExtCSD_SetBit:
                u8_bootbus_condition |=  (pCmd_st->arg&0xFF00)>>8;
                if (u8_bootbus_condition !=0x2) {
                    pCmd_st->arg &= ~(pCmd_st->arg&0xFF00);//Clear ecsd[177]  bit7~bit0 
                    pCmd_st->arg |= 0x200; //Set boot bus condition is equal 2                  
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: mmc switch to wrong ecsd[177]= %Xh\n", 
                        u8_bootbus_condition);
                }
                break;
                case eMMC_ExtCSD_ClrBit:
                u8_bootbus_condition &= ~((pCmd_st->arg&0xFF00)>>8);
                if (u8_bootbus_condition !=0x2) {
                    pCmd_st->arg &= ~(pCmd_st->arg&0xFF00);//Do not set ecsd[177]               
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: mmc switch to wrong ecsd[177]= %Xh\n", 
                        u8_bootbus_condition);
                }      
                break;
                case eMMC_ExtCSD_WByte:
                u8_bootbus_condition = (pCmd_st->arg&0xFF00)>>8;
                if (u8_bootbus_condition !=0x2) {
                    pCmd_st->arg &= ~(pCmd_st->arg&0xFF00);//Clear ecsd[177]  bit7~bit0 
                    pCmd_st->arg |= 0x200; //Set boot bus condition is equal 2
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: mmc switch to wrong ecsd[177]= %Xh\n", 
                        u8_bootbus_condition);
                }
                break;
           }
       }

   }

}

static void mstar_mci_send_command(struct mstar_mci_host *pMStarHost_st, struct mmc_command *pCmd_st)
{
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
      #if !(defined(eMMC_ONLY_W_INTERRUPT) && eMMC_ONLY_W_INTERRUPT)
    u32 u32_mie_int     = 0;
      #endif
    #endif
    u32 u32_sd_ctl      = 0;
    u32 u32_sd_mode     = 0;
	static  u8  u8_retry_cmd    = 0;
    u8  u8_retry_D0H    = 0;
    struct mmc_data *pData_st;

    #if !(defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO)
    static u8  u8_retry_data=0;
    U32 u32_per_read_time =0, u32_per_write_time =0;
    #endif


    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
      #if !(defined(eMMC_ONLY_W_INTERRUPT) && eMMC_ONLY_W_INTERRUPT)
    unsigned long flags;
      #endif
    #endif

    u32 err = 0;

    LABEL_SEND_CMD:
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_ERROR_RETRY;
    u32_sd_mode = g_eMMCDrv.u16_Reg10_Mode;
    pMStarHost_st->cmd = pCmd_st;
    pData_st = pCmd_st->data;

    eMMC_FCIE_ClearEvents();

    if(12!=pCmd_st->opcode)
    {
        if(eMMC_ST_SUCCESS != mstar_mci_WaitD0High(TIME_WAIT_DAT0_HIGH))
        {
            u32_ok_cnt = 0;

            eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: retry wait D0 H.\n");

            u8_retry_D0H++;
            if(u8_retry_D0H < 10)
            {
                eMMC_pads_switch(g_eMMCDrv.u8_PadType);
                eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
                goto LABEL_SEND_CMD;
            }
            else
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: wait D0 H timeout\n");
                eMMC_FCIE_ErrHandler_Stop();
            }
        }
    }

    if(u8_retry_D0H)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC: wait D0 H [ok]\n");
        u8_retry_D0H = 0;
    }

    mstar_check_BootMode_config(pCmd_st);

    if(pData_st)
    {
        pData_st->bytes_xfered = 0;

        if (pData_st->flags & MMC_DATA_READ)
        {
            #if defined(ENABLE_FCIE_ADMA) && ENABLE_FCIE_ADMA
            u32_sd_ctl |= (BIT_SD_DAT_EN | BIT_ADMA_EN | BIT_ERR_DET_ON);
            mstar_mci_pre_adma_read(pMStarHost_st);
            #else
            u32_sd_ctl |= (BIT_SD_DAT_EN | BIT_ERR_DET_ON);
            if(pCmd_st->opcode == 18)
                u32_sd_mode |= BIT_SD_DMA_R_CLK_STOP;
            else
                u32_sd_mode &= ~BIT_SD_DMA_R_CLK_STOP;

            mstar_mci_pre_dma_read(pMStarHost_st);
            #endif
        }
    }

    u32_sd_ctl |= BIT_SD_CMD_EN;
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
      #if !(defined(eMMC_ONLY_W_INTERRUPT) && eMMC_ONLY_W_INTERRUPT)    
    u32_mie_int |= BIT_CMD_END;
      #endif
    #endif  
    REG_FCIE_W(GET_REG_ADDR(FCIE_CMDFIFO_BASE_ADDR, 0x00),
        (((pCmd_st->arg >> 24)<<8) | (0x40|pCmd_st->opcode)));
    REG_FCIE_W(GET_REG_ADDR(FCIE_CMDFIFO_BASE_ADDR, 0x01),
        ((pCmd_st->arg & 0xFF00) | ((pCmd_st->arg>>16)&0xFF)));
    REG_FCIE_W(GET_REG_ADDR(FCIE_CMDFIFO_BASE_ADDR, 0x02),
        (pCmd_st->arg & 0xFF));

    REG_FCIE_CLRBIT(FCIE_CMD_RSP_SIZE, BIT_RSP_SIZE_MASK);
    if(mmc_resp_type(pCmd_st) == MMC_RSP_NONE)
    {
        u32_sd_ctl &= ~BIT_SD_RSP_EN;
    }
    else
    {
        u32_sd_ctl |= BIT_SD_RSP_EN;
        if(mmc_resp_type(pCmd_st) == MMC_RSP_R2)
        {
            u32_sd_ctl |= BIT_SD_RSPR2_EN;
            REG_FCIE_SETBIT(FCIE_CMD_RSP_SIZE, 16);
        }
        else
        {
            REG_FCIE_SETBIT(FCIE_CMD_RSP_SIZE, 5);
        }
    }

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
      #if !(defined(eMMC_ONLY_W_INTERRUPT) && eMMC_ONLY_W_INTERRUPT)
    spin_lock_irqsave(pMStarHost_st->lock, flags);
    REG_FCIE_W(FCIE_MIE_INT_EN, u32_mie_int);
    spin_unlock_irqrestore(pMStarHost_st->lock, flags);
      #endif
    #endif

    #if 0
    //eMMC_debug(0,0,"\n");
    eMMC_debug(0,0,"cmd:%u, arg:%Xh, buf:%lXh, block:%u, ST:%Xh, mode:%Xh, ctrl:%Xh \n",
        pCmd_st->opcode, pCmd_st->arg, (unsigned long)pData_st, pData_st ? pData_st->blocks : 0,
        REG_FCIE(FCIE_SD_STATUS), u32_sd_mode, u32_sd_ctl);
    //eMMC_debug(0,0,"cmd:%u arg:%Xh\n", pCmd_st->opcode, pCmd_st->arg);
    //while(1);
    #endif



    #if defined(ENABLE_EMMC_POWER_SAVING_MODE) && ENABLE_EMMC_POWER_SAVING_MODE
    // -----------------------------------
    if((pCmd_st->opcode==12)||(pCmd_st->opcode==24)||(pCmd_st->opcode==25))
    {
        eMMC_CheckPowerCut();
    }
    #endif
    if(gu32_eMMC_monitor_enable)
    {
        if((pCmd_st->opcode==17)||(pCmd_st->opcode==18)||
            (pCmd_st->opcode==24)||(pCmd_st->opcode==25))
            starttime = ktime_get();
    }
    REG_FCIE_W(FCIE_SD_MODE, u32_sd_mode);
    REG_FCIE_W(FCIE_SD_CTRL, u32_sd_ctl);

    #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
    if (pData_st)
        eMMC_clear_miu_chksum();

    #endif

    REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

    // [FIXME]: retry and timing, and more...
    if(eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_CMD_END, HW_TIMER_DELAY_1s) != eMMC_ST_SUCCESS ||
       (REG_FCIE(FCIE_SD_STATUS)&(BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)))
    {
        // ------------------------------------
        #if !(defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM)
        if(NULL==pData_st
        #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
        && NULL==pMStarHost_st->request->sbc
        #endif
        ) // no data, no retry
        {
            if ((pCmd_st->opcode == 1) && (REG_FCIE(FCIE_SD_STATUS) & BIT_SD_RSP_TIMEOUT))
            {
                u8_retry_cmd++;

                if(u8_retry_cmd < MCI_RETRY_CNT_CMD_TO)
                {
                    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "CMD1 SD_STS=%04X\n", REG_FCIE(FCIE_SD_STATUS));

                    err = eMMC_FCIE_Init();
                    if (err) {
                        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: FCIE_Init fail, %Xh\n", err);
                        eMMC_FCIE_ErrHandler_Stop();
                    }

                    g_eMMCDrv.u32_DrvFlag = 0;

                    eMMC_PlatformInit();

                    eMMC_RST_L();  eMMC_hw_timer_sleep(1);
                    eMMC_RST_H();  eMMC_hw_timer_sleep(1);

                    if (eMMC_ST_SUCCESS != eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH)) {
                        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: WaitD0High TO\n");
                        eMMC_FCIE_ErrHandler_Stop();
                    }

                    g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_1;
                    g_eMMCDrv.u16_Reg10_Mode = BIT_SD_DEFAULT_MODE_REG;
                    g_eMMCDrv.u16_RCA=1;

                    // CMD0
                    err = eMMC_CMD0(0); // reset to idle state
                    if (eMMC_ST_SUCCESS != err)
                    {
                        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Resend CMD0 fail, %Xh\n", err);
                        eMMC_FCIE_ErrHandler_Stop();
                    }

                    goto LABEL_SEND_CMD;
                }
                else
                {
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "CMD1 retry cannot help anything\n");
                    eMMC_FCIE_ErrHandler_Stop();
                }
            }

            mstar_mci_completed_command(pMStarHost_st);
            eMMC_UnlockFCIE((U8*)__FUNCTION__);
            mmc_request_done(pMStarHost_st->mmc, pMStarHost_st->request);
            return;
        }
        #endif

        // ------------------------------------
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
                   "eMMC Err: cmd.%u arg.%Xh St: %Xh, retry %u \n",
                   pCmd_st->opcode, pCmd_st->arg, REG_FCIE(FCIE_SD_STATUS), u8_retry_cmd);

        if(u8_retry_cmd < MCI_RETRY_CNT_CMD_TO)
        {
            u8_retry_cmd++;

            if(12==pCmd_st->opcode)
            {
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();

                #if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
                mstar_mci_completed_command_FromRAM(pMStarHost_st);
                #endif

                return;
            }

            #if 0
            if(25==pCmd_st->opcode)
                eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
            #endif

            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();

            u32_ok_cnt = 0;
            #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
            if( pMStarHost_st->request->sbc)
            {
                mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->sbc);
                return;
            }
            #endif
            goto LABEL_SEND_CMD;
        }

        eMMC_FCIE_ErrHandler_Stop();
    }

    if(u8_retry_cmd)
    {
        u8_retry_cmd =0;
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: cmd.%u arg.%Xh CMD retry ok\n",pCmd_st->opcode, pCmd_st->arg);
    }

    if(pData_st)
    {
        #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO

        queue_work(mci_workqueue, &pMStarHost_st->async_work);

        #else

        if(pData_st->flags & MMC_DATA_WRITE)
        {
            err = mstar_mci_dma_write(pMStarHost_st);
        }
        else if(pData_st->flags & MMC_DATA_READ)
        {
            #if defined(ENABLE_FCIE_ADMA) && ENABLE_FCIE_ADMA
            err = mstar_mci_post_adma_read(pMStarHost_st);
            #else
            err = mstar_mci_post_dma_read(pMStarHost_st);
            #endif
        }

        if( err )
        {
            if(pData_st->flags & MMC_DATA_WRITE)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: w timeout, cmd.%u arg.%Xh, ST: %Xh \n",
                    pCmd_st->opcode, pCmd_st->arg, REG_FCIE(FCIE_SD_STATUS));
            }
            else if(pData_st->flags & MMC_DATA_READ)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: r timeout, cmd.%u arg.%Xh, ST: %Xh \n",
                    pCmd_st->opcode, pCmd_st->arg, REG_FCIE(FCIE_SD_STATUS));
            }

            if(u8_retry_data < MCI_RETRY_CNT_CMD_TO)
            {
                u8_retry_data++;
                eMMC_FCIE_ErrHandler_ReInit();
                u32_ok_cnt = 0;
                if((g_eMMCDrv.u8_partition_config & 0x7) == 0x3) //Disable RPMB retry
                {
                    u8_retry_data = 0;
                    pMStarHost_st->request->cmd->data->error =-EIO;
                    eMMC_UnlockFCIE((U8*)__FUNCTION__);
                    mmc_request_done(pMStarHost_st->mmc, pMStarHost_st->request);
                }
                else
                {
                    eMMC_FCIE_ErrHandler_Retry();
                    #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
                    if( pMStarHost_st->request->sbc)
                        mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->sbc);
                    else
                    #endif
                        mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->cmd);
                }
            }
            else
            {
                #if defined(ENABLE_FCIE_ADMA) && ENABLE_FCIE_ADMA
                eMMC_dump_mem((U8*)gAdmaDesc_st, (U32)(sizeof(struct _AdmaDescriptor)*(pData_st->sg_len+1)));
                #endif
                eMMC_FCIE_ErrHandler_Stop();
            }
        }
        else
        {
            if(u8_retry_data)
                eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC: data retry ok \n");

            if(gu32_eMMC_monitor_enable)
            {
                if((pCmd_st->opcode==17)||(pCmd_st->opcode==18)) {
                    u32_per_read_time = (U32)(ktime_to_ns(ktime_sub(ktime_get(), starttime))/1000);
                    if ((emmc_rw_speed.total_read_bytes + pData_st->blocks*eMMC_SECTOR_512BYTE) > 0 &&
                        (emmc_rw_speed.total_read_time_usec + u32_per_read_time) > 0) {
                        emmc_rw_speed.total_read_bytes += pData_st->blocks*eMMC_SECTOR_512BYTE;
                        emmc_rw_speed.total_read_time_usec += u32_per_read_time;
                    }
                }
                else if((pCmd_st->opcode==24)||(pCmd_st->opcode==25)) {
                    u32_per_write_time = (U32)(ktime_to_ns(ktime_sub(ktime_get(), starttime))/1000);
                    if ((emmc_rw_speed.total_write_bytes + pData_st->blocks*eMMC_SECTOR_512BYTE) > 0 &&
                        (emmc_rw_speed.total_write_time_usec + u32_per_write_time) > 0) {
                        emmc_rw_speed.total_write_bytes += pData_st->blocks*eMMC_SECTOR_512BYTE;
                        emmc_rw_speed.total_write_time_usec += u32_per_write_time;
                    }
                }
            }

            #if !(defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM)
            if( pCmd_st->opcode == 8 )
            {
                mstar_mci_config_ecsd(pData_st, pMStarHost_st->mmc);
            }
            #endif

            u8_retry_data =0;
            eMMC_UnlockFCIE((U8*)__FUNCTION__);
            mmc_request_done(pMStarHost_st->mmc, pMStarHost_st->request);
        }

        #endif
    }
    else
    {
        if( (mmc_resp_type(pCmd_st) & MMC_RSP_R1B) == MMC_RSP_R1B )
        {
            #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
            if (pMStarHost_st->cmd->cmd_timeout_ms)
                err = mstar_mci_WaitD0High((u32)(pMStarHost_st->cmd->cmd_timeout_ms*1000));
            #else
            if (pMStarHost_st->cmd->busy_timeout)
                err = mstar_mci_WaitD0High((u32)(pMStarHost_st->cmd->busy_timeout*1000));
            #endif
            else
                err = mstar_mci_WaitD0High(TIME_WAIT_DAT0_HIGH);

            while(err != eMMC_ST_SUCCESS)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: retry wait D0 H.\n");

                u8_retry_D0H++;

                if(u8_retry_D0H < 10)
                {
                    err = mstar_mci_WaitD0High(TIME_WAIT_DAT0_HIGH);
                }
                else
                {
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: wait D0 H timeout\n");

                    eMMC_DumpDriverStatus();
                    eMMC_DumpPadClk();
                    eMMC_FCIE_DumpRegisters();
                    eMMC_FCIE_DumpDebugBus();
                    eMMC_Dump_eMMCStatus();

                    mstar_mci_completed_command(pMStarHost_st);

                    pCmd_st->error = -ETIMEDOUT;

                    eMMC_UnlockFCIE((U8*)__FUNCTION__);

                    mmc_request_done(pMStarHost_st->mmc, pMStarHost_st->request);

                    return;
                }
            }
        }

        mstar_mci_completed_command(pMStarHost_st); // copy back rsp for cmd without data

        if( pCmd_st->opcode == 23 && pMStarHost_st->request->sbc)
            mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->cmd); // CMD18 or CMD25
        else
        {
            if(MCI_RETRY_CNT_OK_CLK_UP == u32_ok_cnt++)
            {
                //eMMC_debug(0,1,"eMMC: restore IF\n");
                eMMC_FCIE_ErrHandler_RestoreClk();
            }
            if( pCmd_st->opcode != 12 )
            {
                if(pCmd_st->opcode ==6)
                {
                    if((pCmd_st->arg & 0xff0000) == 0xB30000)
                    {
                        switch((pCmd_st->arg>>24)&3)
                        {
                            case eMMC_ExtCSD_SetBit:
                            g_eMMCDrv.u8_partition_config |=  (pCmd_st->arg&0xFF00)>>8;
                            break;
                            case eMMC_ExtCSD_ClrBit:
                            g_eMMCDrv.u8_partition_config &= ~((pCmd_st->arg&0xFF00)>>8);
                            break;
                            case eMMC_ExtCSD_WByte:
                            g_eMMCDrv.u8_partition_config = (pCmd_st->arg&0xFF00)>>8;
                            break;
                        }
                    }
                    else if((pCmd_st->arg & 0xff0000) == 0x210000)
                    {
                        switch((pCmd_st->arg>>24)&3)
                        {
                            case eMMC_ExtCSD_SetBit:
                            g_eMMCDrv.u8_cache_ctrl |=  (pCmd_st->arg&0xFF00)>>8;
                            break;
                            case eMMC_ExtCSD_ClrBit:
                            g_eMMCDrv.u8_cache_ctrl &= ~((pCmd_st->arg&0xFF00)>>8);
                            break;
                            case eMMC_ExtCSD_WByte:
                            g_eMMCDrv.u8_cache_ctrl = (pCmd_st->arg&0xFF00)>>8;
                            break;
                        }
                    }
                }

                eMMC_UnlockFCIE((U8*)__FUNCTION__);

                mmc_request_done(pMStarHost_st->mmc, pMStarHost_st->request);
            }
        }
    }
}

#if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
static int mstar_mci_pre_dma_transfer(struct mstar_mci_host *host, struct mmc_data *data, struct mstar_mci_host_next *next)
{
    int dma_len;

    /* Check if next job is already prepared */
    if (next || (!next && data->host_cookie == 0))
    {
        dma_len = dma_map_sg(mmc_dev(host->mmc),
                             data->sg,
                             data->sg_len,
                             mstar_mci_get_dma_dir(data));
    }
    else
    {
        dma_len = host->next_data.dma_len;
        host->next_data.dma_len = 0;
    }

    if (dma_len == 0)
        return -EINVAL;

    if (next)
    {
        next->dma_len = dma_len;
        if(++next->cookie < 0)
            next->cookie = 1;
        data->host_cookie = next->cookie;
    }

    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,53)
static void mstar_mci_pre_req(struct mmc_host *mmc, struct mmc_request *mrq)
#else
static void mstar_mci_pre_req(struct mmc_host *mmc, struct mmc_request *mrq, bool is_first_req)
#endif
{
    struct mstar_mci_host *host = mmc_priv(mmc);

    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "\n");

    if( mrq->data->host_cookie )
    {
        mrq->data->host_cookie = 0;
        return;
    }

    if (mstar_mci_pre_dma_transfer(host, mrq->data, &host->next_data))
        mrq->data->host_cookie = 0;
}

static void mstar_mci_post_req(struct mmc_host *mmc, struct mmc_request *mrq, int err)
{
    #if !(defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM)
    struct mstar_mci_host *host = mmc_priv(mmc);
    #endif
    struct mmc_data *data = mrq->data;

    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "\n");

    if (data->host_cookie)
    {
        #if !(defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM)
        dma_unmap_sg(mmc_dev(host->mmc),
                     data->sg,
                     data->sg_len,
                     mstar_mci_get_dma_dir(data));
        #endif
    }

    data->host_cookie = 0;
}
#endif

static void mstar_mci_request(struct mmc_host *pMMCHost_st, struct mmc_request *pMRQ_st)
{
    struct mstar_mci_host *pMStarHost_st;

    pMStarHost_st = mmc_priv(pMMCHost_st);
    if (!pMMCHost_st)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pMMCHost_st is NULL \n");
        return;
    }

    if (!pMRQ_st)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pMRQ_st is NULL \n");
        return;
    }

    pMStarHost_st->request = pMRQ_st;

    // SD command filter
    if( (pMStarHost_st->request->cmd->opcode == 52) ||
        (pMStarHost_st->request->cmd->opcode == 55) ||
        ((pMStarHost_st->request->cmd->opcode == 8) && pMStarHost_st->request->cmd->arg) ||
        ((pMStarHost_st->request->cmd->opcode == 5) && (pMStarHost_st->request->cmd->arg == 0)) )
    {
        pMStarHost_st->request->cmd->error = -ETIMEDOUT;

        mmc_request_done(pMStarHost_st->mmc, pMStarHost_st->request);

        return;
    }

    eMMC_LockFCIE((U8*)__FUNCTION__);

    // ---------------------------------------------
    #if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
    if(0 == (g_eMMCDrv.u32_DrvFlag & DRV_FLAG_INIT_DONE))
    {
        eMMC_Init_Device();
    }

    #if 0 //def CONFIG_MP_EMMC_TRIM
    switch(pMStarHost_st->request->cmd->opcode)
    {
        case 35:
            eMMC_CMD35_CMD36(pMStarHost_st->request->cmd->arg, 35);
            break;
        case 36:
            eMMC_CMD35_CMD36(pMStarHost_st->request->cmd->arg, 36);
            break;
        case 38:
            eMMC_CMD38();
            break;
        default:
            break;
    }
    #endif

    if(NULL == pMStarHost_st->request->cmd->data && 6 != pMStarHost_st->request->cmd->opcode)
    {
        pMStarHost_st->cmd = pMStarHost_st->request->cmd;
        mstar_mci_completed_command_FromRAM(pMStarHost_st);
        return;
    }

    #if 0
    if(6 == pMStarHost_st->request->cmd->opcode &&
       (((pMStarHost_st->request->cmd->arg & 0xff0000) == 0xB70000)||
        ((pMStarHost_st->request->cmd->arg & 0xff0000) == 0xB90000)))
    {
        pMStarHost_st->cmd = pMStarHost_st->request->cmd;
        mstar_mci_completed_command_FromRAM(pMStarHost_st);
        return;
    }
    #endif

    #endif

    // ---------------------------------------------
    #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
    if( pMStarHost_st->request->sbc)
        mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->sbc);
    else
    #endif
        mstar_mci_send_command(pMStarHost_st, pMStarHost_st->request->cmd);
}


#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
static void mstar_mci_completed_command_FromRAM(struct mstar_mci_host *pMStarHost_st)
{
    struct mmc_command *pCmd_st = pMStarHost_st->cmd;
    u16 au16_cifc[32] = {0};
    u16 u16_i;
    u8 *pTemp;

    #if 0
    eMMC_debug(0,1,"\n");
    eMMC_debug(0,1,"cmd:%u, arg:%Xh\n", pCmd_st->opcode, pCmd_st->arg);
    #endif

    if(eMMC_ST_SUCCESS != eMMC_ReturnRsp((u8*)au16_cifc, (u8)pCmd_st->opcode))
    {
        if( mmc_resp_type(pCmd_st) != MMC_RSP_NONE )
        {
            pCmd_st->error = -ETIMEDOUT;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Info: no rsp\n");
        }
    }
    else
    {
        pCmd_st->error = 0;
        pTemp = (u8*)&(pCmd_st->resp[0]);
        for(u16_i=0; u16_i < 15; u16_i++)
        {
            pTemp[(3-(u16_i%4)) + 4*(u16_i/4)] =
                (u8)(au16_cifc[(u16_i+1)/2] >> 8*((u16_i+1)%2));
        }
        #if 0
        eMMC_debug(0,1,"------------------\n");
        eMMC_dump_mem((u8*)&(pCmd_st->resp[0]), 0x10);
        eMMC_debug(0,1,"------------------\n");
        #endif
    }

    eMMC_UnlockFCIE((U8*)__FUNCTION__);

    mmc_request_done(pMStarHost_st->mmc, pMStarHost_st->request);
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
static u8 u8_cur_timing = 0;

#endif

#if 0

#if (defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400) || \
    (defined(ENABLE_eMMC_HS200) && ENABLE_eMMC_HS200)
static u32 mstar_mci_read_blocks(struct mmc_host *host, u8 *buf, ulong blkaddr, ulong blkcnt)
{
	struct mmc_request mrq = {NULL};
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	struct scatterlist sg;

	mrq.cmd = &cmd;
	mrq.data = &data;

	cmd.opcode = MMC_READ_SINGLE_BLOCK;
	cmd.arg = blkaddr;

	cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;

	data.blksz = 512;
	data.blocks = blkcnt;
	data.flags = MMC_DATA_READ;
	data.sg = &sg;
	data.sg_len = 1;

	sg_init_one(&sg, buf, 512*blkcnt);

	data.timeout_ns = TIME_WAIT_1_BLK_END * 1000;
	data.timeout_clks = 0;

	mmc_wait_for_req(host, &mrq);

	if (cmd.error)
		return cmd.error;

	if (data.error)
		return data.error;

	return 0;
}
#endif

#endif

static void mstar_mci_mmc_hw_reset(struct mmc_host *pMMCHost_st)
{
	eMMC_LockFCIE((U8*)__FUNCTION__);

	eMMC_RST_L();  eMMC_hw_timer_sleep(1);
	eMMC_RST_H();  eMMC_hw_timer_sleep(1);

	eMMC_FCIE_Init();

	eMMC_UnlockFCIE((U8*)__FUNCTION__);
}

static int mstar_mci_card_busy(struct mmc_host *pMMCHost_st)
{
	U16 u16_read0, u16_read1;

	REG_FCIE_R(FCIE_SD_STATUS, u16_read0);
	eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
	REG_FCIE_R(FCIE_SD_STATUS, u16_read1);

    if((u16_read0&BIT_SD_CARD_BUSY) ==0 && (u16_read1&BIT_SD_CARD_BUSY) ==0)
		return 0;
	else
		return 1;
}

static void mstar_mci_set_ios(struct mmc_host *pMMCHost_st, struct mmc_ios *pIOS_st)
{
    /* Define Local Variables */
    struct mstar_mci_host *pMStarHost_st;
    static u8 u8_IfLock=0;
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
    u8 u8_pad_type = 0;
    char *s8_timing_str = 0;
    #endif

    if(0 == (REG_FCIE(FCIE_MIE_FUNC_CTL) & BIT_EMMC_ACTIVE))
    {
        eMMC_LockFCIE((U8*)__FUNCTION__);
        u8_IfLock = 1;
        //eMMC_debug(0,1,"lock\n");
    }

    pMStarHost_st = mmc_priv(pMMCHost_st);
    if (!pMMCHost_st)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pMMCHost_st is NULL \n");
        goto LABEL_END;
    }

    if (!pIOS_st)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pIOS_st is NULL \n");
        goto LABEL_END;
    }

    //eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "eMMC: clock: %u, bus_width %Xh \n",
    //	pIOS_st->clock, pIOS_st->bus_width);

    // ----------------------------------
    if (pIOS_st->clock == 0)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_HIGH, 1, "eMMC Warn: disable clk \n");
        eMMC_clock_gating();
    }
    // ----------------------------------
    #if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
    else
    {
        eMMC_clock_setting(g_eMMCDrv.u16_ClkRegVal);
    }
    #else
    // ----------------------------------
    else
    {
        if( pIOS_st->bus_width == MMC_BUS_WIDTH_8 )
        {
            g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_8;
            g_eMMCDrv.u16_Reg10_Mode =
                (g_eMMCDrv.u16_Reg10_Mode & ~BIT_SD_DATA_WIDTH_MASK) | BIT_SD_DATA_WIDTH_8;
        }
        else if( pIOS_st->bus_width == MMC_BUS_WIDTH_4 )
        {
            g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_4;
            g_eMMCDrv.u16_Reg10_Mode =
                (g_eMMCDrv.u16_Reg10_Mode & ~BIT_SD_DATA_WIDTH_MASK) | BIT_SD_DATA_WIDTH_4;
        }
        else if( pIOS_st->bus_width == MMC_BUS_WIDTH_1 )
        {
            g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_1;
            g_eMMCDrv.u16_Reg10_Mode =
                (g_eMMCDrv.u16_Reg10_Mode & ~BIT_SD_DATA_WIDTH_MASK);
        }

        #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)

        if( u8_cur_timing != pIOS_st->timing )
        {
            if( pIOS_st->timing == MMC_TIMING_LEGACY )
            {
                eMMC_pads_switch(FCIE_eMMC_BYPASS);

                if( pIOS_st->clock > CLK_400KHz )
                    eMMC_clock_setting(FCIE_SLOW_CLK);
                else
                    eMMC_clock_setting(FCIE_SLOWEST_CLK);

                u8_cur_timing = pIOS_st->timing;
            }
            else if( pIOS_st->timing == MMC_TIMING_MMC_HS )
            {
                eMMC_pads_switch(FCIE_eMMC_SDR);
                eMMC_clock_setting(FCIE_DEFAULT_CLK);

                u8_cur_timing = pIOS_st->timing;
            }
            else if( (pIOS_st->timing == MMC_TIMING_UHS_DDR50) ||
                     (pIOS_st->timing == MMC_TIMING_MMC_DDR52) )
            {
                eMMC_pads_switch(FCIE_eMMC_DDR);
                if(g_eMMCDrv.TimingTable_t.u8_SetCnt)
                    eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);

				u8_cur_timing = pIOS_st->timing;
				eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: DDR %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);             
            }
            else
            {
                #if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400
                if( pIOS_st->timing == MMC_TIMING_MMC_HS200 && 
					g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_HS400_1_8V &&
					((pMMCHost_st->caps2 & MMC_CAP2_HS400_ES) || (pMMCHost_st->caps2 & MMC_CAP2_HS400_1_8V)))
                {
                    u8_cur_timing = pIOS_st->timing;
                    goto LABEL_END;
                }
                #endif

                switch( pIOS_st->timing )
                {
                    case MMC_TIMING_MMC_HS200:
                        u8_pad_type = FCIE_eMMC_HS200;
                        s8_timing_str = "HS200";
                        break;
                    case MMC_TIMING_MMC_HS400:
						#if defined(ENABLE_eMMC_HS400_5_1) && ENABLE_eMMC_HS400_5_1
                        if (g_eMMCDrv.u8_ECSD184_Stroe_Support && pMMCHost_st->caps2 & MMC_CAP2_HS400_ES){
							u8_pad_type = FCIE_eMMC_HS400_5_1;
							s8_timing_str = "HS400 5.1";
                        }
						else {
							u8_pad_type = FCIE_eMMC_HS400;
						    s8_timing_str = "HS400";
						}
						#else
                        u8_pad_type = FCIE_eMMC_HS400;
						s8_timing_str = "HS400";
						#endif
                        break;
                }

                eMMC_pads_switch(u8_pad_type);
                eMMC_clock_setting(FCIE_DEFAULT_CLK);
                #if (defined(ENABLE_eMMC_HS400_5_1) && ENABLE_eMMC_HS400_5_1) ||\
                    (defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400)
                if(g_eMMCDrv.TimingTable_G_t.u8_SetCnt && 
	                (u8_pad_type == FCIE_eMMC_HS400 || u8_pad_type == FCIE_eMMC_HS400_5_1))
	            eMMC_FCIE_ApplyTimingSet(g_eMMCDrv.TimingTable_G_t.u8_CurSetIdx);
	            else
                #endif
                if(g_eMMCDrv.TimingTable_t.u8_SetCnt)
                    eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);

                u8_cur_timing = pIOS_st->timing;
                eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: %s %uMHz \n", s8_timing_str, g_eMMCDrv.u32_ClkKHz/1000);
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0,
                "\n Irwin: Reg(0x123F1A):%04Xh, Reg(0x123F45):%04Xh, Reg(0x123F46):%04Xh\n",
                REG_FCIE(EMMC_PLL_BASE+0x1A*4), REG_FCIE(EMMC_PLL_BASE+0x45*4), REG_FCIE(EMMC_PLL_BASE+0x46*4));            
            }
        }

        #else
        if(pIOS_st->clock > CLK_400KHz)
        {
            switch( pIOS_st->timing )
            {
                case MMC_TIMING_LEGACY:
                     eMMC_pads_switch(FCIE_eMMC_BYPASS);
                     eMMC_clock_setting(FCIE_SLOW_CLK);
                     break;
                case MMC_TIMING_UHS_DDR50:
                    if(u8_IfLock==0)
                        eMMC_LockFCIE((U8*)__FUNCTION__);
                    eMMC_FCIE_EnableSDRMode();
                    if(g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_DDR)
                    {
                        if(eMMC_ST_SUCCESS != eMMC_FCIE_EnableFastMode(FCIE_eMMC_DDR))
                        {
                            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
                                 "eMMC Err: eMMC_FCIE_EnableFastMode DDR fail\n");
                            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\neMMC: SDR %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
                        }
                        else
                            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\neMMC: DDR %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
                    }
                    if(u8_IfLock==0)
                        eMMC_UnlockFCIE((U8*)__FUNCTION__);
                    break;
                case MMC_TIMING_MMC_HS200:
                    eMMC_pads_switch(FCIE_eMMC_HS200);
                    eMMC_clock_setting(FCIE_DEFAULT_CLK);
                    break;
                case MMC_TIMING_MMC_HS400:
                    eMMC_pads_switch(FCIE_eMMC_HS400);
                    eMMC_clock_setting(FCIE_DEFAULT_CLK);
                    break;
                //case MMC_TIMING_MMC_HS:
                default:
                    eMMC_pads_switch(FCIE_eMMC_SDR);
                    eMMC_clock_setting(FCIE_DEFAULT_CLK);
                    break;
            }
        }
        else
        {
            eMMC_pads_switch(FCIE_eMMC_BYPASS);
            eMMC_clock_setting(FCIE_SLOWEST_CLK);
        }
        #endif
    }
    #endif

    LABEL_END:

    if(u8_IfLock)
    {
        u8_IfLock = 0;
        eMMC_UnlockFCIE((U8*)__FUNCTION__);
        //eMMC_debug(0,1,"unlock\n");
    }
}

static int mstar_mci_execute_tuning(struct mmc_host *host, u32 opcode)
{
    u32 u32_err = 0;

    #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)

    eMMC_LockFCIE((U8*)__FUNCTION__);
    eMMC_pads_switch(FCIE_eMMC_SDR);
    eMMC_clock_setting(FCIE_DEFAULT_CLK);

    #if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400
    #if defined(CONFIG_MSTAR_MONACO) || defined(CONFIG_MSTAR_CLIPPERS)
    if( (REG_FCIE(reg_chip_version)>>8) >= 0x01 )
    {
    #endif

        if((g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_HS400_1_8V) && (host->caps2 & MMC_CAP2_HS400_1_8V_DDR) )
        {
            if(eMMC_ST_SUCCESS != eMMC_FCIE_EnableFastMode(FCIE_eMMC_HS400))
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
                       "eMMC Err: eMMC_FCIE_EnableFastMode HS400 fail\n");
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\neMMC: SDR %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
            }
            else
                eMMC_debug(eMMC_DEBUG_LEVEL,0,"\neMMC: HS400 %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
            eMMC_UnlockFCIE((U8*)__FUNCTION__);
            return u32_err;
        }

    #if defined(CONFIG_MSTAR_MONACO) || defined(CONFIG_MSTAR_CLIPPERS)
    }
    #endif
    #endif     //defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400

    #if defined(ENABLE_eMMC_HS200) && ENABLE_eMMC_HS200
    if((g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_HS200_1_8V) && (host->caps2 & MMC_CAP2_HS200_1_8V_SDR))
    {
        if(eMMC_ST_SUCCESS != eMMC_FCIE_EnableFastMode(FCIE_eMMC_HS200))
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
                   "eMMC Err: eMMC_FCIE_EnableFastMode HS200 fail\n");
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\neMMC: SDR %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
        }
        else
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\neMMC: HS200 %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
        eMMC_UnlockFCIE((U8*)__FUNCTION__);
        return u32_err;
    }
    #endif

    eMMC_UnlockFCIE((U8*)__FUNCTION__);

    #endif

    return u32_err;
}

//=======================================================================
static void mstar_mci_enable(void)
{
    u32 u32_err;

    memset((void*)&g_eMMCDrv, '\0', sizeof(eMMC_DRIVER));
	g_eMMCDrv.u8_partition_config = 8;
	//reset retry status to default
	sgu8_IfNeedRestorePadType=0xFF;
	u8_sdr_retry_count = 0;

    eMMC_PlatformInit();
	if(eMMC_ST_SUCCESS != eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH))
	{
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: WaitD0High TO\n");
	    eMMC_FCIE_ErrHandler_Stop();
	}
    eMMC_RST_L();  eMMC_hw_timer_sleep(1);
    eMMC_RST_H();  eMMC_hw_timer_sleep(1);

    g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_1;
    g_eMMCDrv.u16_Reg10_Mode = BIT_SD_DEFAULT_MODE_REG;
    g_eMMCDrv.u16_RCA=1;

    u32_err = eMMC_FCIE_Init();
    if(eMMC_ST_SUCCESS != u32_err)
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1, "eMMC Err: eMMC_FCIE_Init fail: %Xh \n", u32_err);
}

static void mstar_mci_disable(void)
{
    u32 u32_err;

    eMMC_debug(eMMC_DEBUG_LEVEL,1,"\n");

    u32_err = eMMC_FCIE_Reset();
    if(eMMC_ST_SUCCESS != u32_err)
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1, "eMMC Err: eMMC_FCIE_Reset fail: %Xh\n", u32_err);

    eMMC_clock_gating();
}

#if 0
static ssize_t fcie_pwrsvr_gpio_trigger_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", gu32_pwrsvr_gpio_trigger);
}

static ssize_t fcie_pwrsvr_gpio_trigger_store(struct device *dev, struct device_attribute *attr,
                                              const char *buf, size_t count)
{
    unsigned long u32_pwrsvr_gpio_trigger = 0;

    if(kstrtoul(buf, 0, &u32_pwrsvr_gpio_trigger))
        return -EINVAL;

    if(u32_pwrsvr_gpio_trigger > 1)
        return -EINVAL;

    gu32_pwrsvr_gpio_trigger = u32_pwrsvr_gpio_trigger;

    return count;
}

DEVICE_ATTR(fcie_pwrsvr_gpio_trigger,
            S_IRUSR | S_IWUSR,
            fcie_pwrsvr_gpio_trigger_show,
            fcie_pwrsvr_gpio_trigger_store);

static ssize_t fcie_pwrsvr_gpio_bit_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", gu32_pwrsvr_gpio_bit);
}

static ssize_t fcie_pwrsvr_gpio_bit_store(struct device *dev, struct device_attribute *attr,
                                          const char *buf, size_t count)
{
    unsigned long u32_pwrsvr_gpio_bit = 0;

    if(kstrtoul(buf, 0, &u32_pwrsvr_gpio_bit))
        return -EINVAL;

    if(u32_pwrsvr_gpio_bit)
    {
        if(u32_pwrsvr_gpio_bit > 0xF)
            return -EINVAL;
        gu32_pwrsvr_gpio_bit = u32_pwrsvr_gpio_bit;
    }

    return count;
}

DEVICE_ATTR(fcie_pwrsvr_gpio_bit,
            S_IRUSR | S_IWUSR,
            fcie_pwrsvr_gpio_bit_show,
            fcie_pwrsvr_gpio_bit_store);

static ssize_t fcie_pwrsvr_gpio_addr_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "0x%X\n", gu32_pwrsvr_gpio_addr);
}

static ssize_t fcie_pwrsvr_gpio_addr_store(struct device *dev, struct device_attribute *attr,
                                           const char *buf, size_t count)
{
    unsigned long u32_pwrsvr_gpio_addr = 0;

    if(kstrtoul(buf, 0, &u32_pwrsvr_gpio_addr))
        return -EINVAL;

    if(u32_pwrsvr_gpio_addr)
        gu32_pwrsvr_gpio_addr = u32_pwrsvr_gpio_addr;

    return count;
}

DEVICE_ATTR(fcie_pwrsvr_gpio_addr,
            S_IRUSR | S_IWUSR,
            fcie_pwrsvr_gpio_addr_show,
            fcie_pwrsvr_gpio_addr_store);

static ssize_t fcie_pwrsvr_gpio_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", gu32_pwrsvr_gpio_enable);
}

static ssize_t fcie_pwrsvr_gpio_enable_store(struct device *dev, struct device_attribute *attr,
                                             const char *buf, size_t count)
{
    unsigned long u32_pwrsvr_gpio_enable = 0;

    #if 0
    if(u8_enable_sar5)
        return count;
    #endif

    if(kstrtoul(buf, 0, &u32_pwrsvr_gpio_enable))
        return -EINVAL;

    if(u32_pwrsvr_gpio_enable)
        gu32_pwrsvr_gpio_enable = u32_pwrsvr_gpio_enable;

    return count;
}

DEVICE_ATTR(fcie_pwrsvr_gpio_enable,
            S_IRUSR | S_IWUSR,
            fcie_pwrsvr_gpio_enable_show,
            fcie_pwrsvr_gpio_enable_store);


static ssize_t emmc_sanitize_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", gu32_emmc_sanitize);
}

static ssize_t emmc_sanitize_store(struct device *dev, struct device_attribute *attr,
                                   const char *buf, size_t count)
{
    unsigned long u32_temp = 0;

    if(kstrtoul(buf, 0, &u32_temp))
        return -EINVAL;

    gu32_emmc_sanitize = u32_temp;
    //printk("%Xh\n", gu32_emmc_sanitize);

    if(gu32_emmc_sanitize)
    {
        eMMC_LockFCIE((U8*)__FUNCTION__);
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: santizing ...\n");
        eMMC_Sanitize(0xAA);
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: done\n");
        eMMC_UnlockFCIE((U8*)__FUNCTION__);
    }

    return count;
}

DEVICE_ATTR(emmc_sanitize,
            S_IRUSR | S_IWUSR,
            emmc_sanitize_show,
            emmc_sanitize_store);
#endif

static ssize_t eMMC_monitor_count_enable_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    U32 u32_speed_read=0, u32_speed_write=0;
    U32 u32_multi, u32_total_read_GB, u32_total_write_GB, u32_total_read_MB, u32_total_write_MB;

    u32_multi = emmc_rw_speed.total_read_bytes/(emmc_rw_speed.total_read_time_usec/1000);
    u32_speed_read = u32_multi*1000/(1024*1024);//MB/sec

    u32_multi = emmc_rw_speed.total_write_bytes/(emmc_rw_speed.total_write_time_usec/1000);
    u32_speed_write = u32_multi*1000/(1024*1024);//MB/sec

    u32_total_read_GB = emmc_rw_speed.total_read_bytes/(1024*1024*1024);
    u32_total_read_MB = (emmc_rw_speed.total_read_bytes%(1024*1024*1024))/(1024*1024);

    u32_total_write_GB = emmc_rw_speed.total_write_bytes/(1024*1024*1024);
    u32_total_write_MB = (emmc_rw_speed.total_write_bytes%(1024*1024*1024))/(1024*1024);


    return scnprintf(buf, PAGE_SIZE, "eMMC: read total %dGB %dMB, %uMB/sec\n"
        "eMMC: write total %dGB %dMB, %uMB/sec\n""%xh\n",
        u32_total_read_GB, u32_total_read_MB, u32_speed_read,
        u32_total_write_GB, u32_total_write_MB, u32_speed_write, gu32_eMMC_monitor_enable);
}



static ssize_t eMMC_monitor_count_enable_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned long u32_temp = 0;


    if(kstrtoul(buf, 0, &u32_temp))
        return -EINVAL;

    gu32_eMMC_monitor_enable = u32_temp;

    if (gu32_eMMC_monitor_enable == 0)
        memset(&emmc_rw_speed, 0, sizeof(struct mstar_rw_speed));

    return count;
}

DEVICE_ATTR(eMMC_monitor_count_enable,
            S_IRUSR | S_IWUSR,
            eMMC_monitor_count_enable_show,
            eMMC_monitor_count_enable_store);

static ssize_t emmc_write_log_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", gu32_eMMC_write_log_enable);
}

static ssize_t eMMC_write_log_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long u32_temp = 0;

	if(kstrtoul(buf, 0, &u32_temp))
		return -EINVAL;

	gu32_eMMC_write_log_enable = u32_temp;

	if(gu32_eMMC_write_log_enable)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: write log enable\n");
	}

	return count;
}

DEVICE_ATTR(eMMC_write_log_enable,
            S_IRUSR | S_IWUSR,
            emmc_write_log_show,
            eMMC_write_log_store);

static ssize_t emmc_read_log_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", gu32_eMMC_read_log_enable);
}

static ssize_t eMMC_read_log_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long u32_temp = 0;

	if(kstrtoul(buf, 0, &u32_temp))
		return -EINVAL;

	gu32_eMMC_read_log_enable = u32_temp;

	if(gu32_eMMC_read_log_enable)
	{
		eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: read log enable\n");
	}

	return count;
}

DEVICE_ATTR(eMMC_read_log_enable,
            S_IRUSR | S_IWUSR,
            emmc_read_log_show,
            eMMC_read_log_store);

static ssize_t emmc_sar5_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "SAR5: %u\n", u8_enable_sar5);
}

DEVICE_ATTR(eMMC_sar5_status,
            S_IRUSR,
            emmc_sar5_status_show,
            NULL);
static struct attribute *mstar_mci_attr[] =
{
    #if 0
    &dev_attr_fcie_pwrsvr_gpio_enable.attr,
    &dev_attr_fcie_pwrsvr_gpio_addr.attr,
    &dev_attr_fcie_pwrsvr_gpio_bit.attr,
    &dev_attr_fcie_pwrsvr_gpio_trigger.attr,
    &dev_attr_emmc_sanitize.attr,
    #endif
    &dev_attr_eMMC_read_log_enable.attr,
    &dev_attr_eMMC_write_log_enable.attr,
    &dev_attr_eMMC_monitor_count_enable.attr,
	&dev_attr_eMMC_sar5_status.attr,
    NULL,
};

static struct attribute_group mstar_mci_attr_grp =
{
    .attrs = mstar_mci_attr,
};

static const struct mmc_host_ops sg_mstar_mci_ops =
{
    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    .pre_req        = mstar_mci_pre_req,
    .post_req       = mstar_mci_post_req,
    #endif
    .request        = mstar_mci_request,
    .set_ios        = mstar_mci_set_ios,
    .execute_tuning = mstar_mci_execute_tuning,
	.card_busy      = mstar_mci_card_busy,
	.hw_reset       = mstar_mci_mmc_hw_reset,
};

#ifdef CONFIG_MP_MSTAR_STR_OF_ORDER
static struct str_waitfor_dev waitfor;
static s32 mstar_mci_suspend(struct platform_device *pDev_st, pm_message_t state);
static s32 mstar_mci_resume(struct platform_device *pDev_st);

static s32 mstar_mci_pm_suspend(struct device *dev)
{
    struct platform_device *pdev = to_platform_device(dev);

    if (WARN_ON(!pdev))
        return -ENODEV;

    if (waitfor.stage1_s_wait)
        wait_for_completion(&(waitfor.stage1_s_wait->power.completion));

    return mstar_mci_suspend(pdev, dev->power.power_state);
}

static s32 mstar_mci_pm_resume(struct device *dev)
{
    struct platform_device *pdev = to_platform_device(dev);

    if (WARN_ON(!pdev))
        return -ENODEV;

    if (waitfor.stage1_r_wait)
        wait_for_completion(&(waitfor.stage1_r_wait->power.completion));

    return mstar_mci_resume(pdev);
}

static struct dev_pm_ops mstar_mci_pm_ops;
#endif

static s32 mstar_mci_probe(struct platform_device *pDev_st)
{
    /* Define Local Variables */
    struct mmc_host *pMMCHost_st            = 0;
    struct mstar_mci_host *pMStarHost_st    = 0;
    s32 s32_ret                             = 0;
	u8  u8_dts_config                       = 0;
#ifdef CONFIG_OF
	struct device_node *np;
	int irq;
#endif

    #if IF_FCIE_SHARE_IP
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: has SD 0723\n");
    #else
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: no SD 0723\n");
    #endif

    memset(gAdmaDesc_st, 0, sizeof(struct _AdmaDescriptor)*FCIE_ADMA_DESC_COUNT);

    #ifndef CONFIG_MSTAR_ARM_BD_FPGA//#if 1//defined(CONFIG_MMC_MSTAR_MMC_EMMC_CHK_VERSION)
    if(eMMC_DRIVER_VERSION != REG_FCIE(FCIE_RESERVED_FOR_SW))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n eMMC: please update MBoot.\n");
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n MBoot ver:%u, Kernel ver: %u\n",
            REG_FCIE(FCIE_RESERVED_FOR_SW), eMMC_DRIVER_VERSION);
        while(1);
    }
    #endif
    // --------------------------------
    if (!pDev_st)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: pDev_st is NULL \n");
        s32_ret = -EINVAL;
        goto LABEL_END;
    }

    g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
    g_eMMCDrv.u16_ClkRegVal = FCIE_SLOWEST_CLK;

    // --------------------------------
    eMMC_LockFCIE((U8*)__FUNCTION__);
    mstar_mci_enable();
    eMMC_UnlockFCIE((U8*)__FUNCTION__);

    pMMCHost_st = mmc_alloc_host(sizeof(struct mstar_mci_host), &pDev_st->dev);
    if (!pMMCHost_st)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: mmc_alloc_host fail \n");
        s32_ret = -ENOMEM;
        goto LABEL_END;
    }

    pMMCHost_st->ops            = &sg_mstar_mci_ops;

#ifdef CONFIG_OF
	np = of_find_matching_node(NULL, mstar_emmc_dt_match);
	if (np) {
		pr_info("mstar-mci new setup flow\n");
		u8_dts_config = 1;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)
		mmc_of_parse(pMMCHost_st);
#else
		s32_ret = mmc_of_parse(pMMCHost_st);
		if (s32_ret) {
			u8_dts_config = 0;
			eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,"result of mmc parse is %d\n", s32_ret);
			mmc_free_host(pMMCHost_st);
			goto LABEL_END;
		}
#endif
	} else
		pr_info("mstar-mci legacy setup flow\n");
#endif

    // [FIXME]->
    pMMCHost_st->f_min          = CLK_400KHz;
    pMMCHost_st->f_max          = CLK_200MHz;
    pMMCHost_st->ocr_avail      = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 | \
                                  MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;

    pMMCHost_st->max_blk_count  = BIT_SD_JOB_BLK_CNT_MASK;
    pMMCHost_st->max_blk_size   = 512;
    pMMCHost_st->max_req_size   = pMMCHost_st->max_blk_count * pMMCHost_st->max_blk_size; //could be optimized

    pMMCHost_st->max_seg_size   = pMMCHost_st->max_req_size;
    pMMCHost_st->max_segs       = 32;

    //---------------------------------------
    pMStarHost_st               = mmc_priv(pMMCHost_st);
    pMStarHost_st->mmc          = pMMCHost_st;

    //---------------------------------------
	if(!u8_dts_config)
		pMMCHost_st->caps           = MMC_CAP_8_BIT_DATA | MMC_CAP_4_BIT_DATA | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_NONREMOVABLE;

    #if defined(ENABLE_EMMC_PRE_DEFINED_BLK) && ENABLE_EMMC_PRE_DEFINED_BLK
    pMMCHost_st->caps           |= MMC_CAP_CMD23;
    #endif
	pMMCHost_st->caps           |= MMC_CAP_WAIT_WHILE_BUSY|MMC_CAP_HW_RESET;

    #if (defined(ENABLE_eMMC_ATOP)&&ENABLE_eMMC_ATOP)

    #if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
	if(!u8_dts_config)
		pMMCHost_st->caps2          |= MMC_CAP2_HS400_1_8V;

    #if defined(ENABLE_eMMC_HS400_5_1) && ENABLE_eMMC_HS400_5_1
	if(!u8_dts_config)
		pMMCHost_st->caps2          |= MMC_CAP2_HS400_ES;
    #endif

    #else
	if(!u8_dts_config)
		pMMCHost_st->caps2          |= MMC_CAP2_HS400_1_8V_DDR;
    #endif
    #endif

    #if defined(ENABLE_eMMC_HS200) && ENABLE_eMMC_HS200
	if(!u8_dts_config)
		pMMCHost_st->caps2          |= MMC_CAP2_HS200_1_8V_SDR;
    #endif

    pMMCHost_st->caps           |= MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50;

    #endif

    #if 1 //#ifdef CONFIG_MP_EMMC_TRIM
    pMMCHost_st->caps           |= MMC_CAP_ERASE;
    #if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,53)
    pMMCHost_st->caps2          |= MMC_CAP2_HC_ERASE_SZ;
    #endif
    #endif

    #ifdef CONFIG_MP_EMMC_CACHE
    #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
    pMMCHost_st->caps2          |= MMC_CAP2_CACHE_CTRL;
    #endif
    #endif

    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,1) && LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
    pMMCHost_st->caps2          |= MMC_CAP2_NO_SLEEP_CMD;
    #endif

    #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
    pMMCHost_st->caps2 |= MMC_CAP2_POWEROFF_NOTIFY;
    #endif

    #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
    pMMCHost_st->caps2 |= MMC_CAP2_FULL_PWR_CYCLE;
    #endif

	// <-[FIXME]
	#if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    pMStarHost_st->next_data.cookie = 1;
    INIT_WORK(&pMStarHost_st->async_work, mstar_mci_send_data);
    #endif
	if(u8_dts_config)
		s32_ret = dma_set_mask_and_coherent(&pDev_st->dev, DMA_BIT_MASK(64));

    mmc_add_host(pMMCHost_st);
    platform_set_drvdata(pDev_st, pMMCHost_st);

    pMStarHost_st->lock = &fcie_lock;

    #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
    eMMC_enable_miu_chksum();
    #endif

#if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
#if defined(CONFIG_OF)
	irq = platform_get_irq(pDev_st, 0);
	if (irq < 0) {
		pr_warn("mmc: dt: interrupt node is missing, use mst legacy irq %d\n", E_IRQ_NFIE);
		irq = E_IRQ_NFIE;
	}

	pr_info("mstar mci: irq: %d\n", irq);
	s32_ret = request_irq(irq, eMMC_FCIE_IRQ, IRQF_SHARED, DRIVER_NAME, pMStarHost_st);

	pMStarHost_st->irq = (s32)irq;
#else
	s32_ret = request_irq(E_IRQ_NFIE, eMMC_FCIE_IRQ, IRQF_SHARED, DRIVER_NAME, pMStarHost_st);
#endif
	if (s32_ret) {
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: request_irq fail \n");
		mmc_free_host(pMMCHost_st);
		goto LABEL_END;
	}
#endif

    #if 1
    // For getting and showing device attributes from/to user space.
    s32_ret = sysfs_create_group(&pDev_st->dev.kobj, &mstar_mci_attr_grp);
    #endif

    sgp_eMMCThread_st = kthread_create(mstar_mci_Housekeep, NULL, "eMMC_bg_thread");
    if(IS_ERR(sgp_eMMCThread_st))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: create thread fail \n");
        return PTR_ERR(sgp_eMMCThread_st);
    }
    wake_up_process(sgp_eMMCThread_st);

    #if defined(CONFIG_MMC_MSTAR_MMC_EMMC_LIFETEST)
    g_eMMCDrv.u64_CNT_TotalRBlk = 0;
    g_eMMCDrv.u64_CNT_TotalWBlk = 0;
    writefile = create_proc_entry (procfs_name, 0644, NULL);
    if(writefile == NULL)
        eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Err: Can not initialize /proc/%s\n", procfs_name);
    else
    {
        writefile->read_proc = procfile_read;
        writefile->mode      = S_IFREG | S_IRUGO;
        writefile->uid       = 0;
        writefile->gid       = 0;
        writefile->size      = 0x10;
    }
    #endif

    memset(&emmc_rw_speed, 0, sizeof(struct mstar_rw_speed));

    LABEL_END:

    #ifdef CONFIG_MP_MSTAR_STR_OF_ORDER
    of_mstar_str(DRIVER_NAME, &pDev_st->dev, &mstar_mci_pm_ops, &waitfor,
		&mstar_mci_pm_suspend, &mstar_mci_pm_resume,
		NULL, NULL);
    #endif

    return s32_ret;
}

static s32 __exit mstar_mci_remove(struct platform_device *pDev_st)
{
    /* Define Local Variables */
    struct mmc_host *pMMCHost_st;
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    struct mstar_mci_host *pMStarHost_st;
    #endif
    s32 s32_ret                             = 0;

    eMMC_LockFCIE((U8*)__FUNCTION__);

    if (!pDev_st)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: pDev_st is NULL\n");
        s32_ret = -EINVAL;
        goto LABEL_END;
    }

	pMMCHost_st = platform_get_drvdata(pDev_st);

    if (!pMMCHost_st)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: pMMCHost_st is NULL\n");
        s32_ret= -1;
        goto LABEL_END;
    }

    eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC, remove +\n");

    mmc_remove_host(pMMCHost_st);

    mstar_mci_disable();

#if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
	pMStarHost_st = mmc_priv(pMMCHost_st);
#if defined(CONFIG_OF)
	free_irq((int)pMStarHost_st->irq, pMStarHost_st);
#else
	free_irq(E_IRQ_NFIE, pMStarHost_st);
#endif
#endif

    mmc_free_host(pMMCHost_st);

    platform_set_drvdata(pDev_st, NULL);

    eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC, remove -\n");

    LABEL_END:

    eMMC_UnlockFCIE((U8*)__FUNCTION__);

    if(sgp_eMMCThread_st)
        kthread_stop(sgp_eMMCThread_st);

    return s32_ret;
}

#ifdef CONFIG_PM
static s32 mstar_mci_suspend(struct platform_device *pDev_st, pm_message_t state)
{
    /* Define Local Variables */
    struct mmc_host *pMMCHost_st    = platform_get_drvdata(pDev_st);
    s32 ret                         = 0;

    eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC, suspend + \n");
    #if 1
    if (pMMCHost_st)
    {
        #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
        ret = mmc_suspend_host(pMMCHost_st);
        #endif
    }

    #else
    eMMC_LockFCIE((U8*)__FUNCTION__);

    // wait for D0 high before losing eMMC Vcc
    if(eMMC_ST_SUCCESS != mstar_mci_WaitD0High(TIME_WAIT_DAT0_HIGH))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: wait D0 H TO\n");
        eMMC_FCIE_ErrHandler_Stop();
    }
    eMMC_CMD0(0);  mdelay(10); //msleep(10);
    eMMC_PlatformDeinit();
    eMMC_UnlockFCIE((U8*)__FUNCTION__);

    #endif


    eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC, suspend -, %Xh\n", ret);

    u16_OldPLLClkParam = 0xFFFF;
    u16_OldPLLDLLClkParam = 0xFFFF;
	u8_tune_overtone = 1;


    return ret;
}

static s32 mstar_mci_resume(struct platform_device *pDev_st)
{
    struct mmc_host *pMMCHost_st    = platform_get_drvdata(pDev_st);
    s32 ret                         = 0;
    static u8 u8_IfLock             = 0;
    u32 u32_err;

    if(0 == (REG_FCIE(FCIE_MIE_FUNC_CTL) & BIT_EMMC_ACTIVE))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"lock\n");
        eMMC_LockFCIE((U8*)__FUNCTION__);
        u8_IfLock = 1;
    }

    eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC, resume +\n");


    #if 1

    #if defined(ENABLE_FCIE_MIU_CHECKSUM) && ENABLE_FCIE_MIU_CHECKSUM
    eMMC_enable_miu_chksum();
    #endif

    g_eMMCDrv.u32_DrvFlag = 0;
    g_eMMCDrv.u8_partition_config = 8;
    //reset retry status to default
    sgu8_IfNeedRestorePadType=0xFF;
    u8_sdr_retry_count = 0;

    eMMC_PlatformInit();
    if(eMMC_ST_SUCCESS != eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: WaitD0High TO\n");
        eMMC_FCIE_ErrHandler_Stop();
    }
    eMMC_RST_L();  eMMC_hw_timer_sleep(1);
    eMMC_RST_H();  eMMC_hw_timer_sleep(1);

    g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_1;
    g_eMMCDrv.u16_Reg10_Mode = BIT_SD_DEFAULT_MODE_REG;
    g_eMMCDrv.u16_RCA=1;

    u32_err = eMMC_FCIE_Init();
    if(eMMC_ST_SUCCESS != u32_err)
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1, "eMMC Err: eMMC_FCIE_Init fail: %Xh \n", u32_err);

    #endif
    if(u8_IfLock)
    {
        u8_IfLock = 0;
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"unlock\n");
        eMMC_UnlockFCIE((U8*)__FUNCTION__);
    }

    if (pMMCHost_st)
    {
        #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
        ret = mmc_resume_host(pMMCHost_st);
        #endif
    }

    eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC, resume -\n");

    return ret;
}
#endif  /* End ifdef CONFIG_PM */

/******************************************************************************
 * Define Static Global Variables
 ******************************************************************************/
static struct platform_driver sg_mstar_mci_driver =
{
	.probe = mstar_mci_probe,
	.remove = __exit_p(mstar_mci_remove),

#if defined(CONFIG_PM) && !defined(CONFIG_MP_MSTAR_STR_OF_ORDER)
	.suspend = mstar_mci_suspend,
	.resume = mstar_mci_resume,
#endif

	.driver  =
	{
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_MP_MSTAR_STR_OF_ORDER
		.pm = &mstar_mci_pm_ops,
#endif
#if defined(CONFIG_OF)
		.of_match_table = mstar_emmc_dt_match,
#endif
	},
};

struct platform_device sg_mstar_emmc_device_st =
{
    .name =	DRIVER_NAME,
    .id = 0,
    .resource =	NULL,
    .num_resources = 0,
    .dev.dma_mask = &sg_mstar_emmc_device_st.dev.coherent_dma_mask,
#if defined(CONFIG_CC_IS_CLANG) && defined(CONFIG_MSTAR_CHIP)
    .dev.coherent_dma_mask = ~0ULL,
#else
    .dev.coherent_dma_mask = DMA_BIT_MASK(64),
#endif
};


/******************************************************************************
 * Init & Exit Modules
 ******************************************************************************/
extern struct completion mmc_done;

static s32 __init mstar_mci_init(void)
{
	int err = 0;
	U16 u16_regval = 0;
#ifdef CONFIG_OF
	struct device_node *np;
#endif

	u16_regval = REG_FCIE(FCIE_NC_FUN_CTL);	//fcie reset doesn't reset to default value

    if( (u16_regval & BIT0) == BIT0 )       //if nc_en is set
    {
        #if !defined(CONFIG_MP_PURE_SN_32BIT) && !defined(CONFIG_MSTAR_ARM_BD_FPGA)
        complete(&mmc_done);
        #endif  
        return 0;
    }
	memset(&g_eMMCDrv, 0, sizeof(eMMC_DRIVER));

	eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"\n");

	if ((err = platform_driver_register(&sg_mstar_mci_driver)) < 0)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: platform_driver_register fail, %Xh\n", err);

#ifdef CONFIG_OF
	np = of_find_matching_node(NULL, mstar_emmc_dt_match);
	if (np)
		return err;
#endif
	if((err = platform_device_register(&sg_mstar_emmc_device_st)) < 0)
		eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: platform_device_register fail, %Xh\n", err);


	mci_workqueue =  alloc_workqueue("mstar_mci", WQ_HIGHPRI | WQ_MEM_RECLAIM, 1);

	if (!mci_workqueue) {
		pr_err("mstar_mci: not enough memory to create workqueue\n");
		return -ENOMEM;
	}
	return err;
}

static void __exit mstar_mci_exit(void)
{
	flush_workqueue(mci_workqueue);
	destroy_workqueue(mci_workqueue);
    platform_driver_unregister(&sg_mstar_mci_driver);
}

#if 0
static int __init write_seg_size_setup(char *str)
{
    wr_seg_size =  simple_strtoul(str, NULL, 16);
    return 1;
}

static int __init write_seg_theshold_setup(char *str)
{
    wr_split_threshold =  simple_strtoul(str, NULL, 16);
    return 1;
}
#endif

/* Enable by default, but SAR5=OFF in set_config will disable this feature */
static int __init sar5_setup_for_pwr_cut(char * str)
{
    if(str != NULL)
    {
        printk(KERN_CRIT"SAR5=%s", str);
        if(strcmp((const char *) str, "OFF") == 0)
            u8_enable_sar5 = 0;
    }

    return 0;
}
early_param("SAR5", sar5_setup_for_pwr_cut);

#if 0
__setup("mmc_wrsize=", write_seg_size_setup);
__setup("mmc_wrupsize=", write_seg_theshold_setup);
#endif

subsys_initcall(mstar_mci_init);
module_exit(mstar_mci_exit);

MODULE_LICENSE("Propritery");
MODULE_DESCRIPTION("Mstar Multimedia Card Interface driver");
MODULE_AUTHOR("MStar");
