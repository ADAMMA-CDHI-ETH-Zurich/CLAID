package JavaCLAID;
public class Reflector extends jbind11.JBindWrapper
{

	public <T> void reflect(java.lang.String param0, T param1)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);
		invocationStack.add((java.lang.Object) param1);

		nativeInvoke("reflect", invocationStack);
	};

	public <T> void reflectWithDefaultValue(java.lang.String param0, T param1, T param2)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);
		invocationStack.add((java.lang.Object) param1);
		invocationStack.add((java.lang.Object) param2);

		nativeInvoke("reflectWithDefaultValue", invocationStack);
	};

}