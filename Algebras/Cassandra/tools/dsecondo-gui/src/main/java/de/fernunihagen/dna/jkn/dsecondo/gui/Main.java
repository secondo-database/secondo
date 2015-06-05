package de.fernunihagen.dna.jkn.dsecondo.gui;

public class Main {
	
	/**
	 * Main Method 
	 * @param args
	 * @throws Exception 
	 */
	public static void main(String[] args) throws Exception {
		
		CassandraClient cassandraClient = new CassandraClient();
		cassandraClient.connect();
		
		final CassandraGUIModel guiModel = new CassandraGUIModel(cassandraClient);
		guiModel.updateModel();
		
		final CassandraGUI cassandraGUI = new CassandraGUI(guiModel);
		cassandraGUI.run();
		
		while(! cassandraGUI.shutdown) {
			guiModel.updateModel();
			cassandraGUI.updateView();
			Thread.sleep(1000);
		}
		
		cassandraGUI.dispose();
		
		// Wait for pending gui updates to complete
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e1) {
			// Ignore exception
		}
		
		cassandraClient.close();
	}
}
