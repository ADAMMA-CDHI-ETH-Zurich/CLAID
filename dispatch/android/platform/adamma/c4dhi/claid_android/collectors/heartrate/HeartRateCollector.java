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

package adamma.c4dhi.claid_android.collectors.heartrate;

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

import adamma.c4dhi.claid_sensor_data.HeartRateSample;
import adamma.c4dhi.claid_sensor_data.HeartRateData;
import adamma.c4dhi.claid_sensor_data.HeartRateStatus;

import adamma.c4dhi.claid_platform_impl.CLAID;

import adamma.c4dhi.claid.Module.ModuleAnnotator;
import adamma.c4dhi.claid.Module.Properties;

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;

public class HeartRateCollector extends Module implements SensorEventListener 
{
    private Channel<HeartRateData> HeartRateDataChannel;


    private ReentrantLock mutex = new ReentrantLock();
   
   
    // Volatile to be thread safe.
    private volatile AtomicReference<HeartRateSample> latestSample = new AtomicReference<>();
    private ArrayList<HeartRateSample> collectedHeartRateSamples = new ArrayList<HeartRateSample>();

    HeartRateData oldHeartRateData = null;

    SensorManager sensorManager;
    Sensor sensor;

    private Integer samplingFrequency;
    private String outputMode;

    public static void annotateModule(ModuleAnnotator annotator)
    {
        annotator.setModuleCategory("DataCollection");
        annotator.setModuleDescription("The HeartRateCollector allows to record heart rate data using PPG sensor of the device.."
        + "The sampling frequency can be freely configured, however is subject to the limitations of the device (i.e., built-in sensor specifications)."
        + "The HeartRateCollector features two recording modes: \"Batched\" and \"Streaming\"\n");

        annotator.describeProperty("samplingFrequency", "Frequency in Hz with which to record acceleration data. Only decimal values allowed." + 
        "Subject to physical limitations of the device and Accelerometer.", annotator.makeIntegerProperty(1, 10));
        
        annotator.describeProperty("outputMode", "Two modes are available: \"BATCHED\" and \"STREAM\"."
        + "The BATCHED Mode is the normal mode for most scenarios. In this mode, acceleration data is aggregated and only posted to the output channel, "
        + "if the amount of samples spans 1 second (e.g., 10 samples if configured to 10Hz)."
        + "In the STREAM mode, each individual sample is posted to the channel without aggregation, which can be used for real-time scenarios.",
        annotator.makeEnumProperty(new String[]{"STREAM", "BATCHED"}));
    
        annotator.describePublishChannel("HeartRateData", HeartRateData.class, "Channel where the recorded Date will be streamed to.");
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

        moduleInfo("HeartRateCollector init " + this.samplingFrequency + " " + this.outputMode);


        this.HeartRateDataChannel = this.publish("HeartRateData", HeartRateData.class);

        sensorManager = (SensorManager) CLAID.getContext().getSystemService(Context.SENSOR_SERVICE); 
        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_HEART_RATE); 
        sensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_NORMAL);

        int samplingPerioid = 1000/samplingFrequency;

        registerPeriodicFunction("AccelerometerSampling", () -> sampleHeartRateData(), Duration.ofMillis(samplingPerioid));
    }


    public synchronized void sampleHeartRateData()
    {
        HeartRateSample sample = latestSample.get();

        if(sample == null)
        {
            return;
        }

        if(this.outputMode.toUpperCase().equals("STREAM"))
        {
            HeartRateData.Builder data = HeartRateData.newBuilder();

          
            data.addSamples(sample);
        

            this.HeartRateDataChannel.post(data.build());
        }
        else
        {
            collectedHeartRateSamples.add(sample);

            if(collectedHeartRateSamples.size() == samplingFrequency)
            {
                HeartRateData.Builder data = HeartRateData.newBuilder();

                for(HeartRateSample collectedSample : collectedHeartRateSamples)
                {   
                    data.addSamples(collectedSample);
                }

                this.HeartRateDataChannel.post(data.build());
                collectedHeartRateSamples.clear();
            }
        }
        
    }


    @Override
    public synchronized void onSensorChanged(SensorEvent sensorEvent)
    {
        if (sensorEvent.sensor.getType() == Sensor.TYPE_HEART_RATE)
        {
            // Dividing per g to uniform with iOS
            float hr = sensorEvent.values[0] ;/// SensorManager.GRAVITY_EARTH;


            LocalDateTime currentTime = LocalDateTime.now();

            HeartRateSample.Builder sample = HeartRateSample.newBuilder();
            sample.setHr((double) hr);
 
            sample.setUnixTimestampInMs(currentTime.atZone(ZoneId.systemDefault()).toInstant().toEpochMilli());
            sample.setStatus(getHrStatusFromSensorEvent(sensorEvent));

            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:mm:ss'Z'");
            String formattedString = currentTime.format(formatter);
            sample.setEffectiveTimeFrame(formattedString);

            this.latestSample.set(sample.build());
           // System.out.println("Sensor data " +  x + " " +  y + " " + z);
        }
    }

    HeartRateStatus getHrStatusFromSensorEvent(SensorEvent event)
    {
        if(event.accuracy == SensorManager.SENSOR_STATUS_NO_CONTACT)
        {
            return HeartRateStatus.OFF_BODY;
        }
        else if(event.accuracy == SensorManager.SENSOR_STATUS_UNRELIABLE)
        {
            return HeartRateStatus.MEASUREMENT_UNRELIABLE_DUE_TO_MOVEMENT_OR_WRONG_ATTACHMENT_PPG_WEAK;
        }
        else
        {
            return HeartRateStatus.OK;
        }

        

    }


    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {}
}