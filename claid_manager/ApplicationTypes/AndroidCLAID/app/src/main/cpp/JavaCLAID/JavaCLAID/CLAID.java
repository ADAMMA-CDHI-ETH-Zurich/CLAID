package JavaCLAID;
public class CLAID extends jbind11.JBindWrapper
{

	static
	{
		System.loadLibrary("JavaCLAID");
	}

	public static void addModule(JavaCLAID.Module param0)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);

		nativeInvokeStatic(JavaCLAID.CLAID.class, "addModule", invocationStack);
	};

	public static void connectTo(java.lang.String param0, java.lang.Integer param1)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);
		invocationStack.add((java.lang.Object) param1);

		nativeInvokeStatic(JavaCLAID.CLAID.class, "connectTo", invocationStack);
	};

	public static void disableLoggingToFile()
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();

		nativeInvokeStatic(JavaCLAID.CLAID.class, "disableLoggingToFile", invocationStack);
	};

	public static void enableLoggingToFile(java.lang.String param0)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);

		nativeInvokeStatic(JavaCLAID.CLAID.class, "enableLoggingToFile", invocationStack);
	};

	public static java.lang.Object getActivity()
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();

		java.lang.Object result = nativeInvokeStatic(JavaCLAID.CLAID.class, "getActivity", invocationStack);
		return (java.lang.Object) result;
	};

	public static java.lang.Object getContext()
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();

		java.lang.Object result = nativeInvokeStatic(JavaCLAID.CLAID.class, "getContext", invocationStack);
		return (java.lang.Object) result;
	};

	public static void loadFromXML(java.lang.String param0)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);

		nativeInvokeStatic(JavaCLAID.CLAID.class, "loadFromXML", invocationStack);
	};

	public static void loadFromXMLString(java.lang.String param0)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);

		nativeInvokeStatic(JavaCLAID.CLAID.class, "loadFromXMLString", invocationStack);
	};

	public static void setActivity(java.lang.Object param0)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);

		nativeInvokeStatic(JavaCLAID.CLAID.class, "setActivity", invocationStack);
	};

	public static void setContext(java.lang.Object param0)
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();
		invocationStack.add((java.lang.Object) param0);

		nativeInvokeStatic(JavaCLAID.CLAID.class, "setContext", invocationStack);
	};

	public static void start()
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();

		nativeInvokeStatic(JavaCLAID.CLAID.class, "start", invocationStack);
	};

	public static void startInSeparateThread()
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();

		nativeInvokeStatic(JavaCLAID.CLAID.class, "startInSeparateThread", invocationStack);
	};

	public static void startNonBlockingWithoutUpdates()
	{
		java.util.ArrayList<java.lang.Object> invocationStack = new java.util.ArrayList<java.lang.Object>();

		nativeInvokeStatic(JavaCLAID.CLAID.class, "startNonBlockingWithoutUpdates", invocationStack);
	};

}