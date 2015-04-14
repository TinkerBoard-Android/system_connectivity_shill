// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_PROXY_FACTORY_H_
#define SHILL_PROXY_FACTORY_H_

#include <memory>
#include <string>

#include <base/lazy_instance.h>
#include <base/macros.h>
#include <dbus-c++/dbus.h>

#include "shill/refptr_types.h"

namespace shill {

class DBusObjectManagerProxyInterface;
class DBusPropertiesProxyInterface;
class DBusServiceProxyInterface;
class DHCPProxyInterface;
class ModemCDMAProxyInterface;
class ModemGSMCardProxyInterface;
class ModemGSMNetworkProxyInterface;
class ModemGobiProxyInterface;
class ModemManagerClassic;
class ModemManagerProxyInterface;
class ModemProxyInterface;
class ModemSimpleProxyInterface;
class PermissionBrokerProxyInterface;
class PowerManagerProxyDelegate;
class PowerManagerProxyInterface;
class UpstartProxyInterface;
class WiMaxDeviceProxyInterface;
class WiMaxManagerProxyInterface;
class WiMaxNetworkProxyInterface;

#if !defined(DISABLE_WIFI)
class SupplicantBSSProxyInterface;
#endif  // DISABLE_WIFI

#if !defined(DISABLE_WIFI) || !defined(DISABLE_WIRED_8021X)
class SupplicantEventDelegateInterface;
class SupplicantInterfaceProxyInterface;
class SupplicantNetworkProxyInterface;
class SupplicantProcessProxyInterface;
#endif  // DISABLE_WIFI || DISABLE_WIRED_8021X

namespace mm1 {

class BearerProxyInterface;
class ModemLocationProxyInterface;
class ModemModem3gppProxyInterface;
class ModemModemCdmaProxyInterface;
class ModemProxyInterface;
class ModemSimpleProxyInterface;
class ModemTimeProxyInterface;
class SimProxyInterface;

}  // namespace mm1

// Global DBus proxy factory that can be mocked out in tests.
class ProxyFactory {
 public:
  virtual ~ProxyFactory();

  // Since this is a singleton, use ProxyFactory::GetInstance()->Foo().
  static ProxyFactory *GetInstance();

  virtual DBusPropertiesProxyInterface *CreateDBusPropertiesProxy(
      const std::string &path,
      const std::string &service);

  virtual DBusServiceProxyInterface *CreateDBusServiceProxy();

  // The caller retains ownership of 'delegate'.  It must not be deleted before
  // the proxy.
  virtual PowerManagerProxyInterface *CreatePowerManagerProxy(
      PowerManagerProxyDelegate *delegate);

#if !defined(DISABLE_WIFI) || !defined(DISABLE_WIRED_8021X)
  virtual SupplicantProcessProxyInterface *CreateSupplicantProcessProxy(
      const char *dbus_path,
      const char *dbus_addr);

  virtual SupplicantInterfaceProxyInterface *CreateSupplicantInterfaceProxy(
      SupplicantEventDelegateInterface *delegate,
      const DBus::Path &object_path,
      const char *dbus_addr);

  virtual SupplicantNetworkProxyInterface *CreateSupplicantNetworkProxy(
      const DBus::Path &object_path,
      const char *dbus_addr);
#endif  // DISABLE_WIFI || DISABLE_WIRED_8021X

#if !defined(DISABLE_WIFI)
  // See comment in supplicant_bss_proxy.h, about bare pointer.
  virtual SupplicantBSSProxyInterface *CreateSupplicantBSSProxy(
      WiFiEndpoint *wifi_endpoint,
      const DBus::Path &object_path,
      const char *dbus_addr);
#endif  // DISABLE_WIFI

  virtual UpstartProxyInterface *CreateUpstartProxy();

  virtual DHCPProxyInterface *CreateDHCPProxy(const std::string &service);

  virtual PermissionBrokerProxyInterface *CreatePermissionBrokerProxy();

#if !defined(DISABLE_CELLULAR)

  virtual DBusObjectManagerProxyInterface *CreateDBusObjectManagerProxy(
      const std::string &path,
      const std::string &service);

  virtual ModemManagerProxyInterface *CreateModemManagerProxy(
      ModemManagerClassic *manager,
      const std::string &path,
      const std::string &service);

  virtual ModemProxyInterface *CreateModemProxy(const std::string &path,
                                                const std::string &service);

  virtual ModemSimpleProxyInterface *CreateModemSimpleProxy(
      const std::string &path,
      const std::string &service);

  virtual ModemCDMAProxyInterface *CreateModemCDMAProxy(
      const std::string &path,
      const std::string &service);

  virtual ModemGSMCardProxyInterface *CreateModemGSMCardProxy(
      const std::string &path,
      const std::string &service);

  virtual ModemGSMNetworkProxyInterface *CreateModemGSMNetworkProxy(
      const std::string &path,
      const std::string &service);

  virtual ModemGobiProxyInterface *CreateModemGobiProxy(
      const std::string &path,
      const std::string &service);

  // Proxies for ModemManager1 interfaces
  virtual mm1::ModemModem3gppProxyInterface *CreateMM1ModemModem3gppProxy(
      const std::string &path,
      const std::string &service);

  virtual mm1::ModemModemCdmaProxyInterface *CreateMM1ModemModemCdmaProxy(
      const std::string &path,
      const std::string &service);

  virtual mm1::ModemProxyInterface *CreateMM1ModemProxy(
      const std::string &path,
      const std::string &service);

  virtual mm1::ModemLocationProxyInterface *CreateMM1ModemLocationProxy(
      const std::string &path,
      const std::string &service);

  virtual mm1::ModemSimpleProxyInterface *CreateMM1ModemSimpleProxy(
      const std::string &path,
      const std::string &service);

  virtual mm1::ModemTimeProxyInterface *CreateMM1ModemTimeProxy(
      const std::string &path,
      const std::string &service);

  virtual mm1::SimProxyInterface *CreateSimProxy(
      const std::string &path,
      const std::string &service);

  virtual mm1::BearerProxyInterface *CreateBearerProxy(
      const std::string &path,
      const std::string &service);

#endif  // DISABLE_CELLULAR

#if !defined(DISABLE_WIMAX)

  virtual WiMaxDeviceProxyInterface *CreateWiMaxDeviceProxy(
      const std::string &path);
  virtual WiMaxManagerProxyInterface *CreateWiMaxManagerProxy();
  virtual WiMaxNetworkProxyInterface *CreateWiMaxNetworkProxy(
      const std::string &path);

#endif  // DISABLE_WIMAX

 protected:
  ProxyFactory();

 private:
  friend struct base::DefaultLazyInstanceTraits<ProxyFactory>;

  DBus::Connection *GetConnection() const;

  DISALLOW_COPY_AND_ASSIGN(ProxyFactory);
};

}  // namespace shill

#endif  // SHILL_PROXY_FACTORY_H_
