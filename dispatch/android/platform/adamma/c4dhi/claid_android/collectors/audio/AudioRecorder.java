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

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;


import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.Collections;
import java.util.Vector;

import adamma.c4dhi.claid_sensor_data.AudioData;
import adamma.c4dhi.claid_sensor_data.AudioEncoding;
import adamma.c4dhi.claid_android.collectors.audio.AudioDataHelper;
import adamma.c4dhi.claid_sensor_data.AudioChannels;

import com.google.protobuf.ByteString;

/**
 * Class managing the continuous recording  of audio from the microphone.
 */
public class AudioRecorder
{
    // We only support MONO for now.
    private int recorderChannels = AudioFormat.CHANNEL_IN_MONO;

    // We only support PCM_FLOAT for now.
    private int recorderAudioEncoding = AudioFormat.ENCODING_PCM_FLOAT;


    private int sampleRate = 44100;


    private AudioRecord recorder = null;
    private Thread recordingThread = null;

    private boolean isRecording = false;



    private int getExpectedNumBytes(int secondsToRecord)
    {
        // TODO: Update when AudioFormat changes.
        // TODO: Update when number of channels changes.
        // TODO: Update when sample rate changes.
        return secondsToRecord * this.sampleRate * 4;
    }

    AudioRecorder(int sampleRate, int recorderChannels, int recorderAudioEncoding)
    {
        this.sampleRate = sampleRate;
        this.recorderChannels = recorderChannels;
        this.recorderAudioEncoding = recorderAudioEncoding;
    }

    @SuppressLint("MissingPermission")
    public boolean start()
    {
        recorder = new AudioRecord(MediaRecorder.AudioSource.MIC,
                this.sampleRate,
                this.recorderChannels,
                this.recorderAudioEncoding,
                AudioRecord.getMinBufferSize(this.sampleRate, this.recorderChannels,
                        this.recorderAudioEncoding));

        if(recorder.getState() != AudioRecord.STATE_INITIALIZED)
        {
            System.out.println("Failed to initialize recorder!");
            return false;
        }

        recorder.startRecording();
        this.isRecording = true;
        return true;
    }


    @SuppressLint("MissingPermission")
    public AudioData record(int secondsToRecord) throws Exception {
        // Missing permission -> is checked externally in MainActivity.


        //  this.bufferSize = AudioRecord.getMinBufferSize(this.sampleRate, recorderChannels, this.recorderAudioEncoding);
        // 4 because float requires 4 byte.
        int bufferSizeInBytes;
        int floatBufferSize;

        floatBufferSize = this.sampleRate * secondsToRecord;
        bufferSizeInBytes = floatBufferSize * 4;

        int minBufferSize = secondsToRecord * sampleRate;//recorder.getMinBufferSize(sampleRate, recorderChannels, recorderAudioEncoding);
        float[] tmpBuffer = new float[minBufferSize];

        int readSize = recorder.read(tmpBuffer, 0, tmpBuffer.length, recorder.READ_BLOCKING);

        if(readSize * 4 != getExpectedNumBytes(secondsToRecord))
        {
            throw new IOException("Error in AudioRecorder: Requested to record " + secondsToRecord +
                    " of data, which would be " + getExpectedNumBytes(secondsToRecord) + " of bytes given "+
                    " the current configuration of the recorder, however only " + readSize*4 + " bytes were recorded.");
        }

        // convert float to byte
        byte[] bytes = new byte[bufferSizeInBytes];
        ByteBuffer.wrap(bytes).order(ByteOrder.nativeOrder()).asFloatBuffer().put(tmpBuffer);
        AudioData.Builder audioData = AudioData.newBuilder();

        audioData.setData(ByteString.copyFrom(bytes));

        return audioData.build();
    }

    /**
     * Stops the recording and releases the audio recorder.
     */
    public void stopRecording()
    {
        if (this.isRecording)
        {
            this.isRecording = false;

            if (this.recorder.getState() == AudioRecord.RECORDSTATE_RECORDING)
            {
                this.recorder.stop();
            }
            // Release recorder. Otherwise, on some android devices,
            // recording can not be restarted (at least if this.audioRecord is always
            // reinitialized), as the microphone is always locked.
            this.recorder.release();
        }
    }


}