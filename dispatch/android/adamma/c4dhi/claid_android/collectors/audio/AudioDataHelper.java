package adamma.c4dhi.claid_android.collectors.audio;

import adamma.c4dhi.claid_sensor_data.AudioData;
import adamma.c4dhi.claid_sensor_data.AudioEncoding;
import adamma.c4dhi.claid_sensor_data.AudioChannels;

import android.media.AudioFormat;

public class AudioDataHelper 
{
    public static int audioEncodingToAndroidEncoding(AudioEncoding protoEncoding) 
    {
        switch (protoEncoding) 
        {
            case ENCODING_PCM_8BIT:
                return AudioFormat.ENCODING_PCM_8BIT;
            case ENCODING_PCM_16BIT:
                return AudioFormat.ENCODING_PCM_16BIT;
            case ENCODING_PCM_FLOAT:
                return AudioFormat.ENCODING_PCM_FLOAT;
            case ENCODING_PCM_24_BIT_PACKED:
                return AudioFormat.ENCODING_PCM_8BIT; // Android does not have a specific constant for 24-bit packed, using 8-bit as an example
            case ENCODING_PCM_32_BIT:
                return AudioFormat.ENCODING_PCM_32BIT;
            default:
                throw new IllegalArgumentException("Unsupported Audio Encoding: " + protoEncoding);
        }
    }

    public static AudioEncoding audioEncodingFromString(String encodingString) 
    {
        switch (encodingString.toUpperCase()) {
            case "ENCODING_PCM_8BIT":
                return AudioEncoding.ENCODING_PCM_8BIT;
            case "ENCODING_PCM_16BIT":
                return AudioEncoding.ENCODING_PCM_16BIT;
            case "ENCODING_PCM_FLOAT":
                return AudioEncoding.ENCODING_PCM_FLOAT;
            case "ENCODING_PCM_24_BIT_PACKED":
                return AudioEncoding.ENCODING_PCM_24_BIT_PACKED;
            case "ENCODING_PCM_32_BIT":
                return AudioEncoding.ENCODING_PCM_32_BIT;
            default:
                throw new IllegalArgumentException("Unsupported Audio Encoding String: " + encodingString);
        }
    }

    
}
