package eu.ehnes.secondoandroid;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.List;
import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

/**
 * The Class HandleAssets. This Class handles the files in the asset directory which
 * holds the needed files for the installation. You can decide witch of these files you
 * want to copy to workspace and where 
 * 
 * @author juergen
 * @version 1.000
 */
public class HandleAssets {
	private AssetManager assetManager; 
	private String[] files = null;
	private List<String> filesList;
	
	private static HandleAssets instance=null;
	
	// Private Constructor
	private HandleAssets() {}
	
	private HandleAssets(Context context) {
		
		assetManager = context.getAssets();
		
		try {
			files=assetManager.list("");
		} catch(IOException e)
		{
			Log.e("FU", "Failed to get asset file list.", e);
		}
		filesList = Arrays.asList(files);

		
	} 
	
	public static synchronized HandleAssets getInstance(Context contextArg) {
		if (instance == null) {
			instance = new HandleAssets(contextArg);

		}
		return instance;
	}

	/**
	 * Copies the files from the within memory assets to the file system.
	 *
	 * @param filename The file to copy from assets
	 * @param destinationdir the destination directory
	 */
	public void copyAssets(String filename, String destinationdir)
	{
	
		if (!filesList.contains(filename)) {
			return;
		}
	
		InputStream in=null;
		OutputStream out=null;
	
		java.io.File file = new java.io.File(destinationdir + "/" + filename);
	
		if(!file.exists()) {
			try {
				in=assetManager.open(filename);
				out=new FileOutputStream(destinationdir + "/" + filename);
				copyFile(in, out);
				in.close();
				in=null;
				out.flush();
				out.close();
				out=null;
			}catch(IOException e) {
				Log.e("FU", "Failed to copy asset file: " + destinationdir + filename, e );
			}
		} else {
			System.out.println("File "+filename+" already exists");
		}
	}
	
	/**
	 * Realizes the copy action
	 *
	 * @param in Stream for in
	 * @param out Stream for out
	 * @throws IOException Signals that an I/O exception has occurred.
	 */
	private void copyFile(InputStream in, OutputStream out) throws IOException {
	    byte[] buffer = new byte[1024];
	    int read;
	    while((read = in.read(buffer)) != -1){
	      out.write(buffer, 0, read);
	    }
	}
	
	
}
