package JavaCLAID;
public class Module extends jbind11.JBindWrapper
{

	public java.lang.Long getUniqueIdentifier()
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();

		java.lang.Object result = nativeInvoke("getUniqueIdentifier", invocationStack);
		return (java.lang.Long) result;
	};

	public <T> JavaCLAID.Channel<T> publish(java.lang.Class param0, java.lang.String param1)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);
		invocationStack.add((java.lang.Object) param1);

		java.lang.Object result = nativeInvoke("publish", invocationStack);
		return (JavaCLAID.Channel<T>) result;
	};

	public void registerPeriodicFunction(java.lang.String param0, java.lang.String param1, java.lang.Integer param2)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);
		invocationStack.add((java.lang.Object) param1);
		invocationStack.add((java.lang.Object) param2);

		nativeInvoke("registerPeriodicFunction", invocationStack);
	};

	public <T> JavaCLAID.Channel<T> subscribe(java.lang.Class param0, java.lang.String param1, java.util.function.Consumer<ChannelData<T>> param2)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);
		invocationStack.add((java.lang.Object) param1);
		invocationStack.add((java.lang.Object) param2);

		java.lang.Object result = nativeInvoke("subscribe", invocationStack);
		return (JavaCLAID.Channel<T>) result;
	};

}