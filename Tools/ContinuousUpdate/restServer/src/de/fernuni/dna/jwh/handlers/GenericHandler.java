package de.fernuni.dna.jwh.handlers;

import java.io.IOException;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.restlet.data.Status;
import org.restlet.representation.Representation;
import org.restlet.resource.Get;
import org.restlet.resource.Put;
import org.restlet.resource.ServerResource;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonParseException;
import com.google.gson.JsonSyntaxException;

import de.fernuni.dna.jwh.Configuration;
import de.fernuni.dna.jwh.representation.NLRepresentation;
import de.fernuni.dna.jwh.secondo.SecondoException;
import de.fernuni.dna.jwh.secondo.SecondoManager;

/**
 * Generic Restlet-Handler Instead of using a specialized Handler for each
 * Handler this generic handler should be able to be used with any type that
 * should be sent so Secondo
 * 
 * @author Jerome White
 *
 */
public class GenericHandler extends ServerResource {

	private static final Logger log4j = LogManager
			.getLogger(GenericHandler.class.getName());

	/**
	 * Implements the HTTP-GET method Information about the expected
	 * Secondo-Stream-Type will be returned to the client
	 * 
	 * @return HTTP-Body
	 */
	@Get
	public String showInfo() {
		log4j.debug("Get Request...");
		try {
			Gson g = new GsonBuilder().serializeNulls()
					.setDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS'Z'").create();
			String retstr = "Expecting a JSON-Object in the following format:\n"
					+ g.toJson(getRepresentationClass().newInstance()) + "\n\n";

			try {
				retstr = retstr
						+ SecondoManager.getInstance(
								getReference().getLastSegment())
								.getPrintableQueueSize();
			} catch (IOException | SecondoException e) {
				log4j.error(e);
			}
			return retstr;
		} catch (InstantiationException | IllegalAccessException e) {
			log4j.error(e);
			getResponse().setStatus(Status.SERVER_ERROR_INTERNAL,
					"Could not instantiate representation class!");
			return "Could not instantiate representation class!";
		}
	}

	/**
	 * Implements the HTTP-PUT method Receives a Secondo-Tuple, its type is
	 * defined in the config-file and sends it to the Secondo-Server
	 * 
	 * @return HTTP-Response with information about the success of the operation
	 */
	@Put
	public Representation receiveJSONObject(Representation entity) {
		log4j.debug("Put Request...");

		// Setup the Gson-Builder
		GsonBuilder builder = new GsonBuilder();
		Gson gson = builder.setDateFormat(Configuration.values.jsonDateFormat)
				.create();

		String stringValue = null;

		try {
			stringValue = entity.getText();
			log4j.debug("Getting Object for " + stringValue);
			NLRepresentation obj = gson.fromJson(stringValue,
					getRepresentationClass());

			if (!obj.isValid()) {
				throw new JsonParseException(
						"Object is not valid in respect to SECONDOs requirements");
			}

			// The last PATH-Part (http://localhost/Trains = Trains) is used to
			// identify the SecondoManagers for each handler
			log4j.debug("Getting SecondoManager-Instance for "
					+ getReference().getLastSegment());
			SecondoManager manager = SecondoManager.getInstance(getReference()
					.getLastSegment());

			log4j.debug("Hand the received Tuple to the SecondoManager");
			manager.addTuple(obj);
		} catch (JsonSyntaxException syntax) {
			log4j.error(syntax);
			getResponse().setStatus(Status.CLIENT_ERROR_UNSUPPORTED_MEDIA_TYPE,
					"JSON not parsable" + "\n" + syntax.getMessage());
		} catch (NullPointerException npe) {
			npe.printStackTrace();
			log4j.error("Got no data from client - NullPointerException");
			getResponse().setStatus(Status.CLIENT_ERROR_BAD_REQUEST,
					"No data provided!");
		} catch (IOException | SecondoException e) {
			log4j.error(e);
			log4j.error("Could not fulfill Rquest!");
			getResponse().setStatus(Status.CLIENT_ERROR_FAILED_DEPENDENCY,
					"Could not retrieve value from provided data!");
		}

		return getResponseEntity();
	}

	/**
	 * Gets the desired Secondo-Type (Representation Class) from the
	 * Configuration
	 * 
	 * @return Class-Object of the Representation Class
	 */
	@SuppressWarnings("unchecked")
	private Class<? extends NLRepresentation> getRepresentationClass() {
		String handler = getReference().getLastSegment();
		String typeClassName = Configuration.values.handlers.get(handler).representationClass;
		try {
			return (Class<? extends NLRepresentation>) Class
					.forName(typeClassName);
		} catch (ClassNotFoundException e) {
			log4j.error(e);
			return null;
		}
	}
}
