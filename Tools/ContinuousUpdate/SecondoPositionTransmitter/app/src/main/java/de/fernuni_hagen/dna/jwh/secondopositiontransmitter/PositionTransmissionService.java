package de.fernuni_hagen.dna.jwh.secondopositiontransmitter;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.support.v4.app.NotificationCompat;
import android.support.v4.content.LocalBroadcastManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.android.volley.NetworkResponse;
import com.android.volley.ParseError;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.HttpHeaderParser;
import com.android.volley.toolbox.JsonObjectRequest;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.UnsupportedEncodingException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

import de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation.ExtendedPosition;
import de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation.Position;
import de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation.TimeInterval;
import de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation.UPoint;

/**
 * Backgroundservice to handle Position updates
 */
public class PositionTransmissionService extends Service {

    private SharedPreferences prefs;
    private Handler mHandler = new Handler();
    private Timer timer;
    private String userName;
    private SecondoRequestQueue requestQueue;
    private LocalBroadcastManager broadcaster;
    private LocalLocationManager locationManager;
    private DatabaseManager db;
    private LogFileManager logfile;
    private Calendar lastSend;
    private Calendar lastPosition;
    private Integer sendInterval;
    private Integer logInterval;

    public static boolean isRunning = false;

    private final int NOTIFICATION_ID = 1704;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    /**
     * Inform the Activity about a new DB entry
     * @param message
     */
    public void sendMessage(String message) {
        Date time = Calendar.getInstance().getTime();
        db.insertLog(time, message);

        Intent intent = new Intent(this.getClass().getName());
        SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        if (message != null)
            intent.putExtra("message", df.format(time) + ": " + message);
        broadcaster.sendBroadcast(intent);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        isRunning = true;
        db = new DatabaseManager(getApplicationContext());
        logfile = new LogFileManager();
        broadcaster = LocalBroadcastManager.getInstance(this);
    }

    /**
     * Sets the Used to the configured username or the IMEI of the device
     * For Emulator usage a dynamic ID will be generated
     */
    private void setUser() {
        String user = prefs.getString("user", "");
        if (!user.equals("")) {
            userName = user;
        } else {
            if (Build.FINGERPRINT.startsWith("generic")) {
            /* Generate a random ID for the Emulator */
                userName = UUID.randomUUID().toString();
            } else {
                userName = ((TelephonyManager) getSystemService(TELEPHONY_SERVICE)).getDeviceId();
            }
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        isRunning = true;
        prefs = getApplicationContext().getSharedPreferences("configuration", MODE_PRIVATE);
        requestQueue = SecondoRequestQueue.getInstance(getApplicationContext());
        locationManager = new LocalLocationManager(this, prefs.getInt("logInterval", 60) * 1000);

        setUser();

        sendInterval = prefs.getInt("sendInterval", 60);
        lastSend = Calendar.getInstance();
        lastSend.add(Calendar.SECOND, sendInterval);
        Log.e(this.getClass().getSimpleName(), "Send interval" + sendInterval + "s");

        logInterval = prefs.getInt("logInterval", 60);
        lastPosition = Calendar.getInstance();
        lastPosition.add(Calendar.SECOND, logInterval);
        Log.e(this.getClass().getSimpleName(), "Update interval" + logInterval + "s");

        Toast.makeText(this, "Service Started " + prefs.getString("host", "no-host"), Toast.LENGTH_LONG).show();

        Intent notificationIntent = new Intent(this, MainActivity.class);
        notificationIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP |
                Intent.FLAG_ACTIVITY_SINGLE_TOP);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
        Notification notification = new NotificationCompat.Builder(this).setSmallIcon(R.mipmap.ic_launcher).setContentTitle(getText(R.string.Title)).setContentText(getText(R.string.NotificationText)).setOngoing(true).setContentIntent(pendingIntent).build();
        startForeground(NOTIFICATION_ID, notification);

        if (timer != null) {
            timer.cancel();
        }

        timer = new Timer();

        requestQueue.getRequestQueue().start();
        Log.e(this.getClass().getSimpleName(), "Schedule interval " + (Math.min(sendInterval, logInterval) * 1000) + "ms");
        timer.scheduleAtFixedRate(new PositionTimerTask(), 0, Math.min(sendInterval, logInterval) * 1000);
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        isRunning = false;
        timer.cancel();
        db.close();
        logfile.close();
        locationManager.close();
        Toast.makeText(this, "Service Destroyed", Toast.LENGTH_LONG).show();
    }

