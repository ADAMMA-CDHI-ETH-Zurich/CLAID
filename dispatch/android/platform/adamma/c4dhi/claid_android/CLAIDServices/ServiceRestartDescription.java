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

package adamma.c4dhi.claid_android.CLAIDServices;

import adamma.c4dhi.claid.Logger.Logger;

import java.io.*;
import java.util.HashMap;
import java.util.Map;

public class ServiceRestartDescription
{
    private Map<String, String> properties;

    public ServiceRestartDescription() {
        this.properties = new HashMap<>();
    }

    public void put(String key, String value) {
        properties.put(key, value);
    }

    public Map<String, String> getProperties() {
        return properties;
    }

    public boolean serializeToFile(String fileName) {
        try (ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(fileName))) {
            oos.writeObject(this.properties);
            Logger.logInfo("Serialization successful. Object written to file: " + fileName);
            return true;
        } catch (IOException e) {
            Logger.logError("Error writing to file \"" + fileName + "\".");
            e.printStackTrace();
            return false;
        }
    }

    public boolean deserializeFromFile(String fileName) {
        try (ObjectInputStream ois = new ObjectInputStream(new FileInputStream(fileName))) {
            Object obj = ois.readObject();
            if (obj instanceof Map) {
                this.properties = (Map) obj;
                Logger.logInfo("Deserialization successful. Object read from file: " + fileName);
                return true;
            } else {
                Logger.logError("Error: The file does not contain a ServiceRestartDescription Map object.");
                return false;
            }
        } catch (IOException | ClassNotFoundException e) {
            e.printStackTrace();
            Logger.logError("Failed to parse file \"" + fileName + "\".");
            return false;

        }
    }

    public String get(final String key)
    {
        return this.properties.get(key);
    }

}
