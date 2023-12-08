package adamma.c4dhi.claid_android.Permissions;
import adamma.c4dhi.claid_android.Permissions.Permission;
import adamma.c4dhi.claid.Module.ThreadSafeChannel;
// Executes Permission.blockingRequest() on a separate thread for incoming permissions.
public class PermissionHelperThread 
{
    private Thread thread;
    private ThreadSafeChannel<Permission> permissionsChannel;
    private boolean started = false;


    private void processPermissions()
    {
        while(this.started)
        {
            Permission permission = permissionsChannel.blockingGet();
            permission.blockingRequest();
        }
    }

    public void insertRunnable(Permission permission)
    {
        this.permissionsChannel.add(permission);
    }

    public boolean start()
    {
        if(this.started)
        {
            return false;
        }

        this.started = true;
        this.thread = new Thread(() -> processPermissions());
        this.thread.start();
        return true;
    }

    public boolean stop()
    {
        if(!this.started)
        {
            return false;
        }

        this.started = false;

        try{
            this.thread.join();
        }
        catch(InterruptedException e)
        {
            e.printStackTrace();
        }

        return true;
    }
}
