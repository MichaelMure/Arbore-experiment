package chat;

import libArbore.chimera.ChatMessageListener;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.GregorianCalendar;

import javax.swing.JLabel;

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
	private Key me = Key.GetRandomKey();

	public ControllerChat(ChatWindow view){
		this.view = view;
		initWindowListener();
		Scheduler.StartSchedulers(5);
		network.Start();

	}

	private void initChimeraListener() {
		chimera.addListener(new ChatMsgListenImpl());
		
	}

	/**
	 * Initialize the listener for the main window
	 */
	private void initWindowListener() {
		view.addOkButtonListener(new OkButtonListener());
		view.addSendButtonListener(new SendButtonListener());
		view.addConnectButtonListener(new ConnectButtonListener());
	}
	
	private void refreshHostList() {
		view.getList().setModel(chimera.getLeafset().getHostsModel());
		System.out.println("leafset " + chimera.getLeafset().toString());
	}

	class OkButtonListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			view.hidePortField();
			int port = Integer.parseInt(view.getPortField().getText());
			chimera = new Chimera(network, port, me);
			initChimeraListener();
			}
		}

	class SendButtonListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			String msg = view.getTextField().getText();
			int[] list = view.getList().getSelectedIndices();
			for(int i=0; i<list.length; i++)
			{
			ChatPacket pack = new ChatPacket(chimera.getMe().getKey(),
					chimera.getLeafset().getHost(i).getKey(),
					msg);
			chimera.route(pack);
			}
			refreshHostList();
			String fmsg = view.getChatText().getText();
			GregorianCalendar now = new GregorianCalendar();
			String hour = String.valueOf(now.getMaximum(GregorianCalendar.HOUR_OF_DAY));
			fmsg += "\n" + hour + "  ME  " + " - " + msg;
			view.getChatText().setText(fmsg);
		}
	}

	class ConnectButtonListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			Host bootstrap_host = network.getHost_List().DecodeHost(view.getAdressField().getText());
			System.out.println("Joining host: " + bootstrap_host);
			chimera.join(bootstrap_host);
			view.hideConnectField();
			refreshHostList();
		}
	}
	
	class ChatMsgListenImpl implements ChatMessageListener {

		@Override
		public void MessageReceived(String s, Host h) {
			String fmsg = view.getChatText().getText();
			GregorianCalendar now = new GregorianCalendar();
			String hour = String.valueOf(now.getMaximum(GregorianCalendar.HOUR_OF_DAY));
			fmsg += "\n" + hour + "  - from " + h.toString() + " - " + s;
			view.getChatText().setText(fmsg);
			refreshHostList();
		}
	}
}
