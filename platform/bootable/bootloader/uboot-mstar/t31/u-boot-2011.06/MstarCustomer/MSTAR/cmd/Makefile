#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	=  $(obj)libmscustomer_cmd.a

#AOBJS	=

COBJS	=
#MstarCustomer:cmd
COBJS += ./cmd_CusSystem.o
COBJS += ./cmd_CusUpgrade.o
COBJS += ./cmd_CusUpgradeUtility.o
COBJS += ./cmd_CusPM.o
COBJS += ./cmd_CusLED.o
COBJS += ./cmd_CusUrsa8Init.o
ifeq ($(CONFIG_DISPLAY_PICTURE_UPDATE),y)
COBJS += ./cmd_CusCastTV.o
endif
ifeq ($(CONFIG_MTK_BT_USB),y)
COBJS += ./cmd_CusMtkBT.o
endif

ifeq ($(CONFIG_TCON_POWER_IC_UPGRADE),y)
COBJS += ./cmd_CusTconUpgrade.o
endif

ifeq ($(CONFIG_FRC_UPGRADE),y)
COBJS += ./cmd_CusFrcUpgrade.o
endif

SRCS	:= $(AOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS))

#CPPFLAGS += -I..

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
