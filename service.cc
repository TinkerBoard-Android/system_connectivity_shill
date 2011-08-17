// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/service.h"

#include <time.h>
#include <stdio.h>

#include <map>
#include <string>
#include <vector>

#include <base/logging.h>
#include <base/memory/scoped_ptr.h>
#include <base/string_number_conversions.h>
#include <chromeos/dbus/service_constants.h>

#include "shill/control_interface.h"
#include "shill/error.h"
#include "shill/manager.h"
#include "shill/profile.h"
#include "shill/property_accessor.h"
#include "shill/refptr_types.h"
#include "shill/service_dbus_adaptor.h"
#include "shill/store_interface.h"

using std::map;
using std::string;
using std::vector;

namespace shill {

const char Service::kCheckPortalAuto[] = "auto";
const char Service::kCheckPortalFalse[] = "false";
const char Service::kCheckPortalTrue[] = "true";

const char Service::kStorageAutoConnect[] = "AutoConnect";
const char Service::kStorageCheckPortal[] = "CheckPortal";
const char Service::kStorageEapAnonymousIdentity[] = "EAP.AnonymousIdentity";
const char Service::kStorageEapCACert[] = "EAP.CACert";
const char Service::kStorageEapCACertID[] = "EAP.CACertID";
const char Service::kStorageEapCertID[] = "EAP.CertID";
const char Service::kStorageEapClientCert[] = "EAP.ClientCert";
const char Service::kStorageEapEap[] = "EAP.EAP";
const char Service::kStorageEapIdentity[] = "EAP.Identity";
const char Service::kStorageEapInnerEap[] = "EAP.InnerEAP";
const char Service::kStorageEapKeyID[] = "EAP.KeyID";
const char Service::kStorageEapKeyManagement[] = "EAP.KeyMgmt";
const char Service::kStorageEapPIN[] = "EAP.PIN";
const char Service::kStorageEapPassword[] = "EAP.Password";
const char Service::kStorageEapPrivateKey[] = "EAP.PrivateKey";
const char Service::kStorageEapPrivateKeyPassword[] = "EAP.PrivateKeyPassword";
const char Service::kStorageEapUseSystemCAs[] = "EAP.UseSystemCAs";
const char Service::kStorageFavorite[] = "Favorite";
const char Service::kStorageName[] = "Name";
const char Service::kStoragePriority[] = "Priority";
const char Service::kStorageProxyConfig[] = "ProxyConfig";
const char Service::kStorageSaveCredentials[] = "SaveCredentials";

// static
unsigned int Service::serial_number_ = 0;

Service::Service(ControlInterface *control_interface,
                 EventDispatcher *dispatcher,
                 Manager *manager)
    : auto_connect_(false),
      check_portal_(kCheckPortalAuto),
      connectable_(false),
      favorite_(false),
      priority_(kPriorityNone),
      save_credentials_(true),
      dispatcher_(dispatcher),
      name_(base::UintToString(serial_number_++)),
      available_(false),
      configured_(false),
      configuration_(NULL),
      connection_(NULL),
      adaptor_(control_interface->CreateServiceAdaptor(this)),
      manager_(manager) {

  store_.RegisterBool(flimflam::kAutoConnectProperty, &auto_connect_);

  // flimflam::kActivationStateProperty: Registered in CellularService
  // flimflam::kCellularApnProperty: Registered in CellularService
  // flimflam::kCellularLastGoodApnProperty: Registered in CellularService
  // flimflam::kNetworkTechnologyProperty: Registered in CellularService
  // flimflam::kOperatorNameProperty: Registered in CellularService
  // flimflam::kOperatorCodeProperty: Registered in CellularService
  // flimflam::kRoamingStateProperty: Registered in CellularService
  // flimflam::kPaymentURLProperty: Registered in CellularService

  store_.RegisterString(flimflam::kCheckPortalProperty, &check_portal_);
  store_.RegisterConstBool(flimflam::kConnectableProperty, &connectable_);
  HelpRegisterDerivedString(flimflam::kDeviceProperty,
                            &Service::GetDeviceRpcId,
                            NULL);

  store_.RegisterString(flimflam::kEapIdentityProperty, &eap_.identity);
  store_.RegisterString(flimflam::kEAPEAPProperty, &eap_.eap);
  store_.RegisterString(flimflam::kEapPhase2AuthProperty, &eap_.inner_eap);
  store_.RegisterString(flimflam::kEapAnonymousIdentityProperty,
                        &eap_.anonymous_identity);
  store_.RegisterString(flimflam::kEAPClientCertProperty, &eap_.client_cert);
  store_.RegisterString(flimflam::kEAPCertIDProperty, &eap_.cert_id);
  store_.RegisterString(flimflam::kEapPrivateKeyProperty, &eap_.private_key);
  store_.RegisterString(flimflam::kEapPrivateKeyPasswordProperty,
                        &eap_.private_key_password);
  store_.RegisterString(flimflam::kEAPKeyIDProperty, &eap_.key_id);
  store_.RegisterString(flimflam::kEapCaCertProperty, &eap_.ca_cert);
  store_.RegisterString(flimflam::kEapCaCertIDProperty, &eap_.ca_cert_id);
  store_.RegisterString(flimflam::kEAPPINProperty, &eap_.pin);
  store_.RegisterString(flimflam::kEapPasswordProperty, &eap_.password);
  store_.RegisterString(flimflam::kEapKeyMgmtProperty, &eap_.key_management);
  store_.RegisterBool(flimflam::kEapUseSystemCAsProperty, &eap_.use_system_cas);

  store_.RegisterConstString(flimflam::kErrorProperty, &error_);
  store_.RegisterConstBool(flimflam::kFavoriteProperty, &favorite_);
  HelpRegisterDerivedBool(flimflam::kIsActiveProperty,
                          &Service::IsActive,
                          NULL);
  // flimflam::kModeProperty: Registered in WiFiService
  store_.RegisterConstString(flimflam::kNameProperty, &name_);
  // flimflam::kPassphraseProperty: Registered in WiFiService
  // flimflam::kPassphraseRequiredProperty: Registered in WiFiService
  store_.RegisterInt32(flimflam::kPriorityProperty, &priority_);
  HelpRegisterDerivedString(flimflam::kProfileProperty,
                            &Service::GetProfileRpcId,
                            NULL);
  store_.RegisterString(flimflam::kProxyConfigProperty, &proxy_config_);
  // TODO(cmasone): Create VPN Service with this property
  // store_.RegisterConstStringmap(flimflam::kProviderProperty, &provider_);

  store_.RegisterBool(flimflam::kSaveCredentialsProperty, &save_credentials_);
  // flimflam::kSecurityProperty: Registered in WiFiService
  HelpRegisterDerivedString(flimflam::kStateProperty,
                            &Service::CalculateState,
                            NULL);
  // flimflam::kSignalStrengthProperty: Registered in WiFi/CellularService
  // flimflam::kTypeProperty: Registered in all derived classes.
  // flimflam::kWifiAuthMode: Registered in WiFiService
  // flimflam::kWifiHiddenSsid: Registered in WiFiService
  // flimflam::kWifiFrequency: Registered in WiFiService
  // flimflam::kWifiPhyMode: Registered in WiFiService
  // flimflam::kWifiHexSsid: Registered in WiFiService
  VLOG(2) << "Service initialized.";
}

Service::~Service() {}

void Service::ActivateCellularModem(const std::string &carrier) {
  NOTREACHED() << "Attempt to activate a non-cellular service?";
}

string Service::GetRpcIdentifier() const {
  return adaptor_->GetRpcIdentifier();
}

string Service::GetStorageIdentifier() {
  return UniqueName();
}

void Service::HelpRegisterDerivedBool(const string &name,
                                  bool(Service::*get)(void),
                                  bool(Service::*set)(const bool&)) {
  store_.RegisterDerivedBool(
      name,
      BoolAccessor(new CustomAccessor<Service, bool>(this, get, set)));
}

void Service::HelpRegisterDerivedString(const string &name,
                                    string(Service::*get)(void),
                                    bool(Service::*set)(const string&)) {
  store_.RegisterDerivedString(
      name,
      StringAccessor(new CustomAccessor<Service, string>(this, get, set)));
}

void Service::SaveString(StoreInterface *storage,
                         const string &key,
                         const string &value,
                         bool crypted,
                         bool save) {
  if (value.empty() || !save) {
    storage->DeleteKey(GetStorageIdentifier(), key);
    return;
  }
  if (crypted) {
    storage->SetCryptedString(GetStorageIdentifier(), key, value);
    return;
  }
  storage->SetString(GetStorageIdentifier(), key, value);
}

void Service::LoadEapCredentials(StoreInterface *storage) {
  const string id = GetStorageIdentifier();
  storage->GetCryptedString(id, kStorageEapIdentity, &eap_.identity);
  storage->GetString(id, kStorageEapEap, &eap_.eap);
  storage->GetString(id, kStorageEapInnerEap, &eap_.inner_eap);
  storage->GetCryptedString(id,
                            kStorageEapAnonymousIdentity,
                            &eap_.anonymous_identity);
  storage->GetString(id, kStorageEapClientCert, &eap_.client_cert);
  storage->GetString(id, kStorageEapCertID, &eap_.cert_id);
  storage->GetString(id, kStorageEapPrivateKey, &eap_.private_key);
  storage->GetCryptedString(id,
                            kStorageEapPrivateKeyPassword,
                            &eap_.private_key_password);
  storage->GetString(id, kStorageEapKeyID, &eap_.key_id);
  storage->GetString(id, kStorageEapCACert, &eap_.ca_cert);
  storage->GetString(id, kStorageEapCACertID, &eap_.ca_cert_id);
  storage->GetBool(id, kStorageEapUseSystemCAs, &eap_.use_system_cas);
  storage->GetString(id, kStorageEapPIN, &eap_.pin);
  storage->GetCryptedString(id, kStorageEapPassword, &eap_.password);
  storage->GetString(id, kStorageEapKeyManagement, &eap_.key_management);
}

void Service::SaveEapCredentials(StoreInterface *storage) {
  bool save = save_credentials_;
  SaveString(storage, kStorageEapIdentity, eap_.identity, true, save);
  SaveString(storage, kStorageEapEap, eap_.eap, false, true);
  SaveString(storage, kStorageEapInnerEap, eap_.inner_eap, false, true);
  SaveString(storage,
             kStorageEapAnonymousIdentity,
             eap_.anonymous_identity,
             true,
             save);
  SaveString(storage, kStorageEapClientCert, eap_.client_cert, false, save);
  SaveString(storage, kStorageEapCertID, eap_.cert_id, false, save);
  SaveString(storage, kStorageEapPrivateKey, eap_.private_key, false, save);
  SaveString(storage,
             kStorageEapPrivateKeyPassword,
             eap_.private_key_password,
             true,
             save);
  SaveString(storage, kStorageEapKeyID, eap_.key_id, false, save);
  SaveString(storage, kStorageEapCACert, eap_.ca_cert, false, true);
  SaveString(storage, kStorageEapCACertID, eap_.ca_cert_id, false, true);
  storage->SetBool(GetStorageIdentifier(),
                   kStorageEapUseSystemCAs,
                   eap_.use_system_cas);
  SaveString(storage, kStorageEapPIN, eap_.pin, false, save);
  SaveString(storage, kStorageEapPassword, eap_.password, true, save);
  SaveString(storage,
             kStorageEapKeyManagement,
             eap_.key_management,
             false,
             true);
}

bool Service::Load(StoreInterface *storage) {
  const string id = GetStorageIdentifier();
  if (!storage->ContainsGroup(id)) {
    LOG(WARNING) << "Service is not available in the persistent store: " << id;
    return false;
  }
  storage->GetBool(id, kStorageAutoConnect, &auto_connect_);
  storage->GetString(id, kStorageCheckPortal, &check_portal_);
  storage->GetBool(id, kStorageFavorite, &favorite_);
  storage->GetInt(id, kStoragePriority, &priority_);
  storage->GetString(id, kStorageProxyConfig, &proxy_config_);
  storage->GetBool(id, kStorageSaveCredentials, &save_credentials_);

  LoadEapCredentials(storage);

  // TODO(petkov): Load these:

  // "Name"
  // "WiFi.HiddenSSID"
  // "SSID"
  // "Failure"
  // "Modified"
  // "LastAttempt"
  // WiFiService: "Passphrase"
  // "APN"
  // "LastGoodAPN"

  return true;
}

bool Service::Save(StoreInterface *storage) {
  const string id = GetStorageIdentifier();

  // TODO(petkov): We could choose to simplify the saving code by removing most
  // conditionals thus saving even default values.
  if (favorite_) {
    storage->SetBool(id, kStorageAutoConnect, auto_connect_);
  }
  if (check_portal_ == kCheckPortalAuto) {
    storage->DeleteKey(id, kStorageCheckPortal);
  } else {
    storage->SetString(id, kStorageCheckPortal, check_portal_);
  }
  storage->SetBool(id, kStorageFavorite, favorite_);
  storage->SetString(id, kStorageName, name_);
  SaveString(storage, kStorageProxyConfig, proxy_config_, false, true);
  if (priority_ != kPriorityNone) {
    storage->SetInt(id, kStoragePriority, priority_);
  } else {
    storage->DeleteKey(id, kStoragePriority);
  }
  if (save_credentials_) {
    storage->DeleteKey(id, kStorageSaveCredentials);
  } else {
    storage->SetBool(id, kStorageSaveCredentials, false);
  }

  SaveEapCredentials(storage);

  // TODO(petkov): Save these:

  // "WiFi.HiddenSSID"
  // "SSID"
  // "Failure"
  // "Modified"
  // "LastAttempt"
  // WiFiService: "Passphrase"
  // "APN"
  // "LastGoodAPN"

  return true;
}

const ProfileRefPtr &Service::profile() const { return profile_; }

void Service::set_profile(const ProfileRefPtr &p) { profile_ = p; }

}  // namespace shill
