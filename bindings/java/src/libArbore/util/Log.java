package libArbore.util;

public class Log {

	public void SetLoggedFlags(String s, boolean to_syslog) {
		N_SetLoggedFlags(instance, s, to_syslog);
	}
	
	public int LoggedFlags() {
		return N_LoggedFlags(instance);
	}
	
	public boolean ToSyslog() {
		return N_ToSyslog(instance);
	}
	
	public void print(String s) {
		N_print(instance, s);
	}
	
	static {
		System.loadLibrary("javautil");
    }
	
	public Log()  {
        initCppSide();
    }
	
	public void finalize() {
		destroyCppSide(instance);
	}
    
	private native void N_SetLoggedFlags(long instance, String s, boolean to_syslog);
	private native int N_LoggedFlags(long instance);
	private native boolean N_ToSyslog(long instance);
	private native void N_print(long instance, String s);
	
	private native long initCppSide();
	private native void destroyCppSide(long instance);
    private long instance;
}
