$(PRODUCT_OUT)/obj/lib_vendor/x86/vendor_btusb.ko:
	cp kernel/prebuilts/common-modules/virtual-device/$(TARGET_KERNEL_USE)/x86-64/btusb.ko $@

$(PRODUCT_OUT)/obj/lib_vendor/x86_64/vendor_btusb.ko:
	cp kernel/prebuilts/common-modules/virtual-device/$(TARGET_KERNEL_USE)/x86-64/btusb.ko $@

$(PRODUCT_OUT)/obj/lib_vendor/arm64/vendor_btusb.ko:
	cp kernel/prebuilts/common-modules/virtual-device/$(TARGET_KERNEL_USE)/arm64/btusb.ko $@
