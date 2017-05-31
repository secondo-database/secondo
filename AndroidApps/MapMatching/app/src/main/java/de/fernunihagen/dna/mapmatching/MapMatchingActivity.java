package de.fernunihagen.dna.mapmatching;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.location.LocationManager;
import android.os.Build;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.support.annotation.RequiresApi;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.text.Html;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TabHost;
import android.widget.TextView;


import org.osmdroid.tileprovider.constants.OpenStreetMapTileProviderConstants;
import org.osmdroid.views.MapView;

import java.util.ArrayList;
import java.util.List;

public class MapMatchingActivity extends AppCompatActivity {

    private TabHost tabHost;
    private Button startStopButton;
    private TextView console;
    private ScrollView consoleScrollView;
    private MapView mapView;

    private MenuItem menuSettingsGeneral;
    private MenuItem menuSettingsAdvanced;
    private MenuItem menuSettingsGps;
    private MenuItem menuSettingsServer;
    private MenuItem menuReset;
    private MenuItem menuShowHideMapOptions;
    private MenuItem menuShowHideMapLabels;
    private MenuItem menuExitApp;

    private LinearLayout mapOptions;
    private CheckBox checkBoxGpsPoints;
    private CheckBox checkBoxMatchFootways;

    private LinearLayout mapLabels;
    private TextView labelMapStreetname;
    private TextView labelMapDirection;

    private TextView labelOverviewStreetname1;
    private TextView labelOverviewStreetname2;
    private TextView labelOverviewStreetname3;
    private TextView labelOverviewStreetname4;
    private TextView labelOverviewStreetname5;
    private TextView labelOverviewDirection1;
    private TextView labelOverviewDirection2;
    private TextView labelOverviewDirection3;
    private TextView labelOverviewDirection4;
    private TextView labelOverviewDirection5;
    private TextView labelOverviewSpeed1;
    private TextView labelOverviewSpeed2;
    private TextView labelOverviewSpeed3;
    private TextView labelOverviewSpeed4;
    private TextView labelOverviewSpeed5;
    private TextView labelOverviewHeight1;
    private TextView labelOverviewHeight2;
    private TextView labelOverviewHeight3;
    private TextView labelOverviewHeight4;
    private TextView labelOverviewHeight5;

