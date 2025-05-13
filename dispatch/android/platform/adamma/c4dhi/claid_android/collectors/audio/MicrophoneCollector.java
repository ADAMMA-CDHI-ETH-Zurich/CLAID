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

package adamma.c4dhi.claid_android.collectors.audio;


import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.Properties;

import adamma.c4dhi.claid_android.Permissions.MicrophonePermission;
import adamma.c4dhi.claid_sensor_data.AudioData;
import adamma.c4dhi.claid_sensor_data.AudioChannels;
import adamma.c4dhi.claid_sensor_data.AudioEncoding;

import android.media.AudioFormat;

import com.google.type.TimeOfDay;

import java.sql.Time;
import java.time.Duration;
import java.time.LocalDateTime;
import java.time.LocalTime;

public class MicrophoneCollector extends Module
{
    private Channel<AudioData> audioDataChannel;

    private AudioRecorder recorder;
    public static final String OUTPUT_CHANNEL_NAME = "AudioData";

    int sampleRecordingDuration;
    Thread recordingThread;

    public void initialize(Properties properties)
    {
        System.out.println("Calling init of MicrophoneCollector");

        new MicrophonePermission().blockingRequest();

        AudioChannels recorderChannels = properties.getEnumProperty("channels", AudioChannels.class);
        AudioEncoding recorderAudioEncoding = properties.getEnumProperty("encoding", AudioEncoding.class);
        int bitRate = properties.getNumberProperty("bitrate", Integer.class);
        int samplingRate = properties.getNumberProperty("sampling_rate", Integer.class);

        TimeOfDay startTime = properties.getObjectProperty("recording_start_time", TimeOfDay.class, null);
        TimeOfDay endTime = properties.getObjectProperty("recording_end_time", TimeOfDay.class, null);

        this.sampleRecordingDuration = properties.getNumberProperty("sample_recording_duration_seconds", Integer.class);
        if(this.sampleRecordingDuration <= 0) {
            moduleFatal("Cannot set up recorder, sample_recording_duration_seconds must be > 0");
            return;
        }

        if(properties.wasAnyPropertyUnknown())
        {
            this.moduleFatal(properties.getMissingPropertiesErrorString());
            return;
        }

        this.recorder = new AudioRecorder(samplingRate, bitRate, recorderAudioEncoding, recorderChannels);

        if(!this.recorder.initialize()) {
            moduleFatal("Failed to initialize AudioRecorder.");
        }

        scheduleRecording(startTime, endTime);

        this.audioDataChannel = this.publish(OUTPUT_CHANNEL_NAME, AudioData.class);
    }
    
    void scheduleRecording(TimeOfDay startTime, TimeOfDay endTime) {
        if(startTime == null) {
            this.startRecording();
        } else {
            LocalDateTime todayAtTime = LocalDateTime.now().with(
                    LocalTime.of(
                            startTime.getHours(),
                            startTime.getMinutes(),
                            startTime.getSeconds()
                    )
            );
            this.registerPeriodicFunction("StartRecording", this::startRecording, Duration.ofDays(1), todayAtTime);
        }

        if(endTime != null) {
            LocalDateTime todayAtTime = LocalDateTime.now().with(
                    LocalTime.of(
                            endTime.getHours(),
                            endTime.getMinutes(),
                            endTime.getSeconds()
                    )
            );
            this.registerPeriodicFunction("StopRecording", this::stopRecording, Duration.ofDays(1), todayAtTime);
        }
    }

    void startRecording() {
        moduleInfo("Start recording called.");
        if(this.recorder.isRecording()) {
            moduleError("Cannot start recording, recorder is already running");
            return;
        }
        this.recorder.startRecording();
        this.recordingThread = new Thread(this::continuousRecording);
        this.recordingThread.start();
    }

    void stopRecording() {
        moduleInfo("Stop recording called.");
        if(!this.recorder.isRecording()) {
            return;
        }
        this.recorder.stopRecording();
        if(this.recordingThread != null) {
            try {
                this.recordingThread.join();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
            this.recordingThread = null;
        }
    }

    void continuousRecording() {
        while(recorder.isRecording()) {
            AudioData data = recorder.record(sampleRecordingDuration);
            if(data == null) {
                moduleError("Failed to record, AudioData is null!");
                continue;
            }

            audioDataChannel.post(data);
        }
    }
}