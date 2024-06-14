package adamma.c4dhi.claid.Package; 

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

@Retention(RetentionPolicy.RUNTIME)

public @interface CLAIDPackage
{
    String authors[]();
    String date;
    String description;
};