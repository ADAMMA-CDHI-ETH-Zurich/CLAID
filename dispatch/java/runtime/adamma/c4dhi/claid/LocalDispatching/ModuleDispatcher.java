/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

package adamma.c4dhi.claid.LocalDispatching;


import adamma.c4dhi.claid.ClaidServiceGrpc;
import adamma.c4dhi.claid.ClaidServiceGrpc.ClaidServiceStub;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.ModuleFactory;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.ControlPackage;
import adamma.c4dhi.claid.CtrlType;
import adamma.c4dhi.claid.InitRuntimeRequest;
import adamma.c4dhi.claid.Runtime;
import adamma.c4dhi.claid.ModuleListRequest;
import adamma.c4dhi.claid.ModuleListResponse;
import adamma.c4dhi.claid.ModuleListResponse.ModuleDescriptor;
import adamma.c4dhi.claid.ModuleAnnotation;

import io.grpc.Channel;
import io.grpc.ManagedChannel;
import io.grpc.Grpc;
import io.grpc.InsecureChannelCredentials;
import io.grpc.stub.StreamObserver;
import io.grpc.stub.ClientCallStreamObserver;

import io.grpc.Status;
import io.grpc.StatusRuntimeException;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;

import com.google.protobuf.Empty;

import java.util.ArrayList;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;


// Corresponds to ClientDispatcher of the C++ core, which is ModuleDispatcher in dart.
public class ModuleDispatcher  
{
    Channel grpcChannel;
    ClaidServiceStub stub;
    
    String socketPath;

    // Stream for us to receive packages from the Middleware.
    StreamObserver<DataPackage> inStream;

    // Stream for us to send packages to the Middleware
    StreamObserver<DataPackage> outStream;

    // For ModuleManager to receive packages
    Consumer<DataPackage> inConsumer;

    boolean waitingForPingResponse = false;

    private boolean waitForInputStreamCancelled = false;
    private boolean inputStreamCancelled = false;
    // Is this safe?
    private boolean isAndroid() {
        try {
            Class.forName("android.os.Build");
            return true;
        } catch (ClassNotFoundException e) {
            return false;
        }
    }

    public ModuleDispatcher(final String socketPath)
    {
        this.socketPath = socketPath;

        // If android and socket path prefix is unix://
        if (socketPath != null && socketPath.startsWith("unix://") && isAndroid()) {
            String udsPath = socketPath.substring("unix://".length());
            this.grpcChannel = io.grpc.android.UdsChannelBuilder.forPath(udsPath, android.net.LocalSocketAddress.Namespace.FILESYSTEM).build();
        } else {
            this.grpcChannel = Grpc.newChannelBuilder(socketPath, InsecureChannelCredentials.create()).build();
        }

        // this.grpcChannel = ManagedChannelBuilder.forTarget(socketPath).usePlaintext().build();
        // see https://github.com/Hellblazer/GrpcDomainSocketTest/blob/main/src/test/java/domain/DomainSocketReproTest.java
        // this.grpcChannel = NettyChannelBuilder.forAddress(new DomainSocketAddress(socketPath))
        //                                             .eventLoopGroup(new KQueueEventLoopGroup())
        //                                             .channelType(KQueueDomainSocketChannel.class)
        //                                             .keepAliveTime(1, TimeUnit.MILLISECONDS)
        //                                             .usePlaintext()
        //                                             .build();

        this.stub = ClaidServiceGrpc.newStub(this.grpcChannel);
    }

