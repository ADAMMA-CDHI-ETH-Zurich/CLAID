package adamma.c4dhi.claid;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler",
    comments = "Source: dispatch/proto/claidservice.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ClaidRemoteServiceGrpc {

  private ClaidRemoteServiceGrpc() {}

  public static final String SERVICE_NAME = "claidservice.ClaidRemoteService";

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
    if ((getSendReceivePackagesMethod = ClaidRemoteServiceGrpc.getSendReceivePackagesMethod) == null) {
      synchronized (ClaidRemoteServiceGrpc.class) {
        if ((getSendReceivePackagesMethod = ClaidRemoteServiceGrpc.getSendReceivePackagesMethod) == null) {
          ClaidRemoteServiceGrpc.getSendReceivePackagesMethod = getSendReceivePackagesMethod =
              io.grpc.MethodDescriptor.<adamma.c4dhi.claid.DataPackage, adamma.c4dhi.claid.DataPackage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.BIDI_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SendReceivePackages"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  adamma.c4dhi.claid.DataPackage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  adamma.c4dhi.claid.DataPackage.getDefaultInstance()))
              .setSchemaDescriptor(new ClaidRemoteServiceMethodDescriptorSupplier("SendReceivePackages"))
              .build();
        }
      }
    }
    return getSendReceivePackagesMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ClaidRemoteServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ClaidRemoteServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ClaidRemoteServiceStub>() {
        @java.lang.Override
        public ClaidRemoteServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ClaidRemoteServiceStub(channel, callOptions);
        }
      };
    return ClaidRemoteServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ClaidRemoteServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ClaidRemoteServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ClaidRemoteServiceBlockingStub>() {
        @java.lang.Override
        public ClaidRemoteServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ClaidRemoteServiceBlockingStub(channel, callOptions);
        }
      };
    return ClaidRemoteServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ClaidRemoteServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ClaidRemoteServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ClaidRemoteServiceFutureStub>() {
        @java.lang.Override
        public ClaidRemoteServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ClaidRemoteServiceFutureStub(channel, callOptions);
        }
      };
    return ClaidRemoteServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public static abstract class ClaidRemoteServiceImplBase implements io.grpc.BindableService {

    /**
     */
    public io.grpc.stub.StreamObserver<adamma.c4dhi.claid.DataPackage> sendReceivePackages(
        io.grpc.stub.StreamObserver<adamma.c4dhi.claid.DataPackage> responseObserver) {
      return io.grpc.stub.ServerCalls.asyncUnimplementedStreamingCall(getSendReceivePackagesMethod(), responseObserver);
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
          .build();
    }
  }

  /**
   */
  public static final class ClaidRemoteServiceStub extends io.grpc.stub.AbstractAsyncStub<ClaidRemoteServiceStub> {
    private ClaidRemoteServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ClaidRemoteServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ClaidRemoteServiceStub(channel, callOptions);
    }

    /**
     */
    public io.grpc.stub.StreamObserver<adamma.c4dhi.claid.DataPackage> sendReceivePackages(
        io.grpc.stub.StreamObserver<adamma.c4dhi.claid.DataPackage> responseObserver) {
      return io.grpc.stub.ClientCalls.asyncBidiStreamingCall(
          getChannel().newCall(getSendReceivePackagesMethod(), getCallOptions()), responseObserver);
    }
  }

  /**
   */
  public static final class ClaidRemoteServiceBlockingStub extends io.grpc.stub.AbstractBlockingStub<ClaidRemoteServiceBlockingStub> {
    private ClaidRemoteServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ClaidRemoteServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ClaidRemoteServiceBlockingStub(channel, callOptions);
    }
  }

  /**
   */
  public static final class ClaidRemoteServiceFutureStub extends io.grpc.stub.AbstractFutureStub<ClaidRemoteServiceFutureStub> {
    private ClaidRemoteServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ClaidRemoteServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ClaidRemoteServiceFutureStub(channel, callOptions);
    }
  }

  private static final int METHODID_SEND_RECEIVE_PACKAGES = 0;

  private static final class MethodHandlers<Req, Resp> implements
      io.grpc.stub.ServerCalls.UnaryMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ServerStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ClientStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.BidiStreamingMethod<Req, Resp> {
    private final ClaidRemoteServiceImplBase serviceImpl;
    private final int methodId;

    MethodHandlers(ClaidRemoteServiceImplBase serviceImpl, int methodId) {
      this.serviceImpl = serviceImpl;
      this.methodId = methodId;
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public void invoke(Req request, io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
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

  private static abstract class ClaidRemoteServiceBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoFileDescriptorSupplier, io.grpc.protobuf.ProtoServiceDescriptorSupplier {
    ClaidRemoteServiceBaseDescriptorSupplier() {}

    @java.lang.Override
    public com.google.protobuf.Descriptors.FileDescriptor getFileDescriptor() {
      return adamma.c4dhi.claid.ClaidJavaService.getDescriptor();
    }

    @java.lang.Override
    public com.google.protobuf.Descriptors.ServiceDescriptor getServiceDescriptor() {
      return getFileDescriptor().findServiceByName("ClaidRemoteService");
    }
  }

  private static final class ClaidRemoteServiceFileDescriptorSupplier
      extends ClaidRemoteServiceBaseDescriptorSupplier {
    ClaidRemoteServiceFileDescriptorSupplier() {}
  }

  private static final class ClaidRemoteServiceMethodDescriptorSupplier
      extends ClaidRemoteServiceBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoMethodDescriptorSupplier {
    private final String methodName;

    ClaidRemoteServiceMethodDescriptorSupplier(String methodName) {
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
      synchronized (ClaidRemoteServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .setSchemaDescriptor(new ClaidRemoteServiceFileDescriptorSupplier())
              .addMethod(getSendReceivePackagesMethod())
              .build();
        }
      }
    }
    return result;
  }
}
