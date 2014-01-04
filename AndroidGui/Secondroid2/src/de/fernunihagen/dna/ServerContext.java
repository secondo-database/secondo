package de.fernunihagen.dna;

/**
 * Holds sth SecondoServerService
 * @author Michael Küpper
 *
 */
public class ServerContext {
	private static SecondoServerService secondoServerService = null;

	public static void setSecondoServerService(
			SecondoServerService secondoServerService) {
		ServerContext.secondoServerService = secondoServerService;
	}

	public static SecondoServerService getSecondoServerService() {
		return ServerContext.secondoServerService;
	}

	public static SecondoServerService getSecondoServerService(String server,
			String port, String user, String password, String optServer,
			String optPort, boolean usingOptimizer) {
		if (ServerContext.secondoServerService == null) {
			return null;
		}

		// Is the current Server the denied Server?
		if (ServerContext.secondoServerService.getServername().equals(server)
				&& ServerContext.secondoServerService.getPort().equals(port)
				&& (ServerContext.secondoServerService.getUsername() == null ? user == null
						: ServerContext.secondoServerService.getUsername()
								.equals(user))
				&& (ServerContext.secondoServerService.getPassword() == null ? password == null
						: ServerContext.secondoServerService.getPassword()
								.equals(password))
				&& ServerContext.secondoServerService.getOptServername()
						.equals(optServer)
				&& ServerContext.secondoServerService.getOptPort().equals(
						optPort)
				&& ServerContext.secondoServerService.isUsingOptimizer() == usingOptimizer)
			return ServerContext.secondoServerService;

		return null;
	}

}
