package ch.claid.claid_flutter_demo;

import android.os.Bundle;

import androidx.annotation.Nullable;

import adamma.c4dhi.claid.CLAIDFlutterActivity;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.smart_inhaler.smart_inhaler;
import adamma.c4dhi.claid_android.Configuration.CLAIDPersistanceConfig;
import adamma.c4dhi.claid_android.Configuration.CLAIDSpecialPermissionsConfig;
import adamma.c4dhi.claid_android.Package.CLAIDPackageLoader;
import io.flutter.embedding.android.FlutterActivity;
import adamma.c4dhi.claid_platform_impl.CLAID;

public class MainActivity extends CLAIDFlutterActivity {



    /*@Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        CLAIDPackageLoader.loadPackage(new smart_inhaler());
        CLAID.startInBackground(getApplicationContext(),
                "assets://flutter_assets/assets/claid_test.json",
                "test_host",
                "device",
                "user",
                CLAIDSpecialPermissionsConfig.almightyCLAID(),
                CLAIDPersistanceConfig.maximumPersistance());
    }*/
}
