package de.fernuni.dna.jwh;

import java.io.FileNotFoundException;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.restlet.Component;
import org.restlet.Server;
import org.restlet.data.Protocol;
import org.restlet.resource.ServerResource;
import org.restlet.routing.Router;
import org.slf4j.bridge.SLF4JBridgeHandler;

import de.fernuni.dna.jwh.Configuration.Handler;
import de.fernuni.dna.jwh.secondo.SecondoManager;

/**
 * Main Class - Simple Rest server which streams the received JSON-Objects
 * to the Secondo-Server
 * @author Jerome White
 *
 */
public class SecondoRestAccess {

	private static final Logger log4j = LogManager
			.getLogger(SecondoRestAccess.class.getName());
	
	private static Component component;

	/**
	 * main - Adds the handlers to the internal Restlet-Server and starts the HTTP-Server
	 * @param args First argument can be the name of the configuration file
	 */
	public static void main(final String[] args) {
		System.setProperty("org.restlet.engine.loggerFacadeClass", "org.restlet.ext.slf4j.Slf4jLoggerFacade");
		
		log4j.debug("Loading Configuration");
		try {
			if (args.length > 1) {
				Configuration.loadConfiguration(args[0]);
			} else {
				Configuration.loadConfiguration(null);
			}
		} catch (FileNotFoundException e) {
			log4j.fatal(e);
			log4j.fatal("Unable to load configuration, exiting");
			System.exit(1);
		}

		component = new Component();

		log4j.info("Add a new HTTP server listening on port "
				+ Configuration.values.httpPort);
		
		Server server = component.getServers()
				.add(Protocol.HTTP, Configuration.values.httpPort);
		
		server.getContext().getParameters().add("minThreads", "4");

		Router router = new Router(component.getContext().createChildContext());

		log4j.debug("Setting up Handlers");
		try {
			setupHandlers(router);
		} catch (ClassNotFoundException e) {
			log4j.fatal("Error setting up handlers, exiting");
			System.exit(2);
		}

		component.getDefaultHost().attach(router);

		log4j.info("Starting Server...");
		try {
			component.start();
		} catch (Exception e) {
			log4j.fatal(e);
			log4j.fatal("Unable to start HTTP-Services, exiting");
			System.exit(3);
		}
		
		//Shutdown-Hook to handle SIGTERM
		Runtime.getRuntime().addShutdownHook(new Thread()
        {
            @Override
            public void run()
            {
                try {
					SecondoRestAccess.shutdown();
				} catch (Exception e) {
					e.printStackTrace();
				}
            }
        });
	}

	/**
	 * Shuts down the application
	 * HTTP-Server stops after all SecondoManagers have been destroyed 
	 * @throws Exception
	 */
	public static void shutdown() throws Exception {
		if (SecondoManager.isAvailable()) {
			log4j.info("Closing Secondo connections");
			SecondoManager.shutdownManagers();
		}
		Thread.sleep(1000);
		component.stop();
		Thread.sleep(1000);
		log4j.info("Shutdown Complete");
	}

	/**
	 * Dynamically sets up the configured handlers
	 * @param router Restlet-Router to which the handlers will be attached
	 * @throws ClassNotFoundException Will be thrown when the configured class cannot be loaded
	 */
	@SuppressWarnings("unchecked")
	private static void setupHandlers(Router router)
			throws ClassNotFoundException {
		// Reads the map from Configuration and adds a Handler
		// ex. router.attach("/position", GenericHandler.class);
		for (Iterator<Entry<String, Handler>> iterator = Configuration.values.handlers
				.entrySet().iterator(); iterator.hasNext();) {
			Map.Entry<String, Handler> pair = (Map.Entry<String, Handler>) iterator
					.next();
			log4j.info("Adding Router /" + pair.getKey() + " handeld by "
					+ pair.getValue().handlerClass);
			try {
				router.attach("/" + pair.getKey(),
						(Class<? extends ServerResource>) Class.forName(pair
								.getValue().handlerClass));
			} catch (ClassNotFoundException e) {
				log4j.fatal(e);
				log4j.fatal("Unable to find class " + pair.getValue());
				throw e;
			}
		}
	}
}