package eu.ehnes.secondoandroid;


public class starthelper {
	
	//public native int startsecondo(String pfad);
	public native boolean initialize();
	public native void shutdown();
	public native Object query(String sAbfrage);
	public native String errorMessage();
	
	static {
		System.loadLibrary("app");
	}
}
