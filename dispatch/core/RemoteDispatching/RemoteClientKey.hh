#pragma once

#include "dispatch/proto/claidservice.grpc.pb.h"

#include <map>
#include <sstream>

using claidservice::DataPackage;
using claidservice::RemoteClientInfo;


typedef std::pair<std::string, std::string> RemoteClientKey;
inline RemoteClientKey makeRemoteClientKey(const std::string& host, const std::string& userToken)
{
    return std::make_pair(host, userToken);
}

typedef std::pair<std::string, std::string> RemoteClientKey;
inline RemoteClientKey makeRemoteClientKey(const RemoteClientInfo& info)
{
    return makeRemoteClientKey(info.host(), info.user_token());
}

inline std::string makeRemoteClientIdentifier(const RemoteClientInfo& info)
{
    std::stringstream ss;
    ss << info.host() << ":" << info.user_token() << ":" << info.device_id();
    return ss.str();
}