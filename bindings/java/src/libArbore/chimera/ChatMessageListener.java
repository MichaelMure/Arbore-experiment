package libArbore.chimera;

import libArbore.network.Host;

public interface ChatMessageListener {
	public void MessageReceived(String s, Host host);
}
