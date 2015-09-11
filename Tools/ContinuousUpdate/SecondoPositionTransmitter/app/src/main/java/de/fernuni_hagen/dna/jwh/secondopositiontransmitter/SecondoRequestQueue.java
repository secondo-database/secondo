package de.fernuni_hagen.dna.jwh.secondopositiontransmitter;

import android.content.Context;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.toolbox.HurlStack;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.Volley;

import java.util.LinkedList;
import java.util.Queue;
import java.util.Vector;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Created by Jerome on 14.02.2015.
 */
public class SecondoRequestQueue {

    private static SecondoRequestQueue instance;
    private static Context ctx;
    private RequestQueue requestQueue;
    private Queue<Request> cache;

    private SecondoRequestQueue(Context context) {
        ctx = context;
        requestQueue = getRequestQueue();
        cache = new LinkedBlockingQueue<Request>();
    }

    public static synchronized SecondoRequestQueue getInstance(Context context) {
        if (instance == null) {
            instance = new SecondoRequestQueue(context);
        }
        return instance;
    }

    public RequestQueue getRequestQueue() {
        if (requestQueue == null) {
            requestQueue = Volley.newRequestQueue(ctx.getApplicationContext(), new HurlStack());
        }
        return requestQueue;
    }

    public <T> void addToRequestQueue(Request req) {
        cache.offer(req);
    }

    public void dispatchRequestQueue() {
        while(!cache.isEmpty()){
            requestQueue.add(cache.remove());
        }
    }
}
