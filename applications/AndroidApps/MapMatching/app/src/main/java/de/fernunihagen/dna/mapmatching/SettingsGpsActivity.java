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
import android.widget.Toast;

public class SettingsGpsActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener{

    private Preference gpsFirstLocationPref;
    private Preference gpsUpdateRatePref;
    private Preference gpsUpdateDistancePref;
    private Preference gpsUpdateAccuracy;
    private Preference gpsSatUsed;
    private Preference gpsMaxPdop;
    private Preference gpsMaxHdop;
    private Preference gpsMaxVdop;
    private Preference resetToDefaultPref;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout root = (LinearLayout) findViewById(android.R.id.list).getParent().getParent().getParent();
        Toolbar bar = (Toolbar) LayoutInflater.from(this).inflate(R.layout.toolbar_settings_gps, root, false);
        root.addView(bar, 0); // insert at top
        bar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
        addPreferencesFromResource(R.xml.preferences_gps);

        Dialogs.setSettingsGpsActivity(this);

        gpsFirstLocationPref= findPreference(getString(R.string.preference_gpsFirstLocation_key));
        gpsUpdateRatePref = findPreference(getString(R.string.preference_gpsUpdateRate_key));
        gpsUpdateDistancePref = findPreference(getString(R.string.preference_gpsUpdateDistance_key));
        gpsUpdateAccuracy = findPreference(getString(R.string.preference_gpsAccuracy_key));
        gpsSatUsed = findPreference(getString(R.string.preference_gpsSatUsed_key));
        gpsMaxPdop = findPreference(getString(R.string.preference_gpsMaxPdop_key));
        gpsMaxHdop = findPreference(getString(R.string.preference_gpsMaxHdop_key));
        gpsMaxVdop = findPreference(getString(R.string.preference_gpsMaxVdop_key));
        resetToDefaultPref = findPreference(getString(R.string.preference_gpsReset_key));

        gpsFirstLocationPref.setOnPreferenceChangeListener(this);
        gpsUpdateRatePref.setOnPreferenceChangeListener(this);
        gpsUpdateDistancePref.setOnPreferenceChangeListener(this);
        gpsUpdateAccuracy.setOnPreferenceChangeListener(this);
        gpsSatUsed.setOnPreferenceChangeListener(this);
        gpsMaxPdop.setOnPreferenceChangeListener(this);
        gpsMaxHdop.setOnPreferenceChangeListener(this);
        gpsMaxVdop.setOnPreferenceChangeListener(this);
        resetToDefaultPref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Dialogs.showDialogResetGpsSettings();
                return true;
            }
        });

        if (savedInstanceState != null) {
            if(savedInstanceState.getBoolean("dialogResetGpsSettingsActive")){
                Dialogs.showDialogResetGpsSettings();
            }
        }
    }

    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        if(Dialogs.isDialogResetGpsSettingsActive()){
            savedInstanceState.putBoolean("dialogResetGpsSettingsActive", true);
        }
        else{
            savedInstanceState.putBoolean("dialogResetGpsSettingsActive", false);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Dialogs.closeDialogResetGpsSettings();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object value) {

        if(preference == gpsFirstLocationPref){

        }
        else if(preference == gpsUpdateRatePref){
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int updateRate = Integer.parseInt(value.toString());
            if(updateRate>10000){
                Toast.makeText(this, "Invalid Update Rate: Update Rate must be smaller or equal than 10000.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == gpsUpdateDistancePref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int updateDistance = Integer.parseInt(value.toString());
            if(updateDistance>100){
                Toast.makeText(this, "Invalid Update Distance: Update Distance must be smaller or equal than 100.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == gpsUpdateAccuracy)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int updateAccuracy = Integer.parseInt(value.toString());
            if(updateAccuracy<5){
                Toast.makeText(this, "Invalid Update Accuracy: Update Accuracy must be greater or equal than 5.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == gpsSatUsed)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int satUsed = Integer.parseInt(value.toString());
            if(satUsed<2){
                Toast.makeText(this, "Invalid Count of Satellites: Minimal allowed count of Satellites must be greater or equal than 2.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == gpsMaxPdop)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int maxPdop = Integer.parseInt(value.toString());
            if(maxPdop<2){
                Toast.makeText(this, "Invalid Positional DOP: Maximal allowed Positional DOP must be greater or equal than 2.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == gpsMaxHdop)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int maxHdop = Integer.parseInt(value.toString());
            if(maxHdop<2){
                Toast.makeText(this, "Invalid Horizontal DOP: Maximal allowed Horizontal DOP must be greater or equal than 2.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == gpsMaxVdop)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int maxVdop = Integer.parseInt(value.toString());
            if(maxVdop<2){
                Toast.makeText(this, "Invalid Vertical DOP: Maximal allowed Vertical DOP must be greater or equal than 2.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        return true;
    }

    public void resetGpsSettings(){
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean(getString(R.string.preference_gpsFirstLocation_key),Boolean.valueOf(getString(R.string.preference_gpsFirstLocation_default)));
        editor.putString(getString(R.string.preference_gpsUpdateRate_key),getString(R.string.preference_gpsUpdateRate_default));
        editor.putString(getString(R.string.preference_gpsUpdateDistance_key),getString(R.string.preference_gpsUpdateDistance_default));
        editor.putString(getString(R.string.preference_gpsAccuracy_key),getString(R.string.preference_gpsAccuracy_default));
        editor.putString(getString(R.string.preference_gpsSatUsed_key),getString(R.string.preference_gpsSatUsed_default));
        editor.putString(getString(R.string.preference_gpsMaxHdop_key),getString(R.string.preference_gpsMaxHdop_default));
        editor.putString(getString(R.string.preference_gpsMaxPdop_key),getString(R.string.preference_gpsMaxPdop_default));
        editor.putString(getString(R.string.preference_gpsMaxVdop_key),getString(R.string.preference_gpsMaxVdop_default));
        editor.commit();
        finish();
        startActivity(getIntent());
        Toast.makeText(this, "All GPS Settings has been reset.", Toast.LENGTH_LONG).show();
    }
}
