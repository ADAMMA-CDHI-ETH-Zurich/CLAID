package adamma.c4dhi.claid.TypeMapping;

import adamma.c4dhi.claid.Blob;
import adamma.c4dhi.claid.Codec;
import adamma.c4dhi.claid.Logger.Logger;

import com.google.protobuf.GeneratedMessageV3;

import java.util.function.BiFunction;
import java.util.function.Function;
import com.google.protobuf.ByteString;
import com.google.protobuf.Message;
import com.google.protobuf.InvalidProtocolBufferException;

class ProtoCodec {
    private final String fullName;
    private final GeneratedMessageV3 msg;

    // Corresponds to dart -> ProtoEncoder
    private final Function<GeneratedMessageV3, Blob> encoder;

    // Corresponds to dart -> ProtoEncoder
    private final Function<Blob, GeneratedMessageV3> decoder;

  
    ProtoCodec(final GeneratedMessageV3 msg) 
    {
        this.msg = msg;
        this.fullName = msg.getDescriptorForType().getFullName();
        this.encoder = buildEncoder();
        this.decoder = buildDecoder();
    }

    // Function<T, R>
    private Function<GeneratedMessageV3, Blob> buildEncoder() {
        return (protoMessage) -> {
            ByteString serializedData = ByteString.copyFrom(protoMessage.toByteArray());

            Blob.Builder blobBuilder = Blob.newBuilder();
            blobBuilder.setCodec(Codec.CODEC_PROTO);
            blobBuilder.setPayload(serializedData);
            blobBuilder.setMessageType(fullName);

            return blobBuilder.build();
        };
    }

    // Function<T, R>
    private Function<Blob, GeneratedMessageV3> buildDecoder() 
    {
        GeneratedMessageV3.Builder messageBuilder = (GeneratedMessageV3.Builder) this.msg.newBuilderForType();

        return (blob) -> {
            Blob typedBlob = (Blob) blob;
           
            try
            {
                return (GeneratedMessageV3) messageBuilder.mergeFrom(typedBlob.getPayload().toByteArray()).build();
            }
            catch(InvalidProtocolBufferException e)
            {
                Logger.logError(e.getMessage());
                return null;
            }
        
        };  
    }

    public Blob encode(GeneratedMessageV3 t)
    {
        return encoder.apply( t);
    }

    public GeneratedMessageV3 decode(Blob blob)
    {
        return (GeneratedMessageV3) decoder.apply(blob);
    }
}
