// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/portal_detector.h"

#include <string>

#include <base/logging.h>
#include <base/string_number_conversions.h>
#include <base/string_util.h>
#include <base/stringprintf.h>

#include "shill/async_connection.h"
#include "shill/connection.h"
#include "shill/dns_client.h"
#include "shill/event_dispatcher.h"
#include "shill/http_url.h"
#include "shill/ip_address.h"
#include "shill/sockets.h"

using base::StringPrintf;
using std::string;

namespace shill {

const char PortalDetector::kDefaultURL[] =
    "http://clients3.google.com/generate_204";
const char PortalDetector::kResponseExpected[] = "HTTP/1.1 204";

const int PortalDetector::kMaxRequestAttempts = 3;
const int PortalDetector::kMinTimeBetweenAttemptsSeconds = 3;
const int PortalDetector::kRequestTimeoutSeconds = 10;

const char PortalDetector::kPhaseConnectionString[] = "Connection";
const char PortalDetector::kPhaseDNSString[] = "DNS";
const char PortalDetector::kPhaseHTTPString[] = "HTTP";
const char PortalDetector::kPhaseContentString[] = "Content";
const char PortalDetector::kPhaseUnknownString[] = "Unknown";

const char PortalDetector::kStatusFailureString[] = "Failure";
const char PortalDetector::kStatusSuccessString[] = "Success";
const char PortalDetector::kStatusTimeoutString[] = "Timeout";

PortalDetector::PortalDetector(
    ConnectionRefPtr connection,
    EventDispatcher *dispatcher,
    Callback1<const Result &>::Type *callback)
    : attempt_count_(0),
      connection_(connection),
      dispatcher_(dispatcher),
      portal_result_callback_(callback),
      request_read_callback_(
          NewCallback(this, &PortalDetector::RequestReadCallback)),
      request_result_callback_(
          NewCallback(this, &PortalDetector::RequestResultCallback)),
      task_factory_(this),
      time_(Time::GetInstance()) { }

PortalDetector::~PortalDetector() {
  Stop();
}

bool PortalDetector::Start(const std::string &url_string) {
  VLOG(3) << "In " << __func__;

  DCHECK(!request_.get());

  if (!url_.ParseFromString(url_string)) {
    LOG(ERROR) << "Failed to parse URL string: " << url_string;
    return false;
  }

  request_.reset(new HTTPRequest(connection_, dispatcher_, &sockets_));
  attempt_count_ = 0;
  StartAttempt();
  return true;
}

void PortalDetector::Stop() {
  VLOG(3) << "In " << __func__;

  if (!request_.get()) {
    return;
  }

  StopAttempt();
  attempt_count_ = 0;
  request_.reset();
}

// static
const string PortalDetector::PhaseToString(Phase phase) {
  switch (phase) {
    case kPhaseConnection:
      return kPhaseConnectionString;
    case kPhaseDNS:
      return kPhaseDNSString;
    case kPhaseHTTP:
      return kPhaseHTTPString;
    case kPhaseContent:
      return kPhaseContentString;
    case kPhaseUnknown:
    default:
      return kPhaseUnknownString;
  }
}

// static
const string PortalDetector::StatusToString(Status status) {
  switch (status) {
    case kStatusSuccess:
      return kStatusSuccessString;
    case kStatusTimeout:
      return kStatusTimeoutString;
    case kStatusFailure:
    default:
      return kStatusFailureString;
  }
}

void PortalDetector::CompleteAttempt(Result result) {
  LOG(INFO) << StringPrintf("Portal detection completed attempt %d with "
                            "phase==%s, status==%s",
                            attempt_count_,
                            PhaseToString(result.phase).c_str(),
                            StatusToString(result.status).c_str());
  StopAttempt();
  if (result.status != kStatusSuccess && attempt_count_ < kMaxRequestAttempts) {
    StartAttempt();
  } else {
    result.final = true;
    Stop();
  }

  portal_result_callback_->Run(result);
}

PortalDetector::Result PortalDetector::GetPortalResultForRequestResult(
  HTTPRequest::Result result) {
  switch (result) {
    case HTTPRequest::kResultSuccess:
      // The request completed without receiving the expected payload.
      return Result(kPhaseContent, kStatusFailure);
    case HTTPRequest::kResultDNSFailure:
      return Result(kPhaseDNS, kStatusFailure);
    case HTTPRequest::kResultDNSTimeout:
      return Result(kPhaseDNS, kStatusTimeout);
    case HTTPRequest::kResultConnectionFailure:
      return Result(kPhaseConnection, kStatusFailure);
    case HTTPRequest::kResultConnectionTimeout:
      return Result(kPhaseConnection, kStatusTimeout);
    case HTTPRequest::kResultRequestFailure:
    case HTTPRequest::kResultResponseFailure:
      return Result(kPhaseHTTP, kStatusFailure);
    case HTTPRequest::kResultRequestTimeout:
    case HTTPRequest::kResultResponseTimeout:
      return Result(kPhaseHTTP, kStatusTimeout);
    case HTTPRequest::kResultUnknown:
    default:
      return Result(kPhaseUnknown, kStatusFailure);
  }
}

void PortalDetector::RequestReadCallback(const ByteString &response_data) {
  const string response_expected(kResponseExpected);
  bool expected_length_received = false;
  int compare_length = 0;
  if (response_data.GetLength() < response_expected.length()) {
    // There isn't enough data yet for a final decision, but we can still
    // test to see if the partial string matches so far.
    expected_length_received = false;
    compare_length = response_data.GetLength();
  } else {
    expected_length_received = true;
    compare_length = response_expected.length();
  }

  if (ByteString(response_expected.substr(0, compare_length), false).Equals(
          ByteString(response_data.GetConstData(), compare_length))) {
    if (expected_length_received) {
      CompleteAttempt(Result(kPhaseContent, kStatusSuccess));
    }
    // Otherwise, we wait for more data from the server.
  } else {
    CompleteAttempt(Result(kPhaseContent, kStatusFailure));
  }
}

void PortalDetector::RequestResultCallback(
    HTTPRequest::Result result, const ByteString &/*response_data*/) {
  CompleteAttempt(GetPortalResultForRequestResult(result));
}

void PortalDetector::StartAttempt() {
  int64 next_attempt_delay = 0;
  if (attempt_count_ > 0) {
    // Ensure that attempts are spaced at least by a minimal interval.
    struct timeval now, elapsed_time;
    time_->GetTimeMonotonic(&now);
    timersub(&now, &attempt_start_time_, &elapsed_time);

    if (elapsed_time.tv_sec < kMinTimeBetweenAttemptsSeconds) {
      struct timeval remaining_time = { kMinTimeBetweenAttemptsSeconds, 0 };
      timersub(&remaining_time, &elapsed_time, &remaining_time);
      next_attempt_delay =
        remaining_time.tv_sec * 1000 + remaining_time.tv_usec / 1000;
    }
  }
  dispatcher_->PostDelayedTask(
      task_factory_.NewRunnableMethod(&PortalDetector::StartAttemptTask),
      next_attempt_delay);
}

void PortalDetector::StartAttemptTask() {
  time_->GetTimeMonotonic(&attempt_start_time_);
  ++attempt_count_;

  LOG(INFO) << StringPrintf("Portal detection starting attempt %d of %d",
                            attempt_count_, kMaxRequestAttempts);

  HTTPRequest::Result result =
      request_->Start(url_, request_read_callback_.get(),
                      request_result_callback_.get());
  if (result != HTTPRequest::kResultInProgress) {
    CompleteAttempt(GetPortalResultForRequestResult(result));
    return;
  }

  dispatcher_->PostDelayedTask(
      task_factory_.NewRunnableMethod(&PortalDetector::TimeoutAttemptTask),
      kRequestTimeoutSeconds * 1000);
}

void PortalDetector::StopAttempt() {
  request_->Stop();
  task_factory_.RevokeAll();
}

void PortalDetector::TimeoutAttemptTask() {
  LOG(ERROR) << "Request timed out";
  if (request_->response_data().GetLength()) {
    CompleteAttempt(Result(kPhaseContent, kStatusTimeout));
  } else {
    CompleteAttempt(Result(kPhaseUnknown, kStatusTimeout));
  }
}

}  // namespace shill
