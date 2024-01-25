package adamma.c4dhi.claid_android.collectors.acceleration;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

import java.time.Duration;
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

    private int samplingFrequency;


    public void initialize(Map<String, String> properties)
    {
        moduleInfo("AccelerometerCollector init");
        if(!properties.containsKey("samplingFrequency"))
        {
            moduleFatal("Missing property \"samplingFrequency\"");
            return;
        }

        this.samplingFrequency = Integer.parseInt(properties.get("samplingFrequency"));

        this.accelerationDataChannel = this.publish("AccelerationData", AccelerationData.class);

        sensorManager = (SensorManager) CLAID.getContext().getSystemService(Context.SENSOR_SERVICE); 
        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER); 
        sensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_NORMAL);

        int samplingPerioid = 1000/samplingFrequency;

        registerPeriodicFunction("AccelerometerSampling", () -> sampleAccelerationData(), Duration.ofMillis(samplingPerioid));
    }


    public synchronized void sampleAccelerationData()
    {
        AccelerationSample sample = latestSample.get();
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


    @Override
    public synchronized void onSensorChanged(SensorEvent sensorEvent)
    {
        if (sensorEvent.sensor.getType() == Sensor.TYPE_ACCELEROMETER)
        {
            // Dividing per g to uniform with iOS
            double x = sensorEvent.values[0] / SensorManager.GRAVITY_EARTH;;
            double y = sensorEvent.values[1] / SensorManager.GRAVITY_EARTH;
            double z = sensorEvent.values[2] / SensorManager.GRAVITY_EARTH;

            AccelerationSample.Builder sample = AccelerationSample.newBuilder();
            sample.setAccelerationX((double) x);
            sample.setAccelerationY((double) y);
            sample.setAccelerationZ((double) z);
            sample.setSensorBodyLocation("unknown/smartphone");
            sample.setUnixTimestampInMs(System.currentTimeMillis());

            this.latestSample.set(sample.build());
           // System.out.println("Sensor data " +  x + " " +  y + " " + z);
        }
    }


    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {}
}