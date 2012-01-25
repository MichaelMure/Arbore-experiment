package chat;

public class GraphicalChat {

	public static void main(String[] args) {
		ChatWindow cw = new ChatWindow();
		cw.init();
		@SuppressWarnings("unused")
		ControllerChat controllerChat = new ControllerChat(cw);
		cw.setVisible(true);
	}

}
