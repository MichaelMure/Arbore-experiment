package libArbore.chimera;

import libArbore.network.Host;

public class Leafset {
	
	public String toString() {
		return N_toString(instance);
	}
	
	public Host getHost(int index) {
		return new Host(N_getHost(instance, index));
	}

	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javachimera");
    }

	public Leafset(long instance)  {
        this.instance = instance;
    }
	
	public void finalize() {
		destroyCppSide(instance);
	}
	
	private native String N_toString(long instance);
	private native long N_getHost(long instance, int index);
	private native void destroyCppSide(long instance);
    private long instance;
}
