package libArbore.network;

import libArbore.chimera.Chimera;

public class Network {
	
	public long GetInstance() {
		return instance;
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

	public Network(Chimera c)  {
		long chimera_instance;
		if(c == null)
			chimera_instance = 0;
		else
			chimera_instance = c.getInstance();
		
        instance = initCppSide(chimera_instance);
    }
	
	public void finalize() {
		destroyCppSide(instance);
	}
	
	private native long N_getHost_List(long instance);
	private native long initCppSide(long chimera_instance);
	private native void destroyCppSide(long instance);
    private long instance;
}

