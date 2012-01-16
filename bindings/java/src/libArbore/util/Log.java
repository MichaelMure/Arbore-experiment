package libArbore.util;

public class Log {

	static {
        System.loadLibrary("util");
    }
	
	Log()  {
        initCppSide();
    }
	
	public void finalize() {
		destroyCppSide();
	}
    
	public native void SetLoggedFlags(String s, boolean to_syslog);
	public native int LoggedFlags();
	public native boolean ToSyslog();
	
	private native void initCppSide();
	private native void destroyCppSide();
    private long logPtr_;
}
