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

$(call inherit-product, $(SRC_TARGET_DIR)/product/handheld_vendor.mk)

# Need this for gles libraries to load properly
# after moving to /vendor/lib/
PRODUCT_PACKAGES += \
    vndk-sp

DEVICE_PACKAGE_OVERLAYS := device/generic/goldfish/overlay

PRODUCT_CHARACTERISTICS := emulator

PRODUCT_FULL_TREBLE_OVERRIDE := true

# Goldfish vendor partition configurations
$(call inherit-product-if-exists, device/generic/goldfish/vendor.mk)

# Enable Google-specific location features,
# like NetworkLocationProvider and LocationCollector
PRODUCT_VENDOR_PROPERTIES += \
    ro.com.google.locationfeatures=1

# Enable setupwizard
PRODUCT_VENDOR_PROPERTIES += \
    ro.setupwizard.mode?=OPTIONAL

# More configurations
PRODUCT_VENDOR_PROPERTIES += \
    ro.carwatchdog.client_healthcheck.interval=20 \
    ro.carwatchdog.vhal_healthcheck.interval=10 \

ifeq (,$(ENABLE_REAR_VIEW_CAMERA_SAMPLE))
ENABLE_REAR_VIEW_CAMERA_SAMPLE:=true
endif

$(call inherit-product, device/generic/car/common/car.mk)
# This overrides device/generic/car/common/car.mk
$(call inherit-product, device/generic/car/emulator/audio/car_emulator_audio.mk)
$(call inherit-product, device/generic/car/emulator/rotary/car_rotary.mk)
# Enables USB related passthrough
$(call inherit-product, device/generic/car/emulator/usbpt/car_usbpt.mk)

# EVS
# By default, we enable EvsManager, a sample EVS app, and a mock EVS HAL implementation.
# If you want to use your own EVS HAL implementation, please set ENABLE_MOCK_EVSHAL as false
# and add your HAL implementation to the product.  Please also check init.evs.rc and see how
# you can configure EvsManager to use your EVS HAL implementation.  Similarly, please set
# ENABLE_SAMPLE_EVS_APP as false if you want to use your own EVS app configuration or own EVS
# app implementation.
ENABLE_EVS_SAMPLE ?= false
ENABLE_EVS_SERVICE ?= true
ENABLE_MOCK_EVSHAL ?= true
ENABLE_CAREVSSERVICE_SAMPLE ?= false
ENABLE_SAMPLE_EVS_APP ?= false
ENABLE_CARTELEMETRY_SERVICE ?= false
ifeq ($(ENABLE_MOCK_EVSHAL), true)
CUSTOMIZE_EVS_SERVICE_PARAMETER := true
endif  # ENABLE_MOCK_EVSHAL
$(call inherit-product, device/generic/car/emulator/evs/evs.mk)

ifeq ($(EMULATOR_DYNAMIC_MULTIDISPLAY_CONFIG),true)
# This section configures multi-display without hardcoding the
# displays on hwservicemanager.
$(call inherit-product, device/generic/car/emulator/multi-display-dynamic/multi-display-dynamic.mk)
else # EMULATOR_DYNAMIC_MULTIDISPLAY_CONFIG
ifeq (true,$(BUILD_EMULATOR_CLUSTER_DISPLAY))
$(call inherit-product, device/generic/car/emulator/cluster/cluster-hwservicemanager.mk)
endif # BUILD_EMULATOR_CLUSTER_DISPLAY
endif # EMULATOR_DYNAMIC_MULTIDISPLAY_CONFIG