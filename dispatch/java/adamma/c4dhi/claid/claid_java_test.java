package adamma.c4dhi.claid;


import java.lang.ProcessBuilder.Redirect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import adamma.c4dhi.claid.LocalDispatching.ModuleManager;
import adamma.c4dhi.claid.Module.ChannelSubscriberPublisher;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;
import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid.TestModule;

public class claid_java_test {
	static void method(ArrayList<Integer> ints, IntegerType ...ignore) {
        System.out.println("Hello, Integer!");
    }
    
    static void method(ArrayList<Double> dbs, DoubleType ...ignore) {
        System.out.println("Hello, Double!");
    }

	static void method(Map<String, Integer> dbs, IntegerType ...ignore) {
        System.out.println("Hello, map integer");
    }

	static void method(Map<String, Double> dbs, DoubleType ...ignore) {
        System.out.println("Hello, map double");
    }

	static void method(Class<?> d) {
        System.out.println("Other");
    }
    
    static class IntegerType {}
    static class DoubleType {}


	public static void main(String[] args){

       

		
		ModuleFactory moduleFactory = new ModuleFactory();
		moduleFactory.registerModule(TestModule.class);

		ArrayList<String> t = new ArrayList<>();
		Mutator<ArrayList<String>> a = TypeMapping.getMutator(new DataType(ArrayList.class, "ArrayList<String>"));



		// If Linux/Android:
		if(!CLAID.start("unix:///tmp/test_socket.grpc", "/home/lastchance/Documents/ALEX/alex_config.json", "alex_client", "test", "test", moduleFactory))
		{
			System.exit(0);
		}
		// If macOS or Windows:
		//CLAID.start("localhost:1337", "/Users/planger/Documents/ALEX/alex_config.json", "alex_client", "test", "test", moduleFactory);


		System.out.println(TypeMapping.getNewInstance(Double.class).getClass().getName());
		while(true);
	}
}