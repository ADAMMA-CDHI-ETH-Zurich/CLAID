package ch.claid.claid_flutter_demo;

import android.os.Bundle;

import androidx.annotation.Nullable;

import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid_android.Configuration.CLAIDPersistanceConfig;
import adamma.c4dhi.claid_android.Configuration.CLAIDSpecialPermissionsConfig;
import io.flutter.embedding.android.FlutterActivity;
import adamma.c4dhi.claid_platform_impl.CLAID;

public class MainActivity extends FlutterActivity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        CLAID.startInPersistentService(getApplicationContext(),
                "assets://flutter_assets/assets/claid_test.json",
                "test_host",
                "device",
                "user",
                CLAIDSpecialPermissionsConfig.almightyCLAID(),
                CLAIDPersistanceConfig.maximumPersistance());
    }
}
