package adamma.c4dhi.claid.LocalDispatching;

import io.grpc.stub.StreamObserver;
import java.lang.Throwable;

import adamma.c4dhi.claid.Logger.Logger;

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
        System.out.println("SynchronizedStreamObserver: onError " + throwable.getCause());
        resultReceived = true;
    }

    @Override
    public void onCompleted() {
        System.out.println("SynchronizedStreamObserver: completed ");
        resultReceived = true;
    }

    public T await()
    {
        while(!resultReceived)
        {
            try{
                // Sleep is necessary! Otherwise event queue does not get processed.
                Thread.sleep(5);
            }
            catch(InterruptedException e)
            {
                Logger.logError(e.getMessage());
                return null;
            }
        }
        System.out.println("Await return");
        this.resultReceived = false;
        return this.response;
    }
}
