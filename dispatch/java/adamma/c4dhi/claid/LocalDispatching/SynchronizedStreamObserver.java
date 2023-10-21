package adamma.c4dhi.claid.LocalDispatching;

import io.grpc.stub.StreamObserver;
import java.lang.Throwable;

public class SynchronizedStreamObserver<T> implements StreamObserver<T> 
{
    private boolean resultReceived = false;
    T response;

    @Override
    public void onNext(T incomingResponse) 
    {
        response = incomingResponse;
        resultReceived = true;
    }

    @Override
    public void onError(Throwable throwable) {
        resultReceived = true;
    }

    @Override
    public void onCompleted() {
        resultReceived = true;
    }

    public T await()
    {
        while(!resultReceived)
        {}
        this.resultReceived = false;
        return this.response;
    }
}
