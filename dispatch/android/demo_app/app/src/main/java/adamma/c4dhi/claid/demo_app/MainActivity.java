package adamma.c4dhi.claid.demo_app;

import android.app.Activity;
import android.os.Bundle;
import android.view.WindowManager;

import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid_android.Configuration.CLAIDSpecialPermissionsConfig;
import adamma.c4dhi.claid_platform_impl.CLAID;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        ModuleFactory factory = new ModuleFactory();
        factory = CLAID.registerDefaultModulesToFactory(factory);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        CLAID.start(getApplicationContext(), "assets://BatteryManagerTest.json",
                "Smartphone",
                "device",
                "user",
                factory, CLAIDSpecialPermissionsConfig.regularConfig());

    }
}
