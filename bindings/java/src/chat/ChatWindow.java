package chat;

import java.awt.Color;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionListener;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.border.Border;

public class ChatWindow extends JFrame{
	
	private JMenuBar menubar;
	private JMenu file;
	private JMenu edit;
	private JMenu help;
	private JMenuItem quit;
	private JMenuItem about;
	private JPanel main;
	private JTextField adressfield;
	private JTextField txtfield;
	private JScrollPane txtaera;
	private JList list;
	private JButton send;
	private JButton connect;
	private JTextField port;
	private JButton portok;
	 
	public void init()
	{
		/* Components Creation*/
		
		GridBagLayout gridbag = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		
		// Menu
		menubar = new JMenuBar();
		file = new JMenu("File");
		edit = new JMenu("Edit");
		help = new JMenu("?");
		quit = new JMenuItem("Quit");
		about = new JMenuItem("About");
		
		// Panel
		main = new JPanel();
		main.setLayout(gridbag);
		
		// TextField
		adressfield = new JTextField("Enter bootstrap ip:port");
		adressfield.setEnabled(false);
		txtfield = new JTextField("Enter your text");
		port = new JTextField("Enter the port to listen");
		
		// TextArea
		txtaera = new JScrollPane();
		
		// List
		list = new JList();
		Border bd = BorderFactory.createLineBorder(Color.black);
		list.setBorder(bd);
		
		// Button
		send = new JButton("Send");
		connect = new JButton("Connect");
		connect.setEnabled(false);
		portok = new JButton("Ok");
		
		/* Layout */
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.FIRST_LINE_START;
		
		
		c.gridx = 0;
		c.gridy = 0;
		c.weightx = 1.0;
		gridbag.setConstraints(port, c);
		main.add(port);
		
		c.gridx = 1;
		c.gridy = 0;
		c.weightx = 0.2;
		gridbag.setConstraints(portok, c);
		main.add(portok);
		
		//TOP
		c.gridx = 0;
		c.gridy = 1;
		c.weightx = 1.0;
		gridbag.setConstraints(adressfield, c);
		main.add(adressfield);
		
		c.gridx = 1;
		c.gridy = 1;
		c.weightx = 0.2;
		gridbag.setConstraints(connect, c);
		main.add(connect);
		
		//MIDDLE
		c.anchor = GridBagConstraints.CENTER;
		c.insets = new Insets(10,0,0,0);  //top padding
		
		c.gridx = 0;
		c.gridy = 2;
		c.weightx = 3.0; 
		c.weighty = 4.0;
		gridbag.setConstraints(txtaera, c);
		main.add(txtaera);
		
		c.gridx = 1;
		c.gridy = 2;
		c.weightx = 1.0;
		gridbag.setConstraints(list, c);
		main.add(list);
		
		//BOTTOM
		c.anchor = GridBagConstraints.PAGE_END;
		c.insets = new Insets(10,0,0,0);  //top padding
		c.gridx = 0;
		c.gridy = 3;
		c.weightx = 1.8;
		c.weighty = 0.1;
		gridbag.setConstraints(txtfield, c);
		main.add(txtfield);
		c.gridx = 1;
		c.gridy = 3;
		c.weightx = 0.2; 
		c.weighty = 0.1;
		gridbag.setConstraints(send, c);
		main.add(send);
		
		file.add(quit);
		help.add(about);
		menubar.add(file);
		menubar.add(edit);
		menubar.add(help);
		//setJMenuBar(menubar);
		setContentPane(main);
		
		/* Window proprieties*/
		setSize(800,600);
		setTitle("Arbore Chat");
	    setResizable(true);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	}
	 
	 public void addOkButtonListener(ActionListener actLst) {
	      portok.addActionListener(actLst);
	  }
	 
	 public void addConnectButtonListener(ActionListener actLst) {
	      connect.addActionListener(actLst);
	  }
	 
	 public void addSendButtonListener(ActionListener actLst) {
	      send.addActionListener(actLst);
	  }
	 
	 public JTextField getPortField(){
		 return port;
	 }
	 
	 public JTextField getAdressField(){
		 return adressfield;
	 }
	 
	 public JTextField getTextField(){
		 return txtfield;
	 }
	 
	 public JScrollPane getTextAera(){
		 return txtaera;
	 }
	 
	 public JList getList() {
		 return list;
	 }
	 
	 public JScrollPane getTxtAera() {
		 return txtaera;
	 }
	 
	 public void hidePortField(){
		 port.setEnabled(false);
		 portok.setEnabled(false);
		 adressfield.setEnabled(true);
		 connect.setEnabled(true);
	 }
	 
	 public void hideConnectField(){
		 adressfield.setEnabled(false);
		 connect.setEnabled(false);
	 }

	
}

	
