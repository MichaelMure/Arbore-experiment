package chat;

public class GraphicalChat {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		ChatWindow cw = new ChatWindow();
		cw.init();
		ControllerChat controllerChat = new ControllerChat(cw);
		cw.setVisible(true);

	}

}
