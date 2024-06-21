package adamma.c4dhi.claid.demo_app;

import adamma.c4dhi.claid_android.Package.CLAIDPackage;
import adamma.c4dhi.claid_android.Package.CLAIDPackageAnnotation;

@CLAIDPackageAnnotation(authors = {}, date = "", description = "", version = "")
public class TestPackage extends CLAIDPackage
{

    @Override
    public void register() {
        System.out.println("Initializing testxx package");
    }
}
