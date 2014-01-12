package de.fernunihagen.dna.osm;

import javamini.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import android.os.Bundle;
import android.util.Log;
import android.util.TypedValue;
import android.view.Menu;
import org.osmdroid.DefaultResourceProxyImpl;
import org.osmdroid.ResourceProxy;
import de.fernunihagen.dna.CommandActivity;
import de.fernunihagen.dna.R;
import de.fernunihagen.dna.ResourceProxyImpl;
import de.fernunihagen.dna.constants.OpenStreetMapConstants;
import org.osmdroid.api.IMapController;
import org.osmdroid.tileprovider.tilesource.ITileSource;
import org.osmdroid.tileprovider.tilesource.TileSourceFactory;
import org.osmdroid.tileprovider.util.CloudmadeUtil;
import org.osmdroid.util.BoundingBoxE6;
import org.osmdroid.util.GeoPoint;
import org.osmdroid.views.overlay.ItemizedIconOverlay;
import org.osmdroid.views.overlay.MinimapOverlay;
import org.osmdroid.views.overlay.Overlay;
import org.osmdroid.views.overlay.OverlayItem;
import org.osmdroid.views.overlay.PathOverlay;
import org.osmdroid.views.overlay.ScaleBarOverlay;
import org.osmdroid.views.overlay.SimpleLocationOverlay;
import org.osmdroid.bonuspack.overlays.ExtendedOverlayItem;
import org.osmdroid.bonuspack.overlays.PolygonOverlay;
import sj.lang.ListExpr;
import de.fernunihagen.dna.hoese.Category;
import de.fernunihagen.dna.hoese.CurrentState;
import de.fernunihagen.dna.hoese.DsplBase;
import de.fernunihagen.dna.hoese.DsplGraph;
import de.fernunihagen.dna.hoese.Interval;
import de.fernunihagen.dna.hoese.Layer;
import de.fernunihagen.dna.hoese.QueryResultHelper;
import de.fernunihagen.dna.hoese.Viewer;
import javamini.awt.geom.Ellipse2D;
import javamini.awt.geom.GeneralPath;
import javamini.awt.geom.PathIterator;
import de.fernunihagen.dna.hoese.LEUtils;
import de.fernunihagen.dna.hoese.QueryResult;
import javamini.awt.Shape;
import javamini.awt.geom.Area;
import de.fernunihagen.dna.hoese.algebras.DisplayGraph;
import de.fernunihagen.dna.osm.TextOverlay.Position;
import android.app.backup.RestoreObserver;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.drawable.Drawable;
import android.location.Location;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.RelativeLayout.LayoutParams;
import android.widget.SeekBar.OnSeekBarChangeListener;

/**
 * OsmMapActivity copy from the Samples on http://code.google.com/p/osmdroid/ 
 * and extended with Secondroid Features
 * Shows the Resuls on PathOverlay or PolygonOverlay
 * @author Michael Küpper
 *
 */