    private SharedPreferences sharedPrefs;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_map_matching_activity);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        tabHost = (TabHost) findViewById(R.id.tabHost);
        tabHost.setup();
        TabHost.TabSpec spec = tabHost.newTabSpec(getString(R.string.activityMapMatchingTabMap));
        spec.setContent(R.id.tab1);
        spec.setIndicator(getString(R.string.activityMapMatchingTabMap));
        tabHost.addTab(spec);
        spec = tabHost.newTabSpec(getString(R.string.activityMapMatchingTabConsole));
        spec.setContent(R.id.tab2);
        spec.setIndicator(getString(R.string.activityMapMatchingTabConsole));
        tabHost.addTab(spec);
        spec = tabHost.newTabSpec(getString(R.string.activityMapMatchingTabOverview));
        spec.setContent(R.id.tab3);
        spec.setIndicator(getString(R.string.activityMapMatchingTabOverview));
        tabHost.addTab(spec);

        sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);

        console = (TextView) findViewById(R.id.console);
        consoleScrollView = (ScrollView) findViewById(R.id.consoleScrollView);

        mapOptions = (LinearLayout) findViewById(R.id.mapOptions);
        checkBoxGpsPoints = (CheckBox) findViewById(R.id.checkBoxGpsPoints);
        checkBoxMatchFootways = (CheckBox) findViewById(R.id.checkBoxMatchFootways);

        mapLabels = (LinearLayout) findViewById(R.id.mapLabels);
        labelMapStreetname = (TextView) findViewById(R.id.labelMapStreetname);
        labelMapDirection = (TextView) findViewById(R.id.labelMapDirection);

        labelOverviewStreetname1 = (TextView) findViewById(R.id.labelOverviewStreetname1);
        labelOverviewStreetname2 = (TextView) findViewById(R.id.labelOverviewStreetname2);
        labelOverviewStreetname3 = (TextView) findViewById(R.id.labelOverviewStreetname3);
        labelOverviewStreetname4 = (TextView) findViewById(R.id.labelOverviewStreetname4);
        labelOverviewStreetname5 = (TextView) findViewById(R.id.labelOverviewStreetname5);
        labelOverviewDirection1 = (TextView) findViewById(R.id.labelOverviewDirection1);
        labelOverviewDirection2 = (TextView) findViewById(R.id.labelOverviewDirection2);
        labelOverviewDirection3 = (TextView) findViewById(R.id.labelOverviewDirection3);
        labelOverviewDirection4 = (TextView) findViewById(R.id.labelOverviewDirection4);
        labelOverviewDirection5 = (TextView) findViewById(R.id.labelOverviewDirection5);
        labelOverviewSpeed1 = (TextView) findViewById(R.id.labelOverviewSpeed1);
        labelOverviewSpeed2 = (TextView) findViewById(R.id.labelOverviewSpeed2);
        labelOverviewSpeed3 = (TextView) findViewById(R.id.labelOverviewSpeed3);
        labelOverviewSpeed4 = (TextView) findViewById(R.id.labelOverviewSpeed4);
        labelOverviewSpeed5 = (TextView) findViewById(R.id.labelOverviewSpeed5);
        labelOverviewHeight1 = (TextView) findViewById(R.id.labelOverviewHeight1);
        labelOverviewHeight2 = (TextView) findViewById(R.id.labelOverviewHeight2);
        labelOverviewHeight3 = (TextView) findViewById(R.id.labelOverviewHeight3);
        labelOverviewHeight4 = (TextView) findViewById(R.id.labelOverviewHeight4);
        labelOverviewHeight5 = (TextView) findViewById(R.id.labelOverviewHeight5);

        startStopButton = (Button) findViewById(R.id.startStopButton);
        startStopButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                startStopButtonClick();
            }
        });

        tabHost.setOnTabChangedListener(new TabHost.OnTabChangeListener(){
            @Override
            public void onTabChanged(String tabId) {
                if(tabId.equals(getString(R.string.activityMapMatchingTabConsole))) {
                    if(MapMatchingCoreInterface.isMapMatchingActive()){
                        scrollConsoleDown();
                    }
                }
            }});

        checkBoxGpsPoints.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                checkBoxShowGpsPointsClick();
            }
        });
        checkBoxMatchFootways.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                checkBoxMatchFootwaysClick();
            }
        });

        OpenStreetMapTileProviderConstants.setUserAgentValue(BuildConfig.APPLICATION_ID);
        mapView = (MapView) findViewById(R.id.map);
        mapView.setMultiTouchControls(true);

        MapMatchingCoreInterface.setMapMatchingActivity(this);
        MapMatchingCoreInterface.getMapMatchingCoreInterface().getMapMatchingGps().setMapMatchingActivity(this);
        MapMatchingOverview.setMapMatchingActivity(this);
        Dialogs.setMapMatchingActivity(this);
        MapMatchingMap.setMapMatchingActivity(this);
        MapMatchingMap.setMapView(mapView);
        boolean savedMapMatchingShowMap = sharedPrefs.getBoolean(getString(R.string.preference_mapMatchingShowMap_key), true);
        MapMatchingMap.setShowMap(savedMapMatchingShowMap);

        boolean savedShowMapOptions = sharedPrefs.getBoolean("showMapOptions", true);
        boolean savedShowMapLabels = sharedPrefs.getBoolean("showMapLabels", true);
        if(!savedShowMapOptions){
            mapOptions.removeAllViews();
        }
        if(!savedShowMapLabels){
            mapLabels.removeAllViews();
        }

        if (savedInstanceState != null) {
            if (savedInstanceState.getBoolean("consoleTabSelected")) {
                tabHost.setCurrentTab(1);
            }
            if (savedInstanceState.getBoolean("overviewTabSelected")) {
                tabHost.setCurrentTab(2);
            }
            if (savedInstanceState.getBoolean("dialogGpsActive")) {
                Dialogs.showDialogGpsDisabled();
            }
            if (savedInstanceState.getBoolean("dialogSettingsActive")) {
                Dialogs.showDialogConncetionFailed();
            }
            if (savedInstanceState.getBoolean("dialogCloseAppActive")) {
                Dialogs.showDialogCloseApp();
            }
            if(savedInstanceState.getBoolean("dialogResetDataActive")){
                Dialogs.showDialogResetData();
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
                if (savedInstanceState.getBoolean("dialogPermissionsActive")) {
                    Dialogs.showDialogPermissions();
                }
            }
            if (savedInstanceState.getBoolean("startStopButtonDisabled")) {
                disableStartStopButton();
            }

            labelMapStreetname.setText(savedInstanceState.getString("labelMapStreetname"));
            labelMapDirection.setText(savedInstanceState.getString("labelMapDirection"));
            labelOverviewStreetname1.setText(savedInstanceState.getString("labelOverviewStreetname1"));
            labelOverviewStreetname2.setText(savedInstanceState.getString("labelOverviewStreetname2"));
            labelOverviewStreetname3.setText(savedInstanceState.getString("labelOverviewStreetname3"));
            labelOverviewStreetname4.setText(savedInstanceState.getString("labelOverviewStreetname4"));
            labelOverviewStreetname5.setText(savedInstanceState.getString("labelOverviewStreetname5"));
            labelOverviewDirection1.setText(savedInstanceState.getString("labelOverviewDirection1"));
            labelOverviewDirection2.setText(savedInstanceState.getString("labelOverviewDirection2"));
            labelOverviewDirection3.setText(savedInstanceState.getString("labelOverviewDirection3"));
            labelOverviewDirection4.setText(savedInstanceState.getString("labelOverviewDirection4"));
            labelOverviewDirection5.setText(savedInstanceState.getString("labelOverviewDirection5"));
            labelOverviewSpeed1.setText(savedInstanceState.getString("labelOverviewSpeed1"));
            labelOverviewSpeed2.setText(savedInstanceState.getString("labelOverviewSpeed2"));
            labelOverviewSpeed3.setText(savedInstanceState.getString("labelOverviewSpeed3"));
            labelOverviewSpeed4.setText(savedInstanceState.getString("labelOverviewSpeed4"));
            labelOverviewSpeed5.setText(savedInstanceState.getString("labelOverviewSpeed5"));
            labelOverviewHeight1.setText(savedInstanceState.getString("labelOverviewHeight1"));
            labelOverviewHeight2.setText(savedInstanceState.getString("labelOverviewHeight2"));
            labelOverviewHeight3.setText(savedInstanceState.getString("labelOverviewHeight3"));
            labelOverviewHeight4.setText(savedInstanceState.getString("labelOverviewHeight4"));
            labelOverviewHeight5.setText(savedInstanceState.getString("labelOverviewHeight5"));

            double mapLat = savedInstanceState.getDouble("mapLat");
            double mapLon = savedInstanceState.getDouble("mapLon");
            int mapZoom = savedInstanceState.getInt("mapZoom");
            MapMatchingMap.refreshMap(mapLat, mapLon, mapZoom);
            refreshConsole();
            refreshButton();
        }
        else{
            permissionCheck();

            if(MapMatchingCoreInterface.getMapResults().size()!=0){
                MapMatchingMap.updateMap(MapMatchingCoreInterface.getMapResults());
                MapMatchingOverview.update();
                refreshConsole();
                refreshButton();
            }
            else{
                showWelcomeMessage();
            }
        }
    }

    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        super.onSaveInstanceState(savedInstanceState);

        savedInstanceState.putString("labelMapStreetname", labelMapStreetname.getText().toString());
        savedInstanceState.putString("labelMapDirection", labelMapDirection.getText().toString());
        savedInstanceState.putString("labelOverviewStreetname1", labelOverviewStreetname1.getText().toString());
        savedInstanceState.putString("labelOverviewStreetname2", labelOverviewStreetname2.getText().toString());
        savedInstanceState.putString("labelOverviewStreetname3", labelOverviewStreetname3.getText().toString());
        savedInstanceState.putString("labelOverviewStreetname4", labelOverviewStreetname4.getText().toString());
        savedInstanceState.putString("labelOverviewStreetname5", labelOverviewStreetname5.getText().toString());
        savedInstanceState.putString("labelOverviewDirection1", labelOverviewDirection1.getText().toString());
        savedInstanceState.putString("labelOverviewDirection2", labelOverviewDirection2.getText().toString());
        savedInstanceState.putString("labelOverviewDirection3", labelOverviewDirection3.getText().toString());
        savedInstanceState.putString("labelOverviewDirection4", labelOverviewDirection4.getText().toString());
        savedInstanceState.putString("labelOverviewDirection5", labelOverviewDirection5.getText().toString());
        savedInstanceState.putString("labelOverviewSpeed1", labelOverviewSpeed1.getText().toString());
        savedInstanceState.putString("labelOverviewSpeed2", labelOverviewSpeed2.getText().toString());
        savedInstanceState.putString("labelOverviewSpeed3", labelOverviewSpeed3.getText().toString());
        savedInstanceState.putString("labelOverviewSpeed4", labelOverviewSpeed4.getText().toString());
        savedInstanceState.putString("labelOverviewSpeed5", labelOverviewSpeed5.getText().toString());
        savedInstanceState.putString("labelOverviewHeight1", labelOverviewHeight1.getText().toString());
        savedInstanceState.putString("labelOverviewHeight2", labelOverviewHeight2.getText().toString());
        savedInstanceState.putString("labelOverviewHeight3", labelOverviewHeight3.getText().toString());
        savedInstanceState.putString("labelOverviewHeight4", labelOverviewHeight4.getText().toString());
        savedInstanceState.putString("labelOverviewHeight5", labelOverviewHeight5.getText().toString());
        savedInstanceState.putDouble("mapLat", mapView.getMapCenter().getLatitude());
        savedInstanceState.putDouble("mapLon", mapView.getMapCenter().getLongitude());
        savedInstanceState.putInt("mapZoom", mapView.getZoomLevel());

        if (tabHost.getTabWidget().getChildTabViewAt(1).isSelected()) {
            savedInstanceState.putBoolean("consoleTabSelected", true);
        } else {
            savedInstanceState.putBoolean("consoleTabSelected", false);
        }
        if (tabHost.getTabWidget().getChildTabViewAt(2).isSelected()) {
            savedInstanceState.putBoolean("overviewTabSelected", true);
        } else {
            savedInstanceState.putBoolean("overviewTabSelected", false);
        }
        if (Dialogs.isDialogGpsDisabledActive()) {
            savedInstanceState.putBoolean("dialogGpsActive", true);
        } else {
            savedInstanceState.putBoolean("dialogGpsActive", false);
        }
        if (Dialogs.isDialogConncetionFailedActive()) {
            savedInstanceState.putBoolean("dialogSettingsActive", true);
        } else {
            savedInstanceState.putBoolean("dialogSettingsActive", false);
        }
        if (Dialogs.isDialogCloseAppActive()) {
            savedInstanceState.putBoolean("dialogCloseAppActive", true);
        } else {
            savedInstanceState.putBoolean("dialogCloseAppActive", false);
        }
        if (Dialogs.isDialogResetDataActive()) {
            savedInstanceState.putBoolean("dialogResetDataActive", true);
        } else {
            savedInstanceState.putBoolean("dialogResetDataActive", false);
        }
        if (Dialogs.isDialogPermissionsActive()) {
            savedInstanceState.putBoolean("dialogPermissionsActive", true);
        } else {
            savedInstanceState.putBoolean("dialogPermissionsActive", false);
        }
        if (!startStopButton.isEnabled()) {
            savedInstanceState.putBoolean("startStopButtonDisabled", true);
        } else {
            savedInstanceState.putBoolean("startStopButtonDisabled", false);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        refreshTabs();
        MapMatchingMap.reload();
        invalidateOptionsMenu();
        refreshMapOptions();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu, menu);
        menuSettingsServer = menu.getItem(0);
        menuSettingsGeneral = menu.getItem(1);
        menuSettingsGps = menu.getItem(2);
        menuSettingsAdvanced = menu.getItem(3);
        menuReset = menu.getItem(4);
        menuShowHideMapOptions = menu.getItem(5);
        menuShowHideMapLabels = menu.getItem(6);
        menuExitApp = menu.getItem(7);

        if(MapMatchingMap.isShowMap()){
            menuShowHideMapOptions.setEnabled(true);
            menuShowHideMapLabels.setEnabled(true);
        }
        else {
            menuShowHideMapOptions.setEnabled(false);
            menuShowHideMapLabels.setEnabled(false);
        }
        boolean savedShowMapOptions = sharedPrefs.getBoolean("showMapOptions", true);
        boolean savedShowMapLabels = sharedPrefs.getBoolean("showMapLabels", true);
        if(!savedShowMapOptions){
            menuShowHideMapOptions.setTitle(getString(R.string.menuShowMapOptions));
        }
        if(!savedShowMapLabels){
            menuShowHideMapLabels.setTitle(getString(R.string.menuShowMapLabels));
        }

        if (MapMatchingCoreInterface.isMapMatchingActive()) {
            menuSettingsServer.setEnabled(false);
            menuSettingsGeneral.setEnabled(false);
            menuSettingsAdvanced.setEnabled(false);
            menuSettingsGps.setEnabled(false);
            menuReset.setEnabled(false);
        } else {
            menuSettingsServer.setEnabled(true);
            menuSettingsGeneral.setEnabled(true);
            menuSettingsAdvanced.setEnabled(true);
            menuSettingsGps.setEnabled(true);
            menuReset.setEnabled(true);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();

        if (id == R.id.settingsServer) {
            startActivity(new Intent(this, SettingsServerActivity.class));
            return true;
        } else if (id == R.id.settingsGeneral) {
            startActivity(new Intent(this, SettingsGeneralActivity.class));
            return true;
        }else if (id == R.id.settingsGps) {
            startActivity(new Intent(this, SettingsGpsActivity.class));
            return true;
        } else if (id == R.id.settingsAdvanced) {
            startActivity(new Intent(this, SettingsAdvancedActivity.class));
            return true;
        }else if (id == R.id.reset) {
            Dialogs.showDialogResetData();
            return true;
        }else if (id == R.id.showHideMapOptions) {
            boolean savedShowMapOptions = sharedPrefs.getBoolean("showMapOptions", true);
            if(savedShowMapOptions){
                hideMapOptions();
            }
            else{
                showMapOptions();
            }
            return true;
        }else if (id == R.id.showHideMapLabels) {
            boolean savedShowMapLabels = sharedPrefs.getBoolean("showMapLabels", true);
            if(savedShowMapLabels){
                hideMapLabels();
            }
            else{
                showMapLabels();
            }
            return true;
        }else if (id == R.id.exitApp) {
            Dialogs.showDialogCloseApp();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Dialogs.closeDialogGpsDisabled();
        Dialogs.closeDialogConncetionFailed();
        Dialogs.closeDialogCloseApp();
        Dialogs.closeDialogResetData();
    }

    @RequiresApi(api = Build.VERSION_CODES.M)
    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {

        if(requestCode==1){
            if (!shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                if(ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED){
                    if(!Dialogs.isDialogPermissionsActive()){
                        Dialogs.showDialogPermissions();
                    }
                    return;
                }
            }
        }
        else if(requestCode==2){
            if (!shouldShowRequestPermissionRationale(Manifest.permission.INTERNET)) {
                if(ActivityCompat.checkSelfPermission(this, Manifest.permission.INTERNET) != PackageManager.PERMISSION_GRANTED){
                    if(!Dialogs.isDialogPermissionsActive()){
                        Dialogs.showDialogPermissions();
                    }
                    return;
                }
            }
        }
        else if(requestCode==3){
            if (!shouldShowRequestPermissionRationale(Manifest.permission.ACCESS_FINE_LOCATION)) {
                if(ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED){
                    if(!Dialogs.isDialogPermissionsActive()){
                        Dialogs.showDialogPermissions();
                    }
                    return;
                }
            }
        }
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(this, Manifest.permission.INTERNET) == PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED) {
            finish();
            startActivity(getIntent());
        }
        else{
            permissionCheck();
        }
    }

    @Override
    public void onBackPressed() {
        Dialogs.showDialogCloseApp();
    }

    private void startStopButtonClick() {
        if (MapMatchingCoreInterface.isMapMatchingActive()) {
            mapMatchingStop();
        } else {
            if (checkGpsEnabledAndPermissions()) {
                mapMatchingStart();
            }
        }
    }

    private boolean checkGpsEnabledAndPermissions() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED) {
            LocationManager locationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
            if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                return true;
            } else {
                Dialogs.showDialogGpsDisabled();
            }
        } else {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 1);
        }
        return false;
    }

    public void enableMenu(){
        menuSettingsServer.setEnabled(true);
        menuSettingsGps.setEnabled(true);
        menuSettingsGeneral.setEnabled(true);
        menuSettingsAdvanced.setEnabled(true);
        menuReset.setEnabled(true);
    }

    private void disableMenu(){
        menuSettingsServer.setEnabled(false);
        menuSettingsGps.setEnabled(false);
        menuSettingsGeneral.setEnabled(false);
        menuSettingsAdvanced.setEnabled(false);
        menuReset.setEnabled(false);
    }

    public void mapMatchingStop() {
        MapMatchingCoreInterface.setMapMatchingActive(false);
        startStopButton.setText(getString(R.string.activityMapMatchingButtonStart));
        Intent i = new Intent(getApplicationContext(),MapMatchingCoreInterface.class);
        stopService(i);
        MapMatchingCoreInterface.stopMapMatching();
    }

    public void mapMatchingStart() {
        labelMapDirection.setText("-");
        labelMapStreetname.setText("-");
        MapMatchingOverview.clearData();
        MapMatchingCoreInterface.setMapMatchingActive(true);
        disableMenu();
        startStopButton.setText(getString(R.string.activityMapMatchingButtonStop));
        MapMatchingCoreInterface.setSettings();

        Intent i = new Intent(getApplicationContext(),MapMatchingCoreInterface.class);
        startService(i);
        //TestData.test();
    }

    public void enableStartStopButton() {
        startStopButton.setEnabled(true);
    }

    public void disableStartStopButton() {
        startStopButton.setEnabled(false);
    }

    public void permissionCheck() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            disableStartStopButton();
            tabHost.setCurrentTab(1);
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
        }
        else if (ActivityCompat.checkSelfPermission(this, Manifest.permission.INTERNET) != PackageManager.PERMISSION_GRANTED) {
            disableStartStopButton();
            tabHost.setCurrentTab(1);
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.INTERNET}, 2);
        }
        else if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            disableStartStopButton();
            tabHost.setCurrentTab(1);
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 3);
        }
    }

    private void checkBoxShowGpsPointsClick(){
        SharedPreferences.Editor editor = sharedPrefs.edit();
        editor.putBoolean(getString(R.string.preference_mapMatchingShowGpsPoints_key), checkBoxGpsPoints.isChecked());
        editor.commit();
        MapMatchingMap.setShowGpsPoints(checkBoxGpsPoints.isChecked());
        MapMatchingMap.reload();
    }

    private void checkBoxMatchFootwaysClick(){
        SharedPreferences.Editor editor = sharedPrefs.edit();
        editor.putBoolean(getString(R.string.preference_mapMatchingMatchFootways_key), checkBoxMatchFootways.isChecked());
        editor.commit();
        MapMatchingCoreInterface.changeMatchFootwaysPref(checkBoxMatchFootways.isChecked());
    }

    private void showMapOptions(){
        menuShowHideMapOptions.setTitle(getString(R.string.menuHideMapOptions));
        SharedPreferences.Editor editor = sharedPrefs.edit();
        editor.putBoolean("showMapOptions", true);
        editor.commit();
        mapOptions.addView(checkBoxGpsPoints);
        mapOptions.addView(checkBoxMatchFootways);
    }

    private void hideMapOptions(){
        menuShowHideMapOptions.setTitle(getString(R.string.menuShowMapOptions));
        SharedPreferences.Editor editor = sharedPrefs.edit();
        editor.putBoolean("showMapOptions", false);
        editor.commit();
        mapOptions.removeAllViews();
    }

    private void showMapLabels(){
        menuShowHideMapLabels.setTitle(getString(R.string.menuHideMapLabels));
        SharedPreferences.Editor editor = sharedPrefs.edit();
        editor.putBoolean("showMapLabels", true);
        editor.commit();
        mapLabels.addView(labelMapStreetname);
        mapLabels.addView(labelMapDirection);
    }

    private void hideMapLabels(){
        menuShowHideMapLabels.setTitle(getString(R.string.menuShowMapLabels));
        SharedPreferences.Editor editor = sharedPrefs.edit();
        editor.putBoolean("showMapLabels", false);
        editor.commit();
        mapLabels.removeAllViews();
    }

    public void showLineResultMessage(String message) {
        message = "<font color=#00FF00>" + message + "</font><br><br>";
        MapMatchingCoreInterface.getConsoleText().add(message);
        cleanConsoleText();
        console.append(Html.fromHtml(message));
        scrollConsoleDown();
    }

    public void showResultMessage(String message) {
        message = "<font color=#00FF00>" + message + "</font>";
        MapMatchingCoreInterface.getConsoleText().add(message);
        cleanConsoleText();
        console.append(Html.fromHtml(message));
        scrollConsoleDown();
    }

    public void showLineInfoMessage(String message) {
        message = "<font color=#CCCCCC>" + message + "</font><br><br>";
        MapMatchingCoreInterface.getConsoleText().add(message);
        cleanConsoleText();
        console.append(Html.fromHtml(message));
        scrollConsoleDown();
    }

    public void showInfoMessage(String message) {
        message = "<font color=#CCCCCC>" + message + "</font>";
        MapMatchingCoreInterface.getConsoleText().add(message);
        cleanConsoleText();
        console.append(Html.fromHtml(message));
        scrollConsoleDown();
    }

    public void showLineErrorMessage(String message) {
        if (!Dialogs.isDialogConncetionFailedActive()) {
            tabHost.setCurrentTab(1);
        }
        message = "<font color=#FF0000>" + message + "</font><br><br>";
        MapMatchingCoreInterface.getConsoleText().add(message);
        cleanConsoleText();
        console.append(Html.fromHtml(message));
        scrollConsoleDown();
    }

    public void showErrorMessage(String message) {
        if (!Dialogs.isDialogConncetionFailedActive()) {
            tabHost.setCurrentTab(1);
        }
        message = "<font color=#FF0000>" + message + "</font>";
        MapMatchingCoreInterface.getConsoleText().add(message);
        cleanConsoleText();
        console.append(Html.fromHtml(message));
        scrollConsoleDown();
    }

    private void scrollConsoleDown() {
        consoleScrollView.post(new Runnable() {
            @Override
            public void run() {
                consoleScrollView.fullScroll(ScrollView.FOCUS_DOWN);
            }
        });
    }

    public void clearConsoleData(){
        console.setText("");
        MapMatchingCoreInterface.getConsoleText().clear();
        showWelcomeMessage();
    }

    private void cleanConsoleText(){
        if(MapMatchingCoreInterface.getConsoleText().size()>2000){
            MapMatchingCoreInterface.setConsoleText(new ArrayList<String>(MapMatchingCoreInterface.getConsoleText().subList(250,MapMatchingCoreInterface.getConsoleText().size())));
            console.setText("");
            refreshConsole();
        }
    }

    private void refreshButton() {
        if (MapMatchingCoreInterface.isMapMatchingActive()) {
            startStopButton.setText(getString(R.string.activityMapMatchingButtonStop));
        } else {
            startStopButton.setText(getString(R.string.activityMapMatchingButtonStart));
        }
    }

    private void refreshMapOptions(){
        boolean savedShowGpsPoints = sharedPrefs.getBoolean(getString(R.string.preference_mapMatchingShowGpsPoints_key), Boolean.valueOf(getString(R.string.preference_mapMatchingShowGpsPoints_default)));
        boolean savedMatchFootways = sharedPrefs.getBoolean(getString(R.string.preference_mapMatchingMatchFootways_key), Boolean.valueOf(getString(R.string.preference_mapMatchingMatchFootways_default)));
        if(savedShowGpsPoints){
            checkBoxGpsPoints.setChecked(true);
        }
        else{
            checkBoxGpsPoints.setChecked(false);
        }
        if(savedMatchFootways){
            checkBoxMatchFootways.setChecked(true);
        }
        else{
            checkBoxMatchFootways.setChecked(false);
        }
    }

    private void refreshTabs() {
        if (!MapMatchingMap.isShowMap()) {
            tabHost.getTabWidget().getChildTabViewAt(0).setEnabled(false);
            TextView tv = (TextView) tabHost.getTabWidget().getChildAt(0).findViewById(android.R.id.title);
            tv.setText(getString(R.string.activityMapMatchingTabMapDisabled));
            if(tabHost.getTabWidget().getChildTabViewAt(0).isSelected()){
                tabHost.setCurrentTab(1);
            }
        } else {
            tabHost.getTabWidget().getChildTabViewAt(0).setEnabled(true);
            TextView tv = (TextView) tabHost.getTabWidget().getChildAt(0).findViewById(android.R.id.title);
            tv.setText(getString(R.string.activityMapMatchingTabMap));
        }
    }

    private void refreshConsole() {
        for (String message : MapMatchingCoreInterface.getConsoleText()) {
            console.append(Html.fromHtml(message));
        }
        scrollConsoleDown();
    }

    private void showWelcomeMessage(){
        String androidId = Settings.Secure.getString(getContentResolver(),Settings.Secure.ANDROID_ID);
        String welcomeMessage = "<font color=#FF4081>" + "Welcome to Secondo Map Matching." + "</font><br><br>";
        MapMatchingCoreInterface.getConsoleText().add(welcomeMessage);
        console.append(Html.fromHtml(welcomeMessage));

        showInfoMessage("Results will be stored using your Android Device Id.<br>");
        showLineInfoMessage("Your Android Device Id is: "+androidId);
    }

    public TextView getLabelMapStreetname() {
        return labelMapStreetname;
    }

    public TextView getLabelMapDirection() {
        return labelMapDirection;
    }

    public List<TextView> getLabelsOverviewStreetname() {
        List<TextView> labelsStreetname = new ArrayList<TextView>();
        labelsStreetname.add(labelOverviewStreetname1);
        labelsStreetname.add(labelOverviewStreetname2);
        labelsStreetname.add(labelOverviewStreetname3);
        labelsStreetname.add(labelOverviewStreetname4);
        labelsStreetname.add(labelOverviewStreetname5);
        return labelsStreetname;
    }

    public List<TextView> getLabelsOverviewDirection() {
        List<TextView> labelsDirection = new ArrayList<TextView>();
        labelsDirection.add(labelOverviewDirection1);
        labelsDirection.add(labelOverviewDirection2);
        labelsDirection.add(labelOverviewDirection3);
        labelsDirection.add(labelOverviewDirection4);
        labelsDirection.add(labelOverviewDirection5);
        return labelsDirection;
    }

    public List<TextView> getLabelsOverviewSpeed() {
        List<TextView> labelsSpeed = new ArrayList<TextView>();
        labelsSpeed.add(labelOverviewSpeed1);
        labelsSpeed.add(labelOverviewSpeed2);
        labelsSpeed.add(labelOverviewSpeed3);
        labelsSpeed.add(labelOverviewSpeed4);
        labelsSpeed.add(labelOverviewSpeed5);
        return labelsSpeed;
    }

    public List<TextView> getLabelsOverviewHeight() {
        List<TextView> labelsHeight = new ArrayList<TextView>();
        labelsHeight.add(labelOverviewHeight1);
        labelsHeight.add(labelOverviewHeight2);
        labelsHeight.add(labelOverviewHeight3);
        labelsHeight.add(labelOverviewHeight4);
        labelsHeight.add(labelOverviewHeight5);
        return labelsHeight;
    }
}
