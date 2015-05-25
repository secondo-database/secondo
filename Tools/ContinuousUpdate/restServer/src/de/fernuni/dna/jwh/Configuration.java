package de.fernuni.dna.jwh;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.util.Map;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.google.gson.Gson;

/**
 * This class handles the configuration
 * The config-file will be defaultConfigFile as specified in this class
 * or the first argument will be used as filename.
 * All config-values will be exposed through the values member.
 * @author Jerome White
 *
 */
public class Configuration {

	private static final Logger log4j = LogManager
			.getLogger(Configuration.class.getName());
	/*
	 * Default configuration filename
	 */
	private static final String defaultConfigFile = "restAccess.config";

	/**
	 * Member which is used to access the config-values
	 */
	public static ConfigValues values;
	
	/**
	 * Local class which is used for the mapping of the JSON-values in
	 * the config-file (simple config values) to actual Java objects
	 * @author Jerome White
	 *
	 */
	public class ConfigValues {
		public String hostname;
		public int httpPort;
		public String secondoDateFormat;
		public String jsonDateFormat;
		public Map<String,Handler> handlers;
	}

	/**
	 * Local class which is used for the mapping of the JSON-values in
	 * the config-file (handlers) to actual Java objects
	 * @author Jerome White
	 *
	 */
	public class Handler {
		public String router;
		public int nestedListPort;
		public int secondoPort;
		public String secondoHost;
		public String secondoDatabase;
		public String secondoRelation;
		public String representationClass;
		public String handlerClass;
	}

	/*
	 * Constructor will be private, the loading of the Configuration
	 * will be done in a static function
	 */
	private Configuration() {
	}

	/**
	 * Reads the configuration values from the configuration file
	 * and converts the JSON-Value into a Java object which will be
	 * globally exposed through the values member 
	 * @param configFileName
	 * @throws FileNotFoundException
	 */
	public static void loadConfiguration(String configFileName)
			throws FileNotFoundException {
		String configFile;
		if (configFileName != null && new File(configFileName).exists()) {
			configFile = configFileName;
		} else {
			configFile = defaultConfigFile;
		}
		Gson gson = new Gson();
		log4j.info("Loading Configuration from " + configFile);
		BufferedReader br = new BufferedReader(new FileReader(configFile));
		values = gson.fromJson(br, ConfigValues.class);
	}
}
