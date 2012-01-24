package libArbore.chimera;

import javax.swing.DefaultListModel;

import libArbore.network.Host;

public class Leafset {
	
	public String toString() {
		return N_toString(instance);
	}
	
	public Host getHost(int index) {
		return new Host(N_getHost(instance, index));
	}

	public DefaultListModel getHostsModel() {
		DefaultListModel model = new DefaultListModel();
		int number = N_getHostNumber(instance);
		for(int x = 0; x < number; x++) {
			model.addElement(new Host(N_getHost(instance, x)));
		}
		
		return model;
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
	private native int N_getHostNumber(long instance);
	private native void destroyCppSide(long instance);
    private long instance;
}
