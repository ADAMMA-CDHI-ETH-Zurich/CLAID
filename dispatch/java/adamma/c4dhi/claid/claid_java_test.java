package adamma.c4dhi.claid;

import adamma.c4dhi.claid.CLAID;

public class claid_java_test {


	public static void main(String[] args){

		System.out.println("Hello, World!");
		CLAID.start("unix:///tmp/claid_socket_java_test.grpc", "alex_client", "/home/lastchance/Documents/alex_config.json", "test", "test");
	
	}
}