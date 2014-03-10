package de.fernunihagen.dna.secondo4android.activity;

import de.fernunihagen.dna.secondo4android.R;
import android.app.Activity;
import android.os.Bundle;

/**
 * The Class SplashActivity opens a window with the configured picture.
 * 
 * @author juergen
 * @version 1.0
 */
public class SplashActivity extends Activity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
    }
}
