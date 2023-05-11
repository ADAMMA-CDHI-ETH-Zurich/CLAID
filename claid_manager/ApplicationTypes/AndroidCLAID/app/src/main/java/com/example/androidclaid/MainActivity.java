package com.example.androidclaid;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import com.example.androidclaid.databinding.ActivityMainBinding;

import JavaCLAID.CLAID;

public class MainActivity extends AppCompatActivity {

    static
    {
        System.loadLibrary("JavaCLAID");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Best way to start CLAID on Android without blocking the main activity.
        // You could also use CLAID.startNonBlocking(), but then you would have to manually
        // call the CLAID update loop periodically.
        CLAID.startInSeparateThread();

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText("Hello from CLAID!\nIf you see this,\nCLAID should be working ;)");

    }

    /**
     * A native method that is implemented by the 'androidclaid' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}