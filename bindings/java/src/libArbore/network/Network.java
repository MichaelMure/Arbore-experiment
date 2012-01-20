package libArbore.network;

public class Network {
	
	public long GetInstance() {
		return instance;
	}
	
	public void Start() {
		
		N_Start(instance);
	}
	
	public Host_List getHost_List() {
		return new Host_List((N_getHost_List(instance)));
	}
	
	public Network(long net)
	{
		instance = net;
	}
	
	
	
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javanetwork");
    }

	public Network()  {
        instance = initCppSide();
    }
	
	public void finalize() {
		destroyCppSide(instance);
	}
	
	private native void N_Start(long instance);
	private native long N_getHost_List(long instance);
	private native long initCppSide();
	private native void destroyCppSide(long instance);
    private long instance;
}

