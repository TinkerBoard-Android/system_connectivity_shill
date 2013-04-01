// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/cellular_service.h"

#include <chromeos/dbus/service_constants.h>
#include <gtest/gtest.h>
#include <mm/mm-modem.h>

#include "shill/cellular_capability.h"
#include "shill/cellular_capability_cdma.h"
#include "shill/mock_adaptors.h"
#include "shill/mock_cellular.h"
#include "shill/mock_manager.h"
#include "shill/mock_metrics.h"
#include "shill/mock_modem_info.h"
#include "shill/mock_profile.h"
#include "shill/mock_store.h"
#include "shill/nice_mock_control.h"
#include "shill/proxy_factory.h"
#include "shill/service_property_change_test.h"

using std::string;
using testing::_;
using testing::InSequence;
using testing::Mock;
using testing::NiceMock;
using testing::Return;

namespace shill {

class CellularServiceTest : public testing::Test {
 public:
  CellularServiceTest()
      : modem_info_(NULL, &dispatcher_, NULL, NULL, NULL),
        device_(new MockCellular(&modem_info_,
                                 "usb0",
                                 kAddress,
                                 3,
                                 Cellular::kTypeCDMA,
                                 "",
                                 "",
                                 "",
                                 ProxyFactory::GetInstance())),
        service_(new CellularService(&modem_info_, device_)),
        adaptor_(NULL) {}

  virtual ~CellularServiceTest() {
    adaptor_ = NULL;
  }

  virtual void SetUp() {
    adaptor_ =
        dynamic_cast<ServiceMockAdaptor *>(service_->adaptor());
  }

  CellularCapabilityCDMA *GetCapabilityCDMA() {
    return dynamic_cast<CellularCapabilityCDMA *>(device_->capability_.get());
  }

 protected:
  static const char kAddress[];

  string GetFriendlyName() const { return service_->friendly_name(); }

