package JavaCLAIDDataTypes;
public class BufferInfo extends jbind11.JBindWrapper
{

	public java.lang.Integer get_bufferDataType(){ return (java.lang.Integer) nativeGet("bufferDataType"); }
	public void set_bufferDataType (java.lang.Integer value){ nativeSet("bufferDataType", value); }

	public java.util.ArrayList<java.lang.Integer> get_shape(){ return (java.util.ArrayList<java.lang.Integer>) nativeGet("shape"); }
	public void set_shape(java.util.ArrayList<java.lang.Integer> value){ nativeSet("shape", value); }
public void set_shape (int[] value){ nativeSet("shape", value); }

	public java.util.ArrayList<java.lang.Integer> get_strides(){ return (java.util.ArrayList<java.lang.Integer>) nativeGet("strides"); }
	public void set_strides(java.util.ArrayList<java.lang.Integer> value){ nativeSet("strides", value); }
public void set_strides (int[] value){ nativeSet("strides", value); }

}