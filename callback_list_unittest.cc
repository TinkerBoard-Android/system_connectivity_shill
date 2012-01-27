// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/callback_list.h"

#include <base/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "shill/mock_callback.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Test;

namespace shill {

typedef MockCallback<bool()> MyMockCallback;

class CallbackListTest : public Test {
 protected:
  CallbackList callbacks_;
};

TEST_F(CallbackListTest, AllTrue) {
  // Note that the EXPECT_CALLs cause gmock to verify that the
  // CallbackList dtor cleanups up callbacks.
  MyMockCallback *callback1 = NewMockCallback();
  callbacks_.Add("callback1", callback1);
  EXPECT_CALL(*callback1, OnRun()).WillOnce(Return(true));

  MyMockCallback *callback2 = NewMockCallback();
  callbacks_.Add("callback2", callback2);
  EXPECT_CALL(*callback2, OnRun()).WillOnce(Return(true));

  EXPECT_TRUE(callbacks_.Run());
}

TEST_F(CallbackListTest, AllFalse) {
  // Note that the EXPECT_CALLs verify that Run() doesn't
  // short-circuit on the first callback failure. Also, we have both
  // callbacks return false, so we catch short-circuiting regardless
  // of the order in which Run() executes the callbacks.
  MyMockCallback *callback1 = NewMockCallback();
  callbacks_.Add("callback1", callback1);
  EXPECT_CALL(*callback1, OnRun()).WillOnce(Return(false));

  MyMockCallback *callback2 = NewMockCallback();
  callbacks_.Add("callback2", callback2);
  EXPECT_CALL(*callback2, OnRun()).WillOnce(Return(false));

  EXPECT_FALSE(callbacks_.Run());
}

TEST_F(CallbackListTest, MixedReturnValues) {
  MyMockCallback *callback1 = NewMockCallback();
  callbacks_.Add("callback1", callback1);
  EXPECT_CALL(*callback1, OnRun()).WillOnce(Return(true));

  MyMockCallback *callback2 = NewMockCallback();
  callbacks_.Add("callback2", callback2);
  EXPECT_CALL(*callback2, OnRun()).WillOnce(Return(false));

  EXPECT_FALSE(callbacks_.Run());
}

TEST_F(CallbackListTest, Remove) {
  MyMockCallback *callback = NewMockCallback();
  callbacks_.Add("callback", callback);
  EXPECT_CALL(*callback, OnRun()).WillOnce(Return(false));
  EXPECT_FALSE(callbacks_.Run());

  EXPECT_CALL(*callback, OnRun()).Times(0);

  // By virtue of the EXPECT_CALLs on |callback|, we implicitly tests
  // that Remove does not leak objects.
  callbacks_.Remove("callback");
  EXPECT_TRUE(callbacks_.Run());
}

}  // namespace shill
