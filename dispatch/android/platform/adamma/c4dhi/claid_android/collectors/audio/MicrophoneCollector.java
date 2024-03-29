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
package adamma.c4dhi.claid_android.collectors.audio;


import android.os.Handler;

import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.Properties;
import adamma.c4dhi.claid_android.collectors.audio.AudioDataHelper;
import adamma.c4dhi.claid_android.collectors.audio.AudioRecorder;
import adamma.c4dhi.claid_sensor_data.AudioData;
import adamma.c4dhi.claid_sensor_data.AudioEncoding;
import adamma.c4dhi.claid_sensor_data.AudioChannels;
import adamma.c4dhi.claid_sensor_data.AudioData;
import adamma.c4dhi.claid_sensor_data.AudioEncoding;
import java.util.Map;

import android.media.AudioFormat;

public class MicrophoneCollector extends Module
{
    private Channel<AudioData> audioDataChannel;

    private Integer samplingRate = 0;
    private Integer bitRate = 0;
    private AudioEncoding audioEncoding;
    private AudioChannels audioChannels;


    //private PeriodicValue continuousChunksRecordingRate;
    private String startRecording;
    private String outputChannel;


   /*  public void reflect(Reflector reflector)
    {
        reflector.reflect("samplingRate", this.samplingRate);
        reflector.reflect("encoding", this.encoding);
        reflector.reflect("bitRate", this.bitRate);
        reflector.reflect("channels", this.channels);
        reflector.reflectWithDefaultValue("continuousChunksRecordingRate",
                    this.continuousChunksRecordingRate, new PeriodicValue());
        reflector.reflect("startRecording", this.startRecording);
        reflector.reflect("outputChannel", this.outputChannel);
    }*/

    private AudioRecorder recorder;
    private final String DATA_IDENTIFIER = "MicrophoneAudioData";


    public void initialize(Properties properties)
    {
        System.out.println("Calling init of MicrophoneCollector");
        //new MicrophonePermission().blockingRequest();

        int recorderChannels = AudioFormat.CHANNEL_IN_MONO;

        // We only support PCM_FLOAT for now.
        int recorderAudioEncoding = AudioFormat.ENCODING_PCM_FLOAT;
       this.recorder = new AudioRecorder(16000, recorderChannels, recorderAudioEncoding);
       this.recorder.start();
      //  this.audioDataChannel = this.publish(AudioData.class, this.outputChannel);
      //  System.out.println("Microphone collector initialized");
    }

    @Override
    protected void onPause()
    {
        this.recorder.stopRecording();
    }

    @Override 
    protected void onResume()
    {
        this.recorder.start();

    }
  /*
    /*public void onRequest(ChannelData<Request> data)
    {
        Request r = data.value();

        if(r.get_dataIdentifier().compareTo(DATA_IDENTIFIER) == 0)
        {
            onAudioDataRequested(r);
        }
    }*/

    /*public void onAudioDataRequested(Request r)
    {
        //Integer length = r.get_length();
        Integer length = 2;
        System.out.println("Received request to record audio data for " + length + " seconds.");

        AudioData data = null;
        try {
            data = recorder.record(length);
        } catch (Exception e) {
            e.printStackTrace();
        }

        if(data != null)
        {
            System.out.println("Recorded data");
            audioDataChannel.post(data);
        }
        else
        {
            System.out.println("Data invalid");
        }
    }*/
}