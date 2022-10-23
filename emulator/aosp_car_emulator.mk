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

# EVS
# By default, we enable EvsManager, a sample EVS app, and a mock EVS HAL implementation.
# If you want to use your own EVS HAL implementation, please set ENABLE_MOCK_EVSHAL as false
# and add your HAL implementation to the product.  Please also check init.evs.rc and see how
# you can configure EvsManager to use your EVS HAL implementation.  Similarly, please set
# ENABLE_SAMPLE_EVS_APP as false if you want to use your own EVS app configuration or own EVS
# app implementation.
ENABLE_EVS_SERVICE ?= true
ENABLE_MOCK_EVSHAL ?= true
ENABLE_CAREVSSERVICE_SAMPLE ?= true
ENABLE_SAMPLE_EVS_APP ?= true
ENABLE_CARTELEMETRY_SERVICE ?= true
ifeq ($(ENABLE_MOCK_EVSHAL), true)
CUSTOMIZE_EVS_SERVICE_PARAMETER := true
endif  # ENABLE_MOCK_EVSHAL
$(call inherit-product, device/generic/car/emulator/evs/evs.mk)

ifeq (true,$(BUILD_EMULATOR_CLUSTER_DISPLAY))
PRODUCT_COPY_FILES += \
    device/generic/car/emulator/cluster/display_settings.xml:system/etc/display_settings.xml \

PRODUCT_PRODUCT_PROPERTIES += \
    hwservicemanager.external.displays=1,400,600,120,0 \
    persist.service.bootanim.displays=8140900251843329 \

ifeq (true,$(ENABLE_CLUSTER_OS_DOUBLE))
PRODUCT_PACKAGES += CarServiceOverlayEmulatorOsDouble
else
PRODUCT_PACKAGES += CarServiceOverlayEmulator
endif  # ENABLE_CLUSTER_OS_DOUBLE
endif  # BUILD_EMULATOR_CLUSTER_DISPLAY

PRODUCT_PRODUCT_PROPERTIES += \
    ro.carwatchdog.vhal_healthcheck.interval=10 \
    ro.carwatchdog.client_healthcheck.interval=20 \
