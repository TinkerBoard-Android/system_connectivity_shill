// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_CELLULAR_
#define SHILL_CELLULAR_

#include <string>

#include <base/basictypes.h>
#include <gtest/gtest_prod.h>  // for FRIEND_TEST

#include "shill/device.h"
#include "shill/refptr_types.h"
#include "shill/shill_event.h"

namespace shill {

class ModemProxyInterface;

class Cellular : public Device {
 public:
  enum Type {
    kTypeGSM,
    kTypeCDMA
  };

  enum State {
    kStateDisabled,
    kStateEnabled,
    kStateRegistered,
    kStateConnected,
  };

  class Network {
   public:
    Network();
    ~Network();

    const std::string &GetStatus() const;
    void SetStatus(const std::string &status);

    const std::string &GetId() const;
    void SetId(const std::string &id);

    const std::string &GetShortName() const;
    void SetShortName(const std::string &name);

    const std::string &GetLongName() const;
    void SetLongName(const std::string &name);

    const std::string &GetTechnology() const;
    void SetTechnology(const std::string &technology);

    const Stringmap &ToDict() const;

   private:
    Stringmap dict_;

    DISALLOW_COPY_AND_ASSIGN(Network);
  };

  struct SimLockStatus {
   public:
    SimLockStatus() : retries_left(0) {}

    std::string lock_type;
    uint32 retries_left;
  };

  // |owner| is the ModemManager DBus service owner (e.g., ":1.17"). |path| is
  // the ModemManager.Modem DBus object path (e.g.,
  // "/org/chromium/ModemManager/Gobi/0").
  Cellular(ControlInterface *control_interface,
           EventDispatcher *dispatcher,
           Manager *manager,
           const std::string &link_name,
           int interface_index,
           Type type,
           const std::string &owner,
           const std::string &path);
  virtual ~Cellular();

  // Inherited from Device.
  virtual void Start();
  virtual void Stop();
  virtual bool TechnologyIs(Technology type);

 private:
  FRIEND_TEST(CellularTest, GetStateString);
  FRIEND_TEST(CellularTest, GetTypeString);
  FRIEND_TEST(CellularTest, Start);

  Stringmaps EnumerateNetworks();
  StrIntPair SimLockStatusToProperty();
  void HelpRegisterDerivedStringmaps(const std::string &name,
                                     Stringmaps(Cellular::*get)(void),
                                     bool(Cellular::*set)(const Stringmaps&));
  void HelpRegisterDerivedStrIntPair(const std::string &name,
                                     StrIntPair(Cellular::*get)(void),
                                     bool(Cellular::*set)(const StrIntPair&));

  std::string GetTypeString();
  std::string GetStateString();

  Type type_;
  State state_;

  const std::string dbus_owner_;  // ModemManager.Modem
  const std::string dbus_path_;  // ModemManager.Modem
  scoped_ptr<ModemProxyInterface> proxy_;

  ServiceRefPtr service_;
  bool service_registered_;

  // Properties
  bool allow_roaming_;
  std::string carrier_;
  std::string meid_;
  std::string imei_;
  std::string imsi_;
  std::string esn_;
  std::string mdn_;
  std::string min_;
  std::string model_id_;
  std::string manufacturer_;
  std::string firmware_revision_;
  std::string hardware_revision_;
  int16 prl_version_;
  bool scanning_;
  uint16 scan_interval_;
  std::vector<Network> found_networks_;
  SimLockStatus sim_lock_status_;

  DISALLOW_COPY_AND_ASSIGN(Cellular);
};

}  // namespace shill

#endif  // SHILL_CELLULAR_
