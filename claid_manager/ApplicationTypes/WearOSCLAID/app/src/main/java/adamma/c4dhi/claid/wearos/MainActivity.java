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

        CLAID.setContext(this.getBaseContext());
        CLAID.registerExceptionHandler(exception -> onCLAIDException(exception));
        CLAID.loadFromAssets("CLAID.xml");
        
        CLAID.startInSeparateThread();

        // CLAIDForegroundServiceManager.startService(getApplicationContext());
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