package test.config_reload_test;


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
import test.config_reload_test.TestModule1;

public class config_reload_test {
	

	public static void main(String[] args){

       

		
		ModuleFactory moduleFactory = new ModuleFactory();
		moduleFactory.registerModule(TestModule1.class);

		ArrayList<String> t = new ArrayList<>();
		Mutator<ArrayList<String>> a = TypeMapping.getMutator(new DataType(ArrayList.class, "ArrayList<String>"));


		if(!CLAID.start("localhost:1337", "/Users/planger/Development/ModuleAPIV2/dispatch/test/config_reload_test_1.json", "test_client", "test", "test", moduleFactory))
		{
			System.exit(0);
		}

		System.out.println(TypeMapping.getNewInstance(Double.class).getClass().getName());
		CLAID.loadNewConfig("/Users/planger/Development/ModuleAPIV2/dispatch/test/config_reload_test_2.json");
		while(true);
	}
}