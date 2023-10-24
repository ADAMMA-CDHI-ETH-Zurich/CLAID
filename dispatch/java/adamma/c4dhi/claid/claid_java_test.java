package adamma.c4dhi.claid;

import javax.xml.crypto.Data;

import adamma.c4dhi.claid.CLAID;
import adamma.c4dhi.claid.LocalDispatching.ModuleManager;
import adamma.c4dhi.claid.Module.ChannelSubscriberPublisher;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;
import adamma.c4dhi.claid.TestModule;

public class claid_java_test {



	public static void main(String[] args){

		CLAID.sayHelloDD();
		ModuleFactory moduleFactory = new ModuleFactory();
		moduleFactory.registerModule(TestModule.class);

		// If Linux/Android:
		//CLAID.start("unix:///tmp/test_socket.grpc", "/home/lastchance/Documents/ALEX/alex_config.json", "alex_client", "test", "test", moduleFactory);
		// If macOS or Windows:
	CLAID.start("localhost:1337", "/Users/planger/Documents/ALEX/alex_config.json", "alex_client", "test", "test", moduleFactory);


		System.out.println(TypeMapping.getNewInstance(Double.class).getClass().getName());
		while(true);
	}
}