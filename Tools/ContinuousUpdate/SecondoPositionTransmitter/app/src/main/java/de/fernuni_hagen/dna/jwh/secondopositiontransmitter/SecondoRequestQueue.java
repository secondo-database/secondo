package de.fernuni_hagen.dna.jwh.secondopositiontransmitter;

import android.content.Context;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.toolbox.HurlStack;
import com.android.volley.toolbox.Volley;

/**
 * Created by Jerome on 14.02.2015.
 */
public class SecondoRequestQueue {

    private static SecondoRequestQueue instance;
    private static Context ctx;
    private RequestQueue requestQueue;

    private SecondoRequestQueue(Context context) {
        ctx = context;
        requestQueue = getRequestQueue();
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

    public <T> void addToRequestQueue(Request<T> req) {
        getRequestQueue().add(req);
    }
}
