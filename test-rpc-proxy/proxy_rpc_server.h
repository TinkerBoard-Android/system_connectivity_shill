//
// Copyright (C) 2015 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef PROXY_RPC_SERVER_H
#define PROXY_RPC_SERVER_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <iostream>

#include <base/logging.h>

#include "XmlRpc.h"

#include "proxy_dbus_client.h"

using namespace XmlRpc;

class ProxyRpcServer : public XmlRpcServer {
 public:
  ProxyRpcServer(
      int server_port,
      int xml_rpc_lib_verbosity) :
    XmlRpcServer(),
    server_port_(server_port),
    xml_rpc_lib_verbosity_(xml_rpc_lib_verbosity) {}
  void Run();

 private:
  int server_port_;
  int xml_rpc_lib_verbosity_;
};

// Generic class for all the RPC methods exposed by Shill RPC server
class ProxyRpcServerMethod : public XmlRpcServerMethod {
 public:
   ProxyRpcServerMethod(std::string method_name, ProxyRpcServer* rpc_server) :
     XmlRpcServerMethod(method_name, rpc_server) {}

 protected:
   int kSuccess = 0;
   int kFailure = 1;
   int kInvalidArgs = -1;

 private:
};

// MethodName: ConnectWifi
// Param1: SSID <string>
// Param2: IS_HEX_SSID <boolean>
// Param3: PSK <string>
// Return: 0 <success>, -1 <Invalid args>, 1 <Failure>
class ConnectWifi : public ProxyRpcServerMethod {
 public:
  ConnectWifi(ProxyRpcServer* rpc_server) :
    ProxyRpcServerMethod("ConnectWifi", rpc_server) {}
 private:
  void execute(XmlRpcValue& params, XmlRpcValue& result);
};

// MethodName: DisconnectWifi
// Return: 0 <success>, -1 <Invalid args>, 1 <Failure>
class DisconnectWifi : public ProxyRpcServerMethod {
 public:
   DisconnectWifi(ProxyRpcServer* rpc_server) :
     ProxyRpcServerMethod("DisconnectWifi", rpc_server) {}
 private:
  void execute(XmlRpcValue& params, XmlRpcValue& result);
};

#endif //PROXY_RPC_SERVER_H
