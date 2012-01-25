package chat;

import libArbore.chimera.ChatMessageListener;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Calendar;
import java.util.GregorianCalendar;

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

	@SuppressWarnings(value = { "static-access" })
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
			String fmsg = view.getChatText().getText();
			for(int i=0; i<list.length; i++)
			{
			ChatPacket pack = new ChatPacket(chimera.getMe().getKey(),
					chimera.getLeafset().getHost(i).getKey(),
					msg);
			fmsg += "\n" + getTime() + " to " + chimera.getLeafset().getHost(i).toString() +" ~ " + msg;
			chimera.route(pack);
			}
			refreshHostList();
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
			fmsg += "\n" + getTime() + " from " + h.toString() + " ~ " + s;
			view.getChatText().setText(fmsg);
			refreshHostList();
		}
	}
	
	private String getTime() {
		Calendar cal = new GregorianCalendar();

		// Get the components of the time
		Integer hour24 = cal.get(Calendar.HOUR_OF_DAY);
		Integer min = cal.get(Calendar.MINUTE);
		Integer sec = cal.get(Calendar.SECOND);
		return "[" + String.format("%02d", hour24)
				+ ":" + String.format("%02d", min)
				+ ":" + String.format("%02d", sec) + "]";
	}
}
