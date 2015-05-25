package de.fernuni.dna.jwh.handlers;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.restlet.resource.Get;
import org.restlet.resource.ServerResource;

import de.fernuni.dna.jwh.SecondoRestAccess;

/**
 * Handles the coordinated shutdown of the server
 * @author Jerome White
 *
 */
public class ShutdownHandler extends ServerResource {

	private static final Logger log4j = LogManager
			.getLogger(ShutdownHandler.class.getName());

	/**
	 * Implements the HTTP-GET method
	 * Server will forced to shut down in a created thread,
	 * therefore this HTTP-Request can be completed without errors 
	 * @return HTTP-Body
	 */
	@Get
	public String showInfo() {
		log4j.info("Online-Shutdown called");
		// Start a new Thread to shut down the server
		new Thread() {
			@Override
			public void run() {
				try {
					// Wait for a short while to let the shutdown-http-request complete
					Thread.sleep(1000);
					SecondoRestAccess.shutdown();
				} catch (Exception e) {;
				}
			}
		}.start();
		return "Shutting down!";
	}
}
