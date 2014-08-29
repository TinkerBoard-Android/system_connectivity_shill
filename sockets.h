// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_SOCKETS_H_
#define SHILL_SOCKETS_H_

#include <linux/filter.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>

#include <base/basictypes.h>

namespace shill {

// A "sys/socket.h" abstraction allowing mocking in tests.
class Sockets {
 public:
  Sockets();
  virtual ~Sockets();

  static const int kInvalidFileDescriptor = -1;

  // accept
  virtual int Accept(int sockfd,
                     struct sockaddr *addr,
                     socklen_t *addrlen) const;

  // getsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER, ...)
  virtual int AttachFilter(int sockfd, struct sock_fprog *pf) const;

  // bind
  virtual int Bind(int sockfd,
                   const struct sockaddr *addr,
                   socklen_t addrlen) const;

  // setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE ...)
  virtual int BindToDevice(int sockfd, const std::string &device) const;

  // close
  virtual int Close(int fd) const;

  // connect
  virtual int Connect(int sockfd,
                      const struct sockaddr *addr,
                      socklen_t addrlen) const;

  // errno
  virtual int Error() const;

  // errno
  virtual std::string ErrorString() const;

  // getsockname
  virtual int GetSockName(int sockfd,
                          struct sockaddr *addr,
                          socklen_t *addrlen) const;

  // getsockopt(sockfd, SOL_SOCKET, SO_ERROR, ...)
  virtual int GetSocketError(int sockfd) const;

  // ioctl
  virtual int Ioctl(int d, int request, void *argp) const;

  // listen
  virtual int Listen(int sockfd, int backlog) const;

  // recvfrom
  virtual ssize_t RecvFrom(int sockfd, void *buf, size_t len, int flags,
                           struct sockaddr *src_addr, socklen_t *addrlen) const;

  // select
  virtual int Select(int nfds,
                     fd_set *readfds,
                     fd_set *writefds,
                     fd_set *exceptfds,
                     struct timeval *timeout) const;

  // send
  virtual ssize_t Send(int sockfd,
                       const void *buf,
                       size_t len,
                       int flags) const;

  // sendto
  virtual ssize_t SendTo(int sockfd,
                         const void *buf,
                         size_t len,
                         int flags,
                         const struct sockaddr *dest_addr,
                         socklen_t addrlen) const;

  // fcntl(sk, F_SETFL, fcntl(sk, F_GETFL) | O_NONBLOCK)
  virtual int SetNonBlocking(int sockfd) const;

  // setsockopt(SO_RCVBUFFORCE)
  virtual int SetReceiveBuffer(int sockfd, int size) const;

  // shutdown
  virtual int ShutDown(int sockfd, int how) const;

  // socket
  virtual int Socket(int domain, int type, int protocol) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(Sockets);
};

class ScopedSocketCloser {
 public:
  ScopedSocketCloser(Sockets *sockets, int fd);
  ~ScopedSocketCloser();

 private:
  Sockets *sockets_;
  int fd_;

  DISALLOW_COPY_AND_ASSIGN(ScopedSocketCloser);
};

}  // namespace shill

#endif  // SHILL_SOCKETS_H_
