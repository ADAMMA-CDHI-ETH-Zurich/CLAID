package JavaCLAID;
public class ChannelData<T> extends jbind11.JBindWrapper
{

	public T value()
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();

		java.lang.Object result = nativeInvoke("value", invocationStack);
		return (T) result;
	};

}