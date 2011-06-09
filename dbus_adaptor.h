// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_DBUS_ADAPTOR_H_
#define SHILL_DBUS_ADAPTOR_H_

#include <map>
#include <string>
#include <vector>

#include <base/basictypes.h>
#include <dbus-c++/dbus.h>

namespace shill {

#define SHILL_INTERFACE "org.chromium.flimflam"
#define SHILL_PATH "/org/chromium/flimflam"

class PropertyStoreInterface;

// Superclass for all DBus-backed Adaptor objects
class DBusAdaptor : public DBus::ObjectAdaptor {
 public:
  DBusAdaptor(DBus::Connection* conn, const std::string& object_path);
  virtual ~DBusAdaptor();

  static bool DispatchOnType(PropertyStoreInterface *store,
                             const std::string& name,
                             const ::DBus::Variant& value,
                             ::DBus::Error &error);

  static ::DBus::Variant BoolToVariant(bool value);
  static ::DBus::Variant ByteToVariant(uint8 value);
  static ::DBus::Variant Int16ToVariant(int16 value);
  static ::DBus::Variant Int32ToVariant(int32 value);
  static ::DBus::Variant StringToVariant(const std::string& value);
  static ::DBus::Variant StringmapToVariant(
      const std::map<std::string, std::string>& value);
  static ::DBus::Variant StringsToVariant(
      const std::vector<std::string>& value);
  static ::DBus::Variant Uint16ToVariant(uint16 value);
  static ::DBus::Variant Uint32ToVariant(uint32 value);

  static bool IsBool(::DBus::Signature signature);
  static bool IsByte(::DBus::Signature signature);
  static bool IsInt16(::DBus::Signature signature);
  static bool IsInt32(::DBus::Signature signature);
  static bool IsString(::DBus::Signature signature);
  static bool IsStringmap(::DBus::Signature signature);
  static bool IsStrings(::DBus::Signature signature);
  static bool IsUint16(::DBus::Signature signature);
  static bool IsUint32(::DBus::Signature signature);

 private:
  static const char kStringmapSig[];
  static const char kStringsSig[];
  DISALLOW_COPY_AND_ASSIGN(DBusAdaptor);
};

}  // namespace shill
#endif  // SHILL_DBUS_ADAPTOR_H_
