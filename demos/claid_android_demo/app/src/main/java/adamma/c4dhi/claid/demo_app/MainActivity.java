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

package adamma.c4dhi.claid.demo_app;

import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.PendingIntent.FLAG_MUTABLE;
import static android.app.PendingIntent.FLAG_UPDATE_CURRENT;
import static android.app.PendingIntent.getBroadcast;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.WindowManager;

import java.io.IOException;
import java.util.Calendar;
import java.util.Enumeration;

import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid_android.Configuration.CLAIDPersistanceConfig;
import adamma.c4dhi.claid_android.Configuration.CLAIDSpecialPermissionsConfig;
import adamma.c4dhi.claid_android.Package.CLAIDPackageAnnotation;
import adamma.c4dhi.claid_android.Package.CLAIDPackageLoader;
import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid_platform_impl.PersistentModuleFactory;
import dalvik.system.DexFile;
import dalvik.system.PathClassLoader;

public class MainActivity extends Activity {
    PersistentModuleFactory factory;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(System.currentTimeMillis()+ 5000);
        //noinspection ConstantConditions

        CLAID.getAppDataDirectory(this);




        CLAID.registerModule(RPCTestModule1.class);
        CLAID.registerModule(RPCTestModule2.class);

        CLAID.startInPersistentService(getApplicationContext(),
                "assets://RPCTest.json",
                "Smartphone",
                "device",
                "user",
                CLAIDSpecialPermissionsConfig.almightyCLAID(),
                CLAIDPersistanceConfig.maximumPersistance());

        CLAID.onStarted(() -> CLAID.enableKeepAppAwake(CLAID.getContext()));


    }

    void scan() throws IOException, ClassNotFoundException, NoSuchMethodException {
        long timeBegin = System.currentTimeMillis();

        PathClassLoader classLoader = (PathClassLoader) this.getClassLoader();
        //PathClassLoader classLoader = (PathClassLoader) Thread.currentThread().getContextClassLoader();//This also works good
        DexFile dexFile = new DexFile(this.getPackageCodePath());
        Enumeration<String> classNames = dexFile.entries();
        while (classNames.hasMoreElements()) {

            String className = classNames.nextElement();
            if(className.contains(".claid."))
            {
                Class<?> aClass = classLoader.loadClass(className);//tested on 魅蓝Note(M463C)_Android4.4.4 and Mi2s_Android5.1.1
                if(aClass.isAnnotationPresent(CLAIDPackageAnnotation.class))
                {
                    System.out.println("Class " + className);

                }
            }
        }

        long timeEnd = System.currentTimeMillis();
        long timeElapsed = timeEnd - timeBegin;
    }
}
