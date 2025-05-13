package adamma.c4dhi.claid.TypeMapping;

import java.lang.reflect.Type;

import adamma.c4dhi.claid.Blob;
import adamma.c4dhi.claid.CLAIDANY;
import adamma.c4dhi.claid.DataPackage;

public class AnyProtoType {

    private Blob blob;

    public AnyProtoType() {
        CLAIDANY any = CLAIDANY.getDefaultInstance();
        ProtoCodec codec = TypeMapping.getProtoCodec(any);
        this.blob = codec.encode(any);
    }

    public void setBlob(Blob blob) {
        this.blob = blob;
    }

    public Blob getBlob() {
        return this.blob;
    }
}