public class OsmMapActivity extends OsmActivity implements
		OpenStreetMapConstants, OnClickListener, Viewer {

	// ===========================================================
	// Constants
	// ===========================================================

	private static final int MENU_ZOOMIN_ID = Menu.FIRST;
	private static final int MENU_ZOOMOUT_ID = MENU_ZOOMIN_ID + 1;
	private static final int MENU_TILE_SOURCE_ID = MENU_ZOOMOUT_ID + 1;
	private static final int MENU_ANIMATION_ID = MENU_TILE_SOURCE_ID + 1;
	private static final int MENU_MINIMAP_ID = MENU_ANIMATION_ID + 1;
	private static final int MENU_SHOWRESULT_ID = MENU_MINIMAP_ID + 1;
	private static final String TAG = "de.fernunihagen.dna.osm.OsmMapActivity";

	// ===========================================================
	// Fields
	// ===========================================================

	private OsmMapView mOsmv;
	private IMapController mOsmvController;
	private SimpleLocationOverlay mMyLocationOverlay;
	private ResourceProxy mResourceProxy;
	private ScaleBarOverlay mScaleBarOverlay;
	private MinimapOverlay mMiniMapOverlay;
	private Drawable pointMarker;
	private Drawable pointMarkerSelected;
	private Rectangle2D.Double bound;

	private QueryResult queryResult;
	private List<QueryResult> queryResults;
	private ScheduledExecutorService scheduleTaskExecutor;
	private boolean running = false;
	private boolean showResult = true;
	private long period = 1000;
	private Layer lay;
	private String actTimeString;
	private boolean dirty = false;
	private boolean timelineOnTop = true;


	/** Called when the activity is first created. */
	@SuppressWarnings("unchecked")
	@Override
	public void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState, false); // Pass true here to actually
													// contribute to OSM!
		Intent intent = getIntent();
		this.queryResult = (QueryResult) intent
				.getSerializableExtra(CommandActivity.EXTRA_RESULT);
		this.queryResult.addViewer(this);

		this.queryResults = (List<QueryResult>) intent
				.getSerializableExtra(CommandActivity.EXTRA_RESULTS);
		
		scheduleTaskExecutor = Executors.newScheduledThreadPool(5);

		mResourceProxy = new ResourceProxyImpl(getApplicationContext());

		pointMarker = this.getResources()
				.getDrawable(R.drawable.mapscircle_red);
		pointMarkerSelected = this.getResources().getDrawable(
				R.drawable.mapscircle_green);

		final RelativeLayout rl = new RelativeLayout(this);

		CloudmadeUtil.retrieveCloudmadeKey(getApplicationContext());

		this.mOsmv = new OsmMapView(this, 256);
		this.mOsmvController = this.mOsmv.getController();
		rl.addView(this.mOsmv, new RelativeLayout.LayoutParams(
				LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));

		this.mOsmv.setMultiTouchControls(true);
		this.mOsmv.setBuiltInZoomControls(true);

		/* Scale Bar Overlay */
		{
			this.mScaleBarOverlay = new ScaleBarOverlay(this);
			this.mOsmv.getOverlays().add(mScaleBarOverlay);
			// Scale bar tries to draw as 1-inch, so to put it in the top
			// center, set x offset to
			// half screen width, minus half an inch.
			this.mScaleBarOverlay.setScaleBarOffset(getResources()
					.getDisplayMetrics().widthPixels
					/ 2
					- getResources().getDisplayMetrics().xdpi / 3, 10);
		}

		/* SingleLocation-Overlay */
		{
			/*
			 * Create a static Overlay showing a single location. (Gets updated
			 * in onLocationChanged(Location loc)!
			 */
			this.mMyLocationOverlay = new SimpleLocationOverlay(this);
			// mResourceProxy);
			this.mOsmv.getOverlays().add(mMyLocationOverlay);
		}

		/* MiniMap */
		{
			mMiniMapOverlay = new MinimapOverlay(this,
					mOsmv.getTileRequestCompleteHandler());
			this.mOsmv.getOverlays().add(mMiniMapOverlay);
		}

		this.setContentView(rl);
		int pixel = (int) TypedValue.applyDimension(
				TypedValue.COMPLEX_UNIT_DIP, 12, getResources()
						.getDisplayMetrics());

		RelativeLayout.LayoutParams lpButtonNext = new RelativeLayout.LayoutParams(
				LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		// align the Seekbar at the bottom of the parent
		lpButtonNext.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
		lpButtonNext.addRule(timelineOnTop ? RelativeLayout.ALIGN_PARENT_TOP : RelativeLayout.ALIGN_PARENT_BOTTOM);

		// Create the play button for timed objects
		ImageButton buttonPlay = new ImageButton(this, null,
				android.R.attr.buttonStyleSmall);
		buttonPlay.setAdjustViewBounds(true);
		buttonPlay.setId(R.id.play);
		buttonPlay.setImageResource(R.drawable.mediaplay);
		buttonPlay.setMaxHeight(pixel * 3);
		rl.addView(buttonPlay, lpButtonNext);
		buttonPlay.setOnClickListener(this);

		// Create the slower button for timed objects
		RelativeLayout.LayoutParams lpSlower = new RelativeLayout.LayoutParams(
				LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		lpSlower.addRule(RelativeLayout.ALIGN_BOTTOM, R.id.play);
		lpSlower.addRule(RelativeLayout.RIGHT_OF, R.id.play);
		lpSlower.setMargins(0, 0, 0, 0);
		ImageButton buttonSlower = new ImageButton(this, null,
				android.R.attr.buttonStyleSmall);
		buttonSlower.setAdjustViewBounds(true);
		buttonSlower.setId(R.id.slower);
		buttonSlower.setImageResource(R.drawable.mediaslower);
		buttonSlower.setMaxHeight(pixel * 3);
		rl.addView(buttonSlower, lpSlower);
		buttonSlower.setOnClickListener(this);

		// Create the faster button for timed objects
		RelativeLayout.LayoutParams lpFaster = new RelativeLayout.LayoutParams(
				LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		lpFaster.addRule(RelativeLayout.ALIGN_BOTTOM, R.id.play);
		lpFaster.addRule(RelativeLayout.RIGHT_OF, R.id.slower);
		lpFaster.setMargins(0, 0, 0, 0);
		ImageButton buttonFaster = new ImageButton(this, null,
				android.R.attr.buttonStyleSmall);
		buttonFaster.setAdjustViewBounds(true);
		buttonFaster.setId(R.id.faster);
		buttonFaster.setImageResource(R.drawable.mediafastforward);
		buttonFaster.setMaxHeight(pixel * 3);
		rl.addView(buttonFaster, lpFaster);
		buttonFaster.setOnClickListener(this);

		// Create the timeline slider for timed objects
		RelativeLayout.LayoutParams lpSeekBar = new RelativeLayout.LayoutParams(
				LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
		// align the Seekbar at the bottom of the parent
		lpSeekBar.addRule(RelativeLayout.ALIGN_BOTTOM, R.id.faster);
		lpSeekBar.addRule(RelativeLayout.RIGHT_OF, R.id.faster);

		SeekBar timeSeekBar = new SeekBar(this);
		// timeSeekBar.setAlpha(0.5f);
		timeSeekBar.setId(R.id.timeSeekBar);
		timeSeekBar.setPadding(0, 0, pixel, pixel);
		timeSeekBar.setMax(QueryResultHelper.getMaxIntervalCount(queryResults));
		timeSeekBar.setProgress(1);
		timeSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {

				setNewTime(progress, seekBar.getMax());
			}

			public void onStartTrackingTouch(SeekBar seekBar) {}

			public void onStopTrackingTouch(SeekBar seekBar) {}
		});
		rl.addView(timeSeekBar, lpSeekBar);

		mOsmv.setMaxZoomLevel(null);
		mOsmv.setMinZoomLevel(null);

		lay = new Layer(queryResult.getGraphObjects(), null);

		Interval boundingInter = QueryResultHelper.getBoundingInterval(queryResults);

		if (boundingInter != null) {
			// Set the time to the starttime of the bounding Intervall
			CurrentState.setActualTime(boundingInter.getStart());
		}

		timeSeekBar.setVisibility(boundingInter == null ? ProgressBar.INVISIBLE
				: ProgressBar.VISIBLE);
		buttonPlay.setVisibility(boundingInter == null ? Button.INVISIBLE
				: Button.VISIBLE);
		buttonFaster.setVisibility(boundingInter == null ? Button.INVISIBLE
				: Button.VISIBLE);
		buttonSlower.setVisibility(boundingInter == null ? Button.INVISIBLE
				: Button.VISIBLE);

		drawQueryResult();

		if (bound != null) {
			BoundingBoxE6 boundingBox = new BoundingBoxE6(bound.y
					+ bound.height, bound.x + bound.width, bound.y, bound.x);
			mOsmv.setBoundingBox(boundingBox); // don't work correctly! Only zoomlevel 0
		}

	}

	@Override
	protected void onStop() {
		Log.w(TAG, "onStop");
		stopTimer();
		super.onStop();
	}

	protected void onPause() {
		Log.w(TAG, "onPause");
		stopTimer();
		super.onPause();
	}

	@Override
	protected void onResume() {
		Log.w(TAG, "onResume");
		super.onResume();
		if (dirty)
			drawQueryResult();
	}
	
	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		// TODO: Move to the selected element
		
		super.onWindowFocusChanged(hasFocus);
	}

	private void TimerMethod() {
		// This method is called directly by the timer
		// and runs in the same thread as the timer.

		// We call the method that will work with the UI
		// through the runOnUiThread method.
		this.runOnUiThread(Timer_Tick);
	}

	private Runnable Timer_Tick = new Runnable() {
		public void run() {
			// This method runs in the same thread as the UI.
			// view.setNextTime();
			if (running) {
				SeekBar seekBar = (SeekBar) findViewById(R.id.timeSeekBar);
				seekBar.setProgress(seekBar.getProgress() + 1); // trigger the
																// timer in the
																// SeekBar
																// ProgressChange
																// Event

				// on end reset the Seekbar
				if (seekBar.getProgress() == seekBar.getMax()) {
					seekBar.setProgress(1);
					stopTimer();
				}
			}
		}
	};

	private void setNewTime(int progress, int progressMaxRange) {
		if (this.queryResult == null) {
			return;
		}

		Interval interval = QueryResultHelper.getBoundingInterval(queryResults);
		if (interval == null) // not a Timed Queryresult
			return;

		double actTime = interval.getStart()
				+ (interval.getEnd() - interval.getStart()) * progress
				/ progressMaxRange;
		CurrentState.setActualTime(actTime);
		actTimeString = LEUtils.convertTimeToString(actTime);
		drawQueryResult();

		Log.w(TAG, "setNewTime " + progress);
	}

	private void drawQueryResult() {
		this.mOsmv.getOverlays().clear(); // remove all Overlays
		if (showResult) {
			this.mOsmv.getOverlays().addAll(drawResult());
		}

		if (actTimeString != null) {
			TextOverlay textOverlay = new TextOverlay(getApplicationContext());
			textOverlay.addText(calcIndependentPixel(30),
					calcIndependentPixel(50), actTimeString,
					timelineOnTop ?  Position.TOP_LEFT : Position.BOTTOM_LEFT);
			this.mOsmv.getOverlays().add(textOverlay);
		}

		this.mOsmv.invalidate();
		dirty = false;
	}

	private int calcIndependentPixel(float radius) {
		final float scale = getResources().getDisplayMetrics().density;
		// Convert the dps to pixels, based on density scale
		return (int) ((radius * scale + 0.5f));
	}

	private ArrayList<Overlay> drawResult() {
		ArrayList<Overlay> overlays = new ArrayList<Overlay>();
		bound = null;

		for (QueryResult actQueryResult : queryResults) {
			if (actQueryResult.isSelected() == false) // Don't show this result
				continue;
			Category cat = actQueryResult.getCategory();
			if (cat == null)
				cat = Category.getDefaultCat();

			//ListExpr resultList = actQueryResult.getResultList();

			// relation
			List<DsplBase> results = actQueryResult.getEntries();
			int index = 0;
			for (DsplBase obj : results) {
				if (obj instanceof DisplayGraph) {
					DisplayGraph dsplGraph = (DisplayGraph) obj;
					dsplGraph.setCategory(cat);
					javamini.awt.geom.AffineTransform af = CurrentState.transform;

					if (bound == null) {
						bound = dsplGraph.getBounds();
					} else {
						Rectangle2D.Double.union(bound, dsplGraph.getBounds(),
								bound);
					}

					dsplGraph.setLayer(lay);
					Shape shape = dsplGraph.getRenderObject(0, af);
					if (shape instanceof Area) {
						Area area = (Area) shape;
						if (dsplGraph.isLineType(0)) {
							overlays.addAll(drawLine(area, dsplGraph));
						} else {
							overlays.addAll(drawPolygon(area, dsplGraph));
						}

					}
					if (shape instanceof Ellipse2D) {
						if (dsplGraph.isPointType(0)) {
							String label = dsplGraph.getLabelText(0);

							overlays.addAll(drawPoint(actQueryResult, index, (Ellipse2D) shape,
									dsplGraph, !dsplGraph.isPointType(0), label));
						} else {
							overlays.addAll(drawPolygon(shape, dsplGraph));

						}
					}

					if (shape instanceof GeneralPath) {
						overlays.addAll(drawLine(shape, dsplGraph));
					}
				}
				++index;
			}
		}

		return overlays;
	}

	private ArrayList<Overlay> drawPolygon(Shape shape, DsplGraph dsplGraph) {
		ArrayList<Overlay> pathOverlays = new ArrayList<Overlay>();
		// Ob ein neues Overlay angelegt werden muss oder nicht, ist abhängig,
		// ob die neue Line eine Fläche (geschlossen) ist
		// und ob diese in der anderen Fläche enthalten ist (Ausschnitt)

		PolygonOverlay pathOverlay = null;
		PathIterator pathIterator = shape.getPathIterator(null);
		while (pathIterator.isDone() == false) {
			double[] coordinates = new double[6];
			int type = pathIterator.currentSegment(coordinates);
			GeoPoint p = new GeoPoint(coordinates[1], coordinates[0]);

			if (type == PathIterator.SEG_MOVETO) {
				// neues Overlay erzeugen
				pathOverlay = createPolygonOverlay(dsplGraph);
				pathOverlays.add(pathOverlay);
				pathOverlay.addPoint(p.getLatitudeE6(), p.getLongitudeE6());
			}
			if (type == PathIterator.SEG_LINETO) {
				pathOverlay.addPoint(p.getLatitudeE6(), p.getLongitudeE6());
			}
			if (type == PathIterator.SEG_CLOSE) {
				// nothing todo
			}
			if (type == PathIterator.SEG_CUBICTO) {
				Log.i(TAG, "SEG_CUBICTO");
//				GeoPoint cp = new GeoPoint(coordinates[5], coordinates[6]);
				pathOverlay.addPoint(p.getLatitudeE6(), p.getLongitudeE6());
			}

			pathIterator.next();
		}

		Rectangle2D bounce = shape.getBounds2D();
		mOsmv.getController().animateTo(
				new GeoPoint(bounce.getCenterY(), bounce.getCenterX()));

		return pathOverlays;
	}

	private ArrayList<Overlay> drawLine(Shape shape, DsplGraph dsplGraph) {
		ArrayList<Overlay> pathOverlays = new ArrayList<Overlay>();
		// GeneralPath gp = (GeneralPath) shape;

		// Ob ein neues Overlay angelegt werden muss oder nicht, ist abhängig,
		// ob die neue Line eine Fläche (geschlossen) ist
		// und ob diese in der anderen Fläche enthalten ist (Ausschnitt)

		PathOverlay pathOverlay = null;
		PathIterator pathIterator = shape.getPathIterator(null);
		while (pathIterator.isDone() == false) {
			double[] coordinates = new double[6];
			int type = pathIterator.currentSegment(coordinates);
			GeoPoint p = new GeoPoint(coordinates[1], coordinates[0]);

			if (type == PathIterator.SEG_MOVETO) {
				// neues Overlay erzeugen
				pathOverlay = createPathOverlay(dsplGraph);
				pathOverlays.add(pathOverlay);
				pathOverlay.addPoint(p);
			}
			if (type == PathIterator.SEG_LINETO) {
				pathOverlay.addPoint(p);
			}
			if (type == PathIterator.SEG_CLOSE) {
				// nothing todo
			}
			if (type == PathIterator.SEG_CUBICTO) {
				Log.i(TAG, "SEG_CUBICTO");
//				GeoPoint cp = new GeoPoint(coordinates[5], coordinates[6]);
				pathOverlay.addPoint(p);
			}

			pathIterator.next();
		}

		Rectangle2D bounce = shape.getBounds2D();
		mOsmv.getController().animateTo(
				new GeoPoint(bounce.getCenterY(), bounce.getCenterX()));

		return pathOverlays;
	}

	private ArrayList<ItemizedIconOverlay<OverlayItem>> drawPoint(
			QueryResult qr, int position, Ellipse2D shape, DsplGraph dsplGraph, boolean lineType, String label) {
		ArrayList<OverlayItem> items = new ArrayList<OverlayItem>();

		Rectangle2D bounce = shape.getBounds2D();
		ExtendedOverlayItem markerOverlayItem = new ExtendedOverlayItem(label,
				label, new GeoPoint(bounce.getCenterY(), bounce.getCenterX()),
				this.getApplicationContext());

		markerOverlayItem
				.setMarker(dsplGraph.getSelected() ? pointMarkerSelected
						: pointMarker);
		markerOverlayItem.setDescription("Desc");
		markerOverlayItem.setSubDescription("subdesc");
		markerOverlayItem.setMarkerHotspot(OverlayItem.HotspotPlace.CENTER);

		items.add(markerOverlayItem);

		if (dsplGraph.getSelected()) {
			// go to the Center of the elements
			mOsmv.getController().animateTo(
					new GeoPoint(bounce.getCenterY(), bounce.getCenterX()));
		}
		DefaultResourceProxyImpl resourceProxy = new DefaultResourceProxyImpl(
				getApplicationContext());

		MyItemizedIconOverlay currentLocationOverlay = new MyItemizedIconOverlay(this.mOsmv, getApplicationContext(),
				items,
				new ItemizedIconOverlay.OnItemGestureListener<OverlayItem>() {
					public boolean onItemSingleTapUp(final int index,
							final OverlayItem item) {

						if (item instanceof ExtendedOverlayItem) {
							Holder holder = (Holder) ((ExtendedOverlayItem) item)
									.getRelatedObject();
							boolean newSelected = !holder.dsplBase.getSelected();
//							int lastActSelected = holder.queryResult.getActSelected();
							
							holder.queryResult.selectItem(holder.position, newSelected);
							//	holder.dsplBase.setSelected(newSelected);
							
							// reset the other Marker
							holder.overlay.resetMarker();

							item.setMarker(newSelected ? pointMarkerSelected
									: pointMarker);
							
							mOsmv.invalidate();
						}
						return true;
					}

					public boolean onItemLongPress(final int index,
							final OverlayItem item) {
						return true;
					}
				}, resourceProxy);

		Holder holder = new Holder(qr, position, dsplGraph, currentLocationOverlay);
		markerOverlayItem.setRelatedObject(holder);

		ArrayList<ItemizedIconOverlay<OverlayItem>> overlays = new ArrayList<ItemizedIconOverlay<OverlayItem>>();
		overlays.add(currentLocationOverlay);


		return overlays;
	}

	private PathOverlay createPathOverlay(DsplGraph dsplGraph) {
		Category cat = dsplGraph.getCategory();

		int color = (dsplGraph != null && dsplGraph.getSelected()) 
				? cat.getSelectedColor() : cat.getColor();
		// Create color with alpha
		int argbcolor = Color.argb(cat.getAlpha(), Color.red(color),
				Color.green(color), Color.blue(color));

		PathOverlay pathOverlay = new PathOverlay(color, this);
		Paint pPaint = createPaint(cat, argbcolor, pathOverlay.getPaint());
		pathOverlay.setPaint(pPaint);
		return pathOverlay;
	}

	private Paint createPaint(Category cat, int color, Paint paint) {
		paint.setStrokeWidth(cat.getStrokeWidth());
		paint.setStyle(Paint.Style.STROKE);
		paint.setAlpha(cat.getAlpha());
		paint.setColor(color);
		return paint;
	}

	private PolygonOverlay createPolygonOverlay(DsplGraph dsplGraph) {
		Category cat = dsplGraph.getCategory();

		int color = (dsplGraph != null && dsplGraph.getSelected()) ? cat
				.getSelectedColor() : cat.getColor();
		// Create color with alpha
		int argbcolor = Color.argb(cat.getAlpha(), Color.red(color),
				Color.green(color), Color.blue(color));
		MyPolygonOverlay polygonOverlay = new MyPolygonOverlay(
				this.getApplicationContext());

		polygonOverlay.setFillColor(argbcolor);
		polygonOverlay.setStrokeColor(color);
		polygonOverlay.setStrokeWidth(cat.getStrokeWidth());

		return polygonOverlay;
	}

	protected Overlay drawMarker(GeoPoint point) {

		OverlayItem myLocationOverlayItem = new OverlayItem("Here",
				"Current Position", point);
		Drawable myCurrentLocationMarker = this.getResources().getDrawable(
				R.drawable.target);
		myLocationOverlayItem.setMarker(myCurrentLocationMarker);

		final ArrayList<OverlayItem> items = new ArrayList<OverlayItem>();
		items.add(myLocationOverlayItem);
		DefaultResourceProxyImpl resourceProxy = new DefaultResourceProxyImpl(
				getApplicationContext());
		
		ItemizedIconOverlay<OverlayItem> currentLocationOverlay = new ItemizedIconOverlay<OverlayItem>(
				items,
				new ItemizedIconOverlay.OnItemGestureListener<OverlayItem>() {
					public boolean onItemSingleTapUp(final int index,
							final OverlayItem item) {
						return true;
					}

					public boolean onItemLongPress(final int index,
							final OverlayItem item) {
						return true;
					}
				}, resourceProxy);

		return currentLocationOverlay;
	}

	// ===========================================================
	// Methods from SuperClass/Interfaces
	// ===========================================================

	@Override
	public void onLocationChanged(final Location pLoc) {
		this.mMyLocationOverlay.setLocation(new GeoPoint(pLoc));
	}

	@Override
	public void onLocationLost() {
		// We'll do nothing here.
	}

	@Override
	public boolean onCreateOptionsMenu(final Menu pMenu) {
		getMenuInflater().inflate(R.menu.canvas, pMenu);

		final SubMenu subMenu = pMenu.addSubMenu(0, MENU_TILE_SOURCE_ID,
				Menu.NONE, "Choose Tile Source");
		{
			for (final ITileSource tileSource : TileSourceFactory
					.getTileSources()) {
				subMenu.add(0, 1000 + tileSource.ordinal(), Menu.NONE,
						tileSource.localizedName(mResourceProxy));
			}
		}

		pMenu.add(0, MENU_MINIMAP_ID, Menu.NONE, "Toggle Minimap");
		pMenu.add(0, MENU_SHOWRESULT_ID, Menu.NONE, "Toggle Result");

		pMenu.findItem(R.id.rotateright).setVisible(false);
		pMenu.findItem(R.id.rotateleft).setVisible(false);
		pMenu.findItem(R.id.invert).setVisible(false);
		return true;
	}

	@Override
	public boolean onMenuItemSelected(final int featureId, final MenuItem item) {
		switch (item.getItemId()) {
		case R.id.zoomIn:
			this.mOsmvController.zoomIn();
			return true;
		case R.id.zoomOut:
			this.mOsmvController.zoomOut();
			return true;
		case R.id.faster:
			period *= 0.5f;
			startTimer();
			return true;
		case R.id.slower:
			period *= 2.0f;
			startTimer();
			return true;
		case MENU_TILE_SOURCE_ID:
			this.mOsmv.invalidate();
			return true;
		case MENU_MINIMAP_ID:
			mMiniMapOverlay.setEnabled(!mMiniMapOverlay.isEnabled());
			this.mOsmv.invalidate();
			return true;
		case MENU_SHOWRESULT_ID:
			showResult = !showResult;
			drawQueryResult();
			return true;

		case MENU_ANIMATION_ID:
			return true;

		default:
			ITileSource tileSource = TileSourceFactory.getTileSource(item
					.getItemId() - 1000);
			mOsmv.setTileSource(tileSource);
			mMiniMapOverlay.setTileSource(tileSource);
		}
		return false;
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.faster:
			period *= 0.5f;
			startTimer();
			break;
		case R.id.slower:
			period *= 2.0f;
			startTimer();
			break;
		case R.id.play:
			period = 1000L; // reset
			if (running) {
				scheduleTaskExecutor.shutdown();
				deactivatePlayButton();
			} else {
				startTimer();
			}
			running = !running;
			break;
		}
	}

	private void startTimer() {
		if (running) {
			scheduleTaskExecutor.shutdown();
		}
		scheduleTaskExecutor = Executors.newScheduledThreadPool(5);
		scheduleTaskExecutor.scheduleAtFixedRate(new Runnable() {
			@Override
			public void run() {
				TimerMethod();
			}

		}, 1, period, TimeUnit.MILLISECONDS);

		activatePlayButton();
	}

	private void stopTimer() {
		if (scheduleTaskExecutor != null && running) {
			scheduleTaskExecutor.shutdown();
			running = false;
			deactivatePlayButton();
		}
	}

	private void activatePlayButton() {
		ImageButton buttonPlay = (ImageButton) findViewById(R.id.play);
		buttonPlay.setImageResource(R.drawable.mediapause);
	}

	private void deactivatePlayButton() {
		ImageButton buttonPlay = (ImageButton) findViewById(R.id.play);
		buttonPlay.setImageResource(R.drawable.mediaplay);
	}

	@Override
	public void setDirtyFlag() {
		dirty = true;
	}

	public void updateActivity() {
		dirty = true;
	}
	
	private class Holder {
		public Holder(QueryResult qr, int position, DsplBase dsplGraph, MyItemizedIconOverlay overlay) {
			this.queryResult = qr;
			this.dsplBase = dsplGraph;
			this.position = position;
			this.overlay = overlay;
		}
		public QueryResult queryResult;
		public DsplBase dsplBase;
		public int position;
		public MyItemizedIconOverlay overlay;
	}

}
