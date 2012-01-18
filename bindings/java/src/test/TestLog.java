package test;

import libArbore.util.Log;

public class TestLog {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Log log = new Log();
		log.debug("hello DEBUG");
		log.warning("hello WARNING");
		log.error("hello ERROR");
		log.info("hello INFO");
		
		Boolean to_syslog = log.ToSyslog();
		log.info(to_syslog.toString());
		
		//log.SetLoggedFlags("ALL", true);
		
		log.debug("hello DEBUG");
		log.warning("hello WARNING");
		log.error("hello ERROR");
		log.info("hello INFO");
		
		to_syslog = log.ToSyslog();
		log.info(to_syslog.toString());
	}

}
