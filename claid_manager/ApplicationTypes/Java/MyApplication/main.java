import JavaCLAID.CLAID;
import JavaCLAID.Module;

public class main 
{
    static
    {
        System.loadLibrary("JavaCLAID");
    }

    public static void main(String[] args)
    {
        CLAID.addModule(new MyModule());
        CLAID.start();
    }
}
