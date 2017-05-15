package de.fernunihagen.dna.mapmatching;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;
import android.support.annotation.RequiresApi;

public class Dialogs {

    private static MapMatchingActivity mapMatchingActivity;
    private static SettingsAdvancedActivity settingsAdvancedActivity;
    private static SettingsGpsActivity settingsGpsActivity;

    private static AlertDialog alertDialogCloseApp;
    private static AlertDialog alertDialogGpsDisabled;
    private static AlertDialog alertDialogConnectionFailed;
    private static AlertDialog alertDialogResetAdvancedSettings;
    private static AlertDialog alertDialogResetGpsSettings;
    private static AlertDialog alertDialogPermissions;
    private static AlertDialog alertDialogResetData;

    public static void showDialogCloseApp() {
        AlertDialog.Builder alertBuilder = new AlertDialog.Builder(mapMatchingActivity);
        alertBuilder.setMessage("Do you want to close Secondo Map Matching?");
        alertBuilder.setCancelable(true);

        alertBuilder.setPositiveButton(
                "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                        MapMatchingCoreInterface.setExitApp(true);
                        mapMatchingActivity.mapMatchingStop();
                    }
                });

        alertBuilder.setNegativeButton(
                "Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });

        alertDialogCloseApp = alertBuilder.create();
        alertDialogCloseApp.show();
    }

    public static void showDialogGpsDisabled() {
        AlertDialog.Builder alertBuilder = new AlertDialog.Builder(mapMatchingActivity);
        alertBuilder.setMessage("GPS has to be enabled to start Map Matching. Open Settings?");
        alertBuilder.setCancelable(true);

        alertBuilder.setPositiveButton(
                "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
                        mapMatchingActivity.startActivity(intent);
                        dialog.cancel();
                    }
                });

        alertBuilder.setNegativeButton(
                "Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });

        alertDialogGpsDisabled = alertBuilder.create();
        alertDialogGpsDisabled.show();
    }

    public static void showDialogConncetionFailed(){
        AlertDialog.Builder alertBuilder = new AlertDialog.Builder(mapMatchingActivity);
        alertBuilder.setMessage("Connection to Secondo Server failed. A Secondo Server with a database containing map data is required to connect. Open Server Settings?");
        alertBuilder.setCancelable(true);
        Intent intent = new Intent(mapMatchingActivity, SettingsServerActivity.class);

        alertBuilder.setPositiveButton(
                "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        mapMatchingActivity.startActivity(intent);
                        dialog.cancel();
                    }
                });

        alertBuilder.setNegativeButton(
                "Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });

        alertDialogConnectionFailed = alertBuilder.create();
        alertDialogConnectionFailed.show();
    }

    public static void showDialogResetAdvancedSettings() {
        AlertDialog.Builder alertBuilder = new AlertDialog.Builder(settingsAdvancedActivity);
        alertBuilder.setMessage("Do you want to reset all Advanced Settings to default values?");
        alertBuilder.setCancelable(true);

        alertBuilder.setPositiveButton(
                "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        settingsAdvancedActivity.resetAdvancedSettings();
                        dialog.cancel();
                    }
                });

        alertBuilder.setNegativeButton(
                "Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });

        alertDialogResetAdvancedSettings = alertBuilder.create();
        alertDialogResetAdvancedSettings.show();
    }

    public static void showDialogResetGpsSettings() {
        AlertDialog.Builder alertBuilder = new AlertDialog.Builder(settingsGpsActivity);
        alertBuilder.setMessage("Do you want to reset all GPS Settings to default values?");
        alertBuilder.setCancelable(true);

        alertBuilder.setPositiveButton(
                "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        settingsGpsActivity.resetGpsSettings();
                        dialog.cancel();
                    }
                });

        alertBuilder.setNegativeButton(
                "Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });

        alertDialogResetGpsSettings = alertBuilder.create();
        alertDialogResetGpsSettings.show();
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR1)
    public static void showDialogPermissions() {
        AlertDialog.Builder alertBuilder = new AlertDialog.Builder(mapMatchingActivity);
        alertBuilder.setMessage("\"Never ask again\" was selected at a permission request. All requested permissions are required. Clear app data or uninstall and reinstall the app to repeat permission check. Open app settings?");
        alertBuilder.setCancelable(true);

        alertBuilder.setPositiveButton(
                "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        try{
                            Intent intent = new Intent(android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
                            Uri uri = Uri.fromParts("package", mapMatchingActivity.getPackageName(), null);
                            intent.setData(uri);
                            mapMatchingActivity.startActivity(intent);
                            mapMatchingActivity.finish();
                            System.exit(0);
                        }catch(Exception e){
                            Intent intent = new Intent(android.provider.Settings.ACTION_MANAGE_APPLICATIONS_SETTINGS);
                            mapMatchingActivity.startActivity(intent);
                            mapMatchingActivity.finish();
                            System.exit(0);
                        }
                        dialog.cancel();
                    }
                });

        alertBuilder.setNegativeButton(
                "Close App",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        mapMatchingActivity.finish();
                        System.exit(0);
                        dialog.cancel();
                    }
                });

        alertBuilder.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                mapMatchingActivity.finish();
                System.exit(0);
            }
        });

        alertDialogPermissions = alertBuilder.create();
        alertDialogPermissions.show();
    }

    public static void showDialogResetData() {
        AlertDialog.Builder alertBuilder = new AlertDialog.Builder(mapMatchingActivity);
        alertBuilder.setMessage("Do you want to reset collected Data?");
        alertBuilder.setCancelable(true);

        alertBuilder.setPositiveButton(
                "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                        MapMatchingCoreInterface.clearData();
                    }
                });

        alertBuilder.setNegativeButton(
                "Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });

        alertDialogResetData = alertBuilder.create();
        alertDialogResetData.show();
    }

    public static boolean isDialogCloseAppActive() {
        if (alertDialogCloseApp != null) {
            return alertDialogCloseApp.isShowing();
        }
        return false;
    }

    public static boolean isDialogGpsDisabledActive() {
        if (alertDialogGpsDisabled != null) {
            return alertDialogGpsDisabled.isShowing();
        }
        return false;
    }
    public static boolean isDialogConncetionFailedActive(){
        if(alertDialogConnectionFailed !=null){
            return alertDialogConnectionFailed.isShowing();
        }
        return false;
    }

    public static boolean isDialogResetAdvancedSettingsActive() {
        if (alertDialogResetAdvancedSettings != null) {
            return alertDialogResetAdvancedSettings.isShowing();
        }
        return false;
    }

    public static boolean isDialogResetGpsSettingsActive() {
        if (alertDialogResetGpsSettings != null) {
            return alertDialogResetGpsSettings.isShowing();
        }
        return false;
    }

    public static boolean isDialogPermissionsActive() {
        if (alertDialogPermissions != null) {
            return alertDialogPermissions.isShowing();
        }
        return false;
    }

    public static boolean isDialogResetDataActive() {
        if (alertDialogResetData != null) {
            return alertDialogResetData.isShowing();
        }
        return false;
    }

    public static void closeDialogCloseApp() {
        if (alertDialogCloseApp != null) {
            if (alertDialogCloseApp.isShowing()) {
                alertDialogCloseApp.dismiss();
            }
        }
    }

    public static void closeDialogGpsDisabled() {
        if (alertDialogGpsDisabled != null) {
            if (alertDialogGpsDisabled.isShowing()) {
                alertDialogGpsDisabled.dismiss();
            }
        }
    }

    public static void closeDialogConncetionFailed(){
        if(alertDialogConnectionFailed !=null){
            if(alertDialogConnectionFailed.isShowing()){
                alertDialogConnectionFailed.dismiss();
            }
        }
    }

    public static void closeDialogResetAdvancedSettings() {
        if (alertDialogResetAdvancedSettings != null) {
            if (alertDialogResetAdvancedSettings.isShowing()) {
                alertDialogResetAdvancedSettings.dismiss();
            }
        }
    }

    public static void closeDialogResetGpsSettings() {
        if (alertDialogResetGpsSettings != null) {
            if (alertDialogResetGpsSettings.isShowing()) {
                alertDialogResetGpsSettings.dismiss();
            }
        }
    }

    public static void closeDialogResetData() {
        if (alertDialogResetData != null) {
            if (alertDialogResetData.isShowing()) {
                alertDialogResetData.dismiss();
            }
        }
    }

    public static void setMapMatchingActivity(MapMatchingActivity mapMatchingActivity) {
        Dialogs.mapMatchingActivity = mapMatchingActivity;
    }

    public static void setSettingsAdvancedActivity(SettingsAdvancedActivity settingsAdvancedActivity) {
        Dialogs.settingsAdvancedActivity = settingsAdvancedActivity;
    }

    public static void setSettingsGpsActivity(SettingsGpsActivity settingsGpsActivity) {
        Dialogs.settingsGpsActivity = settingsGpsActivity;
    }
}
