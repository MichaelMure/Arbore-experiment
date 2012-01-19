package test;

import libArbore.network.*;
import libArbore.util.*;

public class TestHostList {
	public static void main(String[] args) {
		Host_List host_list = new Host_List(5);
		Log log = new Log();
		Host host = host_list.DecodeHost("127.0.0.1:654");
		log.info(host.toString());
	}

}
