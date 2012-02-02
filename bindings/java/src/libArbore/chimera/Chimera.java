package libArbore.chimera;

import java.util.ArrayList;
import java.util.List;

import libArbore.network.ChatPacket;
import libArbore.network.Host;
import libArbore.network.Network;
import libArbore.util.Key;


public class Chimera {

	public Host getMe(){
		return new Host(N_getMe(instance));
	}
	
	public Network getNetwork(){
			return new Network(N_getNetwork(instance));
	}
	
	public long getInstance() {
		return instance;
	}
	
	public void join(Host bootstrap)
	{
		N_join(instance, bootstrap.GetInstance());
	}
	
	public boolean route(ChatPacket cp)
	{
		return N_route(instance, cp.GetInstance());
	}
	
	public Leafset getLeafset() {
		return new Leafset(N_getLeafset(instance));
	}
	
	public static void MessageCallback(String s, long host_instance) {
		Host host = new Host(host_instance);
		System.out.println("Message received from: " + host + "  -->  " + s);
		for (ChatMessageListener cml : listeners)
            cml.MessageReceived(s, host);
	}

    public static void addListener(ChatMessageListener toAdd) {
        listeners.add(toAdd);
    }
    
    private static List<ChatMessageListener> listeners = new ArrayList<ChatMessageListener>();
	
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javachimera");
    }

	public Chimera(int port, Key k)  {
        instance = initCppSide(port, k.GetInstance());
    }
	
	public void finalize() {
		destroyCppSide(instance);
	}

	private native long N_getMe(long instance);
	private native long N_getNetwork(long instance);
	private native long N_getLeafset(long instance);
	private native void N_join(long instance, long bootstrap);
	private native boolean N_route(long instance, long cp);
	private native long initCppSide(int port, long key);
	private native void destroyCppSide(long instance);
    private long instance;
}