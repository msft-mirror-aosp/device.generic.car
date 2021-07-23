/*
 * Copyright (C) 2016 The Android Open Source Project
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
#define LOG_TAG "EmulatedVehicleHal_v2_0"

#include <android-base/chrono_utils.h>
#include <android-base/logging.h>
#include <android-base/macros.h>
#include <android-base/properties.h>
#include <android/log.h>
#include <dirent.h>
#include <sys/system_properties.h>
#include <utils/SystemClock.h>
#include <fstream>
#include <log/log.h>
#include <regex>
#include <vhal_v2_0/JsonFakeValueGenerator.h>
#include <vhal_v2_0/Obd2SensorStore.h>
#include <vhal_v2_0/PropertyUtils.h>

#include "EmulatedVehicleHal.h"

namespace android {
namespace hardware {
namespace automotive {
namespace vehicle {
namespace V2_0 {

namespace impl {

static std::unique_ptr<Obd2SensorStore> fillDefaultObd2Frame(size_t numVendorIntegerSensors,
                                                             size_t numVendorFloatSensors) {
    std::unique_ptr<Obd2SensorStore> sensorStore(
            new Obd2SensorStore(numVendorIntegerSensors, numVendorFloatSensors));

    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::FUEL_SYSTEM_STATUS,
                                  toInt(Obd2FuelSystemStatus::CLOSED_LOOP));
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::MALFUNCTION_INDICATOR_LIGHT_ON, 0);
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::IGNITION_MONITORS_SUPPORTED,
                                  toInt(Obd2IgnitionMonitorKind::SPARK));
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::IGNITION_SPECIFIC_MONITORS,
                                  Obd2CommonIgnitionMonitors::COMPONENTS_AVAILABLE |
                                          Obd2CommonIgnitionMonitors::MISFIRE_AVAILABLE |
                                          Obd2SparkIgnitionMonitors::AC_REFRIGERANT_AVAILABLE |
                                          Obd2SparkIgnitionMonitors::EVAPORATIVE_SYSTEM_AVAILABLE);
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::INTAKE_AIR_TEMPERATURE, 35);
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::COMMANDED_SECONDARY_AIR_STATUS,
                                  toInt(Obd2SecondaryAirStatus::FROM_OUTSIDE_OR_OFF));
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::NUM_OXYGEN_SENSORS_PRESENT, 1);
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::RUNTIME_SINCE_ENGINE_START, 500);
    sensorStore->setIntegerSensor(
            DiagnosticIntegerSensorIndex::DISTANCE_TRAVELED_WITH_MALFUNCTION_INDICATOR_LIGHT_ON, 0);
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::WARMUPS_SINCE_CODES_CLEARED, 51);
    sensorStore->setIntegerSensor(
            DiagnosticIntegerSensorIndex::DISTANCE_TRAVELED_SINCE_CODES_CLEARED, 365);
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::ABSOLUTE_BAROMETRIC_PRESSURE, 30);
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::CONTROL_MODULE_VOLTAGE, 12);
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::AMBIENT_AIR_TEMPERATURE, 18);
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::MAX_FUEL_AIR_EQUIVALENCE_RATIO, 1);
    sensorStore->setIntegerSensor(DiagnosticIntegerSensorIndex::FUEL_TYPE,
                                  toInt(Obd2FuelType::GASOLINE));
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::CALCULATED_ENGINE_LOAD, 0.153);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::SHORT_TERM_FUEL_TRIM_BANK1, -0.16);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::LONG_TERM_FUEL_TRIM_BANK1, -0.16);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::SHORT_TERM_FUEL_TRIM_BANK2, -0.16);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::LONG_TERM_FUEL_TRIM_BANK2, -0.16);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::INTAKE_MANIFOLD_ABSOLUTE_PRESSURE, 7.5);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::ENGINE_RPM, 1250.);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::VEHICLE_SPEED, 40.);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::TIMING_ADVANCE, 2.5);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::THROTTLE_POSITION, 19.75);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::OXYGEN_SENSOR1_VOLTAGE, 0.265);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::FUEL_TANK_LEVEL_INPUT, 0.824);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::EVAPORATION_SYSTEM_VAPOR_PRESSURE,
                                -0.373);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::CATALYST_TEMPERATURE_BANK1_SENSOR1,
                                190.);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::RELATIVE_THROTTLE_POSITION, 3.);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::ABSOLUTE_THROTTLE_POSITION_B, 0.306);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::ACCELERATOR_PEDAL_POSITION_D, 0.188);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::ACCELERATOR_PEDAL_POSITION_E, 0.094);
    sensorStore->setFloatSensor(DiagnosticFloatSensorIndex::COMMANDED_THROTTLE_ACTUATOR, 0.024);

    return sensorStore;
}

EmulatedVehicleHal::EmulatedVehicleHal(VehiclePropertyStore* propStore, VehicleHalClient* client,
                                       EmulatedUserHal* emulatedUserHal)
    : DefaultVehicleHal(propStore, client),
      mHvacPowerProps(std::begin(kHvacPowerProperties), std::end(kHvacPowerProperties)),
      mEmulatedUserHal(emulatedUserHal) {
    mInitVhalValueOverride =
            android::base::GetBoolProperty("persist.vendor.vhal_init_value_override", false);
    if (mInitVhalValueOverride) {
        getAllPropertiesOverride();
    }
}

void EmulatedVehicleHal::getAllPropertiesOverride() {
    if (auto dir = opendir("/vendor/etc/vhaloverride/")) {
        std::regex reg_json(".*[.]json", std::regex::icase);
        while (auto f = readdir(dir)) {
            if (!regex_match(f->d_name, reg_json)) {
                continue;
            }
            std::string file = "/vendor/etc/vhaloverride/" + std::string(f->d_name);
            JsonFakeValueGenerator tmpGenerator(file);

            std::vector<VehiclePropValue> propvalues = tmpGenerator.getAllEvents();
            mVehiclePropertiesOverride.insert(std::end(mVehiclePropertiesOverride),
                                              std::begin(propvalues), std::end(propvalues));
        }
        closedir(dir);
    }
}

VehicleHal::VehiclePropValuePtr EmulatedVehicleHal::get(const VehiclePropValue& requestedPropValue,
                                                        StatusCode* outStatus) {
    auto propId = requestedPropValue.prop;
    ALOGV("get(%d)", propId);

    auto& pool = *getValuePool();
    VehiclePropValuePtr v = nullptr;

    switch (propId) {
        case OBD2_FREEZE_FRAME:
            v = pool.obtainComplex();
            *outStatus = fillObd2FreezeFrame(requestedPropValue, v.get());
            break;
        case OBD2_FREEZE_FRAME_INFO:
            v = pool.obtainComplex();
            *outStatus = fillObd2DtcInfo(v.get());
            break;
        default:
            if (mEmulatedUserHal != nullptr && mEmulatedUserHal->isSupported(propId)) {
                ALOGI("get(): getting value for prop %d from User HAL", propId);
                const auto& ret = mEmulatedUserHal->onGetProperty(requestedPropValue);
                if (!ret.ok()) {
                    ALOGE("get(): User HAL returned error: %s", ret.error().message().c_str());
                    *outStatus = StatusCode(ret.error().code());
                } else {
                    auto value = ret.value().get();
                    if (value != nullptr) {
                        ALOGI("get(): User HAL returned value: %s", toString(*value).c_str());
                        v = getValuePool()->obtain(*value);
                        *outStatus = StatusCode::OK;
                    } else {
                        ALOGE("get(): User HAL returned null value");
                        *outStatus = StatusCode::INTERNAL_ERROR;
                    }
                }
                break;
            }

            auto internalPropValue = mPropStore->readValueOrNull(requestedPropValue);
            if (internalPropValue != nullptr) {
                v = getValuePool()->obtain(*internalPropValue);
            }

            if (!v) {
                *outStatus = StatusCode::INVALID_ARG;
            } else if (v->status == VehiclePropertyStatus::AVAILABLE) {
                *outStatus = StatusCode::OK;
            } else {
                *outStatus = StatusCode::TRY_AGAIN;
            }
            break;
    }
    if (v.get()) {
        v->timestamp = elapsedRealtimeNano();
    }
    return v;
}

StatusCode EmulatedVehicleHal::set(const VehiclePropValue& propValue) {
    constexpr bool updateStatus = false;

    if (propValue.prop == kGenerateFakeDataControllingProperty) {
        // Send the generator controlling request to the server.
        // 'updateStatus' flag is only for the value sent by setProperty (propValue in this case)
        // instead of the generated values triggered by it. 'propValue' works as a control signal
        // here, since we never send the control signal back, the value of 'updateStatus' flag
        // does not matter here.
        auto status = mVehicleClient->setProperty(propValue, updateStatus);
        return status;
    } else if (mHvacPowerProps.count(propValue.prop)) {
        auto hvacPowerOn = mPropStore->readValueOrNull(
                toInt(VehicleProperty::HVAC_POWER_ON),
                (VehicleAreaSeat::ROW_1_LEFT | VehicleAreaSeat::ROW_1_RIGHT |
                 VehicleAreaSeat::ROW_2_LEFT | VehicleAreaSeat::ROW_2_CENTER |
                 VehicleAreaSeat::ROW_2_RIGHT));

        if (hvacPowerOn && hvacPowerOn->value.int32Values.size() == 1 &&
            hvacPowerOn->value.int32Values[0] == 0) {
            return StatusCode::NOT_AVAILABLE;
        }
    } else {
        // Handle property specific code
        switch (propValue.prop) {
            case OBD2_FREEZE_FRAME_CLEAR:
                return clearObd2FreezeFrames(propValue);
            case VEHICLE_MAP_SERVICE:
                // Placeholder for future implementation of VMS property in the default hal. For
                // now, just returns OK; otherwise, hal clients crash with property not supported.
                return StatusCode::OK;
        }
    }

    if (propValue.status != VehiclePropertyStatus::AVAILABLE) {
        // Android side cannot set property status - this value is the
        // purview of the HAL implementation to reflect the state of
        // its underlying hardware
        return StatusCode::INVALID_ARG;
    }
    auto currentPropValue = mPropStore->readValueOrNull(propValue);

    if (currentPropValue == nullptr) {
        return StatusCode::INVALID_ARG;
    }
    if (currentPropValue->status != VehiclePropertyStatus::AVAILABLE) {
        // do not allow Android side to set() a disabled/error property
        return StatusCode::NOT_AVAILABLE;
    }

    /**
     * After checking all conditions, such as the property is available, a real vhal will
     * sent the events to Car ECU to take actions.
     */

    // Send the value to the vehicle server, the server will talk to the (real or emulated) car
    auto setValueStatus = mVehicleClient->setProperty(propValue, updateStatus);
    if (setValueStatus != StatusCode::OK) {
        return setValueStatus;
    }

    return StatusCode::OK;
}

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

