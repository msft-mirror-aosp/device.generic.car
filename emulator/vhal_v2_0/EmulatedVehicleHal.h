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

#ifndef android_hardware_automotive_vehicle_V2_0_impl_EmulatedVehicleHal_H_
#define android_hardware_automotive_vehicle_V2_0_impl_EmulatedVehicleHal_H_

#include <sys/socket.h>
#include <map>
#include <memory>
#include <thread>
#include <unordered_set>

#include <utils/SystemClock.h>

#include <vhal_v2_0/DefaultVehicleHal.h>
#include <vhal_v2_0/RecurrentTimer.h>
#include <vhal_v2_0/VehicleHal.h>
#include <vhal_v2_0/VehiclePropertyStore.h>

#include "EmulatedUserHal.h"

namespace android {
namespace hardware {
namespace automotive {
namespace vehicle {
namespace V2_0 {

namespace impl {

/** Implementation of VehicleHal that connected to emulator instead of real vehicle network. */
class EmulatedVehicleHal : public DefaultVehicleHal {
  public:
    EmulatedVehicleHal(VehiclePropertyStore* propStore, VehicleHalClient* client,
                       EmulatedUserHal* emulatedUserHal = nullptr);
    ~EmulatedVehicleHal() = default;

    //  Methods from VehicleHal
    void onCreate() override;
    VehiclePropValuePtr get(const VehiclePropValue& requestedPropValue,
                            StatusCode* outStatus) override;
    StatusCode set(const VehiclePropValue& propValue) override;
    void getAllPropertiesOverride();

  protected:
    void onContinuousPropertyTimer(const std::vector<int32_t>& properties) override;

  private:
    StatusCode handleGenerateFakeDataRequest(const VehiclePropValue& request);

    void initObd2LiveFrame(const VehiclePropConfig& propConfig);
    void initObd2FreezeFrame(const VehiclePropConfig& propConfig);
    StatusCode fillObd2FreezeFrame(const VehiclePropValue& requestedPropValue,
                                   VehiclePropValue* outValue);
    StatusCode fillObd2DtcInfo(VehiclePropValue* outValue);
    StatusCode clearObd2FreezeFrames(const VehiclePropValue& propValue);
    VehicleHal::VehiclePropValuePtr doInternalHealthCheck();
    VehicleHal::VehiclePropValuePtr createVhalHeartBeatProp();

    /* Private members */
    std::unordered_set<int32_t> mHvacPowerProps;
    bool mInitVhalValueOverride;
    std::vector<VehiclePropValue> mVehiclePropertiesOverride;
    EmulatedUserHal* mEmulatedUserHal;
};

}  // namespace impl

}  // namespace V2_0
}  // namespace vehicle
}  // namespace automotive
}  // namespace hardware
}  // namespace android

#endif  // android_hardware_automotive_vehicle_V2_0_impl_EmulatedVehicleHal_H_
