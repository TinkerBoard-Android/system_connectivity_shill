// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_MOCK_MODEM_CDMA_PROXY_H_
#define SHILL_MOCK_MODEM_CDMA_PROXY_H_

#include <gmock/gmock.h>

#include "shill/modem_cdma_proxy_interface.h"

namespace shill {

class MockModemCDMAProxy : public ModemCDMAProxyInterface {
 public:
  MOCK_METHOD2(GetRegistrationState, void(uint32 *cdma_1x_state,
                                          uint32 *evdo_state));
};

}  // namespace shill

#endif  // SHILL_MOCK_MODEM_CDMA_PROXY_H_