    /**
     * Returns a new JSON object ready for transfer
     * @return
     */
    private JSONObject getNewJsonObject() {
        Position position = new Position();
        ExtendedPosition ePosition = new ExtendedPosition();
        getPositionFromLocation(position, ePosition);
        logfile.write(ePosition.getSecondoValue());

        Gson g = new GsonBuilder().serializeNulls().setDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS").create();
        try {
            Log.d("JSON", g.toJson(position));
            return new JSONObject(g.toJson(position));
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * Creates a new Position to be used in the JSON-Request
     * @param position
     */
    private void getPositionFromLocation(Position position, ExtendedPosition ePosition) {
        LocalLocationManager.LocationInfo info = locationManager.getCurrentLocationInfo();

        position.Id = userName;
        position.Position = new UPoint();
        position.Position.interval = new TimeInterval();

        position.Position.x1 = info.start.getLongitude();
        position.Position.y1 = info.start.getLatitude();
        position.Position.interval.i1 = new Date(info.start.getTime());
        position.Position.interval.i1closed = true;

        position.Position.x2 = info.end.getLongitude();
        position.Position.y2 = info.end.getLatitude();
        position.Position.interval.i2 = new Date(info.end.getTime());
        position.Position.interval.i2closed = false;

        ePosition.Id = userName;
        ePosition.Position = position.Position;
        if(info.start.hasAltitude()){
            ePosition.altitude = info.start.getAltitude();
            ePosition.satellites = info.start.getExtras().getInt("satellites");
        }else if(info.end.hasAltitude()) {
            ePosition.altitude = info.end.getAltitude();
            ePosition.satellites = info.end.getExtras().getInt("satellites");
        }else{
            ePosition.altitude = locationManager.getCurrentAltitude();
            ePosition.satellites = 0;
        }
    }

    /**
     * TimerTask to handle periodic updates
     */
    private class PositionTimerTask extends TimerTask {

        String url;

        public PositionTimerTask() {
            url = "http://" + prefs.getString("host", "") + ":" + prefs.getString("port", "") + "/" + prefs.getString("relation", "");
            Log.i(getClass().getSimpleName(), "Response-URL:" + url);
        }

        @Override
        public void run() {

            if(Calendar.getInstance().compareTo(lastPosition) > 0){
                lastPosition = Calendar.getInstance();
                lastPosition.add(Calendar.SECOND, logInterval);
                createPositionUpdate();
            }

            if(Calendar.getInstance().compareTo(lastSend) > 0){
                lastSend = Calendar.getInstance();
                lastSend.add(Calendar.SECOND, sendInterval);
                requestQueue.dispatchRequestQueue();
            }

        }

        private void createPositionUpdate() {
            if (!locationManager.movementAvailable()) {
                Log.e(getClass().getSimpleName(), "No movement available");
                return;
            }

            if (prefs.getBoolean("goodLocation", true) && !locationManager.significantMovement()) {
                Log.d(getClass().getSimpleName(), "No significant movement!");
                return;
            } else {
            }

            JsonObjectRequest putRequest = new JsonObjectRequest(JsonObjectRequest.Method.PUT, url, getNewJsonObject(), new Response.Listener<JSONObject>() {
                @Override
                public void onResponse(JSONObject response) {
                    sendMessage("Message delivered");
                }
            }, new Response.ErrorListener() {
                @Override
                public void onErrorResponse(VolleyError error) {
                    sendMessage("-!- Transfer error -!-");
                }
            }) {
                @Override
                protected Response<JSONObject> parseNetworkResponse(NetworkResponse response) {
                    try {
                        if (response.statusCode == 204) {
                            return Response.success(new JSONObject("{\"status\":204}"), HttpHeaderParser.parseCacheHeaders(response));
                        } else {
                            String jsonString =
                                    new String(response.data, HttpHeaderParser.parseCharset(response.headers));
                            return Response.success(new JSONObject(jsonString),
                                    HttpHeaderParser.parseCacheHeaders(response));
                        }
                    } catch (UnsupportedEncodingException e) {
                        return Response.error(new ParseError(e));
                    } catch (JSONException je) {
                        return Response.error(new ParseError(je));
                    }
                }
            };

            requestQueue.addToRequestQueue(putRequest);
        }
    }
}
