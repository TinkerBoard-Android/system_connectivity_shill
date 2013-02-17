// Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/activating_iccid_store.h"

#include <base/file_util.h>
#include <base/memory/scoped_ptr.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "shill/glib.h"
#include "shill/mock_store.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgumentPointee;

namespace shill {

class ActivatingIccidStoreTest : public ::testing::Test {
 public:
  ActivatingIccidStoreTest() : mock_store_(new MockStore()) {}

 protected:

  void SetMockStore() {
    iccid_store_.storage_.reset(mock_store_.release());
  }

  GLib glib_;
  scoped_ptr<MockStore> mock_store_;
  ActivatingIccidStore iccid_store_;
};

TEST_F(ActivatingIccidStoreTest, FileInteractions) {
  const char kStoragePath[] = "./";
  const char kIccid1[] = "1234";
  const char kIccid2[] = "4321";
  FilePath path(kStoragePath);
  FilePath file_path(ActivatingIccidStore::kStorageFileName);
  ASSERT_TRUE(file_util::CreateTemporaryFile(&file_path));

  EXPECT_TRUE(iccid_store_.InitStorage(&glib_, path));

  EXPECT_EQ(ActivatingIccidStore::kStateUnknown,
            iccid_store_.GetActivationState(kIccid1));
  EXPECT_EQ(ActivatingIccidStore::kStateUnknown,
            iccid_store_.GetActivationState(kIccid2));

  EXPECT_TRUE(iccid_store_.SetActivationState(
      kIccid1, ActivatingIccidStore::kStatePending));
  EXPECT_TRUE(iccid_store_.SetActivationState(
      kIccid2, ActivatingIccidStore::kStateActivated));

  EXPECT_EQ(ActivatingIccidStore::kStatePending,
            iccid_store_.GetActivationState(kIccid1));
  EXPECT_EQ(ActivatingIccidStore::kStateActivated,
            iccid_store_.GetActivationState(kIccid2));

  EXPECT_TRUE(iccid_store_.SetActivationState(
      kIccid1, ActivatingIccidStore::kStateActivated));
  EXPECT_TRUE(iccid_store_.SetActivationState(
      kIccid2, ActivatingIccidStore::kStatePending));

  EXPECT_EQ(ActivatingIccidStore::kStateActivated,
            iccid_store_.GetActivationState(kIccid1));
  EXPECT_EQ(ActivatingIccidStore::kStatePending,
            iccid_store_.GetActivationState(kIccid2));

  // Close and reopen the file to verify that the entries persisted.
  EXPECT_TRUE(iccid_store_.InitStorage(&glib_, path));

  EXPECT_EQ(ActivatingIccidStore::kStateActivated,
            iccid_store_.GetActivationState(kIccid1));
  EXPECT_EQ(ActivatingIccidStore::kStatePending,
            iccid_store_.GetActivationState(kIccid2));

  EXPECT_TRUE(iccid_store_.RemoveEntry(kIccid1));
  EXPECT_TRUE(iccid_store_.RemoveEntry(kIccid2));

  EXPECT_EQ(ActivatingIccidStore::kStateUnknown,
            iccid_store_.GetActivationState(kIccid1));
  EXPECT_EQ(ActivatingIccidStore::kStateUnknown,
            iccid_store_.GetActivationState(kIccid2));

  EXPECT_TRUE(iccid_store_.InitStorage(&glib_, path));

  EXPECT_EQ(ActivatingIccidStore::kStateUnknown,
            iccid_store_.GetActivationState(kIccid1));
  EXPECT_EQ(ActivatingIccidStore::kStateUnknown,
            iccid_store_.GetActivationState(kIccid2));

  ASSERT_TRUE(file_util::Delete(file_path, false));
}

TEST_F(ActivatingIccidStoreTest, GetActivationState) {
  MockStore *mock_store = mock_store_.get();
  SetMockStore();

  const char kIccid[] = "12345689";

  // Value not found
  EXPECT_CALL(*mock_store, GetInt(ActivatingIccidStore::kGroupId, kIccid, _))
      .WillOnce(Return(false));
  EXPECT_EQ(ActivatingIccidStore::kStateUnknown,
            iccid_store_.GetActivationState(kIccid));

  // File contains invalid entry
  EXPECT_CALL(*mock_store, GetInt(ActivatingIccidStore::kGroupId, kIccid, _))
      .WillOnce(DoAll(SetArgumentPointee<2>(3), Return(true)));
  EXPECT_EQ(ActivatingIccidStore::kStateUnknown,
            iccid_store_.GetActivationState(kIccid));
  EXPECT_CALL(*mock_store, GetInt(ActivatingIccidStore::kGroupId, kIccid, _))
      .WillOnce(DoAll(SetArgumentPointee<2>(0), Return(true)));
  EXPECT_EQ(ActivatingIccidStore::kStateUnknown,
            iccid_store_.GetActivationState(kIccid));

  // All enum values
  EXPECT_CALL(*mock_store, GetInt(ActivatingIccidStore::kGroupId, kIccid, _))
      .WillOnce(DoAll(SetArgumentPointee<2>(1), Return(true)));
  EXPECT_EQ(ActivatingIccidStore::kStatePending,
            iccid_store_.GetActivationState(kIccid));
  EXPECT_CALL(*mock_store, GetInt(ActivatingIccidStore::kGroupId, kIccid, _))
      .WillOnce(DoAll(SetArgumentPointee<2>(2), Return(true)));
  EXPECT_EQ(ActivatingIccidStore::kStateActivated,
            iccid_store_.GetActivationState(kIccid));
}

TEST_F(ActivatingIccidStoreTest, SetActivationState) {
  MockStore *mock_store = mock_store_.get();
  SetMockStore();

  const char kIccid[] = "12345689";

  EXPECT_CALL(*mock_store, Flush()).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock_store, SetInt(ActivatingIccidStore::kGroupId, kIccid, _))
      .WillOnce(Return(false));
  EXPECT_FALSE(iccid_store_.SetActivationState(
      kIccid, ActivatingIccidStore::kStateUnknown));
  EXPECT_FALSE(iccid_store_.SetActivationState(
      kIccid, ActivatingIccidStore::kStateUnknown));
  EXPECT_FALSE(iccid_store_.SetActivationState(
      kIccid, ActivatingIccidStore::kStatePending));

  EXPECT_CALL(*mock_store, SetInt(ActivatingIccidStore::kGroupId, kIccid, _))
      .WillRepeatedly(Return(true));
  EXPECT_TRUE(iccid_store_.SetActivationState(
      kIccid, ActivatingIccidStore::kStatePending));
  EXPECT_TRUE(iccid_store_.SetActivationState(
      kIccid, ActivatingIccidStore::kStateActivated));
}

TEST_F(ActivatingIccidStoreTest, RemoveEntry) {
  MockStore *mock_store = mock_store_.get();
  SetMockStore();

  const char kIccid[] = "12345689";

  EXPECT_CALL(*mock_store, Flush()).WillRepeatedly(Return(true));
  EXPECT_CALL(*mock_store, DeleteKey(ActivatingIccidStore::kGroupId, kIccid))
      .WillOnce(Return(false));
  EXPECT_FALSE(iccid_store_.RemoveEntry(kIccid));
  EXPECT_CALL(*mock_store, DeleteKey(ActivatingIccidStore::kGroupId, kIccid))
      .WillOnce(Return(true));
  EXPECT_TRUE(iccid_store_.RemoveEntry(kIccid));
}

}  // namespace shill
