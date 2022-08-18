#
# Copyright (C) 2019 The Android Open Source Project
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

# Enable Setup Wizard. This overrides the setting in emulator_vendor.mk
PRODUCT_SYSTEM_EXT_PROPERTIES += \
    ro.setupwizard.mode?=OPTIONAL

ifeq (,$(ENABLE_REAR_VIEW_CAMERA_SAMPLE))
ENABLE_REAR_VIEW_CAMERA_SAMPLE:=true
endif

$(call inherit-product, device/generic/car/common/car.mk)
# This overrides device/generic/car/common/car.mk
$(call inherit-product, device/generic/car/emulator/audio/car_emulator_audio.mk)
$(call inherit-product, device/generic/car/emulator/rotary/car_rotary.mk)
# Enables USB related passthrough
$(call inherit-product, device/generic/car/emulator/usbpt/car_usbpt.mk)

# This section configures multi-display without hardcoding the
# displays on hwservicemanager.
ifneq ($(EMULATOR_DYNAMIC_MULTIDISPLAY_CONFIG),false)
$(call inherit-product, device/generic/car/emulator/multi-display-emulator/multidisplay-dynamic.mk)
endif  # EMULATOR_DYNAMIC_MULTIDISPLAY_CONFIG

PRODUCT_PRODUCT_PROPERTIES += \
    ro.carwatchdog.vhal_healthcheck.interval=10 \
    ro.carwatchdog.client_healthcheck.interval=20 \
