package libArbore.network;

public class Host_List {
	

	public Host DecodeHost(String hostname)
	{
		return new Host(N_decodeHost(instance, hostname));
	}
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javanetwork");
    }

	public Host_List(int size)  {
        instance = initCppSide(size);
    }
	
	public Host_List(long hl) {
		instance = hl;
	}
	
	public void finalize() {
		destroyCppSide(instance);
	}
	
	private native long N_decodeHost(long instance, String hostname);
	private native long initCppSide(int size);
	private native void destroyCppSide(long instance);
    private long instance;
}