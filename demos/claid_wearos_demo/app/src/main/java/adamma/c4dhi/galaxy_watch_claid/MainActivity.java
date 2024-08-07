package adamma.c4dhi.galaxy_watch_claid;

import static android.os.Build.VERSION.SDK_INT;

import android.Manifest;
import android.app.Activity;

import android.os.Bundle;
import android.widget.TextView;

import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.galaxy_watch_claid.databinding.ActivityMainBinding;

public class MainActivity extends Activity {


    ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        TextView tv = binding.textView;
        tv.setText("Waiting for CLAID to start.");

        CLAID.onStarted(() -> tv.setText("CLAID has started!"));

    }

}
