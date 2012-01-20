package chat;

import java.awt.Color;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

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
	 
	public void init()
	{
		/* Components Creation*/
		
		GridBagLayout gridbag = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		
		// Menu
		JMenuBar menubar = new JMenuBar();
		JMenu file = new JMenu("File");
		JMenu edit = new JMenu("Edit");
		JMenu help = new JMenu("?");
		JMenuItem quit = new JMenuItem("Quit");
		JMenuItem about = new JMenuItem("About");
		
		// Panel
		JPanel main = new JPanel();
		main.setLayout(gridbag);
		
		// TextField
		JTextField adressfield = new JTextField("Enter bootstrap ip:port");
		JTextField txtfield = new JTextField("Enter your text");
		
		// TextArea
		JScrollPane txtaera = new JScrollPane();
		
		// List
		String[] data = {"one", "two", "three", "four"};
		JList list = new JList(data);
		Border bd = BorderFactory.createLineBorder(Color.black);
		list.setBorder(bd);
		
		// Button
		JButton send = new JButton("Send");
		JButton connect = new JButton("Connect");
		
		/* Layout */
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.FIRST_LINE_START;
		
		//TOP
		c.gridx = 0;
		c.gridy = 0;
		c.weightx = 1.0;
		gridbag.setConstraints(adressfield, c);
		main.add(adressfield);
		
		c.gridx = 1;
		c.gridy = 0;
		c.weightx = 0.2;
		gridbag.setConstraints(connect, c);
		main.add(connect);
		
		//MIDDLE
		c.anchor = GridBagConstraints.CENTER;
		c.insets = new Insets(10,0,0,0);  //top padding
		
		c.gridx = 0;
		c.gridy = 1;
		c.weightx = 3.0; 
		c.weighty = 4.0;
		gridbag.setConstraints(txtaera, c);
		main.add(txtaera);
		
		c.gridx = 1;
		c.gridy = 1;
		c.weightx = 1.0;
		gridbag.setConstraints(list, c);
		main.add(list);
		
		//BOTTOM
		c.anchor = GridBagConstraints.PAGE_END;
		c.insets = new Insets(10,0,0,0);  //top padding
		c.gridx = 0;
		c.gridy = 2;
		c.weightx = 1.8;
		c.weighty = 0.1;
		gridbag.setConstraints(txtfield, c);
		main.add(txtfield);
		c.gridx = 1;
		c.gridy = 2;
		c.weightx = 0.2; 
		c.weighty = 0.1;
		gridbag.setConstraints(send, c);
		main.add(send);
		
		file.add(quit);
		help.add(about);
		menubar.add(file);
		menubar.add(edit);
		menubar.add(help);
		setJMenuBar(menubar);
		setContentPane(main);
		
		/* Window proprieties*/
		setSize(800,600);
		setTitle("Arbore Chat");
	    setResizable(true);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	}
}
