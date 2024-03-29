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
