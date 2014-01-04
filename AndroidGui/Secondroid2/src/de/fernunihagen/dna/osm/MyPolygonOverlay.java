package de.fernunihagen.dna.osm;

import org.osmdroid.bonuspack.overlays.PolygonOverlay;
import org.osmdroid.views.MapView;

import android.content.Context;
import android.view.MotionEvent;


public class MyPolygonOverlay extends PolygonOverlay {

	public MyPolygonOverlay(Context ctx) {
		super(ctx);
	}
	
	@Override
	public boolean onSingleTapUp(MotionEvent e, MapView mapView) {
		return super.onSingleTapUp(e, mapView);
	}
	
	

}
