package eu.ehnes.secondoandroid.activity;

import java.text.SimpleDateFormat;

import eu.ehnes.secondoandroid.R;
import eu.ehnes.secondoandroid.R.id;
import eu.ehnes.secondoandroid.R.layout;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

/**
 * This class handles the Splash screen. when touched, it shows a message and start SecondoActivity.java
 * 
 * @author juergen
 * @version 1.0
 */
public class SplashScreenActivity extends Activity {
	private TextView messageLine;
	private Context context=null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_splashscreen);
        
        
		
	}
	
	/**
	 * This method would be called, when the users touches the screen at Splash
	 *
	 * @param v the View
	 */
	@SuppressLint("SimpleDateFormat")
	public void startSecondo(View v) {
        Intent myIntent = new Intent(v.getContext(), SecondoActivity.class);
        startActivityForResult(myIntent, 0);

        String versionName="";
        long builddate=0;
        context = getApplicationContext();
        SimpleDateFormat sdf = new SimpleDateFormat("y.M.d");
        
        try {
			versionName = context.getPackageManager()
				    .getPackageInfo(context.getPackageName(), 0).versionName;
			builddate = context.getPackageManager().getPackageInfo(context.getPackageName(), 0).lastUpdateTime;
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
        messageLine=(TextView) findViewById(R.id.textView1);
        messageLine.setText("Secondo \""+versionName + " " + sdf.format(Long.valueOf(builddate))+ "\" is starting, please be patient...");
		ProgressDialog.show(this, "Secondo" + versionName, "Secondo is starting, please be patient ...");
	}
	
}
