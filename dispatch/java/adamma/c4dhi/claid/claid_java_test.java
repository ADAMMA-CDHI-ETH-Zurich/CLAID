package adamma.c4dhi.claid;

import adamma.c4dhi.claid.CLAID;
import adamma.c4dhi.claid.LocalDispatching.ModuleManager;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.TestModule;

public class claid_java_test {



	public static void main(String[] args){

		//CLAID.start("unix:///tmp/claid_socket_java_test.grpc", "alex_client", "/home/lastchance/Documents/alex_config.json", "test", "test");
		ModuleFactory manager = new ModuleFactory();
		manager.registerModule(TestModule.class);
	}
}