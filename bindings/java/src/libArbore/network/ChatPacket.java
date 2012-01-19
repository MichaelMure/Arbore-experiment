package libArbore.network;

import libArbore.util.Key;

public class ChatPacket {
	
	/* Packet Flags */
	public static final int REQUESTACK = 1 << 0; /**< This packet request an acknoledged answer. */
	public static final int ACK        = 1 << 1; /**< This is an acknowledge answer. */
	public static final int MUSTROUTE  = 1 << 2; /**< This packet must be routed. */

	
	public String toString() {
		return N_toString(instance);
	}
	
	public ChatPacket(Key source, Key destination, String s) {
		instance = initCppSide(source.GetInstance(), destination.GetInstance(), s);
	}
	
	public void SetFlags(int flags) {
		N_SetFlags(instance, flags);
	}
	
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javanetwork");
    }
	
	public ChatPacket(long instance) {
		this.instance = instance;
	}
	
	public void finalize() {
		destroyCppSide(instance);
	}
	
	private native String N_toString(long instance);
	private native void N_SetFlags(long instance, int flags);
	
	private native long initCppSide(long key_source, long key_dest, String s);
	private native void destroyCppSide(long instance);
    private long instance;
}
