package test;

import libArbore.network.*;
import libArbore.util.Log;

public class TestNetwork {
	
	public static void main(String[] args) {
		Network network = new Network();
		Log log = new Log();
		Host_List hl = network.getHost_List();
		Host host = hl.DecodeHost("127.0.0.1:654");
		log.info(host.toString());

}
	
}
