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

#define LOG_TAG "automotive.vehicle@2.0-connector"

#include <fstream>

#include <android-base/logging.h>
#include <utils/SystemClock.h>
#include <vhal_v2_0/Obd2SensorStore.h>

#include "EmulatedVehicleConnector.h"

namespace android {
namespace hardware {
namespace automotive {
namespace vehicle {
namespace V2_0 {

namespace impl {

EmulatedUserHal* EmulatedVehicleConnector::getEmulatedUserHal() {
    return &mEmulatedUserHal;
}

std::unique_ptr<VehicleEmulator> EmulatedVehicleConnector::getEmulator() {
    return std::make_unique<VehicleEmulator>(this);
}

StatusCode EmulatedVehicleConnector::onSetProperty(const VehiclePropValue& value,
                                                   bool updateStatus) {
    if (mEmulatedUserHal.isSupported(value.prop)) {
        LOG(INFO) << "onSetProperty(): property " << value.prop << " will be handled by UserHal";

        const auto& ret = mEmulatedUserHal.onSetProperty(value);
        if (!ret.ok()) {
            LOG(ERROR) << "onSetProperty(): HAL returned error: " << ret.error().message();
            return StatusCode(ret.error().code());
        }
        auto updatedValue = ret.value().get();
        if (updatedValue != nullptr) {
            LOG(INFO) << "onSetProperty(): updating property returned by HAL: "
                      << toString(*updatedValue);
            onPropertyValueFromCar(*updatedValue, updateStatus);
        }
        return StatusCode::OK;
    }
    return this->EmulatedVehicleHalServer::onSetProperty(value, updateStatus);
}

IVehicleServer::DumpResult EmulatedVehicleConnector::onDump(
        const std::vector<std::string>& options) {
    DumpResult result;
    if (options.size() > 0) {
        if (options[0] == "--help") {
            result.buffer += "Emulator-specific usage:\n";
            result.buffer += mEmulatedUserHal.showDumpHelp();
            result.buffer += "\n";
            // Include caller's help options
            result.callerShouldDumpState = true;
            return result;
        } else if (options[0] == kUserHalDumpOption) {
            result.buffer += mEmulatedUserHal.dump("");
            result.callerShouldDumpState = false;
            return result;

        } else {
            // Let caller handle the options...
            result.callerShouldDumpState = true;
            return result;
        }
    }

    result.buffer += "Emulator-specific state:\n";
    result.buffer += mEmulatedUserHal.dump("  ");
    result.buffer += "\n";
    result.callerShouldDumpState = true;
    return result;
}

}  // namespace impl

}  // namespace V2_0
}  // namespace vehicle
}  // namespace automotive
}  // namespace hardware
}  // namespace android