package adamma.c4dhi.claid_android.UserFeedback;

import android.speech.tts.TextToSpeech;

import java.util.Locale;
import java.util.Map;
import adamma.c4dhi.claid.Module.Channel;

import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.ModuleAnnotator;
import adamma.c4dhi.claid.Module.Properties;
import adamma.c4dhi.claid_platform_impl.CLAID;

public class TextToSpeechModule extends Module implements TextToSpeech.OnInitListener
{
    private String speechLanguage = "";
    private TextToSpeech textToSpeech;
    private boolean initialized = false;

    private Channel<String> inputChannel;

    public static void annotateModule(ModuleAnnotator annotator)
    {
        annotator.setModuleCategory("UserFeedback");
        annotator.setModuleDescription("The TextToSpeechModule can vocalize text using the device's TextToSpeech functionality." +
         "The Module receives a text on an input Channel, which will then be spoken out by a generated voice via the speaker.");


        annotator.describeProperty("speechLanguage", "Language used by the Speech engine. Defines intonation and pronounciation of individual words.",
                annotator.makeEnumProperty(new String[]{"US",  "CANADA", "CANADA_FRENCH", "ITALY", "JAPAN", "CHINA"}));

        annotator.describeSubscribeChannel("TextToSpeak", String.class, "Channel with incoming text to be spoken by the Text to Speak engine.");
    }


    public void initialize(Properties properties)
    {
        moduleInfo("TextToSpeechModule initilized");
       
        this.speechLanguage = properties.getStringProperty("speechLanguage");

        if(properties.wasAnyPropertyUnknown())
        {
            this.moduleFatal(properties.getMissingPropertiesErrorString());
            return;
        }

        this.inputChannel = this.subscribe("TextToSpeak", String.class, (data) -> onData(data));
        textToSpeech = new TextToSpeech(CLAID.getContext(), this);
    }

    Locale getLocaleFromProperty()
    {
        if(this.speechLanguage.toUpperCase().equals("US"))
        {
            return Locale.US;
        }
        else if (this.speechLanguage.toUpperCase().equals("CANADA"))
        {
            return Locale.CANADA;
        }
        else if (this.speechLanguage.toUpperCase().equals("CANADA_FRENCH"))
        {
            return Locale.CANADA_FRENCH;
        }
        else if (this.speechLanguage.toUpperCase().equals("ITALY"))
        {
            return Locale.ITALY;
        }
        else if (this.speechLanguage.toUpperCase().equals("JAPAN"))
        {
            return Locale.JAPAN;
        }
        else if (this.speechLanguage.toUpperCase().equals("CHINA"))
        {
            return Locale.CHINA;
        }
        else
        {
            moduleError("Invalid property \"speechLanguage\". Language \"" + this.speechLanguage + "\"" +
                    " is not supported. Expected one of [\"US\",  \"CANADA\", \"CANADA_FRENCH\", \"ITALY\", \"JAPAN\", \"CHINA\"].");
            return null;
        }
    }

    public void onData(ChannelData<String> data)
    {
        moduleInfo("Text to speech received data");
        String text = data.getData();
        if(!this.initialized)
        {
            moduleError("Cannot output text \"" + text + "\". TextToSpeech engine is not initialized.");
        }
        textToSpeech.speak(text, TextToSpeech.QUEUE_FLUSH, null, null);

    }

    @Override
    public void onInit(int status) {
        if (status == TextToSpeech.SUCCESS) {
            // Set language (Optional)
            int result = textToSpeech.setLanguage(getLocaleFromProperty());

            if (result == TextToSpeech.LANG_MISSING_DATA ||
                    result == TextToSpeech.LANG_NOT_SUPPORTED) 
            {
                this.initialized = false;
                moduleError("Failed to initialize text to speech engine. " + result );
                // Handle language not supported or missing data
            } else {
                // Text-to-Speech is ready
                // Call the method to convert text to speech
                moduleInfo("Initialized text to speech engine.");
                this.initialized = true;
            }
        } else {
            // Initialization failed
            // Handle initialization failure
        }
    }

    @Override
    protected void terminate() {
        // Shutdown the Text-to-Speech engine when the activity is destroyed
        if (textToSpeech != null) {
            textToSpeech.stop();
            textToSpeech.shutdown();
        }
    }
}
