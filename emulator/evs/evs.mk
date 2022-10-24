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

PRODUCT_PACKAGES += CarEvsServiceOverlay

ifeq ($(ENABLE_MOCK_EVSHAL), true)
CUSTOMIZE_EVS_SERVICE_PARAMETER := true
PRODUCT_PACKAGES += android.hardware.automotive.evs@1.1-service \
    android.frameworks.automotive.display@1.0-service
PRODUCT_COPY_FILES += \
    device/generic/car/emulator/evs/init.evs.rc:$(TARGET_COPY_OUT_SYSTEM)/etc/init/init.evs.rc
endif

ifeq ($(ENABLE_SAMPLE_EVS_APP), true)
PRODUCT_PACKAGES += evs_app
PRODUCT_COPY_FILES += \
    device/generic/car/emulator/evs/evs_app_config.json:$(TARGET_COPY_OUT_SYSTEM)/etc/automotive/evs/config_override.json
$(call inherit-product, packages/services/Car/cpp/evs/apps/sepolicy/evsapp.mk)
endif
