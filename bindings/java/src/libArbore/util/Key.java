package libArbore.util;

public class Key {
			
	public String toString() {
		return N_toString(instance);
	}
	
	public static Key GetRandomKey() {
		return new Key(N_GetRandomKey());
	}
	
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javautil");
		Key.InitRandomNumberGenerator();
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
	private static native long N_GetRandomKey();
	
	private native long initCppSide();
	private native void destroyCppSide(long instance);
	private static native void InitRandomNumberGenerator();
    private long instance;
}
