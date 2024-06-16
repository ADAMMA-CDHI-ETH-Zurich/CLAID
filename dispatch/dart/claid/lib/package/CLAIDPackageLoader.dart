import 'CLAIDPackage.dart';
import 'package:claid/logger/Logger.dart';

class CLAIDPackageLoader
{
    static Map<String, CLAIDPackage> loadedPackages = Map<String, CLAIDPackage>();

    static void loadPackage(CLAIDPackage package)
    {
        String packageName = package.runtimeType.toString();

        if(loadedPackages.containsKey(packageName))
        {
            Logger.logWarning("Not loading CLAID package \"$packageName\", because it is already loaded.");
            return;
        }
        Logger.logInfo("Loading package $packageName");
        package.register();
        loadedPackages[packageName] = package;
    }
}