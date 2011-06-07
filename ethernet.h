// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_ETHERNET_
#define SHILL_ETHERNET_

#include <string>

#include "shill/device.h"
#include "shill/shill_event.h"
#include "shill/ethernet_service.h"

namespace shill {

class Ethernet : public Device {
 public:
  explicit Ethernet(ControlInterface *control_interface,
                    EventDispatcher *dispatcher,
                    Manager *manager,
                    const std::string& link_name,
                    int interface_index);
  ~Ethernet();
  void Start();
  void Stop();
  bool TechnologyIs(Device::Technology type);
  void LinkEvent(unsigned flags, unsigned change);
  bool link_up_;

 private:
  bool service_registered_;
  ServiceRefPtr service_;
  DISALLOW_COPY_AND_ASSIGN(Ethernet);
};

}  // namespace shill

#endif  // SHILL_ETHERNET_
