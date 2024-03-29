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

package adamma.c4dhi.claid_android.Permissions;
import adamma.c4dhi.claid_android.Permissions.Permission;
import adamma.c4dhi.claid.Module.ThreadSafeChannel;
// Executes Permission.blockingRequest() on a separate thread for incoming permissions.
public class PermissionHelperThread 
{
    private Thread thread;
    private ThreadSafeChannel<Permission> permissionsChannel;
    private boolean started = false;


    private void processPermissions()
    {
        while(this.started)
        {
            Permission permission = permissionsChannel.blockingGet();
            permission.blockingRequest();
        }
    }

    public void insertRunnable(Permission permission)
    {
        this.permissionsChannel.add(permission);
    }

    public boolean start()
    {
        if(this.started)
        {
            return false;
        }

        this.started = true;
        this.thread = new Thread(() -> processPermissions());
        this.thread.start();
        return true;
    }

    public boolean stop()
    {
        if(!this.started)
        {
            return false;
        }

        this.started = false;

        try{
            this.thread.join();
        }
        catch(InterruptedException e)
        {
            e.printStackTrace();
        }

        return true;
    }
}
