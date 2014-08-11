// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/key_value_store.h"

#include <gtest/gtest.h>

using std::map;
using std::string;
using std::vector;
using testing::Test;

namespace shill {

class KeyValueStoreTest : public Test {
 public:
  KeyValueStoreTest() {}

 protected:
  KeyValueStore store_;
};

TEST_F(KeyValueStoreTest, Bool) {
  const string kKey("foo");
  const bool kDefaultValue = true;
  const bool kValue = false;
  EXPECT_FALSE(store_.ContainsBool(kKey));
  EXPECT_EQ(kDefaultValue, store_.LookupBool(kKey, kDefaultValue));
  store_.SetBool(kKey, kValue);
  EXPECT_TRUE(store_.ContainsBool(kKey));
  // TODO(shenhan): investigate if a newer version of gtest handles EXPECT_EQ
  // for bools in a manner that gcc 4.7 is happy with. (Improper conversion from
  // "false" to "NULL").
  EXPECT_EQ(static_cast<int>(kValue),
            static_cast<int>(store_.LookupBool(kKey, kDefaultValue)));
  EXPECT_EQ(static_cast<int>(kValue),
            static_cast<int>(store_.GetBool(kKey)));
}

TEST_F(KeyValueStoreTest, Int) {
  const string kKey("foo");
  const int kValue = 456;
  EXPECT_FALSE(store_.ContainsInt(kKey));
  const int kDefaultValue = 789;
  EXPECT_EQ(kDefaultValue, store_.LookupInt(kKey, kDefaultValue));
  store_.SetInt(kKey, kValue);
  EXPECT_TRUE(store_.ContainsInt(kKey));
  EXPECT_EQ(kValue, store_.GetInt(kKey));
  EXPECT_EQ(kValue, store_.LookupInt(kKey, kDefaultValue));
  store_.RemoveInt(kKey);
  EXPECT_FALSE(store_.ContainsInt(kKey));
}

TEST_F(KeyValueStoreTest, String) {
  const string kKey("foo");
  const string kDefaultValue("bar");
  const string kValue("baz");
  EXPECT_FALSE(store_.ContainsString(kKey));
  EXPECT_EQ(kDefaultValue, store_.LookupString(kKey, kDefaultValue));
  store_.SetString(kKey, kValue);
  EXPECT_TRUE(store_.ContainsString(kKey));
  EXPECT_EQ(kValue, store_.LookupString(kKey, kDefaultValue));
  EXPECT_EQ(kValue, store_.GetString(kKey));
  store_.RemoveString(kKey);
  EXPECT_FALSE(store_.ContainsString(kKey));
  EXPECT_EQ(kDefaultValue, store_.LookupString(kKey, kDefaultValue));
}

TEST_F(KeyValueStoreTest, Stringmap) {
  const string kKey("foo");
  const map<string, string> kValue{ { "bar0", "baz0" }, { "bar1", "baz1" } };
  EXPECT_FALSE(store_.ContainsStringmap(kKey));
  store_.SetStringmap(kKey, kValue);
  EXPECT_TRUE(store_.ContainsStringmap(kKey));
  EXPECT_EQ(kValue, store_.GetStringmap(kKey));
  store_.RemoveStringmap(kKey);
  EXPECT_FALSE(store_.ContainsStringmap(kKey));
}

TEST_F(KeyValueStoreTest, Strings) {
  const string kKey("foo");
  const vector<string> kValue{ "baz0", "baz1", "baz2" };
  EXPECT_FALSE(store_.ContainsStrings(kKey));
  store_.SetStrings(kKey, kValue);
  EXPECT_TRUE(store_.ContainsStrings(kKey));
  EXPECT_EQ(kValue, store_.GetStrings(kKey));
  store_.RemoveStrings(kKey);
  EXPECT_FALSE(store_.ContainsStrings(kKey));
}

TEST_F(KeyValueStoreTest, Uint) {
  const string kKey("foo");
  const uint32_t kValue = 456;
  EXPECT_FALSE(store_.ContainsUint(kKey));
  store_.SetUint(kKey, kValue);
  EXPECT_TRUE(store_.ContainsUint(kKey));
  EXPECT_EQ(kValue, store_.GetUint(kKey));
}

TEST_F(KeyValueStoreTest, DoubleRemove) {
  const string kKey("foo");
  // Make sure we don't get an exception/infinite loop if we do a
  // "Remove()" when the key does not exist.
  store_.RemoveInt(kKey);
  store_.RemoveInt(kKey);
  store_.RemoveString(kKey);
  store_.RemoveString(kKey);
}

TEST_F(KeyValueStoreTest, Clear) {
  EXPECT_TRUE(store_.IsEmpty());
  const string kBoolKey("foo");
  const bool kBoolValue = true;
  store_.SetBool(kBoolKey, kBoolValue);
  const string kIntKey("bar");
  const int kIntValue = 123;
  store_.SetInt(kIntKey, kIntValue);
  const string kStringKey("baz");
  const string kStringValue("string");
  store_.SetString(kStringKey, kStringValue);
  const string kStringmapKey("stringMapKey");
  const map<string, string> kStringmapValue;
  store_.SetStringmap(kStringmapKey, kStringmapValue);
  const string kStringsKey("stringsKey");
  const vector<string> kStringsValue;
  store_.SetStrings(kStringsKey, kStringsValue);
  const string kUintKey("bun");
  const uint32_t kUintValue = 456;
  store_.SetUint(kUintKey, kUintValue);

  EXPECT_TRUE(store_.ContainsBool(kBoolKey));
  EXPECT_TRUE(store_.ContainsInt(kIntKey));
  EXPECT_TRUE(store_.ContainsString(kStringKey));
  EXPECT_TRUE(store_.ContainsStringmap(kStringmapKey));
  EXPECT_TRUE(store_.ContainsStrings(kStringsKey));
  EXPECT_TRUE(store_.ContainsUint(kUintKey));
  EXPECT_FALSE(store_.IsEmpty());
  store_.Clear();
  EXPECT_TRUE(store_.IsEmpty());
  EXPECT_FALSE(store_.ContainsBool(kBoolKey));
  EXPECT_FALSE(store_.ContainsInt(kIntKey));
  EXPECT_FALSE(store_.ContainsString(kStringKey));
  EXPECT_FALSE(store_.ContainsStringmap(kStringmapKey));
  EXPECT_FALSE(store_.ContainsStrings(kStringsKey));
  EXPECT_FALSE(store_.ContainsUint(kUintKey));
}

TEST_F(KeyValueStoreTest, Equals) {
  KeyValueStore first, second;

  first.SetBool("boolKey", true);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  second.SetBool("boolKey", true);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetBool("boolKey", true);
  second.SetBool("boolOtherKey", true);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetBool("boolKey", true);
  second.SetBool("boolKey", false);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetInt("intKey", 123);
  second.SetInt("intOtherKey", 123);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetInt("intKey", 123);
  second.SetInt("intKey", 456);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetString("stringKey", "string");
  second.SetString("stringOtherKey", "string");
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetString("stringKey", "string");
  second.SetString("stringKey", "otherString");
  EXPECT_FALSE(first.Equals(second));

  const map<string, string> kStringmap1{ { "key", "value" } };
  const map<string, string> kStringmap2{ { "otherKey", "value" } };
  const map<string, string> kStringmap3{ { "key", "otherValue" } };

  first.Clear();
  second.Clear();
  first.SetStringmap("stringmapKey", kStringmap1);
  second.SetStringmap("stringmapOtherKey", kStringmap1);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetStringmap("stringmapKey", kStringmap1);
  second.SetStringmap("stringmapKey", kStringmap2);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetStringmap("stringmapKey", kStringmap1);
  second.SetStringmap("stringmapKey", kStringmap3);
  EXPECT_FALSE(first.Equals(second));

  const vector<string> kStrings1{ "value" };
  const vector<string> kStrings2{ "otherValue" };

  first.Clear();
  second.Clear();
  first.SetStrings("stringsKey", kStrings1);
  second.SetStrings("stringsOtherKey", kStrings1);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetStrings("stringsKey", kStrings1);
  second.SetStrings("stringsKey", kStrings2);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetUint("uintKey", 1);
  second.SetUint("uintOtherKey", 1);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetUint("uintKey", 1);
  second.SetUint("uintKey", 2);
  EXPECT_FALSE(first.Equals(second));

  first.Clear();
  second.Clear();
  first.SetBool("boolKey", true);
  first.SetInt("intKey", 123);
  first.SetString("stringKey", "value");
  first.SetStringmap("stringmapKey", kStringmap1);
  first.SetStrings("stringsKey", kStrings1);
  first.SetUint("uintKey", 1);
  second.SetBool("boolKey", true);
  second.SetInt("intKey", 123);
  second.SetString("stringKey", "value");
  second.SetStringmap("stringmapKey", kStringmap1);
  second.SetStrings("stringsKey", kStrings1);
  second.SetUint("uintKey", 1);
  EXPECT_TRUE(first.Equals(second));
}

TEST_F(KeyValueStoreTest, CopyFrom) {
  KeyValueStore donor;
  const string kBoolKey("foo");
  const bool kBoolValue = true;
  donor.SetBool(kBoolKey, kBoolValue);
  const string kIntKey("bar");
  const int kIntValue = 123;
  donor.SetInt(kIntKey, kIntValue);
  const string kStringKey("baz");
  const string kStringValue("string");
  donor.SetString(kStringKey, kStringValue);
  const string kStringmapKey("stringMapKey");
  const map<string, string> kStringmapValue{ { "key", "value" } };
  donor.SetStringmap(kStringmapKey, kStringmapValue);
  const string kStringsKey("stringsKey");
  const vector<string> kStringsValue{ "string0", "string1" };
  donor.SetStrings(kStringsKey, kStringsValue);
  const string kUintKey("bun");
  const uint32_t kUintValue = 456;
  donor.SetUint(kUintKey, kUintValue);

  EXPECT_TRUE(store_.IsEmpty());
  store_.CopyFrom(donor);
  EXPECT_FALSE(store_.IsEmpty());
  EXPECT_TRUE(donor.Equals(store_));
}

}  // namespace shill
