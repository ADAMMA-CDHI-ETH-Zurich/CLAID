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

import adamma.c4dhi.claid.Logger.Logger;
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


    private AudioRecord recorder = null;

    private boolean recording = false;

    private int sampleRate;
    private int bitRate;
    private int recorderChannels;
    private int recorderAudioEncoding;


    public AudioRecorder(int sampleRate, int bitRate, AudioEncoding encoding, AudioChannels channels) {
        this.sampleRate = sampleRate;
        this.bitRate = bitRate;
        this.recorderAudioEncoding = fromCLAIDAudioEncoding(encoding);
        this.recorderChannels = fromCLAIDAudioChannels(channels);
    }



    @SuppressLint("MissingPermission")
    public boolean initialize() {
        recorder = new AudioRecord(MediaRecorder.AudioSource.MIC,
                this.sampleRate,
                this.recorderChannels,
                this.recorderAudioEncoding,
                AudioRecord.getMinBufferSize(this.sampleRate, this.recorderChannels,
                        this.recorderAudioEncoding));

        if (recorder.getState() != AudioRecord.STATE_INITIALIZED) {
            System.out.println("Failed to initialize recorder!");
            return false;
        }
        return true;
    }

     public boolean startRecording() {
        recorder.startRecording();
        this.recording = true;
        return true;
    }


    @SuppressLint("MissingPermission")
    public AudioData record(int secondsToRecord) {
        if (!recording) {
            Logger.logError("Recorder is not running");
            return null;
        }

        int expectedNumBytes = getExpectedNumBytes(secondsToRecord);
        int minBufferSize = AudioRecord.getMinBufferSize(this.sampleRate, this.recorderChannels, this.recorderAudioEncoding);

        if (minBufferSize <= 0) {
            Logger.logError("Invalid buffer size, possibly unsupported configuration.");
            return null;
        }

        byte[] byteBuffer = new byte[expectedNumBytes];
        int samplesRead = 0;
        int bytesPerSample = 1;
        switch (this.recorderAudioEncoding) {
            case AudioFormat.ENCODING_PCM_FLOAT:
                bytesPerSample = 4;
                float[] floatBuffer = new float[expectedNumBytes / bytesPerSample];  // 4 bytes per float
                samplesRead = recorder.read(floatBuffer, 0, floatBuffer.length, AudioRecord.READ_BLOCKING);
                ByteBuffer.wrap(byteBuffer).order(ByteOrder.nativeOrder()).asFloatBuffer().put(floatBuffer);
                break;

            case AudioFormat.ENCODING_PCM_16BIT:
                bytesPerSample = 2;
                short[] shortBuffer = new short[expectedNumBytes / bytesPerSample]; // 2 bytes per short
                samplesRead = recorder.read(shortBuffer, 0, shortBuffer.length, AudioRecord.READ_BLOCKING);
                ByteBuffer.wrap(byteBuffer).order(ByteOrder.nativeOrder()).asShortBuffer().put(shortBuffer);
                break;

            case AudioFormat.ENCODING_PCM_8BIT:
                bytesPerSample = 1;
                samplesRead = recorder.read(byteBuffer, 0, byteBuffer.length, AudioRecord.READ_BLOCKING);
                break;

            default:
                Logger.logError("Unsupported audio encoding format: " + this.recorderAudioEncoding);
                return null;
        }

        if (samplesRead < 0) {
            Logger.logError("Error reading audio data from the recorder.");
            return null;
        }

        // Ensure the recorded data matches the expected size
        if (samplesRead * bytesPerSample != expectedNumBytes) {
            Logger.logError("Requested " + expectedNumBytes + " bytes but only recorded " +
                    (samplesRead * bytesPerSample) + " bytes.");
            return null;
        }

        AudioData.Builder audioData = AudioData.newBuilder();
        audioData.setData(ByteString.copyFrom(byteBuffer));
        audioData.setSamplingRate(sampleRate);
        audioData.setBitRate(bitRate);
        audioData.setEncoding(toCLAIDAudioEncoding(recorderAudioEncoding));
        audioData.setChannels(toCLAIDAudioChannels(recorderChannels));
        audioData.setRecordingDurationMilliseconds(secondsToRecord*1000);
        return audioData.build();
    }


    /**
     * Stops the recording and releases the audio recorder.
     */
    public void stopRecording()
    {
        if (this.recording)
        {
            this.recording = false;

            if (this.recorder.getState() == AudioRecord.RECORDSTATE_RECORDING)
            {
                this.recorder.stop();
            }
            // Don't release recorder!! Otherwise we may not be able to resume
            // recording from the background.
            // this.recorder.release();
        }
    }

    public boolean isRecording() {
        return this.recording;
    }

    private int fromCLAIDAudioEncoding(AudioEncoding encoding) {
        switch (encoding) {
            case ENCODING_PCM_8BIT:
                return AudioFormat.ENCODING_PCM_8BIT;
            case ENCODING_PCM_16BIT:
                return AudioFormat.ENCODING_PCM_16BIT;
            case ENCODING_PCM_FLOAT:
                return AudioFormat.ENCODING_PCM_FLOAT;
            default:
                Logger.logError("Unsupported encoding format: " + encoding);
                return AudioFormat.ENCODING_INVALID;
        }
    }

    private int fromCLAIDAudioChannels(AudioChannels channels) {
        switch (channels) {
            case CHANNEL_MONO:
                return AudioFormat.CHANNEL_IN_MONO;
            case CHANNEL_STEREO:
                return AudioFormat.CHANNEL_IN_STEREO;
            default:
                Logger.logError("Unsupported channel configuration: " + channels);
                return AudioFormat.CHANNEL_INVALID;
        }
    }

    private AudioEncoding toCLAIDAudioEncoding(int androidEncoding) {
        switch (androidEncoding) {
            case AudioFormat.ENCODING_PCM_8BIT:
                return AudioEncoding.ENCODING_PCM_8BIT;
            case AudioFormat.ENCODING_PCM_16BIT:
                return AudioEncoding.ENCODING_PCM_16BIT;
            case AudioFormat.ENCODING_PCM_FLOAT:
                return AudioEncoding.ENCODING_PCM_FLOAT;
            default:
                Logger.logFatal("Unknown encoding from Android format: " + androidEncoding);
                return AudioEncoding.UNRECOGNIZED;
        }
    }

    private AudioChannels toCLAIDAudioChannels(int androidChannels) {
        switch (androidChannels) {
            case AudioFormat.CHANNEL_IN_MONO:
                return AudioChannels.CHANNEL_MONO;
            case AudioFormat.CHANNEL_IN_STEREO:
                return AudioChannels.CHANNEL_STEREO;
            default:
                Logger.logError("Unknown channel from Android format: " + androidChannels);
                return AudioChannels.CHANNEL_MONO;
        }
    }

    private int getExpectedNumBytes(int secondsToRecord) {
        int bytesPerSample;

        switch (this.recorderAudioEncoding) {
            case AudioFormat.ENCODING_PCM_8BIT:
                bytesPerSample = 1;
                break;
            case AudioFormat.ENCODING_PCM_16BIT:
                bytesPerSample = 2;
                break;
            case AudioFormat.ENCODING_PCM_FLOAT:
                bytesPerSample = 4;
                break;
            default:
                throw new IllegalArgumentException("Unsupported audio encoding format");
        }

        int channelCount;
        switch (this.recorderChannels) {
            case AudioFormat.CHANNEL_IN_MONO:
                channelCount = 1;
                break;
            case AudioFormat.CHANNEL_IN_STEREO:
                channelCount = 2;
                break;
            default:
                throw new IllegalArgumentException("Unsupported channel configuration");
        }

        return secondsToRecord * this.sampleRate * channelCount * bytesPerSample;
    }
}