// Parse supported properties list and generate vector of property values to hold current values.
void EmulatedVehicleHal::onCreate() {
    static constexpr bool shouldUpdateStatus = true;

    auto configs = mVehicleClient->getAllPropertyConfig();

    for (const auto& cfg : configs) {
        if (isDiagnosticProperty(cfg)) {
            // do not write an initial empty value for the diagnostic properties
            // as we will initialize those separately.
            continue;
        }

        int32_t numAreas = isGlobalProp(cfg.prop) ? 0 : cfg.areaConfigs.size();

        for (int i = 0; i < numAreas; i++) {
            int32_t curArea = isGlobalProp(cfg.prop) ? 0 : cfg.areaConfigs[i].areaId;

            // Create a separate instance for each individual zone
            VehiclePropValue prop = {
                    .areaId = curArea,
                    .prop = cfg.prop,
                    .status = VehiclePropertyStatus::UNAVAILABLE,
            };

            if (mInitVhalValueOverride) {
                for (auto& itOverride : mVehiclePropertiesOverride) {
                    if (itOverride.prop == cfg.prop) {
                        prop.status = VehiclePropertyStatus::AVAILABLE;
                        prop.value = itOverride.value;
                    }
                }
            }
            mPropStore->writeValue(prop, shouldUpdateStatus);
        }
    }

    mVehicleClient->triggerSendAllValues();

    initObd2LiveFrame(*mPropStore->getConfigOrDie(OBD2_LIVE_FRAME));
    initObd2FreezeFrame(*mPropStore->getConfigOrDie(OBD2_FREEZE_FRAME));

    registerHeartBeatEvent();
}

