package adamma.c4dhi.galaxy_watch_claid;

import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.Properties;
import adamma.c4dhi.claid_android.Permissions.BodySensorsPermission;

public class TestModule extends Module {
    @Override
    protected void initialize(Properties properties) {
        BodySensorsPermission perm = new BodySensorsPermission();
        perm.blockingRequest();
    }
}
