// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_MOCK_PROFILE_
#define SHILL_MOCK_PROFILE_

#include <string>

#include <gmock/gmock.h>

#include "shill/profile.h"

namespace shill {

class MockProfile : public Profile {
 public:
  MockProfile(ControlInterface *control, Metrics *metrics, Manager *manager);
  MockProfile(ControlInterface *control,
              Metrics *metrics,
              Manager *manager,
              const std::string &identifier);
  virtual ~MockProfile();

  MOCK_METHOD1(AdoptService, bool(const ServiceRefPtr &service));
  MOCK_METHOD1(AbandonService, bool(const ServiceRefPtr &service));
  MOCK_METHOD1(LoadService,  bool(const ServiceRefPtr &service));
  MOCK_METHOD1(ConfigureService,  bool(const ServiceRefPtr &service));
  MOCK_METHOD1(ConfigureDevice, bool(const DeviceRefPtr &device));
  MOCK_METHOD2(DeleteEntry,  void(const std::string &entry_name, Error *error));
  MOCK_METHOD0(GetRpcIdentifier, std::string());
  MOCK_METHOD1(GetStoragePath, bool(base::FilePath *filepath));
  MOCK_METHOD1(UpdateService, bool(const ServiceRefPtr &service));
  MOCK_METHOD1(UpdateDevice, bool(const DeviceRefPtr &device));
  MOCK_METHOD0(Save, bool());
  MOCK_CONST_METHOD0(GetConstStorage, const StoreInterface *());

 private:
  DISALLOW_COPY_AND_ASSIGN(MockProfile);
};

}  // namespace shill

#endif  // SHILL_MOCK_PROFILE_
