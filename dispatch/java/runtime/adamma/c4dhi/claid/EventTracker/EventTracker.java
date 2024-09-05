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

package adamma.c4dhi.claid.EventTracker;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid.PowerProfile;
import adamma.c4dhi.claid.Logger.Logger;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.Path;
import java.time.Duration;
import java.time.LocalDateTime;
import java.time.ZoneOffset;
// Pendant to the C++ EventTracker of CLAID.
// Manages events from the Java side.
// Gets the path to the storage folder (common data path) directly from the Middleware.
// If the folder does not exist, the Java EventTracker will not create it. We trust C++.
public class EventTracker 
{

    public String getTimeString() 
    {
        LocalDateTime time = LocalDateTime.now();
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("dd.MM.yy HH:mm:ss");
        return time.format(formatter);
    }

    public void logModuleEvent(String event, String moduleId, String moduleType)
    {
        logModuleEvent(event, moduleId, moduleType, "");
    }

    public void logModuleEvent(String event, String moduleId, String moduleType, String extra) 
    {
        String storageFolderPath = CLAID.getCommonDataPath();
        File storageFolder = new File(storageFolderPath);
        if (storageFolderPath.equals("") || !storageFolder.isDirectory()) 
        {
            return;
        }

        LocalDateTime time = LocalDateTime.now();
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("dd.MM.yy HH:mm:ss");
        String timeString = time.format(formatter);
        long milliseconds = time.atZone(ZoneId.systemDefault()).toInstant().toEpochMilli();


        Path path = Paths.get(storageFolderPath, moduleId + "_events.txt");
        try (FileWriter file = new FileWriter(path.toString(), true)) 
        {
            if (!extra.isEmpty()) {
                file.write(timeString + ", " + milliseconds + ", " + event + ", " + moduleId + ", " + moduleType + ", " + extra + "\n");
            } else {
                file.write(timeString + ", " + milliseconds + ", " + event + ", " + moduleId + ", " + moduleType + "\n");
            }
        } 
        catch (IOException e) 
        {
            Logger.logError("EventTracker failed to open file \"" + path.toString() + "\". Cannot store event \"" + event + "\" for Module \"" + moduleId + "\" (" + moduleType + ")");
        }
    }

    public void onModuleStarted(String moduleId, String moduleType) 
    {
        logModuleEvent("OnModuleStarted", moduleId, moduleType);
    }

    public void onModuleStopped(String moduleId, String moduleType) 
    {
        logModuleEvent("OnModuleStopped", moduleId, moduleType);
    }

    public void onModulePaused(String moduleId, String moduleType) 
    {
        logModuleEvent("OnModulePaused", moduleId, moduleType);
    }

    public void onModuleResumed(String moduleId, String moduleType) 
    {
        logModuleEvent("OnModuleResumed", moduleId, moduleType);
    }

    public void onModulePowerProfileApplied(String moduleId, String moduleType, PowerProfile powerProfile)
    {
        String powerProfileInfo = powerProfile.toString();
        powerProfileInfo = powerProfileInfo.replace("\n", "");
        logModuleEvent("OnModulePowerProfileApplied", moduleId, moduleType, powerProfileInfo);
    }

}