void EmulatedVehicleHal::initObd2LiveFrame(const VehiclePropConfig& propConfig) {
    static constexpr bool shouldUpdateStatus = true;

    auto liveObd2Frame = createVehiclePropValue(VehiclePropertyType::MIXED, 0);
    auto sensorStore = fillDefaultObd2Frame(static_cast<size_t>(propConfig.configArray[0]),
                                            static_cast<size_t>(propConfig.configArray[1]));
    sensorStore->fillPropValue("", liveObd2Frame.get());
    liveObd2Frame->prop = OBD2_LIVE_FRAME;

    mPropStore->writeValue(*liveObd2Frame, shouldUpdateStatus);
}

void EmulatedVehicleHal::initObd2FreezeFrame(const VehiclePropConfig& propConfig) {
    static constexpr bool shouldUpdateStatus = true;

    auto sensorStore = fillDefaultObd2Frame(static_cast<size_t>(propConfig.configArray[0]),
                                            static_cast<size_t>(propConfig.configArray[1]));

    static std::vector<std::string> sampleDtcs = {"P0070",
                                                  "P0102"
                                                  "P0123"};
    for (auto&& dtc : sampleDtcs) {
        auto freezeFrame = createVehiclePropValue(VehiclePropertyType::MIXED, 0);
        sensorStore->fillPropValue(dtc, freezeFrame.get());
        freezeFrame->prop = OBD2_FREEZE_FRAME;

        mPropStore->writeValue(*freezeFrame, shouldUpdateStatus);
    }
}

