package libArbore.chimera;

import libArbore.network.ChatPacket;
import libArbore.network.Host;
import libArbore.network.Network;

public class Chimera {

	public Host getMe(){
		return new Host(N_getMe(instance));
	}
	
	public Network getNetwork(){
			return new Network(N_getNetwork(instance));
	}
	
	public void join(Host bootstrap)
	{
		N_join(instance, bootstrap);
	}
	
	public boolean route(ChatPacket cp)
	{
		return N_route(instance,cp);
	}
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javachimera");
    }

	public Chimera()  {
        instance = initCppSide();
    }
	
	public void finalize() {
		destroyCppSide(instance);
	}
	
	private native long N_getMe(long instance);
	private native long N_getNetwork(long instance);
	private native void N_join(long instance, Host bootstrap);
	private native boolean N_route(long instance, ChatPacket cp);
	private native long initCppSide();
	private native void destroyCppSide(long instance);
    private long instance;
}