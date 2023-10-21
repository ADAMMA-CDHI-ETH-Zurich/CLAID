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

import io.grpc.Channel;
import io.grpc.ManagedChannel;
import io.grpc.Grpc;
import io.grpc.InsecureChannelCredentials;
import io.grpc.stub.StreamObserver;
import io.grpc.Status;
import io.grpc.StatusRuntimeException;

import com.google.protobuf.Empty;

import java.util.ArrayList;
import java.util.Map;


// Corresponds to ClientDispatcher of the C++ core, which is ModuleDispatcher in dart.
public class ModuleDispatcher  
{
    Channel grpcChannel;
    private ClaidServiceStub stub;
    
    private String socketPath;

    // Stream for us to receive packages from the Middleware.
    StreamObserver<DataPackage> inStream;

    // Stream for us to send packages to the Middleware
    StreamObserver<DataPackage> outStream;


    public ModuleDispatcher(final String socketPath)
    {
        this.socketPath = socketPath;

        this.grpcChannel = Grpc.newChannelBuilder(this.socketPath, InsecureChannelCredentials.create()).build();
        this.stub = ClaidServiceGrpc.newStub(this.grpcChannel);
    }

    // Tells the Middleware Server which Modules are available by this Runtime.
    // The Middleware will resond with the List of Modules that it wants this Runtime to initialize.
    // In general: Once the Middleware has started, it loads a config file. From the config file, it knows
    // which Modules to instantiate, but it does not yet know which Runtime (specifically, the ModuleDispatchers of each Runtime) can load which Module.
    // Therefore, it relies on the Runtimes to tell the Middleware which Modules they can handle. They do so by calling the stub.GetModuleList() function,
    // which contains a ModuleListRequest, which contains a list of all Modules the Runtime can handle.
    // The Middleware then replies with a ModuleListResponse, which contains the Modules it wants this Runtime to load (i.e., a subset of the Modules that were
    // sent in the ModuleListRequest).
    public ModuleListResponse getModuleList(ArrayList<String> registeredModuleClasses)
    {
        ModuleListRequest request = 
            ModuleListRequest.newBuilder()
            .setRuntime(Runtime.RUNTIME_JAVA)
            .addAllSupportedModuleClasses(registeredModuleClasses)
            .build();
        
        // stub.getModuleList() -> calls getModuleList function of RemoteService in the MiddleWare via RPC.

        boolean resultReceived = false;
        ModuleListResponse response = null;

        SynchronizedStreamObserver<ModuleListResponse> responseObserver = new SynchronizedStreamObserver<>();
        
        Logger.logInfo("Java Runtime: Calling getModuleList(...)");
        stub.getModuleList(request, responseObserver);

        response = responseObserver.await();
 

        return response;
    }


    public boolean initRuntime(Map<String, DataPackage> channelExamplePackages)
    {
        InitRuntimeRequest.Builder initRuntimeRequest = InitRuntimeRequest.newBuilder();
        
        for(Map.Entry<String, DataPackage> entry : channelExamplePackages.entrySet())
        {
            InitRuntimeRequest.ModuleChannels.Builder moduleChannels = InitRuntimeRequest.ModuleChannels.newBuilder();
            moduleChannels.setModuleId(entry.getKey());
            moduleChannels.addChannelPackets(entry.getValue());
            initRuntimeRequest.addModules(moduleChannels.build());
        }   
        initRuntimeRequest.setRuntime(Runtime.RUNTIME_JAVA);

        boolean resultReceived = false;
        Empty response = null;

        SynchronizedStreamObserver<Empty> responseObserver = new SynchronizedStreamObserver<>();

        Logger.logInfo("Java Runtime: Calling initRuntime(...)");
        stub.initRuntime(initRuntimeRequest.build(), responseObserver);

        response = responseObserver.await();
    
        
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

    public StreamObserver<DataPackage> sendReceivePackages(StreamObserver<DataPackage> inStream)
    {
        this.inStream = inStream;
        this.outStream = stub.sendReceivePackages(this.inStream);
   
  
        DataPackage pingReq = makeControlRuntimePing();
        this.outStream.onNext(pingReq);

        return this.outStream;

        // if (!stream->Write(pingReq)) {
        //     claid::Logger::printfln("Failed sending ping package to server.");
        //     return false;
        // }

        // // Wait for the valid response ping
        // DataPackage pingResp;
        // if (!stream->Read(&pingResp)) {
        //     claid::Logger::printfln("Did not receive a ping package from server !");
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


    


    public void closeOutputStream()
    {
        this.outStream.onCompleted();
        this.outStream = null;
    }


    boolean postPackage(DataPackage packet)
    {
        return false;
    }



    // private ClaidService stub;
}
