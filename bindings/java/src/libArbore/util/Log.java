package libArbore.util;

public class Log {

	static final int DEBUG = 1 << 0;
	static final int WARNING = 1 << 4;
	static final int ERROR = 1 << 5;
	static final int INFO = 1 << 6;
			
	public void SetLoggedFlags(String str, boolean to_syslog) {
		N_SetLoggedFlags(instance, str, to_syslog);
	}

	public boolean ToSyslog() {
		return N_ToSyslog(instance);
	}
	
	public void debug(String s) {
		N_print(instance, DEBUG, s);
	}
	
	public void warning(String s) {
		N_print(instance, WARNING, s);
	}
	
	public void error(String s) {
		N_print(instance, ERROR, s);
	}
	
	public void info(String s) {
		N_print(instance, INFO, s);
	}
	
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javautil");
    }
	
	public Log()  {
        instance = initCppSide();
    }
	
	public void finalize() {
		destroyCppSide(instance);
	}
    
	private native void N_print(long instance, int level, String s);
	private native void N_SetLoggedFlags(long instance, String str, boolean to_syslog);
	private native boolean N_ToSyslog(long instance);
	
	private native long initCppSide();
	private native void destroyCppSide(long instance);
    private long instance;
}
