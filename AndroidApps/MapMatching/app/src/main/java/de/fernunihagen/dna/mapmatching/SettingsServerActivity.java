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

public class SettingsServerActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

    private Preference serverIpPref;
    private Preference serverPortPref;
    private Preference serverUsernamePref;
    private Preference serverPasswordPref;
    private Preference serverDatabaseNamePref;
    private Preference serverRTreeNamePref;
    private Preference serverEdgeIndexNamePref;

    private Preference serverResultPref;
    private Preference serverResultIpPref;
    private Preference serverResultPortPref;
    private Preference serverResultUsernamePref;
    private Preference serverResultPasswordPref;
    private Preference serverResultDatabaseNamePref;

    private Preference serverTimeoutPref;

    private SharedPreferences sharedPrefs;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout root = (LinearLayout) findViewById(android.R.id.list).getParent().getParent().getParent();
        Toolbar bar = (Toolbar) LayoutInflater.from(this).inflate(R.layout.toolbar_settings_server, root, false);
        root.addView(bar, 0); // insert at top
        bar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });

        addPreferencesFromResource(R.xml.preferences_server);

        sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);

        serverIpPref = findPreference(getString(R.string.preference_serverIp_key));
        serverPortPref = findPreference(getString(R.string.preference_serverPort_key));
        serverUsernamePref = findPreference(getString(R.string.preference_serverUsername_key));
        serverPasswordPref = findPreference(getString(R.string.preference_serverPassword_key));
        serverDatabaseNamePref = findPreference(getString(R.string.preference_serverDatabaseName_key));
        serverRTreeNamePref = findPreference(getString(R.string.preference_serverDatabaseRTreeName_key));
        serverEdgeIndexNamePref = findPreference(getString(R.string.preference_serverDatabaseEdgeIndexName_key));

        serverResultPref = findPreference(getString(R.string.preference_serverResultServer_key));
        serverResultIpPref = findPreference(getString(R.string.preference_serverResultIp_key));
        serverResultPortPref = findPreference(getString(R.string.preference_serverResultPort_key));
        serverResultUsernamePref = findPreference(getString(R.string.preference_serverResultUsername_key));
        serverResultPasswordPref = findPreference(getString(R.string.preference_serverResultPassword_key));
        serverResultDatabaseNamePref = findPreference(getString(R.string.preference_serverResultDatabaseName_key));

        serverTimeoutPref = findPreference(getString(R.string.preference_serverTimeout_key));

        serverIpPref.setOnPreferenceChangeListener(this);
        serverPortPref.setOnPreferenceChangeListener(this);
        serverUsernamePref.setOnPreferenceChangeListener(this);
        serverPasswordPref.setOnPreferenceChangeListener(this);
        serverDatabaseNamePref.setOnPreferenceChangeListener(this);
        serverRTreeNamePref.setOnPreferenceChangeListener(this);
        serverEdgeIndexNamePref.setOnPreferenceChangeListener(this);
        serverResultPref.setOnPreferenceChangeListener(this);
        serverResultIpPref.setOnPreferenceChangeListener(this);
        serverResultPortPref.setOnPreferenceChangeListener(this);
        serverResultUsernamePref.setOnPreferenceChangeListener(this);
        serverResultPasswordPref.setOnPreferenceChangeListener(this);
        serverResultDatabaseNamePref.setOnPreferenceChangeListener(this);
        serverTimeoutPref.setOnPreferenceChangeListener(this);

        String savedServerIpPref = sharedPrefs.getString(serverIpPref.getKey(), "");
        String savedServerPortPref = sharedPrefs.getString(serverPortPref.getKey(), "");
        String savedServerUsernamePref = sharedPrefs.getString(serverUsernamePref.getKey(), "");
        String savedServerPasswordPref = sharedPrefs.getString(serverPasswordPref.getKey(), "");
        savedServerPasswordPref = savedServerPasswordPref.replaceAll(".", "*");
        String savedServerDatabaseNamePref = sharedPrefs.getString(serverDatabaseNamePref.getKey(), "");
        String savedServerDatabaseRTreeNamePref = sharedPrefs.getString(serverRTreeNamePref.getKey(), "");
        String savedServerDatabaseEdgeIndexNamePref = sharedPrefs.getString(serverEdgeIndexNamePref.getKey(), "");

        boolean savedServerResultPref = sharedPrefs.getBoolean(getString(R.string.preference_serverResultServer_key), Boolean.valueOf(getString(R.string.preference_serverResultServer_default)));
        String savedServerResultIpPref = sharedPrefs.getString(serverResultIpPref.getKey(), "");
        String savedServerResultPortPref = sharedPrefs.getString(serverResultPortPref.getKey(), "");
        String savedServerResultUsernamePref = sharedPrefs.getString(serverResultUsernamePref.getKey(), "");
        String savedServerResultPasswordPref = sharedPrefs.getString(serverResultPasswordPref.getKey(), "");
        savedServerResultPasswordPref = savedServerResultPasswordPref.replaceAll(".", "*");
        String savedServerResultDatabaseNamePref = sharedPrefs.getString(serverResultDatabaseNamePref.getKey(), "");

        String savedServerTimeoutPref = sharedPrefs.getString(serverTimeoutPref.getKey(), "");

        onPreferenceChange(serverIpPref, savedServerIpPref);
        onPreferenceChange(serverPortPref, savedServerPortPref);
        onPreferenceChange(serverUsernamePref, savedServerUsernamePref);
        onPreferenceChange(serverPasswordPref, savedServerPasswordPref);
        onPreferenceChange(serverDatabaseNamePref, savedServerDatabaseNamePref);
        onPreferenceChange(serverRTreeNamePref, savedServerDatabaseRTreeNamePref);
        onPreferenceChange(serverEdgeIndexNamePref, savedServerDatabaseEdgeIndexNamePref);

        onPreferenceChange(serverResultIpPref, savedServerResultIpPref);
        onPreferenceChange(serverResultPortPref, savedServerResultPortPref);
        onPreferenceChange(serverResultUsernamePref, savedServerResultUsernamePref);
        onPreferenceChange(serverResultPasswordPref, savedServerResultPasswordPref);
        onPreferenceChange(serverResultDatabaseNamePref, savedServerResultDatabaseNamePref);

        onPreferenceChange(serverTimeoutPref, savedServerTimeoutPref);

        if(savedServerResultPref){
            serverResultIpPref.setEnabled(true);
            serverResultPortPref.setEnabled(true);
            serverResultUsernamePref.setEnabled(true);
            serverResultPasswordPref.setEnabled(true);
            serverResultDatabaseNamePref.setEnabled(true);
        }
        else{
            serverResultIpPref.setEnabled(false);
            serverResultPortPref.setEnabled(false);
            serverResultUsernamePref.setEnabled(false);
            serverResultPasswordPref.setEnabled(false);
            serverResultDatabaseNamePref.setEnabled(false);
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object value) {

        if(preference == serverIpPref){
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(containsWhiteSpace(value.toString())){
                Toast.makeText(this, "Invalid Server Address: Server address is not allowed to contain space characters.", Toast.LENGTH_LONG).show();
                return false;
            }
            preference.setSummary(value.toString());
        }
        else if(preference == serverPortPref || preference == serverResultPortPref){
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int newPort = Integer.parseInt(value.toString());
            if(newPort<1||newPort>65535){
                Toast.makeText(this, "Invalid Port: Port must be between 1 and 65535.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(newPort<1024){
                Toast.makeText(this, "Warning: Ports under 1024 are reserved for system services.", Toast.LENGTH_LONG).show();
            }
            preference.setSummary(value.toString());
        }
        else if (preference == serverUsernamePref || preference == serverResultUsernamePref) {
            if(containsWhiteSpace(value.toString())){
                Toast.makeText(this, "Invalid Username: Username is not allowed to contain space characters.", Toast.LENGTH_LONG).show();
                return false;
            }
            preference.setSummary(value.toString());
        }
        else if (preference == serverPasswordPref || preference == serverResultPasswordPref) {
            if(containsWhiteSpace(value.toString())){
                Toast.makeText(this, "Invalid Password: Password is not allowed to contain space characters.", Toast.LENGTH_LONG).show();
                return false;
            }
            String savedServerPasswordPref = value.toString().replaceAll(".", "*");
            preference.setSummary(savedServerPasswordPref);
        }
        else if (preference == serverDatabaseNamePref||preference == serverResultDatabaseNamePref) {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(containsWhiteSpace(value.toString())){
                Toast.makeText(this, "Invalid Database Name: Database Name is not allowed to contain space characters.", Toast.LENGTH_LONG).show();
                return false;
            }
            preference.setSummary(value.toString());
        }
        else if (preference == serverRTreeNamePref) {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(containsWhiteSpace(value.toString())){
                Toast.makeText(this, "Invalid R-Tree Name: R-Tree Name is not allowed to contain space characters.", Toast.LENGTH_LONG).show();
                return false;
            }
            preference.setSummary(value.toString());
        }
        else if (preference == serverEdgeIndexNamePref) {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(containsWhiteSpace(value.toString())){
                Toast.makeText(this, "Invalid Edge-Index Name: Edge-Index Name is not allowed to contain space characters.", Toast.LENGTH_LONG).show();
                return false;
            }
            preference.setSummary(value.toString());
        }
        else if(preference == serverTimeoutPref){
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int timeout = Integer.parseInt(value.toString());
            if(timeout<10){
                Toast.makeText(this, "Invalid Timeout: Timeout must be greater or equal than 10 seconds.", Toast.LENGTH_LONG).show();
                return false;
            }
            preference.setSummary(value.toString());
        }
        else if (preference == serverResultPref) {
            if(Boolean.valueOf(value.toString())){
                serverResultIpPref.setEnabled(true);
                serverResultPortPref.setEnabled(true);
                serverResultUsernamePref.setEnabled(true);
                serverResultPasswordPref.setEnabled(true);
                serverResultDatabaseNamePref.setEnabled(true);
            }
            else{
                serverResultIpPref.setEnabled(false);
                serverResultPortPref.setEnabled(false);
                serverResultUsernamePref.setEnabled(false);
                serverResultPasswordPref.setEnabled(false);
                serverResultDatabaseNamePref.setEnabled(false);
            }
        }
        else if (preference == serverResultIpPref) {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(containsWhiteSpace(value.toString())){
                Toast.makeText(this, "Invalid Result Server Address: Result Server address is not allowed to contain space characters.", Toast.LENGTH_LONG).show();
                return false;
            }
            preference.setSummary(value.toString());
        }
        return true;
    }

    private boolean containsWhiteSpace(String string){
        for(int i = 0; i < string.length(); i++){
            if(Character.isWhitespace(string.charAt(i))){
                return true;
            }
        }
        return false;
    }
}
