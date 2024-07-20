#
# Copyright (C) 2024 The Android Open Source Project
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

# Exclude AAE Car System UI
DO_NOT_INCLUDE_AAE_CAR_SYSTEM_UI := true

EMULATOR_DYNAMIC_MULTIDISPLAY_CONFIG := false
BUILD_EMULATOR_CLUSTER_DISPLAY := true
ENABLE_CLUSTER_OS_DOUBLE:=true

# Set up cluster and distance display
EMULATOR_MULTIDISPLAY_HW_CONFIG := 1,968,792,160,0,2,3000,792,160,0
EMULATOR_MULTIDISPLAY_BOOTANIM_CONFIG := 4619827259835644672,4619827551948147201

# Define a property to apply stacked layout in QEMU
PRODUCT_SYSTEM_PROPERTIES += \
    ro.emulator.car.distantdisplay=true

PRODUCT_COPY_FILES += \
    device/generic/car/emulator/distant-display/display/display_settings.xml:$(TARGET_COPY_OUT_VENDOR)/etc/display_settings.xml

PRODUCT_PACKAGES += \
    CarDistantDisplaySystemUI \
    CarDistantDisplayEmulatorCarServiceRRO
