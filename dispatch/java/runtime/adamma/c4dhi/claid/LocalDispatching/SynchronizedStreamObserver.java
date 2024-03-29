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

import io.grpc.stub.StreamObserver;
import java.lang.Throwable;

import adamma.c4dhi.claid.Logger.Logger;

public class SynchronizedStreamObserver<T> implements StreamObserver<T> 
{
    private boolean resultReceived = false;
    private boolean error = false;
    String errorMessage;
    T response;

    @Override
    public void onNext(T incomingResponse) 
    {
        response = incomingResponse;
        resultReceived = true;
    }

    @Override
    public void onError(Throwable throwable) {
        System.out.println("SynchronizedStreamObserver: onError ");
        error = true;
        errorMessage = throwable.getMessage();
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

    public boolean errorOccured()
    {
        return this.error;
    }

    public String getErrorMessage()
    {
        return errorMessage;
    }
}
