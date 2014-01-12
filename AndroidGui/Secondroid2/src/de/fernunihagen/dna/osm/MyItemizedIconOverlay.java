package de.fernunihagen.dna.osm;

import java.util.ArrayList;
import java.util.List;

import org.osmdroid.DefaultResourceProxyImpl;
import org.osmdroid.views.overlay.ItemizedIconOverlay;
import org.osmdroid.views.overlay.ItemizedIconOverlay.OnItemGestureListener;
import org.osmdroid.views.overlay.Overlay;
import org.osmdroid.views.overlay.OverlayItem;

import de.fernunihagen.dna.R;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.Log;

public class MyItemizedIconOverlay extends ItemizedIconOverlay<OverlayItem> {
	private Drawable pointMarker;
	private Drawable pointMarkerSelected;
	private OsmMapView osmView;
	private static final String TAG = "de.fernunihagen.dna.osm.MyItemizedIconOverlay";


	public MyItemizedIconOverlay(
			OsmMapView osmView, Context pContext,
			List<OverlayItem> pList,
			org.osmdroid.views.overlay.ItemizedIconOverlay.OnItemGestureListener<OverlayItem> pOnItemGestureListener) {
		super(pContext, pList, pOnItemGestureListener);

		pointMarker = pContext.getResources()
				.getDrawable(R.drawable.mapscircle_red);
		pointMarkerSelected = pContext.getResources().getDrawable(
				R.drawable.mapscircle_green);
		this.osmView = osmView;

	}
	

	public MyItemizedIconOverlay(OsmMapView osmView, Context pContext,
			ArrayList<OverlayItem> pList,
			OnItemGestureListener<OverlayItem> pOnItemGestureListener,
			DefaultResourceProxyImpl resourceProxy) {
		super(pContext, pList, pOnItemGestureListener);

		pointMarker = pContext.getResources().getDrawable(R.drawable.mapscircle_red);
		pointMarkerSelected = pContext.getResources().getDrawable(
				R.drawable.mapscircle_green);
		this.osmView = osmView;
	}


	public void resetMarker() {
		
		for (Overlay overlay: osmView.getOverlays()) {
			if (overlay instanceof MyItemizedIconOverlay) {
				MyItemizedIconOverlay myItemizedIconOverlay = (MyItemizedIconOverlay) overlay;
				for (int index = 0; index < myItemizedIconOverlay.size(); ++index) {
					myItemizedIconOverlay.getItem(index).setMarker(pointMarker);
				}
			}
		}
	}
}
