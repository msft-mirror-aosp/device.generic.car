/*
 * Copyright (C) 2020 Google Inc. All Rights Reserved.
 */

#pragma once

#include <libcanhal/CanClient.h>

#include <optional>

namespace android::hardware::automotive::vehicle::V2_0::implementation {

class ExtraCanClient : public can::V1_0::utils::CanClient {
  public:
    ExtraCanClient();

    void onReady(const sp<can::V1_0::ICanBus>& canBus) override;
    const std::vector<VehicleProperty>& getSupportedProperties() const override;
    Return<void> onReceive(const can::V1_0::CanMessage& message) override;
    StatusCode set(const VehiclePropValue& propValue) override;
};

}  // namespace android::hardware::automotive::vehicle::V2_0::implementation
