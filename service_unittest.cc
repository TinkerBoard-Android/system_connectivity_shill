// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/service.h"

#include <map>
#include <string>
#include <vector>

#include <chromeos/dbus/service_constants.h>
#include <dbus-c++/dbus.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "shill/dbus_adaptor.h"
#include "shill/ethernet_service.h"
#include "shill/event_dispatcher.h"
#include "shill/manager.h"
#include "shill/mock_adaptors.h"
#include "shill/mock_control.h"
#include "shill/mock_manager.h"
#include "shill/mock_store.h"
#include "shill/property_store_unittest.h"
#include "shill/service_under_test.h"

using std::map;
using std::string;
using std::vector;
using testing::_;
using testing::AtLeast;
using testing::NiceMock;
using testing::Return;
using testing::StrictMock;
using testing::Test;

namespace shill {

class ServiceTest : public PropertyStoreTest {
 public:
  ServiceTest()
      : mock_manager_(control_interface(), dispatcher(), glib()),
        service_(new ServiceUnderTest(control_interface(),
                                      dispatcher(),
                                      &mock_manager_)),
        storage_id_(ServiceUnderTest::kStorageId) {
  }

  virtual ~ServiceTest() {}

 protected:
  MockManager mock_manager_;
  scoped_refptr<ServiceUnderTest> service_;
  string storage_id_;
};

TEST_F(ServiceTest, Constructor) {
  EXPECT_TRUE(service_->save_credentials_);
  EXPECT_EQ(Service::kCheckPortalAuto, service_->check_portal_);
}

TEST_F(ServiceTest, GetProperties) {
  map<string, ::DBus::Variant> props;
  Error error(Error::kInvalidProperty, "");
  {
    ::DBus::Error dbus_error;
    string expected("portal_list");
    service_->mutable_store()->SetStringProperty(flimflam::kCheckPortalProperty,
                                                 expected,
                                                 &error);
    DBusAdaptor::GetProperties(service_->store(), &props, &dbus_error);
    ASSERT_FALSE(props.find(flimflam::kCheckPortalProperty) == props.end());
    EXPECT_EQ(props[flimflam::kCheckPortalProperty].reader().get_string(),
              expected);
  }
  {
    ::DBus::Error dbus_error;
    bool expected = true;
    service_->mutable_store()->SetBoolProperty(flimflam::kAutoConnectProperty,
                                               expected,
                                               &error);
    DBusAdaptor::GetProperties(service_->store(), &props, &dbus_error);
    ASSERT_FALSE(props.find(flimflam::kAutoConnectProperty) == props.end());
    EXPECT_EQ(props[flimflam::kAutoConnectProperty].reader().get_bool(),
              expected);
  }
  {
    ::DBus::Error dbus_error;
    DBusAdaptor::GetProperties(service_->store(), &props, &dbus_error);
    ASSERT_FALSE(props.find(flimflam::kConnectableProperty) == props.end());
    EXPECT_EQ(props[flimflam::kConnectableProperty].reader().get_bool(), false);
  }
  {
    ::DBus::Error dbus_error;
    int32 expected = 127;
    service_->mutable_store()->SetInt32Property(flimflam::kPriorityProperty,
                                                expected,
                                                &error);
    DBusAdaptor::GetProperties(service_->store(), &props, &dbus_error);
    ASSERT_FALSE(props.find(flimflam::kPriorityProperty) == props.end());
    EXPECT_EQ(props[flimflam::kPriorityProperty].reader().get_int32(),
              expected);
  }
  {
    ::DBus::Error dbus_error;
    DBusAdaptor::GetProperties(service_->store(), &props, &dbus_error);
    ASSERT_FALSE(props.find(flimflam::kDeviceProperty) == props.end());
    EXPECT_EQ(props[flimflam::kDeviceProperty].reader().get_string(),
              string(ServiceUnderTest::kRpcId));
  }
}

TEST_F(ServiceTest, Dispatch) {
  {
    ::DBus::Error error;
    EXPECT_TRUE(DBusAdaptor::DispatchOnType(service_->mutable_store(),
                                            flimflam::kSaveCredentialsProperty,
                                            PropertyStoreTest::kBoolV,
                                            &error));
  }
  {
    ::DBus::Error error;
    EXPECT_TRUE(DBusAdaptor::DispatchOnType(service_->mutable_store(),
                                            flimflam::kPriorityProperty,
                                            PropertyStoreTest::kInt32V,
                                            &error));
  }
  {
    ::DBus::Error error;
    EXPECT_TRUE(DBusAdaptor::DispatchOnType(service_->mutable_store(),
                                            flimflam::kEAPEAPProperty,
                                            PropertyStoreTest::kStringV,
                                            &error));
  }
  // Ensure that an attempt to write a R/O property returns InvalidArgs error.
  {
    ::DBus::Error error;
    EXPECT_FALSE(DBusAdaptor::DispatchOnType(service_->mutable_store(),
                                             flimflam::kFavoriteProperty,
                                             PropertyStoreTest::kBoolV,
                                             &error));
    EXPECT_EQ(invalid_args(), error.name());
  }
}

TEST_F(ServiceTest, Load) {
  NiceMock<MockStore> storage;
  EXPECT_CALL(storage, ContainsGroup(storage_id_)).WillOnce(Return(true));
  EXPECT_CALL(storage, GetString(storage_id_, _, _))
      .Times(AtLeast(1))
      .WillRepeatedly(Return(true));
  EXPECT_TRUE(service_->Load(&storage));
}

TEST_F(ServiceTest, LoadFail) {
  StrictMock<MockStore> storage;
  EXPECT_CALL(storage, ContainsGroup(storage_id_)).WillOnce(Return(false));
  EXPECT_FALSE(service_->Load(&storage));
}

TEST_F(ServiceTest, SaveString) {
  MockStore storage;
  static const char kKey[] = "test-key";
  static const char kData[] = "test-data";
  EXPECT_CALL(storage, SetString(storage_id_, kKey, kData))
      .WillOnce(Return(true));
  service_->SaveString(&storage, storage_id_, kKey, kData, false, true);
}

TEST_F(ServiceTest, SaveStringCrypted) {
  MockStore storage;
  static const char kKey[] = "test-key";
  static const char kData[] = "test-data";
  EXPECT_CALL(storage, SetCryptedString(storage_id_, kKey, kData))
      .WillOnce(Return(true));
  service_->SaveString(&storage, storage_id_, kKey, kData, true, true);
}

TEST_F(ServiceTest, SaveStringDontSave) {
  MockStore storage;
  static const char kKey[] = "test-key";
  EXPECT_CALL(storage, DeleteKey(storage_id_, kKey))
      .WillOnce(Return(true));
  service_->SaveString(&storage, storage_id_, kKey, "data", false, false);
}

TEST_F(ServiceTest, SaveStringEmpty) {
  MockStore storage;
  static const char kKey[] = "test-key";
  EXPECT_CALL(storage, DeleteKey(storage_id_, kKey))
      .WillOnce(Return(true));
  service_->SaveString(&storage, storage_id_, kKey, "", true, true);
}

TEST_F(ServiceTest, Save) {
  NiceMock<MockStore> storage;
  EXPECT_CALL(storage, SetString(storage_id_, _, _))
      .Times(AtLeast(1))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(storage, DeleteKey(storage_id_, _))
      .Times(AtLeast(1))
      .WillRepeatedly(Return(true));
  EXPECT_TRUE(service_->Save(&storage));
}

TEST_F(ServiceTest, State) {
  EXPECT_EQ(Service::kStateUnknown, service_->state());
  EXPECT_EQ(Service::kFailureUnknown, service_->failure());

  ServiceConstRefPtr service_ref(service_);

  // TODO(quiche): make this EXPECT_CALL work (crosbug.com/20154)
  // EXPECT_CALL(*dynamic_cast<ServiceMockAdaptor *>(service_->adaptor_.get()),
  //     EmitStringChanged(flimflam::kStateProperty, _));
  EXPECT_CALL(mock_manager_, UpdateService(service_ref));
  service_->SetState(Service::kStateConnected);
  // A second state change shouldn't cause another update
  service_->SetState(Service::kStateConnected);

  EXPECT_EQ(Service::kStateConnected, service_->state());
  EXPECT_EQ(Service::kFailureUnknown, service_->failure());
  EXPECT_CALL(mock_manager_, UpdateService(service_ref));
  service_->SetState(Service::kStateDisconnected);

  EXPECT_CALL(mock_manager_, UpdateService(service_ref));
  service_->SetFailure(Service::kFailureOutOfRange);

  EXPECT_EQ(Service::kStateFailure, service_->state());
  EXPECT_EQ(Service::kFailureOutOfRange, service_->failure());
}

TEST_F(ServiceTest, ActivateCellularModem) {
  Error error;
  service_->ActivateCellularModem("Carrier", &error);
  EXPECT_EQ(Error::kInvalidArguments, error.type());
}

}  // namespace shill
