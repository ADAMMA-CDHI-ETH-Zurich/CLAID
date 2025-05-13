package adamma.c4dhi.claid_android.CLAIDServices;

import android.app.Service;

import adamma.c4dhi.claid.R;

public class ServiceAnnotation {
    private String serviceTitle;
    private String serviceText;
    private int iconResource;

    public ServiceAnnotation(String serviceTitle, String serviceText, int iconResource) {
        this.serviceTitle = serviceTitle;
        this.serviceText = serviceText;
        this.iconResource = iconResource;
    }

    public static ServiceAnnotation annotationWithDefaultIcon(String serviceTitle, String serviceText) {
        return new ServiceAnnotation(
                serviceTitle,
                serviceText,
                adamma.c4dhi.claid.R.drawable.ic_launcher_foreground
        );
    }

    public static ServiceAnnotation defaultAnnotation() {
        return new ServiceAnnotation(
                "CLAID Service",
                "CLAID is running in the background..",
                adamma.c4dhi.claid.R.drawable.ic_launcher_foreground
        );
    }

    public String getServiceTitle() {
        return serviceTitle;
    }

    public String getServiceText() {
        return serviceText;
    }

    public int getIconResource() {
        return iconResource;
    }
}