  EventDispatcher dispatcher_;
  MockModemInfo modem_info_;
  scoped_refptr<MockCellular> device_;
  CellularServiceRefPtr service_;
  ServiceMockAdaptor *adaptor_;  // Owned by |service_|.
};

const char CellularServiceTest::kAddress[] = "000102030405";

TEST_F(CellularServiceTest, Constructor) {
  EXPECT_TRUE(service_->connectable());
}

TEST_F(CellularServiceTest, SetActivationState) {
  {
    InSequence call_sequence;
    EXPECT_CALL(*adaptor_, EmitStringChanged(
        flimflam::kActivationStateProperty,
        flimflam::kActivationStateNotActivated));
    EXPECT_CALL(*adaptor_, EmitBoolChanged(
        flimflam::kConnectableProperty, false));
    EXPECT_CALL(*adaptor_, EmitStringChanged(
        flimflam::kActivationStateProperty,
        flimflam::kActivationStateActivating));
    EXPECT_CALL(*adaptor_, EmitBoolChanged(
        flimflam::kConnectableProperty, true));
    EXPECT_CALL(*adaptor_, EmitStringChanged(
        flimflam::kActivationStateProperty,
        flimflam::kActivationStatePartiallyActivated));
    EXPECT_CALL(*adaptor_, EmitStringChanged(
        flimflam::kActivationStateProperty,
        flimflam::kActivationStateActivated));
    EXPECT_CALL(*adaptor_, EmitStringChanged(
        flimflam::kActivationStateProperty,
        flimflam::kActivationStateNotActivated));
    EXPECT_CALL(*adaptor_, EmitBoolChanged(
        flimflam::kConnectableProperty, false));
  }
  EXPECT_CALL(*modem_info_.mock_manager(), HasService(_))
      .WillRepeatedly(Return(false));

  EXPECT_TRUE(service_->activation_state().empty());
  EXPECT_TRUE(service_->connectable());

  service_->SetActivationState(flimflam::kActivationStateNotActivated);
  EXPECT_EQ(flimflam::kActivationStateNotActivated,
            service_->activation_state());
  EXPECT_FALSE(service_->connectable());

  service_->SetActivationState(flimflam::kActivationStateActivating);
  EXPECT_EQ(flimflam::kActivationStateActivating, service_->activation_state());
  EXPECT_TRUE(service_->connectable());

  service_->SetActivationState(flimflam::kActivationStatePartiallyActivated);
  EXPECT_EQ(flimflam::kActivationStatePartiallyActivated,
            service_->activation_state());
  EXPECT_TRUE(service_->connectable());

  service_->SetActivationState(flimflam::kActivationStateActivated);
  EXPECT_EQ(flimflam::kActivationStateActivated, service_->activation_state());
  EXPECT_TRUE(service_->connectable());

  service_->SetActivationState(flimflam::kActivationStateNotActivated);
  EXPECT_EQ(flimflam::kActivationStateNotActivated,
            service_->activation_state());
  EXPECT_FALSE(service_->connectable());
}

TEST_F(CellularServiceTest, SetNetworkTechnology) {
  EXPECT_CALL(*adaptor_, EmitStringChanged(flimflam::kNetworkTechnologyProperty,
                                           flimflam::kNetworkTechnologyUmts));
  EXPECT_TRUE(service_->network_technology().empty());
  service_->SetNetworkTechnology(flimflam::kNetworkTechnologyUmts);
  EXPECT_EQ(flimflam::kNetworkTechnologyUmts, service_->network_technology());
  service_->SetNetworkTechnology(flimflam::kNetworkTechnologyUmts);
}

TEST_F(CellularServiceTest, SetRoamingState) {
  EXPECT_CALL(*adaptor_, EmitStringChanged(flimflam::kRoamingStateProperty,
                                           flimflam::kRoamingStateHome));
  EXPECT_TRUE(service_->roaming_state().empty());
  service_->SetRoamingState(flimflam::kRoamingStateHome);
  EXPECT_EQ(flimflam::kRoamingStateHome, service_->roaming_state());
  service_->SetRoamingState(flimflam::kRoamingStateHome);
}

TEST_F(CellularServiceTest, FriendlyName) {
  static const char kCarrier[] = "Cellular Carrier";
  GetCapabilityCDMA()->carrier_ = kCarrier;
  service_ = new CellularService(&modem_info_, device_);
  EXPECT_EQ(kCarrier, GetFriendlyName());
}

TEST_F(CellularServiceTest, SetStorageIdentifier) {
  EXPECT_EQ(string(flimflam::kTypeCellular) + "_" +
            kAddress + "_" + GetFriendlyName(),
            service_->GetStorageIdentifier());
  service_->SetStorageIdentifier("a b c");
  EXPECT_EQ("a_b_c", service_->GetStorageIdentifier());
}

TEST_F(CellularServiceTest, SetServingOperator) {
  EXPECT_CALL(*adaptor_,
              EmitStringmapChanged(flimflam::kServingOperatorProperty, _));
  static const char kCode[] = "123456";
  static const char kName[] = "Some Cellular Operator";
  Cellular::Operator oper;
  service_->SetServingOperator(oper);
  oper.SetCode(kCode);
  oper.SetName(kName);
  service_->SetServingOperator(oper);
  EXPECT_EQ(kCode, service_->serving_operator().GetCode());
  EXPECT_EQ(kName, service_->serving_operator().GetName());
  service_->SetServingOperator(oper);
}

TEST_F(CellularServiceTest, SetOLP) {
  EXPECT_CALL(*adaptor_,
              EmitStringmapChanged(flimflam::kPaymentPortalProperty, _));
  static const char kURL[] = "payment.url";
  static const char kMethod[] = "GET";
  CellularService::OLP olp;
  service_->SetOLP(olp);
  olp.SetURL(kURL);
  olp.SetMethod(kMethod);
  service_->SetOLP(olp);
  EXPECT_EQ(kURL, service_->olp().GetURL());
  EXPECT_EQ(kMethod, service_->olp().GetMethod());
  service_->SetOLP(olp);
}

TEST_F(CellularServiceTest, SetUsageURL) {
  static const char kUsageURL[] = "usage.url";
  EXPECT_CALL(*adaptor_, EmitStringChanged(flimflam::kUsageURLProperty,
                                           kUsageURL));
  EXPECT_TRUE(service_->usage_url().empty());
  service_->SetUsageURL(kUsageURL);
  EXPECT_EQ(kUsageURL, service_->usage_url());
  service_->SetUsageURL(kUsageURL);
}

TEST_F(CellularServiceTest, SetApn) {
  static const char kApn[] = "TheAPN";
  static const char kUsername[] = "commander.data";
  ProfileRefPtr profile(new NiceMock<MockProfile>(
      modem_info_.control_interface(), modem_info_.metrics(),
      modem_info_.manager()));
  service_->set_profile(profile);
  Error error;
  Stringmap testapn;
  testapn[flimflam::kApnProperty] = kApn;
  testapn[flimflam::kApnUsernameProperty] = kUsername;
  {
    InSequence seq;
    EXPECT_CALL(*adaptor_,
                EmitStringmapChanged(flimflam::kCellularLastGoodApnProperty,
                                     _));
    EXPECT_CALL(*adaptor_,
                EmitStringmapChanged(flimflam::kCellularApnProperty, _));
  }
  service_->SetApn(testapn, &error);
  EXPECT_TRUE(error.IsSuccess());
  Stringmap resultapn = service_->GetApn(&error);
  EXPECT_TRUE(error.IsSuccess());
  EXPECT_EQ(2, resultapn.size());
  Stringmap::const_iterator it = resultapn.find(flimflam::kApnProperty);
  EXPECT_TRUE(it != resultapn.end() && it->second == kApn);
  it = resultapn.find(flimflam::kApnUsernameProperty);
  EXPECT_TRUE(it != resultapn.end() && it->second == kUsername);
  EXPECT_FALSE(service_->GetUserSpecifiedApn() == NULL);
}

TEST_F(CellularServiceTest, ClearApn) {
  static const char kApn[] = "TheAPN";
  static const char kUsername[] = "commander.data";
  ProfileRefPtr profile(new NiceMock<MockProfile>(
      modem_info_.control_interface(), modem_info_.metrics(),
      modem_info_.manager()));
  service_->set_profile(profile);
  Error error;
  // Set up an APN to make sure that it later gets cleared.
  Stringmap testapn;
  testapn[flimflam::kApnProperty] = kApn;
  testapn[flimflam::kApnUsernameProperty] = kUsername;
  {
    InSequence seq;
    EXPECT_CALL(*adaptor_,
                EmitStringmapChanged(flimflam::kCellularLastGoodApnProperty,
                                     _));
    EXPECT_CALL(*adaptor_,
                EmitStringmapChanged(flimflam::kCellularApnProperty, _));
  }
  service_->SetApn(testapn, &error);
  Stringmap resultapn = service_->GetApn(&error);
  ASSERT_TRUE(error.IsSuccess());
  ASSERT_EQ(2, service_->GetApn(&error).size());

  Stringmap emptyapn;
  EXPECT_CALL(*adaptor_,
              EmitStringmapChanged(flimflam::kCellularLastGoodApnProperty,
                                   _)).Times(0);
  EXPECT_CALL(*adaptor_,
              EmitStringmapChanged(flimflam::kCellularApnProperty, _)).Times(1);
  service_->SetApn(emptyapn, &error);
  EXPECT_TRUE(error.IsSuccess());
  resultapn = service_->GetApn(&error);
  EXPECT_TRUE(resultapn.empty());
  EXPECT_TRUE(service_->GetUserSpecifiedApn() == NULL);
}

TEST_F(CellularServiceTest, LastGoodApn) {
  static const char kApn[] = "TheAPN";
  static const char kUsername[] = "commander.data";
  ProfileRefPtr profile(new NiceMock<MockProfile>(
      modem_info_.control_interface(), modem_info_.metrics(),
      modem_info_.manager()));
  service_->set_profile(profile);
  Stringmap testapn;
  testapn[flimflam::kApnProperty] = kApn;
  testapn[flimflam::kApnUsernameProperty] = kUsername;
  EXPECT_CALL(*adaptor_,
              EmitStringmapChanged(flimflam::kCellularLastGoodApnProperty, _));
  service_->SetLastGoodApn(testapn);
  Stringmap *resultapn = service_->GetLastGoodApn();
  EXPECT_FALSE(resultapn == NULL);
  EXPECT_EQ(2, resultapn->size());
  Stringmap::const_iterator it = resultapn->find(flimflam::kApnProperty);
  EXPECT_TRUE(it != resultapn->end() && it->second == kApn);
  it = resultapn->find(flimflam::kApnUsernameProperty);
  EXPECT_TRUE(it != resultapn->end() && it->second == kUsername);
  // Now set the user-specified APN, and check that LastGoodApn got
  // cleared.
  Stringmap userapn;
  userapn[flimflam::kApnProperty] = kApn;
  userapn[flimflam::kApnUsernameProperty] = kUsername;
  {
    InSequence seq;
    EXPECT_CALL(*adaptor_,
                EmitStringmapChanged(flimflam::kCellularLastGoodApnProperty,
                                     _));
    EXPECT_CALL(*adaptor_,
                EmitStringmapChanged(flimflam::kCellularApnProperty, _));
  }
  Error error;
  service_->SetApn(userapn, &error);
  EXPECT_TRUE(service_->GetLastGoodApn() == NULL);
}

TEST_F(CellularServiceTest, IsAutoConnectable) {
  const char *reason = NULL;

  // Auto-connect should be suppressed if the device is not running.
  device_->running_ = false;
  EXPECT_FALSE(service_->IsAutoConnectable(&reason));
  EXPECT_STREQ(CellularService::kAutoConnDeviceDisabled, reason);

  device_->running_ = true;

  // If we're waiting on a disconnect before an activation, don't auto-connect.
  GetCapabilityCDMA()->activation_starting_ = true;
  EXPECT_FALSE(service_->IsAutoConnectable(&reason));

  // If we're waiting on an activation, also don't auto-connect.
  GetCapabilityCDMA()->activation_starting_ = false;
  GetCapabilityCDMA()->activation_state_ =
      MM_MODEM_CDMA_ACTIVATION_STATE_ACTIVATING;
  EXPECT_FALSE(service_->IsAutoConnectable(&reason));

  GetCapabilityCDMA()->activation_state_ =
      MM_MODEM_CDMA_ACTIVATION_STATE_ACTIVATED;

  // Auto-connect should be suppressed if the we're undergoing an
  // out-of-credits detection.
  service_->out_of_credits_detection_in_progress_ = true;
  EXPECT_FALSE(service_->IsAutoConnectable(&reason));
  EXPECT_STREQ(CellularService::kAutoConnOutOfCreditsDetectionInProgress,
               reason);

  // Auto-connect should be suppressed if we're out of credits.
  service_->out_of_credits_detection_in_progress_ = false;
  service_->out_of_credits_ = true;
  EXPECT_FALSE(service_->IsAutoConnectable(&reason));
  EXPECT_STREQ(CellularService::kAutoConnOutOfCredits, reason);

  service_->out_of_credits_ = false;

  // But other activation states are fine.
  GetCapabilityCDMA()->activation_state_ =
      MM_MODEM_CDMA_ACTIVATION_STATE_ACTIVATED;
  EXPECT_TRUE(service_->IsAutoConnectable(&reason));
  GetCapabilityCDMA()->activation_state_ =
      MM_MODEM_CDMA_ACTIVATION_STATE_NOT_ACTIVATED;
  EXPECT_TRUE(service_->IsAutoConnectable(&reason));
  GetCapabilityCDMA()->activation_state_ =
      MM_MODEM_CDMA_ACTIVATION_STATE_PARTIALLY_ACTIVATED;
  EXPECT_TRUE(service_->IsAutoConnectable(&reason));

  // The following test cases are copied from ServiceTest.IsAutoConnectable

  service_->SetConnectable(true);
  EXPECT_TRUE(service_->IsAutoConnectable(&reason));

  // We should not auto-connect to a Service that a user has
  // deliberately disconnected.
  Error error;
  service_->UserInitiatedDisconnect(&error);
  EXPECT_FALSE(service_->IsAutoConnectable(&reason));
  EXPECT_STREQ(Service::kAutoConnExplicitDisconnect, reason);

  // But if the Service is reloaded, it is eligible for auto-connect
  // again.
  NiceMock<MockStore> storage;
  EXPECT_CALL(storage, ContainsGroup(service_->GetStorageIdentifier()))
      .WillOnce(Return(true));
  EXPECT_TRUE(service_->Load(&storage));
  EXPECT_TRUE(service_->IsAutoConnectable(&reason));

  // A non-user initiated Disconnect doesn't change anything.
  service_->Disconnect(&error);
  EXPECT_TRUE(service_->IsAutoConnectable(&reason));

  // A resume also re-enables auto-connect.
  service_->UserInitiatedDisconnect(&error);
  EXPECT_FALSE(service_->IsAutoConnectable(&reason));
  service_->OnAfterResume();
  EXPECT_TRUE(service_->IsAutoConnectable(&reason));

  service_->SetState(Service::kStateConnected);
  EXPECT_FALSE(service_->IsAutoConnectable(&reason));
  EXPECT_STREQ(Service::kAutoConnConnected, reason);

  service_->SetState(Service::kStateAssociating);
  EXPECT_FALSE(service_->IsAutoConnectable(&reason));
  EXPECT_STREQ(Service::kAutoConnConnecting, reason);
}

TEST_F(CellularServiceTest, OutOfCreditsDetected) {
  service_->set_enforce_out_of_credits_detection(true);
  EXPECT_CALL(*device_, Connect(_)).Times(3);
  Error error;
  service_->Connect(&error, "in test");
  service_->SetState(Service::kStateAssociating);
  service_->SetState(Service::kStateFailure);
  EXPECT_TRUE(service_->out_of_credits_detection_in_progress_);
  dispatcher_.DispatchPendingEvents();
  service_->SetState(Service::kStateConfiguring);
  service_->SetState(Service::kStateIdle);
  EXPECT_TRUE(service_->out_of_credits_detection_in_progress_);
  dispatcher_.DispatchPendingEvents();
  service_->SetState(Service::kStateConnected);
  service_->SetState(Service::kStateIdle);
  EXPECT_TRUE(service_->out_of_credits_);
  EXPECT_FALSE(service_->out_of_credits_detection_in_progress_);
}

TEST_F(CellularServiceTest, OutOfCreditsDetectionNotSkippedAfterSlowResume) {
  service_->set_enforce_out_of_credits_detection(true);
  service_->OnAfterResume();
  service_->resume_start_time_ =
      base::Time::Now() -
      base::TimeDelta::FromSeconds(
          CellularService::kOutOfCreditsResumeIgnoreSeconds + 1);
  EXPECT_CALL(*device_, Connect(_)).Times(3);
  Error error;
  service_->Connect(&error, "in test");
  service_->SetState(Service::kStateAssociating);
  service_->SetState(Service::kStateFailure);
  EXPECT_TRUE(service_->out_of_credits_detection_in_progress_);
  dispatcher_.DispatchPendingEvents();
  service_->SetState(Service::kStateConfiguring);
  service_->SetState(Service::kStateIdle);
  EXPECT_TRUE(service_->out_of_credits_detection_in_progress_);
  dispatcher_.DispatchPendingEvents();
  service_->SetState(Service::kStateConnected);
  service_->SetState(Service::kStateIdle);
  EXPECT_TRUE(service_->out_of_credits_);
  EXPECT_FALSE(service_->out_of_credits_detection_in_progress_);
}

TEST_F(CellularServiceTest, OutOfCreditsDetectionSkippedAfterResume) {
  service_->set_enforce_out_of_credits_detection(true);
  service_->OnAfterResume();
  EXPECT_CALL(*device_, Connect(_));
  Error error;
  service_->Connect(&error, "in test");
  service_->SetState(Service::kStateConnected);
  service_->SetState(Service::kStateIdle);
  EXPECT_FALSE(service_->out_of_credits_);
  EXPECT_FALSE(service_->out_of_credits_detection_in_progress_);
  // There should not be any pending connect requests but dispatch pending
  // events anyway to be sure.
  dispatcher_.DispatchPendingEvents();
}

TEST_F(CellularServiceTest, OutOfCreditsDetectionSkippedAlreadyOutOfCredits) {
  service_->set_enforce_out_of_credits_detection(true);
  EXPECT_CALL(*device_, Connect(_));
  Error error;
  service_->Connect(&error, "in test");
  service_->out_of_credits_ = true;
  service_->SetState(Service::kStateConnected);
  service_->SetState(Service::kStateIdle);
  EXPECT_FALSE(service_->out_of_credits_detection_in_progress_);
  // There should not be any pending connect requests but dispatch pending
  // events anyway to be sure.
  dispatcher_.DispatchPendingEvents();
}

TEST_F(CellularServiceTest, OutOfCreditsDetectionSkippedExplicitDisconnect) {
  service_->set_enforce_out_of_credits_detection(true);
  EXPECT_CALL(*device_, Connect(_));
  Error error;
  service_->Connect(&error, "in test");
  service_->SetState(Service::kStateConnected);
  service_->UserInitiatedDisconnect(&error);
  service_->SetState(Service::kStateIdle);
  EXPECT_FALSE(service_->out_of_credits_);
  EXPECT_FALSE(service_->out_of_credits_detection_in_progress_);
  // There should not be any pending connect requests but dispatch pending
  // events anyway to be sure.
  dispatcher_.DispatchPendingEvents();
}

TEST_F(CellularServiceTest, OutOfCreditsNotDetectedConnectionNotDropped) {
  service_->set_enforce_out_of_credits_detection(true);
  EXPECT_CALL(*device_, Connect(_));
  Error error;
  service_->Connect(&error, "in test");
  service_->SetState(Service::kStateAssociating);
  service_->SetState(Service::kStateConfiguring);
  service_->SetState(Service::kStateConnected);
  EXPECT_FALSE(service_->out_of_credits_);
  EXPECT_FALSE(service_->out_of_credits_detection_in_progress_);
  // There should not be any pending connect requests but dispatch pending
  // events anyway to be sure.
  dispatcher_.DispatchPendingEvents();
}

TEST_F(CellularServiceTest, OutOfCreditsNotDetectedIntermittentNetwork) {
  service_->set_enforce_out_of_credits_detection(true);
  EXPECT_CALL(*device_, Connect(_));
  Error error;
  service_->Connect(&error, "in test");
  service_->SetState(Service::kStateConnected);
  service_->connect_start_time_ =
      base::Time::Now() -
      base::TimeDelta::FromSeconds(
          CellularService::kOutOfCreditsConnectionDropSeconds + 1);
  service_->SetState(Service::kStateIdle);
  EXPECT_FALSE(service_->out_of_credits_);
  EXPECT_FALSE(service_->out_of_credits_detection_in_progress_);
  // There should not be any pending connect requests but dispatch pending
  // events anyway to be sure.
  dispatcher_.DispatchPendingEvents();
}

TEST_F(CellularServiceTest, OutOfCreditsNotEnforced) {
  EXPECT_CALL(*device_, Connect(_));
  Error error;
  service_->Connect(&error, "in test");
  service_->SetState(Service::kStateConnected);
  service_->SetState(Service::kStateIdle);
  EXPECT_FALSE(service_->out_of_credits_);
  EXPECT_FALSE(service_->out_of_credits_detection_in_progress_);
  // There should not be any pending connect requests but dispatch pending
  // events anyway to be sure.
  dispatcher_.DispatchPendingEvents();
}

// Some of these tests duplicate signals tested above. However, it's
// convenient to have all the property change notifications documented
// (and tested) in one place.
TEST_F(CellularServiceTest, PropertyChanges) {
  TestCommonPropertyChanges(service_, adaptor_);
  TestAutoConnectPropertyChange(service_, adaptor_);

  bool activate_over_non_cellular =
      service_->activate_over_non_cellular_network();
  EXPECT_CALL(*adaptor_,
              EmitBoolChanged(kActivateOverNonCellularNetworkProperty, _));
  service_->SetActivateOverNonCellularNetwork(!activate_over_non_cellular);
  Mock::VerifyAndClearExpectations(adaptor_);

  EXPECT_NE(flimflam::kActivationStateNotActivated,
            service_->activation_state());
  EXPECT_CALL(*adaptor_,
              EmitStringChanged(flimflam::kActivationStateProperty, _));
  service_->SetActivationState(flimflam::kActivationStateNotActivated);
  Mock::VerifyAndClearExpectations(adaptor_);

  string network_technology = service_->network_technology();
  EXPECT_CALL(*adaptor_,
              EmitStringChanged(flimflam::kNetworkTechnologyProperty, _));
  service_->SetNetworkTechnology(network_technology + "and some new stuff");
  Mock::VerifyAndClearExpectations(adaptor_);

  bool out_of_credits = service_->out_of_credits();
  EXPECT_CALL(*adaptor_, EmitBoolChanged(kOutOfCreditsProperty, _));
  service_->SetOutOfCredits(!out_of_credits);
  Mock::VerifyAndClearExpectations(adaptor_);

  string roaming_state = service_->roaming_state();
  EXPECT_CALL(*adaptor_,
              EmitStringChanged(flimflam::kRoamingStateProperty, _));
  service_->SetRoamingState(roaming_state + "and some new stuff");
  Mock::VerifyAndClearExpectations(adaptor_);
}

}  // namespace shill
