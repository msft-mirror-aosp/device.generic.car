/*
 * Copyright (C) 2021 The Android Open Source Project
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

#define LOG_TAG "EmulatedVehicleHalServer"

#include <utils/SystemClock.h>
#include <vhal_v2_0/VehicleUtils.h>

#include "EmulatedVehicleHalServer.h"

namespace android {
namespace hardware {
namespace automotive {
namespace vehicle {
namespace V2_0 {

namespace impl {

static bool isDiagnosticProperty(VehiclePropConfig propConfig) {
    switch (propConfig.prop) {
        case OBD2_LIVE_FRAME:
        case OBD2_FREEZE_FRAME:
        case OBD2_FREEZE_FRAME_CLEAR:
        case OBD2_FREEZE_FRAME_INFO:
            return true;
    }
    return false;
}

EmulatedVehicleHalServer::EmulatedVehicleHalServer() {
    mInQEMU = isInQEMU();
    ALOGD("mInQEMU=%s", mInQEMU ? "true" : "false");

    for (auto& it : kVehicleProperties) {
        VehiclePropConfig cfg = it.config;
        mServerSidePropStore.registerProperty(cfg);
        // Skip diagnostic properties since EmulatedVehicleHal has special logic to handle those.
        if (isDiagnosticProperty(cfg)) {
            continue;
        }
        storePropInitialValue(it);
    }
}

StatusCode EmulatedVehicleHalServer::onSetProperty(const VehiclePropValue& value,
                                                   bool updateStatus) {
    if (mInQEMU && value.prop == toInt(VehicleProperty::DISPLAY_BRIGHTNESS)) {
        // Emulator does not support remote brightness control, b/139959479
        // do not send it down so that it does not bring unnecessary property change event
        // return other error code, such NOT_AVAILABLE, causes Emulator to be freezing
        // TODO: return StatusCode::NOT_AVAILABLE once the above issue is fixed
        return StatusCode::OK;
    }

    return DefaultVehicleHalServer::onSetProperty(value, updateStatus);
}

bool EmulatedVehicleHalServer::setPropertyFromVehicle(const VehiclePropValue& propValue) {
    auto updatedPropValue = getValuePool()->obtain(propValue);
    updatedPropValue->timestamp = elapsedRealtimeNano();
    mServerSidePropStore.writeValue(*updatedPropValue, true);
    onPropertyValueFromCar(*updatedPropValue, true);
    return true;
}

std::vector<VehiclePropValue> EmulatedVehicleHalServer::getAllProperties() const {
    return mServerSidePropStore.readAllValues();
}

std::vector<VehiclePropConfig> EmulatedVehicleHalServer::listProperties() {
    return mServerSidePropStore.getAllConfigs();
}

EmulatedVehicleHalServer::VehiclePropValuePtr EmulatedVehicleHalServer::get(
        const VehiclePropValue& requestedPropValue, StatusCode* outStatus) {
    EmulatedVehicleHalServer::VehiclePropValuePtr v = nullptr;
    auto prop = mServerSidePropStore.readValueOrNull(requestedPropValue);
    if (prop != nullptr) {
        v = getValuePool()->obtain(*prop);
    }

    if (!v) {
        *outStatus = StatusCode::INVALID_ARG;
    } else if (v->status == VehiclePropertyStatus::AVAILABLE) {
        *outStatus = StatusCode::OK;
    } else {
        *outStatus = StatusCode::TRY_AGAIN;
    }

    if (v.get()) {
        v->timestamp = elapsedRealtimeNano();
    }
    return v;
}

}  // namespace impl

}  // namespace V2_0
}  // namespace vehicle
}  // namespace automotive
}  // namespace hardware
}  // namespace android
