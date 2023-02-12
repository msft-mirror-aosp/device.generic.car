#
# Copyright (C) 2022 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# this overwrites Android Emulator's default input devices for virtual displays in device/generic/goldfish/input/
PRODUCT_COPY_FILES += \
    device/generic/car/emulator/multi-display/input/virtio_input_multi_touch_7.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/virtio_input_multi_touch_7.idc \
    device/generic/car/emulator/multi-display/input/virtio_input_multi_touch_8.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/virtio_input_multi_touch_8.idc \
    device/generic/car/emulator/multi-display/input/virtio_input_multi_touch_9.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/virtio_input_multi_touch_9.idc

# Overrides Goldfish's default display settings
PRODUCT_COPY_FILES += \
    device/generic/car/emulator/multi-display/display_layout_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/displayconfig/display_layout_configuration.xml \
    device/generic/car/emulator/multi-display/display_settings.xml:$(TARGET_COPY_OUT_VENDOR)/etc/display_settings.xml

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.managed_users.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.managed_users.xml

PRODUCT_PACKAGE_OVERLAYS += \
    device/generic/car/emulator/multi-display/overlay

PRODUCT_COPY_FILES += \
    device/generic/car/emulator/multi-display/car_audio_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/car_audio_configuration.xml

# Use to disable cluster display definitions in aosp_car_emulator
BUILD_EMULATOR_CLUSTER_DISPLAY := false

PRODUCT_PRODUCT_PROPERTIES += \
    hwservicemanager.external.displays=1,400,600,120,0,2,800,600,120,0,3,800,600,120,0 \
    persist.service.bootanim.displays=4619827551948147201,4619827124781842690,4619827540095559171

PRODUCT_PACKAGES += ClusterHomeSample ClusterOsDouble ClusterHomeSampleOverlay
PRODUCT_PACKAGES += CarServiceOverlayEmulatorOsDouble CarServiceOverlayMdEmulatorOsDouble ClusterOsDoubleEmulatorPhysicalDisplayOverlay

# Enable MZ audio by default
PRODUCT_SYSTEM_DEFAULT_PROPERTIES += \
    ro.aae.simulateMultiZoneAudio=true \
    persist.sys.max_profiles=5 \
    com.android.car.internal.debug.num_auto_populated_users=1

PRODUCT_PACKAGES += \
    MultiDisplaySecondaryHomeTestLauncher \
    MultiDisplayTest

$(call inherit-product, device/generic/car/sdk_car_x86_64.mk)

# TODO(b/266978709): Set it to true after cleaning up the system partition
# changes from this makefile
PRODUCT_ENFORCE_ARTIFACT_PATH_REQUIREMENTS := false

PRODUCT_NAME := sdk_car_md_x86_64
PRODUCT_DEVICE := emulator_car_x86_64
PRODUCT_BRAND := Android
PRODUCT_MODEL := Car multi-display on x86_64 emulator
