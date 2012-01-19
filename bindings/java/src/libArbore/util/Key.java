package libArbore.util;

public class Key {
			
	public String toString() {
		return N_toString(instance);
	}
	
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javautil");
    }
	
	public Key()  {
        instance = initCppSide();
    }
	
	public Key(long key) {
		instance = key;
	}
	
	public void finalize() {
		destroyCppSide(instance);
	}
	
	private native String N_toString(long instance);
	
	private native long initCppSide();
	private native void destroyCppSide(long instance);
    private long instance;
}
