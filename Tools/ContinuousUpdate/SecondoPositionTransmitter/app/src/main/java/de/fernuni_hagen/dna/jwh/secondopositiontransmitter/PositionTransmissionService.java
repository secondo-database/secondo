package de.fernuni_hagen.dna.jwh.secondopositiontransmitter;

import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
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

import de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation.Position;
import de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation.TimeInterval;
import de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation.UPoint;

/**
 * Created by Jerome on 14.02.2015.
 */
public class PositionTransmissionService extends Service {

    private SharedPreferences prefs;
    private Handler mHandler = new Handler();
    private Timer timer;
    private String imei;
    private SecondoRequestQueue requestQueue;
    private LocalBroadcastManager broadcaster;
    private LocalLocationManager locationManager;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public void sendMessage(String message) {
        Intent intent = new Intent(this.getClass().getName());
        SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        if (message != null)
            intent.putExtra("message", df.format(Calendar.getInstance().getTime()) + ": " + message);
        broadcaster.sendBroadcast(intent);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        broadcaster = LocalBroadcastManager.getInstance(this);
        if (Build.FINGERPRINT.startsWith("generic")) {
            /* Generate a random ID for the Emulator */
            imei = UUID.randomUUID().toString();
        } else {
            imei = ((TelephonyManager) getSystemService(TELEPHONY_SERVICE)).getDeviceId();
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        prefs = getApplicationContext().getSharedPreferences("configuration", MODE_PRIVATE);
        requestQueue = SecondoRequestQueue.getInstance(getApplicationContext());
        locationManager = new LocalLocationManager(this, prefs.getInt("updateInterval", 60) * 1000);

        Toast.makeText(this, "Service Started " + prefs.getString("host", "no-host"), Toast.LENGTH_LONG).show();

        if (timer != null) {
            timer.cancel();
        }

        timer = new Timer();

        requestQueue.getRequestQueue().start();
        timer.scheduleAtFixedRate(new PositionTimerTask(), 0, prefs.getInt("updateInterval", 60) * 1000);
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        timer.cancel();
        Toast.makeText(this, "Service Destroyed", Toast.LENGTH_LONG).show();
    }

    private JSONObject getNewJsonObject() {
        Position position = new Position();

        getPositionFromLocation(position);

        Gson g = new GsonBuilder().serializeNulls().setDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS").create();
        try {
            Log.d("JSON", g.toJson(position));
            return new JSONObject(g.toJson(position));
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    private void getPositionFromLocation(Position position) {
        LocalLocationManager.LocationInfo info = locationManager.getCurrentLocationInfo();

        position.Id = imei;
        position.Position = new UPoint();
        position.Position.interval = new TimeInterval();

        position.Position.x1 = info.start.getLongitude();
        position.Position.y1 = info.start.getLongitude();
        position.Position.interval.i1 = new Date(info.start.getTime());
        position.Position.interval.i1closed = true;

        position.Position.x2 = info.end.getLongitude();
        position.Position.y2 = info.end.getLongitude();
        position.Position.interval.i2 = new Date(info.end.getTime());
        position.Position.interval.i2closed = true;
    }

    private class PositionTimerTask extends TimerTask {

        String url;

        public PositionTimerTask() {
            url = "http://" + prefs.getString("host", "") + ":" + prefs.getString("port", "") + "/" + prefs.getString("relation", "");
            Log.i(getClass().getSimpleName(), "Response-URL:" + url);
        }

        @Override
        public void run() {

            if (!locationManager.significantMovement()) {
                Log.d(getClass().getSimpleName(), "No significant movement!");
                return;
            } else {
                Log.d(getClass().getSimpleName(), "Significant movement!");
            }

            JsonObjectRequest putRequest = new JsonObjectRequest(JsonObjectRequest.Method.PUT, url, getNewJsonObject(), new Response.Listener<JSONObject>() {
                @Override
                public void onResponse(JSONObject response) {
                    sendMessage("Message delivered");
                }
            }, new Response.ErrorListener() {
                @Override
                public void onErrorResponse(VolleyError error) {
                    Log.e("RESPONSE-ERROR", error.toString());
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
