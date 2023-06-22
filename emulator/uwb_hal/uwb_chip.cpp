/*
 * Copyright 2021, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "android.hardware.uwb"

#include <android-base/logging.h>
#include <sys/system_properties.h>

#include "uwb.h"
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace {
constexpr static int32_t kAndroidUciVersion = 1;
}

namespace android {
namespace hardware {
namespace uwb {
namespace impl {
using namespace ::aidl::android::hardware::uwb;

int read_fully(int fd, uint8_t* buffer, int size) {
    int bytes_read = 0;
    while (size > bytes_read) {
      int n = read(fd, buffer + bytes_read, size - bytes_read);
      LOG(INFO) << "UWB HAL attempting to read "<< size-bytes_read << ", got " << n;
      if (n <= 0) return n;
      bytes_read += n;
    }

    return 0;
}

UwbChip::UwbChip(const std::string& name) : name_(name), mClientCallback(nullptr) {
    int len;
    char service_name[PROP_VALUE_MAX];
    len = __system_property_get(PROPERTY_UWB_SERVICE, service_name);
    LOG(INFO) << "UWB HAL " << PROPERTY_UWB_SERVICE << "='" << service_name << "'";
    if (strcmp(service_name, PROPERTY_UWB_SERVICE_GOLDFISH) == 0) {
        LOG(INFO) << "UWB HAL goldfish service enabled.";
        enabled = true;
    } else {
        enabled = false;
    }
}
UwbChip::~UwbChip() {}

::ndk::ScopedAStatus UwbChip::getName(std::string* name) {
    *name = name_;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus UwbChip::open(const std::shared_ptr<IUwbClientCallback>& clientCallback) {
    if (!enabled) {
        LOG(INFO) << "UWB HAL is not enabled. Not opening connection to UWB backend.";
        mClientCallback = clientCallback;
        mClientCallback->onHalEvent(UwbEvent::OPEN_CPLT, UwbStatus::OK);
        return ndk::ScopedAStatus::ok();
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;

    char ip_prop[PROP_VALUE_MAX];
    char port_prop[PROP_VALUE_MAX];
    int len;
    len = __system_property_get(PROPERTY_UWB_SERVICE_IP, ip_prop);
    if (len == 0) {
        LOG(INFO) << "UWB HAL Using default ip: 127.0.0.1.";
        strcpy(ip_prop, "127.0.0.1");
    }
    len = __system_property_get(PROPERTY_UWB_SERVICE_PORT, port_prop);
    if (len == 0) {
        LOG(INFO) << "UWB HAL Using default port: 7654.";
        strcpy(port_prop, "7654");
    }
    if (inet_pton(AF_INET, ip_prop, &serv_addr.sin_addr) <= 0) {
        LOG(INFO) << "UWB HAL Invalid address: " << ip_prop;
        return ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(-1, "Invalid address");
    }
    int port_int = atoi(port_prop);
    if (port_int <= 0) {
        LOG(INFO) << "UWB HAL Invalid port: " << port_prop;
        return ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(-2, "Invalid port");
    }
    serv_addr.sin_port = htons(port_int);

    int status;
    if ((fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG(INFO) << "UWB HAL Error creating socket.";
        return ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(-3, "Connection failed");
    }

    if ((status = connect(fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        LOG(INFO) << "UWB HAL Connection failed: \"" << ip_prop << ":" << port_prop << "\"";
        return ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(-4, "Connection failed");
    } else {
        LOG(INFO) << "UWB HAL Connected to: \"" << ip_prop << ":" << port_prop << "\"";
    }

    connected = true;
    read_thread_ = std::thread([this]() {
        while (connected) {
            std::vector<uint8_t> buffer(1024);
            // Read the packet header
            if (read_fully(fd_, buffer.data(), 4) != 0) {
                LOG(INFO) << "UWB HAL: Cannot read packet header. Closing connection.";
                connected = false;
                break;
            }
            size_t payload_length = buffer[3];
            if (payload_length > 0) {
                if (read_fully(fd_, buffer.data() + 4, payload_length) != 0) {
                    LOG(INFO) << "UWB HAL Could not read the complete packet. Closing connection.";
                    connected = false;
                    break;
                }
            }

            buffer.resize(payload_length + 4);
            mClientCallback->onUciMessage(buffer);
        }
    });
    mClientCallback = clientCallback;
    mClientCallback->onHalEvent(UwbEvent::OPEN_CPLT, UwbStatus::OK);
    LOG(INFO) << "UWB HAL Initialization complete.";
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus UwbChip::close() {
    if (enabled && connected) {
        LOG(INFO) << "UWB HAL Closing connection.";
        ::close(fd_);
        connected = false;
        read_thread_.join();
    }
    // Return success even when the stub implementation is running.
    mClientCallback->onHalEvent(UwbEvent::CLOSE_CPLT, UwbStatus::OK);
    mClientCallback = nullptr;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus UwbChip::coreInit() {
    auto status = mClientCallback->onHalEvent(UwbEvent::POST_INIT_CPLT, UwbStatus::OK);
    if (!status.isOk()) {
        return ndk::ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_STATE,
                                                                "onHalEvent POST_INIT_CPLT failed");
    }

    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus UwbChip::sessionInit(int sessionId) {
    LOG(INFO) << "UWB HAL sessionInit sid=" << sessionId;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus UwbChip::getSupportedAndroidUciVersion(int32_t* version) {
    *version = kAndroidUciVersion;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus UwbChip::sendUciMessage(const std::vector<uint8_t>& data,
                                             int32_t* bytes_written) {
    if (!enabled) {
        LOG(INFO) << "UWB HAL is not enabled. Not sending message.";
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    LOG(VERBOSE) << "UWB HAL sendUciMessage. Size=" << data.size();

    int ret = write(fd_, data.data(), data.size());

    if (ret != data.size()) {
        LOG(WARNING) << "UWB HAL sendUciMessage Illegal argument. Bytes missed=" << ret;
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    *bytes_written = (int32_t)data.size();
    return ndk::ScopedAStatus::ok();
}
}  // namespace impl
}  // namespace uwb
}  // namespace hardware
}  // namespace android
