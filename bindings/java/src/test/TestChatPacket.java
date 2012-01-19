package test;

import libArbore.network.ChatPacket;
import libArbore.util.Key;
import libArbore.util.Log;

public class TestChatPacket {

	public static void main(String[] args) {
		Log log = new Log();
		Key key1 = Key.GetRandomKey();
		Key key2 = Key.GetRandomKey();
		
		ChatPacket packet = new ChatPacket(key1, key2, "Hello");
		
		log.info(packet.toString());
		
		packet.SetFlags(ChatPacket.ACK | ChatPacket.MUSTROUTE);
		
		log.info(packet.toString());
	}

}
