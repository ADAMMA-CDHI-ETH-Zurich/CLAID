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

import adamma.c4dhi.claid_sensor_data.GyroscopeSample;
import adamma.c4dhi.claid_sensor_data.GyroscopeData;

import adamma.c4dhi.claid_platform_impl.CLAID;

import adamma.c4dhi.claid.Module.ModuleAnnotator;
import adamma.c4dhi.claid.Module.PropertyHelper.PropertyHelper;

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;

public class GyroscopeCollector extends Module implements SensorEventListener 
{
    private Channel<GyroscopeData> GyroscopeDataChannel;

    private ReentrantLock mutex = new ReentrantLock();
      
    // Volatile to be thread safe.
    private volatile AtomicReference<GyroscopeSample> latestSample = new AtomicReference<>();
    private ArrayList<GyroscopeSample> collectedGyroscopeSamples = new ArrayList<GyroscopeSample>();

    GyroscopeData oldGyroscopeData = null;

    SensorManager sensorManager;
    Sensor sensor;

    private Integer samplingFrequency;
    private String outputMode;

    public static void annotateModule(ModuleAnnotator annotator)
    {
        annotator.setModuleCategory("DataCollection");
        annotator.setModuleDescription("The GyroscopeCollector allows to record gyrpscpüe data using the devices built-in gyroscope of the device."
        + "The sampling frequency can be freely configured, however is subject to the limitations of the device (i.e., built-in sensor speicifications)."
        + "The GyroscopeCollector features two recording modes: \"Batched\" and \"Streaming\"\n");

        annotator.describeProperty("samplingFrequency", "Frequency in Hz with which to record gyroscope data. Only decimal values allowed. Subject to physical limitations of the device and Gyroscope.");
        annotator.describeProperty("outputMode", "Two modes are available: \"BATCHED\" and \"STREAM\"."
        + "The BATCHED Mode is the normal mode for most scenarios. In this mode, gyroscope data is aggregated and only posted to the output channel, "
        + "if the amount of samples spans 1 second (e.g., 50 samples if configured to 50Hz)."
        + "In the STREAM mode, each individual sample is posted to the channel without aggregation, which can be used for real-time scenarios.",
        annotator.makeEnumProperty(new String[]{"STREAM", "BATCHED"}));
    
        annotator.describePublishChannel("GyroscopeData", GyroscopeData.class, "Channel where the recorded Date will be streamed to.");
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

        moduleInfo("GyroscopeCollector init " + this.samplingFrequency + " " + this.outputMode);


        this.GyroscopeDataChannel = this.publish("GyroscopeData", GyroscopeData.class);

        sensorManager = (SensorManager) CLAID.getContext().getSystemService(Context.SENSOR_SERVICE); 
        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE); 
        sensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_FASTEST);

        int samplingPerioid = 1000/samplingFrequency;

        registerPeriodicFunction("GyroscopeSampling", () -> sampleGyroscopeData(), Duration.ofMillis(samplingPerioid));
    }


    public synchronized void sampleGyroscopeData()
    {
        GyroscopeSample sample = latestSample.get();

        if(sample == null)
        {
            return;
        }

        if(this.outputMode.toUpperCase().equals("STREAM"))
        {
            GyroscopeData.Builder data = GyroscopeData.newBuilder();

          
            data.addSamples(sample);
        

            this.GyroscopeDataChannel.post(data.build());
        }
        else
        {
            collectedGyroscopeSamples.add(sample);

            if(collectedGyroscopeSamples.size() == samplingFrequency)
            {
                GyroscopeData.Builder data = GyroscopeData.newBuilder();

                for(GyroscopeSample collectedSample : collectedGyroscopeSamples)
                {   
                    data.addSamples(collectedSample);
                }

                this.GyroscopeDataChannel.post(data.build());
                collectedGyroscopeSamples.clear();
            }
        }
        
    }


    @Override
    public synchronized void onSensorChanged(SensorEvent sensorEvent)
    {
        if (sensorEvent.sensor.getType() == Sensor.TYPE_GYROSCOPE)
        {
            double x = sensorEvent.values[0];
            double y = sensorEvent.values[1];
            double z = sensorEvent.values[2];

            LocalDateTime currentTime = LocalDateTime.now();

            GyroscopeSample.Builder sample = GyroscopeSample.newBuilder();
            sample.setGyroscopeX((double) x);
            sample.setGyroscopeY((double) y);
            sample.setGyroscopeZ((double) z);
            sample.setSensorBodyLocation("unknown/smartphone");
            sample.setUnixTimestampInMs(currentTime.toInstant(ZoneOffset.UTC).toEpochMilli());

            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:mm:ss'Z'");
            String formattedString = currentTime.format(formatter);
            sample.setEffectiveTimeFrame(formattedString);

            this.latestSample.set(sample.build());
           // System.out.println("Sensor data " +  x + " " +  y + " " + z);
        }
    }


    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {}
}