package jbind11;
public class JBindWrapper
{
	private long nativeJavaHandle = 0;
	protected native java.lang.Object nativeGet(String fieldName);
	protected native void nativeSet(String fieldName, java.lang.Object value);
	protected native java.lang.Object nativeInvoke(String functionName, java.util.ArrayList<java.lang.Object> stack);
	protected native static java.lang.Object nativeInvokeStatic(Class cls, String functionName, java.util.ArrayList<java.lang.Object> stack);
	protected native void nativeInit();
	protected native void nativeFinalize();

	public JBindWrapper() { nativeInit(); }
	protected void finalize() throws Throwable { nativeFinalize(); }
}