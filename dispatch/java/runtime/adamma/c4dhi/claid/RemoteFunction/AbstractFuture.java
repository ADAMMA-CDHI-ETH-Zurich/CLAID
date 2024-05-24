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

package adamma.c4dhi.claid.RemoteFunction;

import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import adamma.c4dhi.claid.RemoteFunction.FutureUniqueIdentifier;
import adamma.c4dhi.claid.RemoteFunction.FuturesTable;
import adamma.c4dhi.claid.Blob;
import adamma.c4dhi.claid.DataPackage;


public abstract class AbstractFuture 
{
    private Boolean finished = false;
    private DataPackage responsePackage = null;
    private boolean successful = false;

    private final Lock mutex = new ReentrantLock();
    private final Condition conditionVariable = mutex.newCondition();

    // The list used by the FutureHandler to manage its associated futures.
    // Note: FuturesTable is thread safe.
    private FuturesTable futuresTableInHandler = null;

    private FutureUniqueIdentifier uniqueIdentifier = new FutureUniqueIdentifier();

    public AbstractFuture(FuturesTable futuresTableInHandler, FutureUniqueIdentifier uniqueIdentifier)
    {
        this.listOfFuturesInHandler = futuresTableInHandler;
        this.uniqueIdentifier = uniqueIdentifier;
    }

    protected DataPackage awaitResponse()
    {
        lock.lock();
        try 
        {
            while (!finished) 
            {
                condition.await();
            }

            // Future is finished or has failed, remove it from the list of the handler.
            if(this.futuresTableInHandler != null)
            {
                // This is thread safe, as FuturesList is thread safe
                this.futuresTableInHandler.removeFuture(this);
            }

            if(this.successful)
            {
                return this.responsePackage;
            }
            // Otherwise, null will be returned
        } 
        catch (InterruptedException e) 
        {
            Thread.currentThread().interrupt();
        } 
        finally 
        {
            lock.unlock();
        }
        return null;
    }

    public void setResponse(DataPackage responsePackage) 
    {
        lock.lock();
        try 
        {
            this.responsePackage = responsePackage;
            this.successful = true;
            this.finished = true;
            this.condition.signalAll();
        } 
        finally 
        {
            lock.unlock();
        }
    }

    public void setFailed()
    {
        lock.lock();
        try 
        {
            this.responsePackage = null;
            this.successful = false;
            this.finished = true;
            this.condition.signalAll();
        } 
        finally 
        {
            lock.unlock();
        }

    }

    public FutureUniqueIdentifier getUniqueIdentifier()
    {
        return this.uniqueIdentifier;
    }

    public boolean wasExecutedSuccessfully()
    {
        return this.successful && this.finished;
    }
}
