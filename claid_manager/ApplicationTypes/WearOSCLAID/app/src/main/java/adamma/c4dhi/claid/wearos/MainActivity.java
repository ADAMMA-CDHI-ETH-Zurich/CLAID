package adamma.c4dhi.claid.wearos;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

import JavaCLAID.CLAID;
import adamma.c4dhi.claid.wearos.databinding.ActivityMainBinding;

public class MainActivity extends Activity {

    private TextView mTextView;
    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        mTextView = binding.text;


        CLAID.loadFromAssets("CLAID.xml");
        CLAID.setContext(this.getBaseContext());
        CLAID.startInSeparateThread();

        // CLAIDForegroundServiceManager.startService(getApplicationContext());
    }
}