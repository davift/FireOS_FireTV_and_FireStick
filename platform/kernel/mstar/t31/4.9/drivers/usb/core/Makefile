#
# Makefile for USB Core files and filesystem
#

# mstar_porting_plat >>>
EXTRA_CFLAGS      += -Idrivers/mstar2/hal/$(CONFIG_MSTAR_CHIP_NAME)/usb

# necessary if defined(DYNAMIC_MIU_SIZE_MAPPING)
EXTRA_CFLAGS      += -Idrivers/mstar2/include/sys/common
EXTRA_CFLAGS      += -Idrivers/mstar2/include
EXTRA_CFLAGS      += -Idrivers/mstar2/drv/miu

# mstar_porting_plat <<<
usbcore-y := usb.o hub.o hcd.o urb.o message.o driver.o
usbcore-y += config.o file.o buffer.o sysfs.o endpoint.o
usbcore-y += devio.o notify.o generic.o quirks.o devices.o
usbcore-y += port.o
# mstar_porting_plat >>>
usbcore-$(CONFIG_MP_USB_MSTAR) += bc-mstar.o mstar-lib.o
# mstar_porting_plat <<<

usbcore-$(CONFIG_OF)		+= of.o
usbcore-$(CONFIG_PCI)		+= hcd-pci.o
usbcore-$(CONFIG_ACPI)		+= usb-acpi.o

obj-$(CONFIG_USB)		+= usbcore.o

obj-$(CONFIG_USB_LEDS_TRIGGER_USBPORT)	+= ledtrig-usbport.o