    // Tells the Middleware Server which Modules are available by this Runtime.
    // The Middleware will resond with the List of Modules that it wants this Runtime to initialize (which will be a subset of the Modules available in this runtime).
    // In general: Once the Middleware has started, it loads a config file. From the config file, it knows
    // which Modules to instantiate, but it does not yet know which Runtime (specifically, the ModuleDispatchers of each Runtime) can load which Module.
    // Therefore, it relies on the Runtimes to tell the Middleware which Modules they can handle. They do so by calling the stub.GetModuleList() function,
    // which contains a ModuleListRequest, which contains a list of all Modules the Runtime can handle.
    // The Middleware then replies with a ModuleListResponse, which contains the Modules it wants to be loaded.
    public ModuleListResponse getModuleList(ArrayList<String> registeredModuleClasses, Map<String, ModuleAnnotation> moduleAnnotations)
    {
        ModuleListRequest.Builder requestBuilder = 
            ModuleListRequest.newBuilder();

        requestBuilder.setRuntime(Runtime.RUNTIME_JAVA)
                      .addAllSupportedModuleClasses(registeredModuleClasses);

        for(Map.Entry<String, ModuleAnnotation> entry : moduleAnnotations.entrySet())
        {
            requestBuilder.putModuleAnnotations(entry.getKey(), entry.getValue());
        }

        ModuleListRequest request = requestBuilder.build();
        Logger.logInfo("Sending ModuleListRequest: " + request);
        
        // stub.getModuleList() -> calls getModuleList function of RemoteService in the MiddleWare via RPC.

        ModuleListResponse response = null;

        SynchronizedStreamObserver<ModuleListResponse> responseObserver = new SynchronizedStreamObserver<>();
        
        Logger.logInfo("Java Runtime: Calling getModuleList(...)");
        stub.getModuleList(request, responseObserver);

        response = responseObserver.await();

        if(responseObserver.errorOccured())
        {
            Logger.logError("Error occured in getModuleList() of JAVA_RUNTIME: " + responseObserver.getErrorMessage());
            System.exit(0);
        }
        System.out.println("Respone: " + response);

        return response;
    }


    public boolean initRuntime(Map<String, ArrayList<DataPackage>> channelExamplePackages)
    {
        InitRuntimeRequest.Builder initRuntimeRequest = InitRuntimeRequest.newBuilder();
        
        for(Map.Entry<String, ArrayList<DataPackage>> entry : channelExamplePackages.entrySet())
        {
            InitRuntimeRequest.ModuleChannels.Builder moduleChannels = InitRuntimeRequest.ModuleChannels.newBuilder();
            moduleChannels.setModuleId(entry.getKey());
            moduleChannels.addAllChannelPackets(entry.getValue());
            System.out.println("Adding " + entry.getValue());
            initRuntimeRequest.addModules(moduleChannels.build());
        }   
        initRuntimeRequest.setRuntime(Runtime.RUNTIME_JAVA);

        boolean resultReceived = false;
        Empty response = null;

        SynchronizedStreamObserver<Empty> responseObserver = new SynchronizedStreamObserver<>();

        Logger.logInfo("Java Runtime: Calling initRuntime(...)");
        stub.initRuntime(initRuntimeRequest.build(), responseObserver);

        response = responseObserver.await();
    
        if(responseObserver.errorOccured())
        {
            Logger.logError("Error occured in initRuntime() of JAVA_RUNTIME: " + responseObserver.getErrorMessage());
            System.exit(0);
        }

        return true;
    }


    private DataPackage makeControlRuntimePing()
    {
        DataPackage.Builder builder = DataPackage.newBuilder();
        ControlPackage.Builder controlPackageBuilder = ControlPackage.newBuilder();
        controlPackageBuilder.setCtrlType(CtrlType.CTRL_RUNTIME_PING);
        controlPackageBuilder.setRuntime(Runtime.RUNTIME_JAVA);

        builder.setControlVal(controlPackageBuilder.build());
        return builder.build();
    }

    private StreamObserver<DataPackage> makeInputStreamObserver(Consumer<DataPackage> onData, Consumer<Throwable> onError, Runnable onCompleted)
    {
        return new StreamObserver<DataPackage>()
            {
                @Override
                public void onNext(DataPackage incomingPackage) {
                    onData.accept(incomingPackage);
                }

                @Override
                public void onError(Throwable throwable) {
                    onError.accept(throwable);
                }

                @Override
                public void onCompleted() {
                    onCompleted.run();
                }
            };
    }


