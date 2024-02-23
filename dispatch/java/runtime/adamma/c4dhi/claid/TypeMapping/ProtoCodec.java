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

 

  
    ProtoCodec(final GeneratedMessageV3 msg) 
    {
        this.msg = msg;
        this.fullName = msg.getDescriptorForType().getFullName();

    }

    
    public <T extends GeneratedMessageV3> Blob encode(T protoMessage)
    {
        ByteString serializedData = ByteString.copyFrom(protoMessage.toByteArray());
        
        Blob.Builder blobBuilder = Blob.newBuilder();
        blobBuilder.setCodec(Codec.CODEC_PROTO);
        blobBuilder.setPayload(serializedData);
        blobBuilder.setMessageType(fullName);

        return blobBuilder.build();
    }

    public GeneratedMessageV3 decode(Blob blob)
    {
        GeneratedMessageV3.Builder messageBuilder = (GeneratedMessageV3.Builder) this.msg.newBuilderForType();

        
           
            try
            {
                return (GeneratedMessageV3) messageBuilder.mergeFrom(blob.getPayload().toByteArray()).build();
            }
            catch(InvalidProtocolBufferException e)
            {
                Logger.logError(e.getMessage());
                return null;
            }
        
        
    }
}
