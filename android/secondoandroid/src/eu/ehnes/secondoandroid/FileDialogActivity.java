// FileDialogActivity

package eu.ehnes.secondoandroid;

import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.view.View;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import eu.ehnes.secondoandroid.R;


/**
 * The Class FileDialogActivity opens a windows with the files in the given path
 * and shows them in a List View
 *  
 */
public class FileDialogActivity extends ListActivity {
	private List<String> item=null;
	private List<String> path=null;

	
	private TextView myPath;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		String root;
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_filedialog);
		
		myPath=(TextView) findViewById(R.id.path);

		// Diese Funktion ermittelt den eigenen Pfad des Programms
		root=OwnPath.getInstance().getPath(); // Default Value

		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
		boolean externalSourceStateBoolean = preferences.getBoolean("externalSourceState", false);
		if (externalSourceStateBoolean) {
			root = preferences.getString("externalSourcePath", root );
		}
		
		getDir(root);
		
	}
	
	// Name: getDir
	// Funktion: 
	// Parameter: dirPath - Enthält das Verzeichnis, das gelesen werden soll.
	
	/**
	 * Ermittelt die Dateien im mitgegebenen Verzeichnis und schreibt 
	 * 			sie in eine Liste item
	 * Finds the Files in the given Directory and writes them to a listitem  
	 *
	 * @param dirPath the given Directory Path path
	 * @return no Return Value
	 */
	private void getDir(String dirPath) {
		
		myPath.setText("Location: "+dirPath);
		
		item = new ArrayList<String>();
		path = new ArrayList<String>();
		File f=new File(dirPath);
		File[] files=f.listFiles();
		
		for(File file : files)
	     {
			// Es wird geprüft ob das File weder versteckt, noch "nicht lesbar" ist.
			if(!file.isHidden() && file.canRead()){
				path.add(file.getPath());

				// Wenn es sich bei dem Element um ein Directory handelt muss ein Slash an den Schluss
				if(file.isDirectory()){
					item.add(file.getName() + "/");
				} else {
					item.add(file.getName());
				}
			} 
	     }

		// Die Liste wird als Listview dargestellt. 
	     ArrayAdapter<String> fileList = new ArrayAdapter<String>(this, R.layout.row, item);
	     setListAdapter(fileList); 
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {

		File file = new File(path.get(position));

		if (file.isDirectory())
		{
			if(file.canRead()){
				getDir(path.get(position));
			}else{
				new AlertDialog.Builder(this)
				.setIcon(R.drawable.ic_launcher)
				.setTitle("[" + file.getName() + "] folder can't be read!")
				.setPositiveButton("OK", null).show(); 
			} 
		}else {

			Intent resultPath = new Intent();
			Bundle rucksack = new Bundle();

			// Argumente für die Rückgabe vorbereiten
			rucksack.putString("resultFile", file.getName());
			rucksack.putString("resultPath", file.getPath());
			resultPath.putExtras(rucksack);
			setResult(RESULT_OK,resultPath);

			// Warndialog vorbereiten und ausgeben
			AlertDialog ad=new AlertDialog.Builder(this).create();
			ad.setCancelable(false);
			ad.setMessage("This process could take very long\nPlease don't shut down the device");
			ad.setButton("Ok", new DialogInterface.OnClickListener() {

				// Die Meldung dient nur zur Information, deshalb
				// wird ein Drücken des OK-Buttons ignoriert 
				@Override
				public void onClick(DialogInterface dialog, int which) {
					dialog.dismiss();

					finish();


				}
			});
			ad.show();
		}
	}

}
