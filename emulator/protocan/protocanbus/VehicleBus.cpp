/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <libvehiclehal/VehicleBus.h>

#include <android-base/logging.h>

namespace android::hardware::automotive::vehicle::V2_0::utils {

VehicleBus::~VehicleBus() {}

void VehicleBus::start() {
    bool wasStarted = mIsStarted.exchange(true);
    CHECK(!wasStarted) << "CanClient was already started";
}

void VehicleBus::setPropertyCallback(PropertyCallback propertyCb) {
    CHECK(!mIsStarted) << "Can't set callback when VehicleBus is started";
    CHECK(mPropertyCallback == nullptr) << "Can't set callback twice";
    mPropertyCallback = propertyCb;
    CHECK(!mIsStarted) << "Can't set callback when VehicleBus is started";
}

void VehicleBus::sendPropertyEvent(const hidl_vec<VehiclePropValue>& propValues) {
    CHECK(mPropertyCallback != nullptr) << "Callback isn't set";
    mPropertyCallback(propValues);
}

void VehicleBus::updateTimestamps(std::vector<VehiclePropValue>& propValues, uint64_t timestamp) {
    for (auto&& pv : propValues) {
        pv.timestamp = timestamp;
    }
}

}  // namespace android::hardware::automotive::vehicle::V2_0::utils
