package de.fernunihagen.dna.osm;

import org.osmdroid.bonuspack.overlays.PolygonOverlay;
import org.osmdroid.views.MapView;

import de.fernunihagen.dna.OutputActivity;
import de.fernunihagen.dna.hoese.DsplBase;
import de.fernunihagen.dna.hoese.QueryResult;

import android.app.Activity;
import android.content.Context;
import android.view.MotionEvent;


public class MyPolygonOverlay extends PolygonOverlay {
	private QueryResult queryResult;
	private int position; 
	private DsplBase dsplBase;
	private Activity activity;

	public MyPolygonOverlay(Activity activity, QueryResult queryResult, DsplBase dsplBase, int position, Context ctx) {
		super(ctx);
		
		this.activity = activity;
		this.position = position;
		this.queryResult = queryResult;
		this.dsplBase = dsplBase;
	}
	
	@Override
	public boolean onSingleTapUp(MotionEvent e, MapView mapView) {
		// Wie kann ich feststellen, dass mein Polygon getippt wurde?
//		boolean newSelected = !dsplBase.getSelected();
//		
//		queryResult.selectItem(position, newSelected);
//				
//		switchTabInActivity(1);

		return super.onSingleTapUp(e, mapView);
	}
	
	public void switchTabInActivity(int indexTabToSwitchTo){
        OutputActivity parentActivity;
        parentActivity = (OutputActivity) activity.getParent();
        parentActivity.switchTab(indexTabToSwitchTo);
}
	

}
