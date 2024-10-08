/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Francesco Feher
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

package adamma.c4dhi.claid_android.collectors.motion;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

import java.time.Duration;
import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.util.ArrayList;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.atomic.AtomicReference;
import java.util.Map;

import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Logger.Logger;

import adamma.c4dhi.claid_sensor_data.AccelerationSample;
import adamma.c4dhi.claid_sensor_data.AccelerationData;

import adamma.c4dhi.claid_platform_impl.CLAID;

import adamma.c4dhi.claid.Module.ModuleAnnotator;
import adamma.c4dhi.claid.Module.Properties;
import adamma.c4dhi.claid.*;

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;

import java.util.concurrent.TimeUnit;
import java.util.Map;
import java.util.TreeMap;
import java.util.List;
import java.sql.Time;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.ArrayList;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;


import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class AccelerometerCollector extends Module implements SensorEventListener 
{
    private Channel<AccelerationData> accelerationDataChannel;

    private ReentrantLock mutex = new ReentrantLock();
   
   
    AccelerationData oldAccelerationData = null;

    SensorManager sensorManager;
    Sensor sensor;

    private Integer samplingFrequency;
    private String outputMode;

    PowerProfile currentPowerProfile = null;

    private boolean samplingIsRunning = false;

    long lastSampleTime = 0;
    AccelerationData.Builder collectedSamples;

    public static void annotateModule(ModuleAnnotator annotator)
    {
        annotator.setModuleCategory("DataCollection");
        annotator.setModuleDescription("The AccelerometerCollector allows to record acceleration data using the devices built-in accelerometer."
        + "The sampling frequency can be freely configured, however is subject to the limitations of the device (i.e., built-in sensor specifications)."
        + "The AccelerometerCollector features two recording modes: \"Batched\" and \"Streaming\"\n");

        annotator.describeProperty("samplingFrequency", "Frequency in Hz with which to record acceleration data. Only decimal values allowed." + 
        "Subject to physical limitations of the device and Accelerometer.", annotator.makeIntegerProperty(1, 200));
        
        annotator.describeProperty("outputMode", "Two modes are available: \"BATCHED\" and \"STREAM\"."
        + "The BATCHED Mode is the normal mode for most scenarios. In this mode, acceleration data is aggregated and only posted to the output channel, "
        + "if the amount of samples spans 1 second (e.g., 50 samples if configured to 50Hz)."
        + "In the STREAM mode, each individual sample is posted to the channel without aggregation, which can be used for real-time scenarios.",
        annotator.makeEnumProperty(new String[]{"STREAM", "BATCHED"}));
    
        annotator.describePublishChannel("AccelerationData", AccelerationData.class, "Channel where the recorded Date will be streamed to.");
    }


    public void initialize(Properties properties)
    {
        this.samplingFrequency = properties.getNumberProperty("samplingFrequency", Integer.class);
        this.outputMode = properties.getStringProperty("outputMode");

        if(properties.wasAnyPropertyUnknown())
        {
            this.moduleFatal(properties.getMissingPropertiesErrorString());
            return;
        }

        if(!this.outputMode.toUpperCase().equals("STREAM") && !this.outputMode.toUpperCase().equals("BATCHED"))
        {
            moduleFatal("Unknown output mode \"" + this.outputMode + "\". Expected one of [STREAM, BATCHED].");
            return;
        }

        moduleInfo("AccelerometerCollector init " + this.samplingFrequency + " " + this.outputMode);

        this.collectedSamples = AccelerationData.newBuilder();
        this.accelerationDataChannel = this.publish("AccelerationData", AccelerationData.class);

        startSampling();
    }


    public synchronized void onNewAccelerationSample(AccelerationSample sample)
    {
        moduleInfo("On New sample");
        if(this.outputMode.toUpperCase().equals("STREAM"))
        {
            this.collectedSamples.addSamples(sample);
        
            this.accelerationDataChannel.post(this.collectedSamples.build(), sample.getUnixTimestampInMs());
            this.collectedSamples = AccelerationData.newBuilder();
        }
        else
        {
            this.collectedSamples.addSamples(sample);

            if(this.collectedSamples.getSamplesList().size() >= samplingFrequency)
            {

                this.accelerationDataChannel.post(collectedSamples.build(), sample.getUnixTimestampInMs());
                this.collectedSamples = AccelerationData.newBuilder();                
            }
        }
        
    }
  
    @Override
    public synchronized void onSensorChanged(SensorEvent sensorEvent)
    {
        if (sensorEvent.sensor.getType() == Sensor.TYPE_ACCELEROMETER)
        {
            if(System.currentTimeMillis() - lastSampleTime < 1000.0/this.samplingFrequency)
            {
                return;
            }   
            lastSampleTime = System.currentTimeMillis();

            double x = sensorEvent.values[0] ;/// SensorManager.GRAVITY_EARTH;
            double y = sensorEvent.values[1] ;/// SensorManager.GRAVITY_EARTH;
            double z = sensorEvent.values[2] ;/// SensorManager.GRAVITY_EARTH;
            LocalDateTime currentTime = LocalDateTime.now();

            AccelerationSample.Builder sample = AccelerationSample.newBuilder();
            sample.setAccelerationX((double) x);
            sample.setAccelerationY((double) y);
            sample.setAccelerationZ((double) z);
            sample.setSensorBodyLocation("unknown/smartphone");
            sample.setUnixTimestampInMs(currentTime.atZone(ZoneId.systemDefault()).toInstant().toEpochMilli());

            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:mm:ss'Z'");
            String formattedString = currentTime.format(formatter);
            sample.setEffectiveTimeFrame(formattedString);

            onNewAccelerationSample(sample.build());

        
           // System.out.println("Sensor data " +  x + " " +  y + " " + z);
        }
    }

    private static String getCurrentDateTime() {
        // Get the current date and time in the desired format
        SimpleDateFormat dateFormat = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss", Locale.getDefault());
        Date currentDate = new Date();
        return dateFormat.format(currentDate);
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {}


    public void startSampling()
    {
        if(samplingIsRunning)
        {
            return;
        }
        CLAID.acquireWakeLock(CLAID.getContext());

        moduleInfo("Starting sampling");

        sensorManager = (SensorManager) CLAID.getContext().getSystemService(Context.SENSOR_SERVICE); 

        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER); 

        boolean powerSavingMode = this.currentPowerProfile != null && this.currentPowerProfile.getPowerProfileType() == PowerProfileType.POWER_SAVING_MODE;
        if(powerSavingMode)
        {
            moduleWarning("PowerSaving mode is active, throttling SensorManager");
        }
        sensorManager.registerListener(this, sensor, powerSavingMode ? SensorManager.SENSOR_DELAY_NORMAL : SensorManager.SENSOR_DELAY_FASTEST);

        int samplingPerioid = 1000/this.samplingFrequency;

        samplingIsRunning = true;
    }

    public void stopSampling()
    {
        if(!samplingIsRunning)
        {
            return;
        }
        moduleWarning("Stopping sampling");
        sensorManager.unregisterListener(this, sensor);


        sensorManager = null;
        sensor = null;
        samplingIsRunning = false;
        samplingIsRunning = false;
        CLAID.releaseWakeLock(CLAID.getContext());
    }

    public void restartSampling()
    {
        // Stop sampling will release the wake lock.
        // If we do not acquire it here (increasing the reference counter),
        // it might get disabled.
        CLAID.acquireWakeLock(CLAID.getContext());
        stopSampling();
        startSampling();
        // Releasing the wake lock, as start sampling acquired one.
        CLAID.releaseWakeLock(CLAID.getContext());
    }

    @Override
    protected void terminate()
    {
        moduleInfo("Terminate called");
        stopSampling();
    }

    @Override
    protected void onPause()
    {
        moduleWarning("onPause called, unregistering from SensorManager");
        stopSampling();
    }

    @Override 
    protected void onResume()
    {
        moduleWarning("onResume called, registering at SensorManager");
        startSampling();
    }

    @Override
    protected void onPowerProfileChanged(PowerProfile profile)
    {
        moduleWarning("onPowerProfileChanged called, using profile: " + profile.toString().replace("\n", ""));

        if(this.currentPowerProfile == null)
        {
            this.currentPowerProfile = profile;
            this.samplingFrequency = (int) (this.currentPowerProfile.getFrequency());
            restartSampling();
            return;
        }
        else
        {
            if(this.currentPowerProfile.getPowerProfileType() != profile.getPowerProfileType() ||
                this.currentPowerProfile.getFrequency() != profile.getFrequency())
            {
                moduleInfo("PowerProfile has changed, restarting");
                this.currentPowerProfile = profile;
                this.samplingFrequency = (int) (this.currentPowerProfile.getFrequency());
                restartSampling();
            }
        }  

    }
}