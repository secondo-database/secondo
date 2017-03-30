package de.fernunihagen.dna.mapmatching;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.support.v7.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;

public class SettingsGeneralActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

    private Preference showMapPref;
    private Preference showGpsPosPref;
    private Preference localModePref;
    private Preference directTransmissionPref;
    private Preference streetnamePref;
    private Preference cardinalDirectionPref;
    private Preference speedPref;
    private Preference movementDataPref;
    private Preference heightPref;
    private SharedPreferences sharedPrefs;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout root = (LinearLayout) findViewById(android.R.id.list).getParent().getParent().getParent();
        Toolbar bar = (Toolbar) LayoutInflater.from(this).inflate(R.layout.toolbar_settings_general, root, false);
        root.addView(bar, 0); // insert at top
        bar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
        addPreferencesFromResource(R.xml.preferences_general);

        showMapPref = findPreference(getString(R.string.preference_mapMatchingShowMap_key));
        showGpsPosPref = findPreference(getString(R.string.preference_mapMatchingShowGpsPoints_key));
        localModePref = findPreference(getString(R.string.preference_resultLocalMode_key));
        directTransmissionPref = findPreference(getString(R.string.preference_resultDirectTransmission_key));
        streetnamePref = findPreference(getString(R.string.preference_resultStreetname_key));
        cardinalDirectionPref = findPreference(getString(R.string.preference_resultCardinalDirection_key));
        speedPref = findPreference(getString(R.string.preference_resultSpeed_key));
        heightPref = findPreference(getString(R.string.preference_resultHeight_key));
        movementDataPref = findPreference(getString(R.string.preference_resultMovementData_key));

        showMapPref.setOnPreferenceChangeListener(this);
        showGpsPosPref.setOnPreferenceChangeListener(this);
        localModePref.setOnPreferenceChangeListener(this);
        streetnamePref.setOnPreferenceChangeListener(this);

        sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        boolean savedShowMapPref = sharedPrefs.getBoolean(showMapPref.getKey(), true);
        boolean savedLocalModePref = sharedPrefs.getBoolean(localModePref.getKey(), true);

        if(savedShowMapPref){
            showGpsPosPref.setEnabled(true);
        }
        else{
            showGpsPosPref.setEnabled(false);
        }
        if(savedLocalModePref){
            directTransmissionPref.setEnabled(false);
            streetnamePref.setEnabled(false);
            cardinalDirectionPref.setEnabled(false);
            speedPref.setEnabled(false);
            heightPref.setEnabled(false);
            movementDataPref.setEnabled(false);
        }
        else{
            directTransmissionPref.setEnabled(true);
            streetnamePref.setEnabled(true);
            cardinalDirectionPref.setEnabled(true);
            speedPref.setEnabled(true);
            heightPref.setEnabled(true);
            movementDataPref.setEnabled(true);
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object value) {
        if(preference == showMapPref){
            if(Boolean.valueOf(value.toString())){
                showGpsPosPref.setEnabled(true);
                MapMatchingMap.setShowMap(true);
            }
            else{
                showGpsPosPref.setEnabled(false);
                MapMatchingMap.setShowMap(false);
            }
        }
        else if (preference == showGpsPosPref)
        {
            if(Boolean.valueOf(value.toString())){
                MapMatchingMap.setShowGpsPoints(true);
            }
            else{
                MapMatchingMap.setShowGpsPoints(false);
            }
        }
        else if(preference == localModePref)
        {
            if(Boolean.valueOf(value.toString())){
                directTransmissionPref.setEnabled(false);
                streetnamePref.setEnabled(false);
                cardinalDirectionPref.setEnabled(false);
                speedPref.setEnabled(false);
                heightPref.setEnabled(false);
                movementDataPref.setEnabled(false);
            }
            else{
                directTransmissionPref.setEnabled(true);
                streetnamePref.setEnabled(true);
                cardinalDirectionPref.setEnabled(true);
                speedPref.setEnabled(true);
                heightPref.setEnabled(true);
                movementDataPref.setEnabled(true);
            }
        }
        return true;
    }
}