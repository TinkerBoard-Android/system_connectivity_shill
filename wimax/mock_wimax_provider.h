//
// Copyright (C) 2012 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef SHILL_WIMAX_MOCK_WIMAX_PROVIDER_H_
#define SHILL_WIMAX_MOCK_WIMAX_PROVIDER_H_

#include <string>

#include <gmock/gmock.h>

#include "shill/wimax/wimax_provider.h"

namespace shill {

class MockWiMaxProvider : public WiMaxProvider {
 public:
  MockWiMaxProvider();
  ~MockWiMaxProvider() override;

  MOCK_METHOD1(OnDeviceInfoAvailable, void(const std::string& link_name));
  MOCK_METHOD0(OnNetworksChanged, void());
  MOCK_METHOD1(OnServiceUnloaded, bool(const WiMaxServiceRefPtr& service));
  MOCK_METHOD1(SelectCarrier,
               WiMaxRefPtr(const WiMaxServiceConstRefPtr& service));

 private:
  DISALLOW_COPY_AND_ASSIGN(MockWiMaxProvider);
};

}  // namespace shill

#endif  // SHILL_WIMAX_MOCK_WIMAX_PROVIDER_H_
