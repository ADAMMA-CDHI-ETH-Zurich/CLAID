package adamma.c4dhi.claid_android.Package;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

@Retention(RetentionPolicy.RUNTIME)
public @interface CLAIDPackageAnnotation
{
    String[] authors();
    String date();
    String description();
    String version();
};