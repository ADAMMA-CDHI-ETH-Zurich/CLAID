/*
 * Compatibility class for standalone Java.
 * On Android, we use okhttp as socket library for gRPC.
 * In standalone Java, we use netty.
 * Netty automatically is able to build a channel based on unix domain sockets automatically,
 * if the socket path contains the prefix unix://
 * However with okhttp, we have to build a UdsChannel (uds = unix domain socket) manually using the UdsChannelBuilder.
 * In ModuleDispatcher class, we check if we are on Android and automatically build the channel using the UdsChannelBuilder,
 * if the prefix unix:// is found in the socket path. In standalone java, the Channel is built automatically, because netty is used.
 * However, since java does not have a preprocessor, it is difficult to add platform specific code.
 * Hence, in standalone java, we provide the UdsChannelBuilder.java and Namespace.java classes as stubs so that we do not get compile errors.
 */

package io.grpc.android;

import io.grpc.Channel;
import io.grpc.ManagedChannelBuilder;

public class UdsChannelBuilder {
    
    

    public static ManagedChannelBuilder forPath(String path, android.net.LocalSocketAddress.Namespace namespace)
    {
        return null;
    }
}