StatusCode EmulatedVehicleHal::fillObd2FreezeFrame(const VehiclePropValue& requestedPropValue,
                                                   VehiclePropValue* outValue) {
    if (requestedPropValue.value.int64Values.size() != 1) {
        ALOGE("asked for OBD2_FREEZE_FRAME without valid timestamp");
        return StatusCode::INVALID_ARG;
    }
    auto timestamp = requestedPropValue.value.int64Values[0];
    auto freezeFrame = mPropStore->readValueOrNull(OBD2_FREEZE_FRAME, 0, timestamp);
    if (freezeFrame == nullptr) {
        ALOGE("asked for OBD2_FREEZE_FRAME at invalid timestamp");
        return StatusCode::INVALID_ARG;
    }
    outValue->prop = OBD2_FREEZE_FRAME;
    outValue->value.int32Values = freezeFrame->value.int32Values;
    outValue->value.floatValues = freezeFrame->value.floatValues;
    outValue->value.bytes = freezeFrame->value.bytes;
    outValue->value.stringValue = freezeFrame->value.stringValue;
    outValue->timestamp = freezeFrame->timestamp;
    return StatusCode::OK;
}

StatusCode EmulatedVehicleHal::clearObd2FreezeFrames(const VehiclePropValue& propValue) {
    if (propValue.value.int64Values.size() == 0) {
        mPropStore->removeValuesForProperty(OBD2_FREEZE_FRAME);
        return StatusCode::OK;
    } else {
        for (int64_t timestamp : propValue.value.int64Values) {
            auto freezeFrame = mPropStore->readValueOrNull(OBD2_FREEZE_FRAME, 0, timestamp);
            if (freezeFrame == nullptr) {
                ALOGE("asked for OBD2_FREEZE_FRAME at invalid timestamp");
                return StatusCode::INVALID_ARG;
            }
            mPropStore->removeValue(*freezeFrame);
        }
    }
    return StatusCode::OK;
}

StatusCode EmulatedVehicleHal::fillObd2DtcInfo(VehiclePropValue* outValue) {
    std::vector<int64_t> timestamps;
    for (const auto& freezeFrame : mPropStore->readValuesForProperty(OBD2_FREEZE_FRAME)) {
        timestamps.push_back(freezeFrame.timestamp);
    }
    outValue->value.int64Values = timestamps;
    outValue->prop = OBD2_FREEZE_FRAME_INFO;
    return StatusCode::OK;
}

}  // namespace impl

}  // namespace V2_0
}  // namespace vehicle
}  // namespace automotive
}  // namespace hardware
}  // namespace android