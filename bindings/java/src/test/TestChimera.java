package test;

import libArbore.chimera.Chimera;
import libArbore.network.ChatPacket;
import libArbore.network.Host;
import libArbore.network.Network;
import libArbore.util.Key;
import libArbore.util.Log;

public class TestChimera {
	
	public static void main(String[] args) {
		
		Network net = new Network();
		Key mykey = new Key(); 
		Host h = new Host();
		ChatPacket cp = new ChatPacket(mykey, h.getKey(), "tutu");
		
		Log log = new Log();
		
		Chimera ch = new Chimera(net, 4200, mykey);
		ch.join(h);
		log.info(ch.getMe().toString());
		ch.route(cp);

}
}
