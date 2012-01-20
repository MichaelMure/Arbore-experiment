package chat;

import java.util.Scanner;

import libArbore.chimera.Chimera;
import libArbore.network.ChatPacket;
import libArbore.network.Host;
import libArbore.network.Network;
import libArbore.scheduler.Scheduler;
import libArbore.util.Key;
import libArbore.util.Log;

public class ConsoleChat {

	public static void main(String[] args) {
		if(args.length < 1) {
			System.out.println("Usage: listen_port [boostrap_host:port]" );
			return;
		}

		ConsoleChat chat;
		
		int port = Integer.parseInt(args[0]);
		
		if(args.length > 1)
			chat = new ConsoleChat(port, args[1]);
		else
			chat = new ConsoleChat(port);
		
		chat.Run();
		
	}
	
	private ConsoleChat(int port) {
		this(port, null);
	}
	
	private ConsoleChat(int port, String bootstrap) {
		Key me = Key.GetRandomKey();
		
		chimera = new Chimera(network, port, me);
		
		//log.SetLoggedFlags("ALL", false);
		Scheduler.StartSchedulers(5);
		network.Start();
		
		if(bootstrap != null) {
			Host bootstrap_host = network.getHost_List().DecodeHost(bootstrap);
			chimera.join(bootstrap_host);
		}
	}
	
	private void Run() {
		Scanner in = new Scanner(System.in);
		
		printHosts();
		
		String str = in.nextLine();
		while(!str.startsWith("q")) {
			String[] split = str.split(" ");
			
			try {
				int host_index = Integer.parseInt(split[0]);
				String message = str.substring(split[0].length());
				
				System.out.println("Send message to: " + host_index + " --> " + message);
				ChatPacket packet = new ChatPacket(chimera.getMe().getKey(),
						chimera.getLeafset().getHost(host_index).getKey(),
						message);
				chimera.route(packet);
			}
			catch(Exception e) {
				
			}
			
			printHosts();
			str = in.nextLine();
		}
	}
	
	private void printHosts() {
		System.out.println("Available host:");
		System.out.println(chimera.getLeafset().toString());
	}
	
	private Log log = new Log();
	private Network network = new Network();
	private Chimera chimera;
}
