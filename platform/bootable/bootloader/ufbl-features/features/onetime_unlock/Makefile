#
# Copyright (c) 2016 - 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
#

#
# Common list of objects files we want to include
#
ONETIME_UNLOCK_OBJECTS = \
    onetime_unlock.o \

# Common set of CFLAGS we need
ONETIME_UNLOCK_CFLAGS = \
    -DSUPPORT_UBOOT \
    -std=c99 \
    -ffunction-sections -fdata-sections \
    -DUFBL_FEATURE_ONETIME_UNLOCK \

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT)))
ONETIME_UNLOCK_OBJECTS += \
    onetime_unlock_ltc.o

ONETIME_UNLOCK_CFLAGS += \
    -DUFBL_FEATURE_ONETIME_UNLOCK_LTC \
    -DLTC_SOURCE \
    -DLTC_NO_FILE \
    -DUSE_LTM -DLTM_DESC \
    -D_WCHAR_T -DLTC_CLEAN_STACK

ONETIME_UNLOCK_HEADERS = \
    -I$(UFBL_PATH)/features/libtomcrypt/ \
    -I$(UFBL_PATH)/features/libtomcrypt/src/headers/
endif

ifeq (true,$(strip $(FEATURE_MBEDTLS)))
ONETIME_UNLOCK_OBJECTS += \
    onetime_unlock_common.o \
    onetime_unlock_mbedtls.o

ONETIME_UNLOCK_CFLAGS += \
    -DUFBL_FEATURE_ONETIME_UNLOCK_MBEDTLS \
    -DMBEDTLS_CONFIG_FILE='<config-ufbl.h>'

ONETIME_UNLOCK_HEADERS = \
    -I$(UFBL_PATH)/features/mbedtls-gplv2/src/configs \
    -I$(UFBL_PATH)/features/mbedtls-gplv2/src/include
endif

# Begin rules for kbuild U-Boot
ifeq (yes,$(strip $(UFBL_UBOOT_IS_KBUILD)))

ccflags-y    += $(ONETIME_UNLOCK_CFLAGS) $(ONETIME_UNLOCK_HEADERS)
obj-y        += $(ONETIME_UNLOCK_OBJECTS)

else # default rules

include $(TOPDIR)/config.mk

ONETIME_UNLOCK_LIB = $(obj)libonetime_unlock.o

# Default CFLAGS
CFLAGS += \
    $(ONETIME_UNLOCK_CFLAGS) \
    $(ONETIME_UNLOCK_HEADERS)

COBJS := $(ONETIME_UNLOCK_OBJECTS)
SRCS  := $(COBJS:.o=.c)
OBJS  := $(addprefix $(obj),$(COBJS))

$(ONETIME_UNLOCK_LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

endif # (yes,$(strip $(UFBL_UBOOT_IS_KBUILD)))
