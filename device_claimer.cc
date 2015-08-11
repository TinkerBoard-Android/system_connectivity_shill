// Copyright 2014 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/device_claimer.h"

#include "shill/control_interface.h"
#include "shill/device_info.h"

using std::string;

namespace shill {

DeviceClaimer::DeviceClaimer(
    const std::string& service_name,
    DeviceInfo* device_info,
    bool default_claimer)
    : service_name_(service_name),
      device_info_(device_info),
      default_claimer_(default_claimer) {}

DeviceClaimer::~DeviceClaimer() {
  // Release claimed devices if there is any.
  if (DevicesClaimed()) {
    for (const auto& device : claimed_device_names_) {
      device_info_->RemoveDeviceFromBlackList(device);
    }
    // Clear claimed device list.
    claimed_device_names_.clear();
  }
}

bool DeviceClaimer::StartServiceWatcher(
    ControlInterface* control_interface,
    const base::Closure& connection_vanished_callback) {
  if (service_watcher_) {
    LOG(ERROR) << "Service watcher already started";
    return false;
  }
  service_watcher_.reset(
      control_interface->CreateRPCServiceWatcher(service_name_,
                                                 connection_vanished_callback));
  return true;
}

bool DeviceClaimer::Claim(const string& device_name, Error* error) {
  // Check if device is claimed already.
  if (claimed_device_names_.find(device_name) != claimed_device_names_.end()) {
    Error::PopulateAndLog(FROM_HERE, error, Error::kInvalidArguments,
                          "Device " + device_name +
                          " had already been claimed");
    return false;
  }

  // Add device to the black list.
  device_info_->AddDeviceToBlackList(device_name);

  claimed_device_names_.insert(device_name);
  released_device_names_.erase(device_name);
  return true;
}

bool DeviceClaimer::Release(const std::string& device_name,
                            Error* error) {
  // Make sure this is a device that have been claimed.
  if (claimed_device_names_.find(device_name) == claimed_device_names_.end()) {
    Error::PopulateAndLog(FROM_HERE, error, Error::kInvalidArguments,
                          "Device " + device_name +
                          " have not been claimed");
    return false;
  }

  // Remove the device from the black list.
  device_info_->RemoveDeviceFromBlackList(device_name);

  claimed_device_names_.erase(device_name);
  released_device_names_.insert(device_name);
  return true;
}

bool DeviceClaimer::DevicesClaimed() {
  return !claimed_device_names_.empty();
}

bool DeviceClaimer::IsDeviceReleased(const string& device_name) {
  return released_device_names_.find(device_name) !=
      released_device_names_.end();
}

}  // namespace shill
