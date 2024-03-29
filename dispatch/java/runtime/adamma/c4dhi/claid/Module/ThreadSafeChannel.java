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

package adamma.c4dhi.claid.Module;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.locks.ReentrantLock;

import adamma.c4dhi.claid.Logger.Logger;

public class ThreadSafeChannel<T>
{
    // BlockingQueue is thred safe: https://docs.oracle.com/javase%2F7%2Fdocs%2Fapi%2F%2F/java/util/concurrent/BlockingQueue.html
    BlockingQueue<T> queue = new LinkedBlockingQueue<>();

    public void add(T element)
    {
        queue.add(element);
    }

    public T blockingGet()
    {
        T value;
        try 
        {
            value = queue.take();
        } 
        catch (InterruptedException e) 
        {
            Logger.logWarning("ThreadSafeChannel interrupted exception (no error, just a warning): " + e.getMessage());
            value = null;
        }
    
        return value;
    }
}
