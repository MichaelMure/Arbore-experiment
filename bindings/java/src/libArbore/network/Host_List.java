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

	public Host_List(long instance)  {
       	this.instance = instance;
    }	
	
	private native long N_decodeHost(long instance, String hostname);
    private long instance;
}