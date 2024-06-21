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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.ReentrantLock;

public class FuturesTable 
{
    private Map<FutureUniqueIdentifier, AbstractFuture> futures = new HashMap<>();
    private final ReentrantLock lock = new ReentrantLock();

    public void addFuture(AbstractFuture future)
    {
        lock.lock();
        futures.put(future.getUniqueIdentifier(), future);
        lock.unlock();
    }

    public boolean removeFuture(AbstractFuture future)
    {
        FutureUniqueIdentifier futureIdentifier = future.getUniqueIdentifier();

        lock.lock();

        if(futures.containsKey(futureIdentifier))
        {
            AbstractFuture other = futures.get(futureIdentifier);
            if(other == future)
            {
                futures.remove(futureIdentifier);
                lock.unlock();
                return true;
            }
        }
        lock.unlock();
        return false;
    }

    public AbstractFuture lookupFuture(FutureUniqueIdentifier uniqueIdentifier)
    {
        AbstractFuture future = null;
        lock.lock();
        if(this.futures.containsKey(uniqueIdentifier))
        {
            future = this.futures.get(uniqueIdentifier);
        }
        lock.unlock();
        return future;
    }
    
}
