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

#pragma once

#include <android/hardware/automotive/vehicle/2.0/types.h>

namespace android::hardware::automotive::vehicle::V2_0::utils {

class Vehicle;

class VehicleBus : public virtual RefBase {
public:
    using PropertyCallback = std::function<void(const hidl_vec<vehicle::V2_0::VehiclePropValue>&)>;

    virtual ~VehicleBus();

    virtual const std::vector<vehicle::V2_0::VehicleProperty>& getSupportedProperties() const = 0;
    virtual vehicle::V2_0::StatusCode set(const vehicle::V2_0::VehiclePropValue& propValue) = 0;

protected:
    virtual void start();
    void sendPropertyEvent(const hidl_vec<vehicle::V2_0::VehiclePropValue>& propValues);

    static void updateTimestamps(std::vector<VehiclePropValue>& propValues, uint64_t timestamp);

  private:
    std::atomic<bool> mIsStarted = false;
    PropertyCallback mPropertyCallback;

    void setPropertyCallback(PropertyCallback propertyCb);

    friend class Vehicle;  // calling setPropertyCallback() and start()
};

}  // namespace android::hardware::automotive::vehicle::V2_0::utils
