package test;

import libArbore.network.Host;
import libArbore.util.Log;

public class TestHost {
	public static void main(String[] args) {
		Host host = new Host();
		Log log = new Log();
		log.info(host.toString());
		log.info(host.getKey().toString());
	}

}
