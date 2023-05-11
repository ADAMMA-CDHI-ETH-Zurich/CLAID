package JavaCLAID;
public class Channel<T> extends jbind11.JBindWrapper
{

	public void post(T param0)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);

		nativeInvoke("post", invocationStack);
	};

	public void postWithTimestamp(T param0, java.lang.Long param1)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);
		invocationStack.add((java.lang.Object) param1);

		nativeInvoke("postWithTimestamp", invocationStack);
	};

}