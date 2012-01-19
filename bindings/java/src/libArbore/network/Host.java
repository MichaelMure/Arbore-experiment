package libArbore.network;

public class Host {
	
	public String toString()
	{
		return N_toString(instance);
	}
	
	/*public Key getKey()
	{
		return N_getKey(instance);
	}
	*/
	
	
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javanetwork");
    }

	public Host()  {
        instance = initCppSide();
    }
	
	public void finalize() {
		destroyCppSide(instance);
	}
	
	//private native Key N_getKey(long instance);
	private native String N_toString(long instance);
	private native long initCppSide();
	private native void destroyCppSide(long instance);
    private long instance;
}
