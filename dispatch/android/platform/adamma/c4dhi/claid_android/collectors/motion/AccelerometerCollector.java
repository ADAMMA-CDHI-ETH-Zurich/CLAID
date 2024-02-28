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

import adamma.c4dhi.claid_sensor_data.AccelerationSample;
import adamma.c4dhi.claid_sensor_data.AccelerationData;

import adamma.c4dhi.claid_platform_impl.CLAID;

import adamma.c4dhi.claid.Module.ModuleAnnotator;
import adamma.c4dhi.claid.Module.PropertyHelper.PropertyHelper;

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;

public class AccelerometerCollector extends Module implements SensorEventListener 
{
    private Channel<AccelerationData> accelerationDataChannel;


    private ReentrantLock mutex = new ReentrantLock();
   
   
    // Volatile to be thread safe.
    private volatile AtomicReference<AccelerationSample> latestSample = new AtomicReference<>();
    private ArrayList<AccelerationSample> collectedAccelerationSamples = new ArrayList<AccelerationSample>();

    AccelerationData oldAccelerationData = null;

    SensorManager sensorManager;
    Sensor sensor;

    private Integer samplingFrequency;
    private String outputMode;

    public static void annotateModule(ModuleAnnotator annotator)
    {
        annotator.setModuleDescription("The AccelerometerCollector allows to record acceleration data using the devices built-in accelerometer."
        + "The sampling frequency can be freely configured, however is subject to the limitations of the device (i.e., built-in sensor speicifcations)."
        + "The AccelerometerCollector features two recording modes: \"Batched\" and \"Streaming\"\n");

        annotator.describeProperty("samplingFrequency", null);
        annotator.describeProperty("outputMode", "Two modes are available: \"BATCHED\" and \"STREAM\"."
        + "The BATCHED Mode is the normal mode for most scenarios. In this mode, acceleration data is aggregated and only posted to the output channel, "
        + "if the amount of samples spans 1 second (e.g., 50 samples if configured to 50Hz)."
        + "In the STREAM mode, each individual sample is posted to the channel without aggregation, which can be used for real-time scenarios.");
    
        annotator.describePublishChannel("AccelerationData", AccelerationData.class, "Channel where the recorded Date will be streamed to.");
    }


    public void initialize(Map<String, String> propertiesMap)
    {

        PropertyHelper propertyHelper = new PropertyHelper(propertiesMap);
        this.samplingFrequency = propertyHelper.getProperty("samplingFrequency", Integer.class);
        this.outputMode = propertyHelper.getProperty("outputMode", String.class);

        if(propertyHelper.wasAnyPropertyUnknown())
        {
            this.moduleFatal(propertyHelper.getMissingPropertiesErrorString());
            return;
        }

        if(!this.outputMode.toUpperCase().equals("STREAM") && !this.outputMode.toUpperCase().equals("BATCHED"))
        {
            moduleFatal("Unknown output mode \"" + this.outputMode + "\". Expected one of [STREAM, BATCHED].");
            return;
        }

        moduleInfo("AccelerometerCollector init " + this.samplingFrequency + " " + this.outputMode);


        this.accelerationDataChannel = this.publish("AccelerationData", AccelerationData.class);

        sensorManager = (SensorManager) CLAID.getContext().getSystemService(Context.SENSOR_SERVICE); 
        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER); 
        sensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_FASTEST);

        int samplingPerioid = 1000/samplingFrequency;

        //registerPeriodicFunction("AccelerometerSampling", () -> sampleAccelerationData(), Duration.ofMillis(samplingPerioid));
    }


    public synchronized void sampleAccelerationData()
    {
        AccelerationSample sample = latestSample.get();

        if(sample == null)
        {
            return;
        }

        if(this.outputMode.toUpperCase().equals("STREAM"))
        {
            AccelerationData.Builder data = AccelerationData.newBuilder();

          
            data.addSamples(sample);
        

            this.accelerationDataChannel.post(data.build());
        }
        else
        {
            collectedAccelerationSamples.add(sample);

            if(collectedAccelerationSamples.size() == samplingFrequency)
            {
                AccelerationData.Builder data = AccelerationData.newBuilder();

                for(AccelerationSample collectedSample : collectedAccelerationSamples)
                {   
                    data.addSamples(collectedSample);
                }

                this.accelerationDataChannel.post(data.build());
                collectedAccelerationSamples.clear();
            }
        }
        
    }


    @Override
    public synchronized void onSensorChanged(SensorEvent sensorEvent)
    {
        if (sensorEvent.sensor.getType() == Sensor.TYPE_ACCELEROMETER)
        {
            // Dividing per g to uniform with iOS
            double x = sensorEvent.values[0] ;/// SensorManager.GRAVITY_EARTH;
            double y = sensorEvent.values[1] ;/// SensorManager.GRAVITY_EARTH;
            double z = sensorEvent.values[2] ;/// SensorManager.GRAVITY_EARTH;

            LocalDateTime currentTime = LocalDateTime.now();

            AccelerationSample.Builder sample = AccelerationSample.newBuilder();
            sample.setAccelerationX((double) x);
            sample.setAccelerationY((double) y);
            sample.setAccelerationZ((double) z);
            sample.setSensorBodyLocation("unknown/smartphone");
            sample.setUnixTimestampInMs(currentTime.toInstant(ZoneOffset.UTC).toEpochMilli());

            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:mm:ss'Z'");
            String formattedString = currentTime.format(formatter);
            sample.setEffectiveTimeFrame(formattedString);

            AccelerationSample theSample = sample.build();
            this.latestSample.set(theSample);
            this.collectedAccelerationSamples.add(theSample);
            if(this.collectedAccelerationSamples.size() == 100)
            {
                AccelerationData.Builder data = AccelerationData.newBuilder();

                for(AccelerationSample collectedSample : collectedAccelerationSamples)
                {   
                    data.addSamples(collectedSample);
                }

                this.accelerationDataChannel.post(data.build());
                collectedAccelerationSamples.clear();
            }
           // System.out.println("Sensor data " +  x + " " +  y + " " + z);
        }
    }


    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {}
}