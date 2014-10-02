// SecondoActivity
// (c) Jürgen Ehnes 2012-2013

package eu.ehnes.secondoandroid;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import eu.ehnes.secondoandroid.R;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.util.Log;
import android.view.Menu;
import android.view.View;


public class SecondoActivity extends Activity {

	public static starthelper sh;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		System.out.println("Start des Programms");
		
		// Initialisierung des aktuellen Pfades.
		OwnPath ownpath=OwnPath.getInstance();
		ownpath.setPath(getFilesDir().toString());
		
		System.out.println("Aktueller Pfad: "+ownpath.getPath());
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_secondo);
		
		
		// Die benötigten Dateien werden vom Asset ins Programmverzeichnis entpackt.
		String filesPath=ownpath.getPath();
		String programPath=ownpath.getPath()+"/../";
		
		copyAssets("Config.ini", programPath);
		copyAssets("ProgressConstants.csv",programPath);
		copyAssets("opt",filesPath);
		copyAssets("berlintest",filesPath);
		copyAssets("literature",filesPath);
		copyAssets("constrainttest",filesPath);
		
		// Die Starthelpervariable wird initialisiert
		sh=new starthelper();
		
		sh.initialize();
		
		
	}

/*	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
	}
*/
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_secondo, menu);
		return true;
	}
	// Name: copyAssets
	// Funktion: Kopiert mitgelieferte Dateien aus dem Assets Speicher 
	//			 ins Filesystem
	// Argumente: Quelldateiname, Zielverzeichnis	
	private void copyAssets(String filename, String destinationdir)
	{
		AssetManager assetManager = getAssets();
		String[] files = null;
		try {
			files=assetManager.list("");
		} catch(IOException e)
		{
			Log.e("FU", "Failed to get asset file list.", e);
		}

		for(String fname: files) {
			InputStream in=null;
			OutputStream out=null;
			
			java.io.File file = new java.io.File(destinationdir + "/" + fname);

			if(!file.exists()) {
				try {

					if(fname.equals(filename)) {
						in=assetManager.open(fname);
						out=new FileOutputStream(destinationdir + "/" + fname);
						copyFile(in, out);
						in.close();
						in=null;
						out.flush();
						out.close();
						out=null;
					} else {
						
					}
				}catch(IOException e) {
					Log.e("FU", "Failed to copy asset file: " + destinationdir + filename, e );
				}
			} else {
				System.out.println("File "+fname+" already exists");
				continue;
			}
		}
	}
	
	// Name: copyFile
	// Funktion: Führt die eigentliche Kopieraktion aus.
	// Argumente: Stream für in und Stream für out
	private void copyFile(InputStream in, OutputStream out) throws IOException {
	    byte[] buffer = new byte[1024];
	    int read;
	    while((read = in.read(buffer)) != -1){
	      out.write(buffer, 0, read);
	    }
	}
	
	// Name: send_Click
	// Funktion: Übergibt den eingebenen String an Secondo und verarbeitet
	// 			dann das Ergebnis
	// Argumente: View
	
	
	public void call_settings(View v) {
		// Settings Dialog aufrufen. Ist momentan noch nicht belegt
		
        Intent myIntent = new Intent(v.getContext(), SettingsActivity.class);
        startActivityForResult(myIntent, 0);
	}
	
	// Name: opengui
	// Öffnet die GUI-Variante
	// Argumente: View 
	public void opengui(View v) {
		// GUI aufrufen 
        Intent myIntent = new Intent(v.getContext(), GuiActivity.class);
        startActivityForResult(myIntent, 0);
	}

	/***
	 * 	call_preferences
	 * 
	 * @param v
	 */
	public void call_preferences(View v) {
		// Settings Dialog aufrufen. Ist momentan noch nicht belegt
		
        Intent myIntent = new Intent(v.getContext(), PreferencesActivity.class);
        startActivityForResult(myIntent, 0);
	}


	@Override
	public void onBackPressed() {
		// TODO Auto-generated method stub
		//super.onBackPressed();
		return;
	}

	// Name: onDestroy
	// Funktion: Schliesst die Datenbank beim verlassen des Programms
	// Argumente: keine
	protected void onDestroy() {
		super.onDestroy();
		System.out.println("closedb");
		sh.shutdown();
		sh=null;
	}
	
	
}

