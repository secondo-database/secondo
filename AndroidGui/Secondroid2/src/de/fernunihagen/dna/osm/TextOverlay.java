package de.fernunihagen.dna.osm;

import java.util.Vector;
import org.osmdroid.views.MapView;
import org.osmdroid.views.overlay.Overlay;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.Rect;

/**
 * Special overlay for textoutput
 * TextOverlay ist used for the timeoutput of moving objects
 * @author Michael Küpper
 *
 */
public class TextOverlay extends Overlay {
	enum Position {
		TOP_LEFT, // absolute from left top corner
		BOTTOM_LEFT, // absolute from left bottom corner
		GEO
	};
	
	private Vector<TextItem> textItems;
	
        public TextOverlay(Context ctx) {
            super(ctx);
            textItems = new Vector<TextOverlay.TextItem>(10);
        }
        
		public void addText(int lat, int lon, String text) {
			textItems.add(new TextItem(text, lat, lon, Position.TOP_LEFT));
		}
		
		public void addText(int lat, int lon, String text, Position pos) {
			textItems.add(new TextItem(text, lat, lon, pos));
		}


        @Override
        protected void draw(Canvas canvas, MapView pOsmv, boolean shadow) {
            if (shadow)
                return;

            if (textItems.size() == 0) 
            	return;
            
            Paint textPaint;
            textPaint = new Paint();
            textPaint.setColor(Color.BLACK);
            textPaint.setAntiAlias(true);
            textPaint.setStyle(Style.FILL);
            textPaint.setStrokeWidth(1);
            textPaint.setTextAlign(Paint.Align.LEFT);
            textPaint.setTextSize(12);
            // Calculate the half-world size
            final Rect viewportRect = new Rect();
            final org.osmdroid.views.MapView.Projection projection = pOsmv.getProjection();

            // Save the Mercator coordinates of what is on the screen
            viewportRect.set(projection.getScreenRect());
            // DON'T set offset with either of below
            // viewportRect.offset(-mWorldSize_2, -mWorldSize_2);
            // viewportRect.offset(mWorldSize_2, mWorldSize_2);
            
			textPaint.setTextSize(18 * pOsmv.getResources().getDisplayMetrics().density);
			
			for (TextItem item : textItems) {
				switch (item.pos) {
				case TOP_LEFT:
					canvas.drawText(item.text, viewportRect.left + item.x, viewportRect.top + item.y, textPaint);
					break;
				case BOTTOM_LEFT: // noch nicht getestet!
					canvas.drawText(item.text, viewportRect.left +item.x, viewportRect.bottom - item.y, textPaint);
					break;
				case GEO: // noch nicht getestet!
					canvas.drawText(item.text, item.x, item.y, textPaint);
					break;
				}
			}
        }

        public void onProviderDisabled(String arg0)
        {
        }

        public void onProviderEnabled(String provider)
        {
        }
        
        private class TextItem {
        	String text;
        	int x, y;
        	Position pos;
        	
        	public TextItem(String text, int x, int y, Position pos) {
        		this.x = x;
        		this.y = y;
        		this.text = text;
        		this.pos = pos;
        	}
        	
        	}
        }

