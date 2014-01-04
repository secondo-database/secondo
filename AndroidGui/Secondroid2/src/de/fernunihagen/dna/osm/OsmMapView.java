package de.fernunihagen.dna.osm;

import org.osmdroid.util.BoundingBoxE6;
import org.osmdroid.util.GeoPoint;
import org.osmdroid.views.MapView;


public class OsmMapView extends MapView {

	private BoundingBoxE6 boundingBox;

	public BoundingBoxE6 getBoundingBox() {
		return boundingBox;
	}

	public void setBoundingBox(BoundingBoxE6 boundingBox) {
		this.boundingBox = boundingBox;
	}

	public OsmMapView(OsmMapActivity osmMapActivity, int i) {
		super(osmMapActivity, i);
	}

	@Override
	protected void onLayout(boolean arg0, int arg1, int arg2, int arg3, int arg4) {
		super.onLayout(arg0, arg1, arg2, arg3, arg4);

		// Now that we have laid out the map view,
		// zoom to any bounding box
		int zoomlevel = this.getZoomLevel();
		if (this.boundingBox != null) {
			zoomToBoundingBox(boundingBox);
			int lon = this.boundingBox.getLongitudeSpanE6();
			int lat = this.boundingBox.getLatitudeSpanE6();
			int max = Math.max(lon, lat);
			if (max < 1000000) {
				zoomlevel = 9;
			} else {
				if (max < 2000000) {
					zoomlevel = 8;
				} else {
					if (max < 4000000) {
						zoomlevel = 7;
					} else {
						if (max < 9000000) {
							zoomlevel = 6;
						} else {
							zoomlevel = 5;
						}
					}
				}
			}

			getController().setZoom(zoomlevel);
			getController().setCenter(
					new GeoPoint(boundingBox.getCenter().getLatitudeE6(), boundingBox.getCenter()
							.getLongitudeE6()));
			boundingBox = null;
		}
	}
}
