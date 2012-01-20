package chat;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import libArbore.chimera.Chimera;
import libArbore.network.ChatPacket;
import libArbore.network.Host;
import libArbore.network.Network;
import libArbore.scheduler.Scheduler;
import libArbore.util.Key;

public class ControllerChat {

	private ChatWindow view;
	private Network network = new Network();
	private Chimera chimera;

	public ControllerChat(ChatWindow view){
		this.view = view;
		initWindowListener();

		Key me = Key.GetRandomKey();
		Scheduler.StartSchedulers(5);
		network.Start();

	}

	/**
	 * Initialize the listener for the main window
	 */
	private void initWindowListener() {
		view.addOkButtonListener(new OkButtonListener());
		view.addSendButtonListener(new SendButtonListener());
		view.addConnectButtonListener(new ConnectButtonListener());
	}

	class OkButtonListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			view.hidePortField();
		}
	}

	class SendButtonListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			String msg = view.getTextField().getText();
			/*TODO boucle pour générer un paquet à tous les hosts sélectionnés dans la liste
			 * for();
			ChatPacket packet = new ChatPacket(chimera.getMe().getKey(),
					chimera.getLeafset().getHost(i).getKey(),
					msg);
			chimera.route(packet);
			}
			 */
		}
	}

	class ConnectButtonListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			Host bootstrap_host = network.getHost_List().DecodeHost(view.getAdressField().getText());
			System.out.println("Joining host: " + bootstrap_host);
			chimera.join(bootstrap_host);
			view.hideConnectField();
		}
	}
}
