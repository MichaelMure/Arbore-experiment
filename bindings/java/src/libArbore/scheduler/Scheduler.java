package libArbore.scheduler;

public class Scheduler {

	public static void StartSchedulers(int number)
	{
		N_StartSchedulers(number);
	}
	
	/* ------------------------------------------------------------------ */

	static {
		System.loadLibrary("javascheduler");
    }
	
	private static native void N_StartSchedulers(int number);
}
