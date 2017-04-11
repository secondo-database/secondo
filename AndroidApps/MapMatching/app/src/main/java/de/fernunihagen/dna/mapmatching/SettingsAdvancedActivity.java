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

public class SettingsAdvancedActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

    private Preference calcStreetnameEntryTime;
    private Preference preciseMPoint;
    private Preference uTurnAllowPref;
    private Preference uTurnMalusPref;
    private Preference uTurnMultiplePref;
    private Preference networkBoundingBoxPref;
    private Preference networkBoundingBoxRadiusPref;
    private Preference hypothesisCountPref;
    private Preference hypothesisExtensionDepthPref;
    private Preference hypothesisPreferSmallerPathsPref;
    private Preference hypothesisPreferSmallerPathsMalusPref;
    private Preference hypothesisResetDistancePref;
    private Preference hypothesisCleanPref;
    private Preference hypothesisCleanCountPref;
    private Preference hypothesisCleanEdgesPref;
    private Preference hypothesisCleanMapPref;
    private Preference hypothesisCleanMapCountPref;
    private Preference matchingFootwaysPref;
    private Preference matchingFootwaysPerformancePref;
    private Preference matchingFootwaysFootwaySpeedPref;
    private Preference matchingFootwaysWayOrRoadMalusPref;
    private Preference scoreDistanceMultiPref;
    private Preference alwaysExtend;
    private Preference resetToDefaultPref;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout root = (LinearLayout) findViewById(android.R.id.list).getParent().getParent().getParent();
        Toolbar bar = (Toolbar) LayoutInflater.from(this).inflate(R.layout.toolbar_settings_advanced, root, false);
        root.addView(bar, 0); // insert at top
        bar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
        addPreferencesFromResource(R.xml.preferences_advanced);

        Dialogs.setSettingsAdvancedActivity(this);

        calcStreetnameEntryTime = findPreference(getString(R.string.preference_advancedStreetnameCalc_key));
        preciseMPoint = findPreference(getString(R.string.preference_advancedPreciseMPoint_key));
        uTurnAllowPref = findPreference(getString(R.string.preference_advancedUTurn_key));
        uTurnMalusPref = findPreference(getString(R.string.preference_advancedUTurnMalus_key));
        uTurnMultiplePref = findPreference(getString(R.string.preference_advancedMultipleUTurn_key));
        networkBoundingBoxPref = findPreference(getString(R.string.preference_advancedBoundingBoxSize_key));
        networkBoundingBoxRadiusPref = findPreference(getString(R.string.preference_advancedBoundingBoxRadius_key));
        hypothesisCountPref = findPreference(getString(R.string.preference_advancedMaxPathsSize_key));
        hypothesisExtensionDepthPref = findPreference(getString(R.string.preference_advancedExtensionDepth_key));
        hypothesisPreferSmallerPathsPref = findPreference(getString(R.string.preference_advancedPreferSmallerPaths_key));
        hypothesisPreferSmallerPathsMalusPref = findPreference(getString(R.string.preference_advancedPreferSmallerPathsMalus_key));
        hypothesisResetDistancePref = findPreference(getString(R.string.preference_advancedResetDistance_key));
        hypothesisCleanPref = findPreference(getString(R.string.preference_advancedCleanPaths_key));
        hypothesisCleanCountPref = findPreference(getString(R.string.preference_advancedCleanPathsCount_key));
        hypothesisCleanEdgesPref = findPreference(getString(R.string.preference_advancedCleanPathsEdges_key));
        hypothesisCleanMapPref = findPreference(getString(R.string.preference_advancedCleanMap_key));
        hypothesisCleanMapCountPref = findPreference(getString(R.string.preference_advancedCleanMapCount_key));
        matchingFootwaysPref = findPreference(getString(R.string.preference_advancedFootwaysDifferent_key));
        matchingFootwaysPerformancePref= findPreference(getString(R.string.preference_advancedFootwaysPerformance_key));
        matchingFootwaysFootwaySpeedPref = findPreference(getString(R.string.preference_advancedFootwaySpeed_key));
        matchingFootwaysWayOrRoadMalusPref = findPreference(getString(R.string.preference_advancedWayOrRoadMalus_key));
        resetToDefaultPref = findPreference(getString(R.string.preference_advancedReset_key));
        scoreDistanceMultiPref = findPreference(getString(R.string.preference_advancedScoreDistanceMulti_key));
        alwaysExtend = findPreference(getString(R.string.preference_advancedAlwaysExtend_key));


        calcStreetnameEntryTime.setOnPreferenceChangeListener(this);
        preciseMPoint.setOnPreferenceChangeListener(this);
        uTurnAllowPref.setOnPreferenceChangeListener(this);
        uTurnMalusPref.setOnPreferenceChangeListener(this);
        uTurnMultiplePref.setOnPreferenceChangeListener(this);
        networkBoundingBoxPref.setOnPreferenceChangeListener(this);
        networkBoundingBoxRadiusPref.setOnPreferenceChangeListener(this);
        hypothesisCountPref.setOnPreferenceChangeListener(this);
        hypothesisExtensionDepthPref.setOnPreferenceChangeListener(this);
        hypothesisPreferSmallerPathsPref.setOnPreferenceChangeListener(this);
        hypothesisPreferSmallerPathsMalusPref.setOnPreferenceChangeListener(this);
        hypothesisResetDistancePref.setOnPreferenceChangeListener(this);
        hypothesisCleanPref.setOnPreferenceChangeListener(this);
        hypothesisCleanCountPref.setOnPreferenceChangeListener(this);
        hypothesisCleanEdgesPref.setOnPreferenceChangeListener(this);
        hypothesisCleanMapPref.setOnPreferenceChangeListener(this);
        hypothesisCleanMapCountPref.setOnPreferenceChangeListener(this);
        matchingFootwaysPref.setOnPreferenceChangeListener(this);
        matchingFootwaysPerformancePref.setOnPreferenceChangeListener(this);
        matchingFootwaysFootwaySpeedPref.setOnPreferenceChangeListener(this);
        matchingFootwaysWayOrRoadMalusPref.setOnPreferenceChangeListener(this);
        scoreDistanceMultiPref.setOnPreferenceChangeListener(this);
        alwaysExtend.setOnPreferenceChangeListener(this);

        resetToDefaultPref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Dialogs.showDialogResetAdvancedSettings();
                return true;
            }
        });

        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        boolean savedUTurnAllowPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedUTurn_key), Boolean.valueOf(getString(R.string.preference_advancedUTurn_default)));
        boolean savedHypothesisCleanPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedCleanPaths_key), Boolean.valueOf(getString(R.string.preference_advancedCleanPaths_default)));
        boolean savedMapCleanPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedCleanMap_key), Boolean.valueOf(getString(R.string.preference_advancedCleanMap_default)));
        boolean savedFootwaysPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedFootwaysDifferent_key), Boolean.valueOf(getString(R.string.preference_advancedFootwaysDifferent_default)));
        boolean savedPreferSmallerPathsPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedPreferSmallerPaths_key), Boolean.valueOf(getString(R.string.preference_advancedPreferSmallerPaths_default)));

        if(savedPreferSmallerPathsPref){
            hypothesisPreferSmallerPathsMalusPref.setEnabled(true);
        }else{
            hypothesisPreferSmallerPathsMalusPref.setEnabled(false);
        }

        if(savedUTurnAllowPref){
            uTurnMalusPref.setEnabled(true);
            uTurnMultiplePref.setEnabled(true);
        }
        else{
            uTurnMalusPref.setEnabled(false);
            uTurnMultiplePref.setEnabled(false);
        }
        if(savedHypothesisCleanPref)
        {
            hypothesisCleanCountPref.setEnabled(true);
            hypothesisCleanEdgesPref.setEnabled(true);
        }
        else
        {
            hypothesisCleanCountPref.setEnabled(false);
            hypothesisCleanEdgesPref.setEnabled(false);
        }
        if(savedMapCleanPref){
            hypothesisCleanMapCountPref.setEnabled(true);
        }else{
            hypothesisCleanMapCountPref.setEnabled(false);
        }
        if(savedFootwaysPref){
            matchingFootwaysFootwaySpeedPref.setEnabled(true);
            matchingFootwaysWayOrRoadMalusPref.setEnabled(true);
        }
        else{
            matchingFootwaysFootwaySpeedPref.setEnabled(false);
            matchingFootwaysWayOrRoadMalusPref.setEnabled(false);
        }

        if (savedInstanceState != null) {
            if(savedInstanceState.getBoolean("dialogResetAdvancedSettingsActive")){
                Dialogs.showDialogResetAdvancedSettings();
            }
        }
    }

    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        if(Dialogs.isDialogResetAdvancedSettingsActive()){
            savedInstanceState.putBoolean("dialogResetAdvancedSettingsActive", true);
        }
        else{
            savedInstanceState.putBoolean("dialogResetAdvancedSettingsActive", false);
        }
    }
    @Override
    protected void onDestroy() {
        super.onDestroy();
        Dialogs.closeDialogResetAdvancedSettings();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object value) {
        if (preference == uTurnAllowPref)
        {
            if(Boolean.valueOf(value.toString())){
                uTurnMalusPref.setEnabled(true);
                uTurnMultiplePref.setEnabled(true);
            }
            else{
                uTurnMalusPref.setEnabled(false);
                uTurnMultiplePref.setEnabled(false);
            }
        }
        else if (preference == uTurnMalusPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == networkBoundingBoxPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int boundingBoxSize = Integer.parseInt(value.toString());
            if(boundingBoxSize<100){
                Toast.makeText(this, "Invalid Bounding Box Size: Bounding Box Size must be greater or equal than 100.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == networkBoundingBoxRadiusPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int boundingBoxRadius = Integer.parseInt(value.toString());
            if(boundingBoxRadius<20 || boundingBoxRadius>100){
                Toast.makeText(this, "Invalid Bounding Box Update Radius: Bounding Box Update Radius must be greater or equal than 20 and smaller or equal than 100.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == hypothesisCountPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int hypothesisCount = Integer.parseInt(value.toString());
            if(hypothesisCount<1){
                Toast.makeText(this, "Invalid Hypothesis Count: Hypothesis Count must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == hypothesisExtensionDepthPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int extensionDepth = Integer.parseInt(value.toString());
            if(extensionDepth<2){
                Toast.makeText(this, "Invalid Extension Depth: Extension Depth must be greater or equal than 2.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == hypothesisResetDistancePref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int resetDistance = Integer.parseInt(value.toString());
            if(resetDistance<50){
                Toast.makeText(this, "Invalid Reset Distance: Reset Distance must be greater or equal than 50.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if(preference == hypothesisPreferSmallerPathsPref)
        {
            if(Boolean.valueOf(value.toString())){
                hypothesisPreferSmallerPathsMalusPref.setEnabled(true);
            }
            else{
                hypothesisPreferSmallerPathsMalusPref.setEnabled(false);
            }
        }
        else if(preference == hypothesisPreferSmallerPathsMalusPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == hypothesisCleanPref)
        {
            if(Boolean.valueOf(value.toString())){
                hypothesisCleanCountPref.setEnabled(true);
                hypothesisCleanEdgesPref.setEnabled(true);
            }
            else{
                hypothesisCleanCountPref.setEnabled(false);
                hypothesisCleanEdgesPref.setEnabled(false);
            }
        }
        else if (preference == hypothesisCleanCountPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int hypothesisCleanCount = Integer.parseInt(value.toString());
            if(hypothesisCleanCount<20){
                Toast.makeText(this, "Invalid Hypothesis Clean Count: Hypothesis Clean Count must be greater or equal than 20.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == hypothesisCleanEdgesPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int cleanEdges = Integer.parseInt(value.toString());
            if(cleanEdges<10 || cleanEdges>50){
                Toast.makeText(this, "Invalid Clean Hypotheses Way-Segments: Count of Way-Segments left in cleaned Hypotheses must be greater or equal than 10 and smaller or equal than 50.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == hypothesisCleanMapPref)
        {
            if(Boolean.valueOf(value.toString())){
                hypothesisCleanMapCountPref.setEnabled(true);
            }
            else{
                hypothesisCleanMapCountPref.setEnabled(false);
            }
        }
        else if (preference == hypothesisCleanMapCountPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int hypothesisCleanCount = Integer.parseInt(value.toString());
            if(hypothesisCleanCount<100){
                Toast.makeText(this, "Invalid Map Clean Count: Map Clean Count must be greater or equal than 100.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == matchingFootwaysPref)
        {
            if(Boolean.valueOf(value.toString())){
                matchingFootwaysFootwaySpeedPref.setEnabled(true);
                matchingFootwaysWayOrRoadMalusPref.setEnabled(true);
            }
            else{
                matchingFootwaysFootwaySpeedPref.setEnabled(false);
                matchingFootwaysWayOrRoadMalusPref.setEnabled(false);
            }
        }
        else if (preference == matchingFootwaysFootwaySpeedPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == matchingFootwaysWayOrRoadMalusPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == scoreDistanceMultiPref)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int distanceMulti = Integer.parseInt(value.toString());
            if(distanceMulti<1){
                Toast.makeText(this, "Invalid Distance Multiplicator: Distance Multiplicator must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        return true;
    }

    public void resetAdvancedSettings(){
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean(getString(R.string.preference_advancedStreetnameCalc_key),Boolean.valueOf(getString(R.string.preference_advancedStreetnameCalc_default)));
        editor.putBoolean(getString(R.string.preference_advancedPreciseMPoint_key),Boolean.valueOf(getString(R.string.preference_advancedPreciseMPoint_default)));
        editor.putBoolean(getString(R.string.preference_advancedUTurn_key),Boolean.valueOf(getString(R.string.preference_advancedUTurn_default)));
        editor.putString(getString(R.string.preference_advancedUTurnMalus_key),getString(R.string.preference_advancedUTurnMalus_default));
        editor.putBoolean(getString(R.string.preference_advancedMultipleUTurn_key),Boolean.valueOf(getString(R.string.preference_advancedMultipleUTurn_default)));
        editor.putString(getString(R.string.preference_advancedBoundingBoxSize_key),getString(R.string.preference_advancedBoundingBoxSize_default));
        editor.putString(getString(R.string.preference_advancedBoundingBoxRadius_key),getString(R.string.preference_advancedBoundingBoxRadius_default));
        editor.putString(getString(R.string.preference_advancedMaxPathsSize_key),getString(R.string.preference_advancedMaxPathsSize_default));
        editor.putString(getString(R.string.preference_advancedExtensionDepth_key),getString(R.string.preference_advancedExtensionDepth_default));
        editor.putString(getString(R.string.preference_advancedResetDistance_key),getString(R.string.preference_advancedResetDistance_default));
        editor.putBoolean(getString(R.string.preference_advancedPreferSmallerPaths_key),Boolean.valueOf(getString(R.string.preference_advancedPreferSmallerPaths_default)));
        editor.putString(getString(R.string.preference_advancedPreferSmallerPathsMalus_key),getString(R.string.preference_advancedPreferSmallerPathsMalus_default));
        editor.putBoolean(getString(R.string.preference_advancedCleanPaths_key),Boolean.valueOf(getString(R.string.preference_advancedCleanPaths_default)));
        editor.putString(getString(R.string.preference_advancedCleanPathsCount_key),getString(R.string.preference_advancedCleanPathsCount_default));
        editor.putString(getString(R.string.preference_advancedCleanPathsEdges_key),getString(R.string.preference_advancedCleanPathsEdges_default));
        editor.putBoolean(getString(R.string.preference_advancedCleanMap_key),Boolean.valueOf(getString(R.string.preference_advancedCleanMap_default)));
        editor.putString(getString(R.string.preference_advancedCleanMapCount_key),getString(R.string.preference_advancedCleanMapCount_default));
        editor.putBoolean(getString(R.string.preference_advancedFootwaysDifferent_key),Boolean.valueOf(getString(R.string.preference_advancedFootwaysDifferent_default)));
        editor.putBoolean(getString(R.string.preference_advancedFootwaysPerformance_key),Boolean.valueOf(getString(R.string.preference_advancedFootwaysPerformance_default)));
        editor.putString(getString(R.string.preference_advancedFootwaySpeed_key),getString(R.string.preference_advancedFootwaySpeed_default));
        editor.putString(getString(R.string.preference_advancedWayOrRoadMalus_key),getString(R.string.preference_advancedWayOrRoadMalus_default));
        editor.putString(getString(R.string.preference_advancedScoreDistanceMulti_key),getString(R.string.preference_advancedScoreDistanceMulti_default));
        editor.putBoolean(getString(R.string.preference_advancedAlwaysExtend_key),Boolean.valueOf(getString(R.string.preference_advancedAlwaysExtend_default)));
        editor.commit();
        finish();
        startActivity(getIntent());
        Toast.makeText(this, "All Advanced Settings has been reset.", Toast.LENGTH_LONG).show();
    }
}
