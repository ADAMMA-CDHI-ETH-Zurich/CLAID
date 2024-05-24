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

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class FutureUniqueIdentifier 
{
    private static final Lock mutex = new ReentrantLock();
    private static long currentId = 0;

    private String identifier;

    public static FutureUniqueIdentifier makeUniqueIdentifier()
    {
        mutex.lock();
        int idCopy = currentId;
        currentId++;
        mutex.unlock();

        String identifier = "CLAID_JAVA_" + idCopy + "_" + System.currentTimeMillis();
        return new FutureUniqueIdentifier(identifier);
    } 

    public static FutureUniqueIdentifier fromString(String identifier)
    {
        return new FutureUniqueIdentifier(identifier);
    }

    private FutureUniqueIdentifier(String id)
    {
        this.identifier = id;
    }

    public String toString()
    {
        return this.identifier;
    }

    public boolean equals(FutureUniqueIdentifier other)
    {
        return this.identifier.equals(other.toString());
    }

    @Override
    public boolean equals(Object o) 
    {
        if (this == o)
            return true;
        if (o == null || getClass() != o.getClass())
            return false;

        FutureUniqueIdentifier other = (FutureUniqueIdentifier) o;
        return this.identifier.equals(other.toString());
    }

    @Override
    public int hashCode() 
    {
        return this.identifier.hashCode();
    }
}
