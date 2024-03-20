/***************************************************************************
* Copyright (C) 2023 ETH Zurich
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

package adamma.c4dhi.claid_android.collectors.location;
//import static com.google.android.gms.location.Priority.PRIORITY_BALANCED_POWER_ACCURACY;
import android.annotation.SuppressLint;
import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Build;
//import com.google.android.gms.location.FusedLocationProviderClient;
////import com.google.android.gms.location.LocationServices;
//import AndroidPermissions.LocationPermission;
import adamma.c4dhi.claid.*;
import adamma.c4dhi.claid_platform_impl.*;

import adamma.c4dhi.claid.Module.*;
import adamma.c4dhi.claid_sensor_data.*;
import adamma.c4dhi.claid_android.Permissions.*;
import java.util.concurrent.atomic.AtomicReference;
import java.time.Duration;
import android.os.Looper;

public class LocationCollector extends Module implements LocationListener
{

    private Channel<LocationData> locationDataChannel;

    // Volatile to be thread safe.
    private volatile AtomicReference<LocationSample> latestSample = new AtomicReference<>();
    
    private LocationManager locationManager;
    
    private boolean isRunning = false;

    PowerProfile currentPowerProfile;
    
    public void initialize(Properties properties) 
    {
        moduleInfo("Initializing");
        // new LocationPermission().blockingRequest();

        Integer samplingPeriod = properties.getNumberProperty("samplingPeriod", Integer.class);

        if(properties.wasAnyPropertyUnknown())
        {
            this.moduleFatal(properties.getMissingPropertiesErrorString());
            return;
        }


        this.locationManager = (LocationManager) CLAID.getContext().getSystemService(Context.LOCATION_SERVICE);


        this.locationDataChannel = this.publish("LocationData", LocationData.class);

        PowerProfile.Builder builder = PowerProfile.newBuilder();
        
        builder.setPowerProfileType(PowerProfileType.UNRESTRICTED);
        builder.setPeriod(samplingPeriod);
        this.currentPowerProfile = builder.build();

        startLocationUpdates();
    }

    @SuppressLint("MissingPermission")
    private void startLocationUpdates()
    {
        if(isRunning)
        {
            return;
        }
        isRunning = true;

        String provider;
        if(this.currentPowerProfile.getPowerProfileType() == PowerProfileType.POWER_SAVING_MODE)
        {
            // Low powe
            provider = LocationManager.NETWORK_PROVIDER;
        }
        else
        {
            provider = LocationManager.FUSED_PROVIDER;
        }
        
        Integer samplingPeriod = (int) (this.currentPowerProfile.getPeriod());



        int minimumTimeBetweenUpdatesInMilliseconds = samplingPeriod;
        int minimumDistanceBetweenUpdatesInMeters = 1;
        locationManager.requestLocationUpdates(provider, 
            minimumTimeBetweenUpdatesInMilliseconds, 
                minimumDistanceBetweenUpdatesInMeters, this, Looper.getMainLooper());

        moduleInfo("Starting location updates with samplingPeriod " + samplingPeriod);
        this.registerPeriodicFunction("SampleLocation", () -> sampleLocation(), Duration.ofMillis(samplingPeriod));
    }

    private void stopLocationUpdates()
    {
        if(!isRunning)
        {
            return;
        }

        isRunning = false;
        unregisterPeriodicFunction("SampleLocation");
    }

    private void restartLocationUpdates()
    {
        this.stopLocationUpdates();
        this.startLocationUpdates();
    }

    @Override
    public void onLocationChanged(Location location) {
        // This method will be called whenever the location is updated.
        setLatestLocation(location);
    }

    void setLatestLocation(Location location)
    {
        LocationSample.Builder locationSample = LocationSample.newBuilder();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            locationSample.setVAccuracy((double) location.getVerticalAccuracyMeters());
        }
        
        locationSample.setHAccuracy((double) location.getAccuracy());
        locationSample.setBearing((double) location.getBearing());
        locationSample.setSpeed((double) location.getSpeed());
        locationSample.setTimestamp((long) location.getTime());
        locationSample.setAltitude(location.getAltitude());
        locationSample.setLatitude(location.getLatitude());
        locationSample.setLongitude(location.getLongitude());
        locationSample.setElapsedRealtimeSeconds((double) location.getElapsedRealtimeNanos()/1000000000);
        locationSample.setProvider(location.getProvider());

        latestSample.set(locationSample.build());
    }

    public void sampleLocation()
    {
        moduleInfo("Sample Location called");
        LocationSample sample = this.latestSample.get();

        if(sample == null)
        {
            return;
        }

        LocationData.Builder locationData = LocationData.newBuilder();
        locationData.addSamples(sample);

        this.locationDataChannel.post(locationData.build());
    }

    @Override
    protected void onPause()
    {
        moduleWarning("onPause called, unregistering from SensorManager");
        stopLocationUpdates();
    }

    @Override 
    protected void onResume()
    {
        moduleWarning("onResume called, registering at SensorManager");
        startLocationUpdates();
    }

    @Override
    protected void onPowerProfileChanged(PowerProfile profile)
    {
        moduleWarning("onPowerProfileChanged called, using profile: " + profile.toString().replace("\n", ""));

    
        if(this.currentPowerProfile.getPowerProfileType() != profile.getPowerProfileType() ||
            this.currentPowerProfile.getPeriod() != profile.getPeriod())
        {
            moduleInfo("PowerProfile has changed, restarting");
            this.currentPowerProfile = profile;
            restartLocationUpdates();
        }

    }
}