package adamma.c4dhi.claid;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler",
    comments = "Source: dispatch/proto/claidservice.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ClaidServiceGrpc {

  private ClaidServiceGrpc() {}

  public static final String SERVICE_NAME = "claidservice.ClaidService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<adamma.c4dhi.claid.DataPackage,
      adamma.c4dhi.claid.DataPackage> getSendReceivePackagesMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SendReceivePackages",
      requestType = adamma.c4dhi.claid.DataPackage.class,
      responseType = adamma.c4dhi.claid.DataPackage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.BIDI_STREAMING)
  public static io.grpc.MethodDescriptor<adamma.c4dhi.claid.DataPackage,
      adamma.c4dhi.claid.DataPackage> getSendReceivePackagesMethod() {
    io.grpc.MethodDescriptor<adamma.c4dhi.claid.DataPackage, adamma.c4dhi.claid.DataPackage> getSendReceivePackagesMethod;
    if ((getSendReceivePackagesMethod = ClaidServiceGrpc.getSendReceivePackagesMethod) == null) {
      synchronized (ClaidServiceGrpc.class) {
        if ((getSendReceivePackagesMethod = ClaidServiceGrpc.getSendReceivePackagesMethod) == null) {
          ClaidServiceGrpc.getSendReceivePackagesMethod = getSendReceivePackagesMethod =
              io.grpc.MethodDescriptor.<adamma.c4dhi.claid.DataPackage, adamma.c4dhi.claid.DataPackage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.BIDI_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SendReceivePackages"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  adamma.c4dhi.claid.DataPackage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  adamma.c4dhi.claid.DataPackage.getDefaultInstance()))
              .setSchemaDescriptor(new ClaidServiceMethodDescriptorSupplier("SendReceivePackages"))
              .build();
        }
      }
    }
    return getSendReceivePackagesMethod;
  }

  private static volatile io.grpc.MethodDescriptor<adamma.c4dhi.claid.ModuleListRequest,
      adamma.c4dhi.claid.ModuleListResponse> getGetModuleListMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetModuleList",
      requestType = adamma.c4dhi.claid.ModuleListRequest.class,
      responseType = adamma.c4dhi.claid.ModuleListResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<adamma.c4dhi.claid.ModuleListRequest,
      adamma.c4dhi.claid.ModuleListResponse> getGetModuleListMethod() {
    io.grpc.MethodDescriptor<adamma.c4dhi.claid.ModuleListRequest, adamma.c4dhi.claid.ModuleListResponse> getGetModuleListMethod;
    if ((getGetModuleListMethod = ClaidServiceGrpc.getGetModuleListMethod) == null) {
      synchronized (ClaidServiceGrpc.class) {
        if ((getGetModuleListMethod = ClaidServiceGrpc.getGetModuleListMethod) == null) {
          ClaidServiceGrpc.getGetModuleListMethod = getGetModuleListMethod =
              io.grpc.MethodDescriptor.<adamma.c4dhi.claid.ModuleListRequest, adamma.c4dhi.claid.ModuleListResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetModuleList"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  adamma.c4dhi.claid.ModuleListRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  adamma.c4dhi.claid.ModuleListResponse.getDefaultInstance()))
              .setSchemaDescriptor(new ClaidServiceMethodDescriptorSupplier("GetModuleList"))
              .build();
        }
      }
    }
    return getGetModuleListMethod;
  }

  private static volatile io.grpc.MethodDescriptor<adamma.c4dhi.claid.InitRuntimeRequest,
      com.google.protobuf.Empty> getInitRuntimeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "InitRuntime",
      requestType = adamma.c4dhi.claid.InitRuntimeRequest.class,
      responseType = com.google.protobuf.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<adamma.c4dhi.claid.InitRuntimeRequest,
      com.google.protobuf.Empty> getInitRuntimeMethod() {
    io.grpc.MethodDescriptor<adamma.c4dhi.claid.InitRuntimeRequest, com.google.protobuf.Empty> getInitRuntimeMethod;
    if ((getInitRuntimeMethod = ClaidServiceGrpc.getInitRuntimeMethod) == null) {
      synchronized (ClaidServiceGrpc.class) {
        if ((getInitRuntimeMethod = ClaidServiceGrpc.getInitRuntimeMethod) == null) {
          ClaidServiceGrpc.getInitRuntimeMethod = getInitRuntimeMethod =
              io.grpc.MethodDescriptor.<adamma.c4dhi.claid.InitRuntimeRequest, com.google.protobuf.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "InitRuntime"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  adamma.c4dhi.claid.InitRuntimeRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  com.google.protobuf.Empty.getDefaultInstance()))
              .setSchemaDescriptor(new ClaidServiceMethodDescriptorSupplier("InitRuntime"))
              .build();
        }
      }
    }
    return getInitRuntimeMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ClaidServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ClaidServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ClaidServiceStub>() {
        @java.lang.Override
        public ClaidServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ClaidServiceStub(channel, callOptions);
        }
      };
    return ClaidServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ClaidServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ClaidServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ClaidServiceBlockingStub>() {
        @java.lang.Override
        public ClaidServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ClaidServiceBlockingStub(channel, callOptions);
        }
      };
    return ClaidServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ClaidServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ClaidServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ClaidServiceFutureStub>() {
        @java.lang.Override
        public ClaidServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ClaidServiceFutureStub(channel, callOptions);
        }
      };
    return ClaidServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public static abstract class ClaidServiceImplBase implements io.grpc.BindableService {

    /**
     */
    public io.grpc.stub.StreamObserver<adamma.c4dhi.claid.DataPackage> sendReceivePackages(
        io.grpc.stub.StreamObserver<adamma.c4dhi.claid.DataPackage> responseObserver) {
      return io.grpc.stub.ServerCalls.asyncUnimplementedStreamingCall(getSendReceivePackagesMethod(), responseObserver);
    }

    /**
     */
    public void getModuleList(adamma.c4dhi.claid.ModuleListRequest request,
        io.grpc.stub.StreamObserver<adamma.c4dhi.claid.ModuleListResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetModuleListMethod(), responseObserver);
    }

    /**
     */
    public void initRuntime(adamma.c4dhi.claid.InitRuntimeRequest request,
        io.grpc.stub.StreamObserver<com.google.protobuf.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getInitRuntimeMethod(), responseObserver);
    }

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return io.grpc.ServerServiceDefinition.builder(getServiceDescriptor())
          .addMethod(
            getSendReceivePackagesMethod(),
            io.grpc.stub.ServerCalls.asyncBidiStreamingCall(
              new MethodHandlers<
                adamma.c4dhi.claid.DataPackage,
                adamma.c4dhi.claid.DataPackage>(
                  this, METHODID_SEND_RECEIVE_PACKAGES)))
          .addMethod(
            getGetModuleListMethod(),
            io.grpc.stub.ServerCalls.asyncUnaryCall(
              new MethodHandlers<
                adamma.c4dhi.claid.ModuleListRequest,
                adamma.c4dhi.claid.ModuleListResponse>(
                  this, METHODID_GET_MODULE_LIST)))
          .addMethod(
            getInitRuntimeMethod(),
            io.grpc.stub.ServerCalls.asyncUnaryCall(
              new MethodHandlers<
                adamma.c4dhi.claid.InitRuntimeRequest,
                com.google.protobuf.Empty>(
                  this, METHODID_INIT_RUNTIME)))
          .build();
    }
  }

  /**
   */
  public static final class ClaidServiceStub extends io.grpc.stub.AbstractAsyncStub<ClaidServiceStub> {
    private ClaidServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ClaidServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ClaidServiceStub(channel, callOptions);
    }

    /**
     */
    public io.grpc.stub.StreamObserver<adamma.c4dhi.claid.DataPackage> sendReceivePackages(
        io.grpc.stub.StreamObserver<adamma.c4dhi.claid.DataPackage> responseObserver) {
      return io.grpc.stub.ClientCalls.asyncBidiStreamingCall(
          getChannel().newCall(getSendReceivePackagesMethod(), getCallOptions()), responseObserver);
    }

    /**
     */
    public void getModuleList(adamma.c4dhi.claid.ModuleListRequest request,
        io.grpc.stub.StreamObserver<adamma.c4dhi.claid.ModuleListResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetModuleListMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void initRuntime(adamma.c4dhi.claid.InitRuntimeRequest request,
        io.grpc.stub.StreamObserver<com.google.protobuf.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getInitRuntimeMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   */
  public static final class ClaidServiceBlockingStub extends io.grpc.stub.AbstractBlockingStub<ClaidServiceBlockingStub> {
    private ClaidServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ClaidServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ClaidServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public adamma.c4dhi.claid.ModuleListResponse getModuleList(adamma.c4dhi.claid.ModuleListRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetModuleListMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.google.protobuf.Empty initRuntime(adamma.c4dhi.claid.InitRuntimeRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getInitRuntimeMethod(), getCallOptions(), request);
    }
  }

  /**
   */
  public static final class ClaidServiceFutureStub extends io.grpc.stub.AbstractFutureStub<ClaidServiceFutureStub> {
    private ClaidServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ClaidServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ClaidServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<adamma.c4dhi.claid.ModuleListResponse> getModuleList(
        adamma.c4dhi.claid.ModuleListRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetModuleListMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.google.protobuf.Empty> initRuntime(
        adamma.c4dhi.claid.InitRuntimeRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getInitRuntimeMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_GET_MODULE_LIST = 0;
  private static final int METHODID_INIT_RUNTIME = 1;
  private static final int METHODID_SEND_RECEIVE_PACKAGES = 2;

  private static final class MethodHandlers<Req, Resp> implements
      io.grpc.stub.ServerCalls.UnaryMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ServerStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ClientStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.BidiStreamingMethod<Req, Resp> {
    private final ClaidServiceImplBase serviceImpl;
    private final int methodId;

    MethodHandlers(ClaidServiceImplBase serviceImpl, int methodId) {
      this.serviceImpl = serviceImpl;
      this.methodId = methodId;
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public void invoke(Req request, io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        case METHODID_GET_MODULE_LIST:
          serviceImpl.getModuleList((adamma.c4dhi.claid.ModuleListRequest) request,
              (io.grpc.stub.StreamObserver<adamma.c4dhi.claid.ModuleListResponse>) responseObserver);
          break;
        case METHODID_INIT_RUNTIME:
          serviceImpl.initRuntime((adamma.c4dhi.claid.InitRuntimeRequest) request,
              (io.grpc.stub.StreamObserver<com.google.protobuf.Empty>) responseObserver);
          break;
        default:
          throw new AssertionError();
      }
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public io.grpc.stub.StreamObserver<Req> invoke(
        io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        case METHODID_SEND_RECEIVE_PACKAGES:
          return (io.grpc.stub.StreamObserver<Req>) serviceImpl.sendReceivePackages(
              (io.grpc.stub.StreamObserver<adamma.c4dhi.claid.DataPackage>) responseObserver);
        default:
          throw new AssertionError();
      }
    }
  }

  private static abstract class ClaidServiceBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoFileDescriptorSupplier, io.grpc.protobuf.ProtoServiceDescriptorSupplier {
    ClaidServiceBaseDescriptorSupplier() {}

    @java.lang.Override
    public com.google.protobuf.Descriptors.FileDescriptor getFileDescriptor() {
      return adamma.c4dhi.claid.ClaidJavaService.getDescriptor();
    }

    @java.lang.Override
    public com.google.protobuf.Descriptors.ServiceDescriptor getServiceDescriptor() {
      return getFileDescriptor().findServiceByName("ClaidService");
    }
  }

  private static final class ClaidServiceFileDescriptorSupplier
      extends ClaidServiceBaseDescriptorSupplier {
    ClaidServiceFileDescriptorSupplier() {}
  }

  private static final class ClaidServiceMethodDescriptorSupplier
      extends ClaidServiceBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoMethodDescriptorSupplier {
    private final String methodName;

    ClaidServiceMethodDescriptorSupplier(String methodName) {
      this.methodName = methodName;
    }

    @java.lang.Override
    public com.google.protobuf.Descriptors.MethodDescriptor getMethodDescriptor() {
      return getServiceDescriptor().findMethodByName(methodName);
    }
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (ClaidServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .setSchemaDescriptor(new ClaidServiceFileDescriptorSupplier())
              .addMethod(getSendReceivePackagesMethod())
              .addMethod(getGetModuleListMethod())
              .addMethod(getInitRuntimeMethod())
              .build();
        }
      }
    }
    return result;
  }
}