    public boolean sendReceivePackages(Consumer<DataPackage> inConsumer)
    {
        this.inConsumer = inConsumer;
     
        if(inConsumer == null)
        {
            Logger.logFatal("Invalid argument in ModuleDispatcher::sendReceivePackages. Provided consumer is null.");
            return false;
        }
     
        this.waitForInputStreamCancelled = false;
        this.inputStreamCancelled = false;
        this.inStream = makeInputStreamObserver(
                dataPackage -> onMiddlewareStreamPackageReceived(dataPackage),
                error -> onMiddlewareStreamError(error),
                () -> onMiddlewareStreamCompleted()); 

        this.outStream = stub.sendReceivePackages(this.inStream);
   
        if(this.outStream == null)
        {
            return false;
        }

        this.waitingForPingResponse = true;
        DataPackage pingReq = makeControlRuntimePing();
        this.outStream.onNext(pingReq);

        awaitPingPackage();

        return true;

        // if (!stream->Write(pingReq)) {
        //     claid::Logger::logInfo("Failed sending ping package to server.");
        //     return false;
        // }

        // // Wait for the valid response ping
        // DataPackage pingResp;
        // if (!stream->Read(&pingResp)) {
        //     claid::Logger::logInfo("Did not receive a ping package from server !");
        //     return false;
        // }

        // TODO: @Stephan, shouldn't this here be pingResp ?
        // @Patrick:
        // // Check whether the package read was a control package with the right type.
        // if (pingReq.control_val().ctrl_type() != CtrlType::CTRL_RUNTIME_PING) {
        //     return false;
        // }

        // // Start the threads to service the input/output queues.
        // writeThread = make_unique<thread>([this]() { processWriting(); });
        // readThread = make_unique<thread>([this]() { processReading(); });
        // return true;*/

    }

    private void awaitPingPackage()
    {
        while(this.waitingForPingResponse)
        {
            try
            {
                Thread.sleep(50);
            }
            catch(InterruptedException e)
            {
                Logger.logError("ModuleDispatcher error in awaitPingPackage: " + e.getMessage());
            }
        }
    }

    private void onMiddlewareStreamPackageReceived(DataPackage packet)
    {
        Logger.logInfo("Java Runtime received message from middleware: " + packet);
        
        if(this.waitingForPingResponse)
        {
            if(packet.getControlVal().getCtrlType() != CtrlType.CTRL_RUNTIME_PING)
            {
                Logger.logError("Error in ModuleDispatcher: Sent CTRL_RUNTIME_PING to middleware and was waiting for a package with the same type as response,\n"
                + "but got package of type " + packet.getControlVal());
                return;
            }

            if(packet.getControlVal().getRuntime() != Runtime.RUNTIME_JAVA)
            {  
                Logger.logError("Error in ModuleDispatcher: Sent CTRL_RUNTIME_PING to middleware and reveived response, however response has wrong Runtime identifier.\n" + 
                "Expected RUNTIME_JAVA but got " + packet.getControlVal().getRuntime() + "\n");
                return;
            }

            Logger.logInfo("Java ModuleDispatcher successfully registered at the Middleware!");

            this.waitingForPingResponse = false;
            return;
        }
        
        // Here, we could check for control vals before forwarding the package to the ModuleManager via the inConsumer
       
        Logger.logInfo("Received package " + packet.getControlVal());
        this.inConsumer.accept(packet);

        
    }

    private void onMiddlewareStreamError(Throwable throwable)
    {
        Logger.logWarning("Middleware stream closed! " + throwable.getMessage() + "\n"
        + "This might happen when reloading a configuration and restarting the JAVA_RUNTIME.");
        
        if(this.waitForInputStreamCancelled)
        {
            this.inputStreamCancelled = true;
        }
       // this.closeOutputStream();
    }

    private void onMiddlewareStreamCompleted()
    {
        Logger.logError("Middleware stream completed!");
       // this.closeOutputStream();
    }

    public void closeOutputStream()
    {
        this.outStream.onCompleted();
        this.outStream = null;
    }


    boolean postPackage(DataPackage packet)
    {
        this.outStream.onNext(packet);
        return false;
    }

    boolean wasInputStreamCancelled()
    {
        return this.inputStreamCancelled;
    }

    void shutdown()
    {
       
        // See: https://github.com/grpc/grpc-java/issues/3095#issuecomment-338724284
        Logger.logInfo("Cancel 1");

        this.waitForInputStreamCancelled = true;
        this.outStream.onError(Status.CANCELLED.asException());
       // ClientCallStreamObserver clientCallObserver = (ClientCallStreamObserver) this.inStream;
      
    }



    // private ClaidService stub;
}