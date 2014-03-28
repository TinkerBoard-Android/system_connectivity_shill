// Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/protobuf_lite_streams.h"

#include <fcntl.h>
#include <sys/types.h>

#include <base/file_util.h>
#include <base/posix/eintr_wrapper.h>

using google::protobuf::io::CopyingInputStream;
using google::protobuf::io::CopyingInputStreamAdaptor;
using std::string;

namespace shill {

CopyingInputStreamAdaptor *protobuf_lite_file_input_stream(
    const string &file_path) {
  int fd = HANDLE_EINTR(open(file_path.c_str(), O_RDONLY));
  if (fd == -1) {
    PLOG(ERROR) << __func__ << ": "
                << "Could not load protobuf file [" << file_path << "] ";
    return NULL;
  }

  auto *file_stream(new ProtobufLiteCopyingFileInputStream(fd));
  auto *adaptor(new CopyingInputStreamAdaptor(
        static_cast<CopyingInputStream*>(file_stream)));
  // Pass ownership of |file_stream|.
  adaptor->SetOwnsCopyingStream(true);
  return adaptor;
}


ProtobufLiteCopyingFileInputStream::ProtobufLiteCopyingFileInputStream(int fd)
  : fd_(fd),
    scoped_fd_closer_(&fd_),
    previous_seek_failed_(false) {}

ProtobufLiteCopyingFileInputStream::~ProtobufLiteCopyingFileInputStream() {}

int ProtobufLiteCopyingFileInputStream::Read(void *buffer, int size) {
  return HANDLE_EINTR(read(fd_, buffer, size));
}

int ProtobufLiteCopyingFileInputStream::Skip(int count) {
  if (!previous_seek_failed_ &&
      lseek(fd_, count, SEEK_CUR) != static_cast<off_t>(-1)) {
    // seek succeeded.
    return count;
  }
  // Let's not attempt to seek again later.
  previous_seek_failed_ = true;
  return CopyingInputStream::Skip(count);
}

}  // namespace shill
