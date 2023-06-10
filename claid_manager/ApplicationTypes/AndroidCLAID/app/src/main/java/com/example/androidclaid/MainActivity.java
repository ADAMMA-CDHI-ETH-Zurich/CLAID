package com.example.androidclaid;

import static android.content.Intent.FLAG_ACTIVITY_NEW_TASK;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.provider.Settings;
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

        CLAID.setContext(this.getApplicationContext());

        CLAID.loadFromAssets("CLAID.xml");

        // This is the best way to start CLAID on Android without blocking the main activity.
        // You could also use CLAID.startNonBlocking(), but then you would have to manually
        // call the CLAID update loop periodically.
        // Do not use CLAID.start() here, as this would block the UI thread.
        CLAID.startInSeparateThread();

        TextView tv = binding.sampleText;
        tv.setText("Hello from CLAID!\nIf you see this,\nCLAID should be working.");
    }
}