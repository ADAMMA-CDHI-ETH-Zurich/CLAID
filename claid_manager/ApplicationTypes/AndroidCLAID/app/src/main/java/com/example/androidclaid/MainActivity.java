package com.example.androidclaid;

import static android.content.Intent.FLAG_ACTIVITY_NEW_TASK;

import androidx.appcompat.app.AppCompatActivity;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
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
        CLAID.registerExceptionHandler(exception -> onCLAIDException(exception));

        CLAID.loadFromAssets("CLAID.xml");

        // This is the best way to start CLAID on Android without blocking the main activity.
        // You could also use CLAID.startNonBlocking(), but then you would have to manually
        // call the CLAID update loop periodically.
        // Do not use CLAID.start() here, as this would block the UI thread.
        CLAID.startInSeparateThread();

        TextView tv = binding.sampleText;
        tv.setText("Hello from CLAID!\nIf you see this,\nCLAID should be working.");
    }

    private void onCLAIDException(String exception)
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // Code here will run in UI thread
                AlertDialog.Builder builder1 = new AlertDialog.Builder(MainActivity.this);
                builder1.setMessage("CLAID encountered an exception:\n\"" + exception + "\"\n" +
                        "Press Ok to exit the application.");
                builder1.setCancelable(true);

                builder1.setPositiveButton(
                        "Ok",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                System.exit(0);
                            }
                        });

                AlertDialog alert11 = builder1.create();
                alert11.show();
            }
        });

        while(true);
    }
}