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
    private Preference compareAngle;
    private Preference compareAngleTolerance;
    private Preference randomizePosition;

    private Preference heightIntervalRange;

    private Preference useSpeedIntervalRange;
    private Preference speedIntervalRange;

    private Preference speedIntervalEndpoint1;
    private Preference speedIntervalEndpoint2;
    private Preference speedIntervalEndpoint3;
    private Preference speedIntervalEndpoint4;
    private Preference speedIntervalEndpoint5;
    private Preference speedIntervalEndpoint6;
    private Preference speedIntervalEndpoint7;
    private Preference speedIntervalEndpoint8;
    private Preference speedIntervalEndpoint9;
    private Preference speedIntervalEndpoint10;

    private Preference resetToDefaultPref;

    SharedPreferences sharedPrefs;

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
        compareAngle = findPreference(getString(R.string.preference_advancedCompareAngle_key));
        compareAngleTolerance = findPreference(getString(R.string.preference_advancedCompareAngleTolerance_key));
        randomizePosition = findPreference(getString(R.string.preference_advancedRandomizePosition_key));
        heightIntervalRange = findPreference(getString(R.string.preference_advancedHeightIntervalRange_key));
        useSpeedIntervalRange = findPreference(getString(R.string.preference_advancedUseSpeedIntervalRange_key));
        speedIntervalRange = findPreference(getString(R.string.preference_advancedSpeedIntervalRange_key));
        speedIntervalEndpoint1 = findPreference(getString(R.string.preference_advancedSpeedIntervalEndpoint1_key));
        speedIntervalEndpoint2 = findPreference(getString(R.string.preference_advancedSpeedIntervalEndpoint2_key));
        speedIntervalEndpoint3 = findPreference(getString(R.string.preference_advancedSpeedIntervalEndpoint3_key));
        speedIntervalEndpoint4 = findPreference(getString(R.string.preference_advancedSpeedIntervalEndpoint4_key));
        speedIntervalEndpoint5 = findPreference(getString(R.string.preference_advancedSpeedIntervalEndpoint5_key));
        speedIntervalEndpoint6 = findPreference(getString(R.string.preference_advancedSpeedIntervalEndpoint6_key));
        speedIntervalEndpoint7 = findPreference(getString(R.string.preference_advancedSpeedIntervalEndpoint7_key));
        speedIntervalEndpoint8 = findPreference(getString(R.string.preference_advancedSpeedIntervalEndpoint8_key));
        speedIntervalEndpoint9 = findPreference(getString(R.string.preference_advancedSpeedIntervalEndpoint9_key));
        speedIntervalEndpoint10 = findPreference(getString(R.string.preference_advancedSpeedIntervalEndpoint10_key));

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
        compareAngle.setOnPreferenceChangeListener(this);
        compareAngleTolerance.setOnPreferenceChangeListener(this);
        randomizePosition.setOnPreferenceChangeListener(this);
        heightIntervalRange.setOnPreferenceChangeListener(this);
        useSpeedIntervalRange.setOnPreferenceChangeListener(this);
        speedIntervalRange.setOnPreferenceChangeListener(this);
        speedIntervalEndpoint1.setOnPreferenceChangeListener(this);
        speedIntervalEndpoint2.setOnPreferenceChangeListener(this);
        speedIntervalEndpoint3.setOnPreferenceChangeListener(this);
        speedIntervalEndpoint4.setOnPreferenceChangeListener(this);
        speedIntervalEndpoint5.setOnPreferenceChangeListener(this);
        speedIntervalEndpoint6.setOnPreferenceChangeListener(this);
        speedIntervalEndpoint7.setOnPreferenceChangeListener(this);
        speedIntervalEndpoint8.setOnPreferenceChangeListener(this);
        speedIntervalEndpoint9.setOnPreferenceChangeListener(this);
        speedIntervalEndpoint10.setOnPreferenceChangeListener(this);

        resetToDefaultPref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Dialogs.showDialogResetAdvancedSettings();
                return true;
            }
        });

        sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        boolean savedUTurnAllowPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedUTurn_key), Boolean.valueOf(getString(R.string.preference_advancedUTurn_default)));
        boolean savedHypothesisCleanPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedCleanPaths_key), Boolean.valueOf(getString(R.string.preference_advancedCleanPaths_default)));
        boolean savedMapCleanPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedCleanMap_key), Boolean.valueOf(getString(R.string.preference_advancedCleanMap_default)));
        boolean savedFootwaysPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedFootwaysDifferent_key), Boolean.valueOf(getString(R.string.preference_advancedFootwaysDifferent_default)));
        boolean savedPreferSmallerPathsPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedPreferSmallerPaths_key), Boolean.valueOf(getString(R.string.preference_advancedPreferSmallerPaths_default)));
        boolean savedAlwaysExtendPref = sharedPrefs.getBoolean(getString(R.string.preference_advancedAlwaysExtend_key), Boolean.valueOf(getString(R.string.preference_advancedAlwaysExtend_default)));
        boolean savedCompareAnglePref = sharedPrefs.getBoolean(getString(R.string.preference_advancedCompareAngle_key), Boolean.valueOf(getString(R.string.preference_advancedCompareAngle_default)));
        boolean savedUseSpeedIntervalRange = sharedPrefs.getBoolean(getString(R.string.preference_advancedUseSpeedIntervalRange_key), Boolean.valueOf(getString(R.string.preference_advancedUseSpeedIntervalRange_default)));

        if(savedUseSpeedIntervalRange){
            speedIntervalRange.setEnabled(true);
            speedIntervalEndpoint1.setEnabled(false);
            speedIntervalEndpoint2.setEnabled(false);
            speedIntervalEndpoint3.setEnabled(false);
            speedIntervalEndpoint4.setEnabled(false);
            speedIntervalEndpoint5.setEnabled(false);
            speedIntervalEndpoint6.setEnabled(false);
            speedIntervalEndpoint7.setEnabled(false);
            speedIntervalEndpoint8.setEnabled(false);
            speedIntervalEndpoint9.setEnabled(false);
            speedIntervalEndpoint10.setEnabled(false);
        }
        else{
            speedIntervalRange.setEnabled(false);
            speedIntervalEndpoint1.setEnabled(true);
            speedIntervalEndpoint2.setEnabled(true);
            speedIntervalEndpoint3.setEnabled(true);
            speedIntervalEndpoint4.setEnabled(true);
            speedIntervalEndpoint5.setEnabled(true);
            speedIntervalEndpoint6.setEnabled(true);
            speedIntervalEndpoint7.setEnabled(true);
            speedIntervalEndpoint8.setEnabled(true);
            speedIntervalEndpoint9.setEnabled(true);
            speedIntervalEndpoint10.setEnabled(true);
        }

        if(savedAlwaysExtendPref){
            compareAngleTolerance.setEnabled(false);
            compareAngle.setEnabled(false);
        }else{
            compareAngleTolerance.setEnabled(true);
            compareAngle.setEnabled(true);
        }

        if(savedCompareAnglePref){
            compareAngleTolerance.setEnabled(true);
        }else{
            compareAngleTolerance.setEnabled(false);
        }

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
        else if(preference == alwaysExtend)
        {
            if(Boolean.valueOf(value.toString())){
                compareAngleTolerance.setEnabled(false);
                compareAngle.setEnabled(false);
            }
            else{
                boolean savedCompareAnglePref = sharedPrefs.getBoolean(getString(R.string.preference_advancedCompareAngle_key), Boolean.valueOf(getString(R.string.preference_advancedCompareAngle_default)));
                compareAngle.setEnabled(true);
                if(savedCompareAnglePref){
                    compareAngleTolerance.setEnabled(true);
                }
                else{
                    compareAngleTolerance.setEnabled(false);
                }
            }
        }
        else if(preference == compareAngle)
        {
            if(Boolean.valueOf(value.toString())){
                compareAngleTolerance.setEnabled(true);
            }
            else{
                compareAngleTolerance.setEnabled(false);
            }
        }
        else if(preference == compareAngleTolerance)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int tolerance = Integer.parseInt(value.toString());
            if(tolerance<25 || tolerance>125){
                Toast.makeText(this, "Invalid Angle Tolerance: Angle Tolerance must be greater or equal than 25 and smaller or equal than 125.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == useSpeedIntervalRange)
        {
            if(Boolean.valueOf(value.toString())){
                speedIntervalRange.setEnabled(true);
                speedIntervalEndpoint1.setEnabled(false);
                speedIntervalEndpoint2.setEnabled(false);
                speedIntervalEndpoint3.setEnabled(false);
                speedIntervalEndpoint4.setEnabled(false);
                speedIntervalEndpoint5.setEnabled(false);
                speedIntervalEndpoint6.setEnabled(false);
                speedIntervalEndpoint7.setEnabled(false);
                speedIntervalEndpoint8.setEnabled(false);
                speedIntervalEndpoint9.setEnabled(false);
                speedIntervalEndpoint10.setEnabled(false);
            }
            else{
                speedIntervalRange.setEnabled(false);
                speedIntervalEndpoint1.setEnabled(true);
                speedIntervalEndpoint2.setEnabled(true);
                speedIntervalEndpoint3.setEnabled(true);
                speedIntervalEndpoint4.setEnabled(true);
                speedIntervalEndpoint5.setEnabled(true);
                speedIntervalEndpoint6.setEnabled(true);
                speedIntervalEndpoint7.setEnabled(true);
                speedIntervalEndpoint8.setEnabled(true);
                speedIntervalEndpoint9.setEnabled(true);
                speedIntervalEndpoint10.setEnabled(true);
            }
        }
        else if (preference == heightIntervalRange)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int heightIntervalRange = Integer.parseInt(value.toString());
            if(heightIntervalRange<1){
                Toast.makeText(this, "Invalid Height Interval Range: Range must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalRange)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalRange = Integer.parseInt(value.toString());
            if(speedIntervalRange<1){
                Toast.makeText(this, "Invalid Speed Interval Range: Range must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalEndpoint1)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalEndpoint = Integer.parseInt(value.toString());
            int savedIntervalEndpoint2 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint2_key), getString(R.string.preference_advancedSpeedIntervalEndpoint2_default)));
            if(speedIntervalEndpoint>=savedIntervalEndpoint2){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 1 must be smaller than Endpoint 2.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint<1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalEndpoint2)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalEndpoint = Integer.parseInt(value.toString());
            int savedIntervalEndpoint1 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint1_key), getString(R.string.preference_advancedSpeedIntervalEndpoint1_default)));
            int savedIntervalEndpoint3 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint3_key), getString(R.string.preference_advancedSpeedIntervalEndpoint3_default)));
            if(speedIntervalEndpoint<=savedIntervalEndpoint1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 2 must be greater than Endpoint 1.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint>=savedIntervalEndpoint3){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 2 must be smaller than Endpoint 3.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint<1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalEndpoint3)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalEndpoint = Integer.parseInt(value.toString());
            int savedIntervalEndpoint2 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint2_key), getString(R.string.preference_advancedSpeedIntervalEndpoint2_default)));
            int savedIntervalEndpoint4 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint4_key), getString(R.string.preference_advancedSpeedIntervalEndpoint4_default)));
            if(speedIntervalEndpoint<=savedIntervalEndpoint2){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 3 must be greater than Endpoint 2.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint>=savedIntervalEndpoint4){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 3 must be smaller than Endpoint 4.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint<1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalEndpoint4)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalEndpoint = Integer.parseInt(value.toString());
            int savedIntervalEndpoint3 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint3_key), getString(R.string.preference_advancedSpeedIntervalEndpoint3_default)));
            int savedIntervalEndpoint5 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint5_key), getString(R.string.preference_advancedSpeedIntervalEndpoint5_default)));
            if(speedIntervalEndpoint<=savedIntervalEndpoint3){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 4 must be greater than Endpoint 3.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint>=savedIntervalEndpoint5){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 4 must be smaller than Endpoint 5.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint<1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalEndpoint5)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalEndpoint = Integer.parseInt(value.toString());
            int savedIntervalEndpoint4 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint4_key), getString(R.string.preference_advancedSpeedIntervalEndpoint4_default)));
            int savedIntervalEndpoint6 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint6_key), getString(R.string.preference_advancedSpeedIntervalEndpoint6_default)));
            if(speedIntervalEndpoint<=savedIntervalEndpoint4){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 5 must be greater than Endpoint 4.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint>=savedIntervalEndpoint6){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 5 must be smaller than Endpoint 6.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint<1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalEndpoint6)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalEndpoint = Integer.parseInt(value.toString());
            int savedIntervalEndpoint5 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint5_key), getString(R.string.preference_advancedSpeedIntervalEndpoint5_default)));
            int savedIntervalEndpoint7 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint7_key), getString(R.string.preference_advancedSpeedIntervalEndpoint7_default)));
            if(speedIntervalEndpoint<=savedIntervalEndpoint5){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 6 must be greater than Endpoint 5.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint>=savedIntervalEndpoint7){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 6 must be smaller than Endpoint 7.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint<1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalEndpoint7)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalEndpoint = Integer.parseInt(value.toString());
            int savedIntervalEndpoint6 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint6_key), getString(R.string.preference_advancedSpeedIntervalEndpoint6_default)));
            int savedIntervalEndpoint8 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint8_key), getString(R.string.preference_advancedSpeedIntervalEndpoint8_default)));
            if(speedIntervalEndpoint<=savedIntervalEndpoint6){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 7 must be greater than Endpoint 6.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint>=savedIntervalEndpoint8){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 7 must be smaller than Endpoint 8.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint<1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalEndpoint8)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalEndpoint = Integer.parseInt(value.toString());
            int savedIntervalEndpoint7 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint7_key), getString(R.string.preference_advancedSpeedIntervalEndpoint7_default)));
            int savedIntervalEndpoint9 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint9_key), getString(R.string.preference_advancedSpeedIntervalEndpoint9_default)));
            if(speedIntervalEndpoint<=savedIntervalEndpoint7){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 8 must be greater than Endpoint 7.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint>=savedIntervalEndpoint9){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 8 must be smaller than Endpoint 9.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint<1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalEndpoint9)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalEndpoint = Integer.parseInt(value.toString());
            int savedIntervalEndpoint8 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint8_key), getString(R.string.preference_advancedSpeedIntervalEndpoint8_default)));
            int savedIntervalEndpoint10 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint10_key), getString(R.string.preference_advancedSpeedIntervalEndpoint10_default)));
            if(speedIntervalEndpoint<=savedIntervalEndpoint8){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 9 must be greater than Endpoint 8.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint>=savedIntervalEndpoint10){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 9 must be smaller than Endpoint 10.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint<1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint must be greater or equal than 1.", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        else if (preference == speedIntervalEndpoint10)
        {
            if(value.toString().isEmpty()){
                Toast.makeText(this, "Preference can not be empty.", Toast.LENGTH_LONG).show();
                return false;
            }
            int speedIntervalEndpoint = Integer.parseInt(value.toString());
            int savedIntervalEndpoint9 = Integer.parseInt(sharedPrefs.getString(getString(R.string.preference_advancedSpeedIntervalEndpoint9_key), getString(R.string.preference_advancedSpeedIntervalEndpoint9_default)));

            if(speedIntervalEndpoint<=savedIntervalEndpoint9){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint 10 must be greater than Endpoint 9.", Toast.LENGTH_LONG).show();
                return false;
            }
            if(speedIntervalEndpoint<1){
                Toast.makeText(this, "Invalid Speed Interval Endpoint: Endpoint must be greater or equal than 1.", Toast.LENGTH_LONG).show();
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
        editor.putBoolean(getString(R.string.preference_advancedCompareAngle_key),Boolean.valueOf(getString(R.string.preference_advancedCompareAngle_default)));
        editor.putString(getString(R.string.preference_advancedCompareAngleTolerance_key),getString(R.string.preference_advancedCompareAngleTolerance_default));
        editor.putBoolean(getString(R.string.preference_advancedRandomizePosition_key),Boolean.valueOf(getString(R.string.preference_advancedRandomizePosition_default)));
        editor.putString(getString(R.string.preference_advancedHeightIntervalRange_key),getString(R.string.preference_advancedHeightIntervalRange_default));
        editor.putBoolean(getString(R.string.preference_advancedUseSpeedIntervalRange_key),Boolean.valueOf(getString(R.string.preference_advancedUseSpeedIntervalRange_default)));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalRange_key),getString(R.string.preference_advancedSpeedIntervalRange_default));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalEndpoint1_key),getString(R.string.preference_advancedSpeedIntervalEndpoint1_default));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalEndpoint2_key),getString(R.string.preference_advancedSpeedIntervalEndpoint2_default));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalEndpoint3_key),getString(R.string.preference_advancedSpeedIntervalEndpoint3_default));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalEndpoint4_key),getString(R.string.preference_advancedSpeedIntervalEndpoint4_default));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalEndpoint5_key),getString(R.string.preference_advancedSpeedIntervalEndpoint5_default));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalEndpoint6_key),getString(R.string.preference_advancedSpeedIntervalEndpoint6_default));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalEndpoint7_key),getString(R.string.preference_advancedSpeedIntervalEndpoint7_default));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalEndpoint8_key),getString(R.string.preference_advancedSpeedIntervalEndpoint8_default));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalEndpoint9_key),getString(R.string.preference_advancedSpeedIntervalEndpoint9_default));
        editor.putString(getString(R.string.preference_advancedSpeedIntervalEndpoint10_key),getString(R.string.preference_advancedSpeedIntervalEndpoint10_default));
        editor.commit();
        finish();
        startActivity(getIntent());
        Toast.makeText(this, "All Advanced Settings has been reset.", Toast.LENGTH_LONG).show();
    }
}
