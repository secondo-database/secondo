package de.fernunihagen.dna;

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import sj.lang.ListExpr;

import javamini.awt.geom.Area;
import javamini.awt.geom.Ellipse2D;
import javamini.awt.geom.GeneralPath;
import javamini.awt.geom.PathIterator;
import javamini.awt.geom.Rectangle2D;
import javamini.awt.Shape;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.Path;
import android.graphics.Point;
import android.graphics.PointF;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.ShapeDrawable;
import android.graphics.drawable.shapes.OvalShape;
import android.graphics.drawable.shapes.PathShape;
import android.util.Log;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.widget.Toast;
import de.fernunihagen.dna.hoese.Category;
import de.fernunihagen.dna.hoese.CurrentState;
import de.fernunihagen.dna.hoese.DsplBase;
import de.fernunihagen.dna.hoese.Interval;
import de.fernunihagen.dna.hoese.LEUtils;
import de.fernunihagen.dna.hoese.QueryResult;
import de.fernunihagen.dna.hoese.QueryResultHelper;
import de.fernunihagen.dna.hoese.algebras.DisplayGraph;
import de.fernunihagen.dna.hoese.algebras.Dsplline;

import android.view.animation.Interpolator;
import android.view.animation.OvershootInterpolator;

public class DrawView extends View {
	private static final String TAG = "DrawView";

	private List<QueryResult> queryResults = new Vector<QueryResult>(5);
	private Drawable[] mDrawables;
	private float scaleFactor = 1.0f;
	private float rotDegree = 0.0f;
	private float invert = -1.0f;

	private GestureDetector gestures;
	private ScaleGestureDetector scaleGestureDetector;

	private Matrix translate;
	private Matrix animateStart;
	private Interpolator animateInterpolator;
	private long startTime;
	private long endTime;
	private float totalAnimDx;
	private float totalAnimDy;
	private float stdWidthX = 0.0f;
	private float stdWidthY = 0.0f;
	private float maxDimension;

	private final Paint textPaint = new Paint();

	private int progress = 0;

	private Point displaySize;
	private Matrix matrix1;

	private String actTimeString;

	public DrawView(Context context, List<QueryResult> queryResults,
			Point displaySize) {
		super(context);
		setFocusable(true);

		addQueryResult(queryResults);

		this.translate = new Matrix();
		this.gestures = new GestureDetector(context, new GestureListener(this));
		this.scaleGestureDetector = new ScaleGestureDetector(context,
				new ScaleListener());
		this.displaySize = displaySize;
		this.setBackgroundColor(Color.LTGRAY);

		// Textausgabe
		textPaint.setColor(Color.BLACK);
		textPaint.setStyle(Paint.Style.FILL);
		textPaint.setAntiAlias(true);
		textPaint.setTextSize(18 * this.getResources().getDisplayMetrics().density);
	}

	void addQueryResult(List<QueryResult> queryResults) {
		this.queryResults = queryResults;
		Interval boundingInter = QueryResultHelper.getBoundingInterval(queryResults);
		if (boundingInter != null)
			CurrentState.setActualTime(boundingInter.getStart());
	}

	void onWindowVisibilityChanged() {
		Log.w(TAG, "onWindowVisibilityChanged");
	}

	private void createDrawables() {
		List<DrawObject> shapeDrawables = getDrawObjects();
		if (shapeDrawables == null)
			return;

		mDrawables = new Drawable[shapeDrawables.size()];

		if (matrix1 == null) {
			matrix1 = new Matrix();
			Rectangle2D.Double totalBound = calcBound(shapeDrawables);
			// maximale Größe der zu zeichnenden Objekte
			double maxDoubleDimension = Math.max(totalBound.getWidth(),
					totalBound.getHeight());
			maxDimension = (float) maxDoubleDimension;

			// Verschieben, damit linke obere Ecke 0.0 Koordinaten hat
			matrix1.setTranslate(-(float) totalBound.x, 
					-(float) totalBound.y); 
			// Set the matrix to mirror the image in the x direction
			// matrix1.preScale(-1.0f, 1.0f);
			// Anzeige so verkleinern/vergrößern, damit sie auf das Dispay passt
			matrix1.postScale(displaySize.x / maxDimension, displaySize.x
					/ maxDimension);
			stdWidthX = maxDimension > displaySize.x ? maxDimension
					: (displaySize.x / maxDimension);
			stdWidthY = maxDimension > displaySize.y ? maxDimension
					: (displaySize.y / maxDimension);

		}

		int i = 0;
		for (DrawObject drawObject : shapeDrawables) {
			Path path = drawObject.getPath();
			path.transform(matrix1);

			Category cat = drawObject.getCategory();

			Style style = Style.STROKE;
			Drawable drawable = null;

			// circlesize undependend from scalefactor and dimensions

			switch (drawObject.getObjectType()) {
			case FACE: {
				style = Style.FILL;
				ShapeDrawable shapeDrawable = new ShapeDrawable(new PathShape(
						path, stdWidthX, stdWidthY));
				shapeDrawable.getPaint().setColor(
						drawObject.isSelected() ? cat.getSelectedColor() : cat
								.getColor());
				shapeDrawable.getPaint().setAlpha(cat.getAlpha());
				shapeDrawable.getPaint().setStyle(style);
				shapeDrawable.getPaint().setStrokeJoin(Paint.Join.ROUND);
				shapeDrawable.getPaint().setStrokeWidth(cat.getStrokeWidth());
				shapeDrawable.getPaint().setStrokeCap(Paint.Cap.ROUND);
				shapeDrawable.setBounds(0, 0, (int) stdWidthX, (int) stdWidthY);
				drawable = shapeDrawable;
				break;
			}
			case LINE: {
				style = Style.STROKE;
				ShapeDrawable shapeDrawable = new ShapeDrawable(new PathShape(
						path, stdWidthX, stdWidthY));
				shapeDrawable.getPaint().setColor(
						drawObject.isSelected() ? cat.getSelectedColor()
								: drawObject.getCategory().getColor());
				shapeDrawable.getPaint().setAlpha(cat.getAlpha());
				shapeDrawable.getPaint().setStyle(style);
				shapeDrawable.getPaint().setStrokeWidth(cat.getStrokeWidth());
				shapeDrawable.getPaint().setStrokeJoin(Paint.Join.ROUND);
				shapeDrawable.getPaint().setStrokeCap(Paint.Cap.ROUND);
				shapeDrawable.setBounds(0, 0, (int) stdWidthX, (int) stdWidthY);
				drawable = shapeDrawable;
				break;
			}
			case POINT: {
				// Get the screen's density scale
				int circleRadius = calcIndependentRadius(6.0f);

				style = Style.FILL;
				RectF bounds = new RectF();
				path.computeBounds(bounds, true);
				PointF centerPoint = new PointF(bounds.centerX(), bounds.centerY());

				ShapeDrawable shapeDrawable = new ShapeDrawable(new OvalShape());
				shapeDrawable.getPaint().setColor(
						drawObject.isSelected() ? cat.getSelectedColor() : cat
								.getColor());
				shapeDrawable.getPaint().setAlpha(cat.getAlpha());
				shapeDrawable.getPaint().setStrokeWidth(cat.getStrokeWidth());
				shapeDrawable.setBounds((int) centerPoint.x - circleRadius,
						(int) centerPoint.y - circleRadius, (int) centerPoint.x
								+ circleRadius, (int) centerPoint.y
								+ circleRadius);
				//
				drawable = shapeDrawable;

				break;
			}
			}

			mDrawables[i++] = drawable;
		}
	}

	private int calcIndependentRadius(float radius) {
		final float scale = getResources().getDisplayMetrics().density;
		// Convert the dps to pixels, based on density scale
		return (int) ((radius * scale + 0.5f) / scaleFactor);
	}

	private Rectangle2D.Double calcBound(List<DrawObject> shapeDrawables) {
		Rectangle2D.Double totalBound = null;
		for (DrawObject drawObject : shapeDrawables) {
			Rectangle2D.Double bound = drawObject.getBounds();
			if (totalBound == null)
				totalBound = bound;
			else
				Rectangle2D.Double.union(bound, totalBound, totalBound);
		}
		if (totalBound == null)
			return new Rectangle2D.Double(); // Empty bounding Rectangle

		return totalBound;
	}

	private List<DrawObject> getDrawObjects() {
		List<DrawObject> shapeDrawables = drawResult();
		if (shapeDrawables == null) { // Kein Ergebnis
			Toast.makeText(this.getContext(), "Keine Queryresult",
					Toast.LENGTH_SHORT).show();
			return null;
		}
		return shapeDrawables;
	}

	@Override
	protected void onDraw(Canvas canvas) {
		canvas.save();
		int cx = this.getMeasuredWidth();
		int cy = this.getMeasuredHeight();

		if (actTimeString != null) {
			canvas.drawText(actTimeString, 50, Math.max(80, cy * 0.1f), textPaint);
//			canvas.drawText(actTimeString, 50, cy - 80, textPaint);
		}

		createDrawables();

		// adjust size and mirror
		canvas.scale(scaleFactor, invert * scaleFactor); // , cx, cy);
		canvas.rotate(rotDegree);

		if (translate != null) {
			// Log.d(this.getClass().getName(), translate.toString());
			float[] matrixValues = new float[9];
			translate.getValues(matrixValues);
			if (invert < 0.0f)
				canvas.translate(matrixValues[2],
						matrixValues[5] - Math.min(cx, cy));
			else
				canvas.translate(matrixValues[2], matrixValues[5]);
			Log.i(TAG, "Translate " + translate.toString());
		}

		if (mDrawables != null) {
			for (Drawable dr : mDrawables) {
				// dr.setLevel(level)
				dr.draw(canvas);

			}
		}

		canvas.restore();
		long memory = android.os.Debug.getNativeHeapAllocatedSize();
		Log.i(TAG, "MemorySize " + memory);

	}

	private class ScaleListener extends
			ScaleGestureDetector.SimpleOnScaleGestureListener {
		@Override
		public boolean onScale(ScaleGestureDetector detector) {
			scaleFactor *= detector.getScaleFactor();
			Log.w(TAG, "scaleFactor = " + scaleFactor);

			// // Don't let the object get too small or too large.
			// scaleFactor = Math.max(0.1f, Math.min(scaleFactor, 5.0f));

			invalidate();
			return true;
		}

	}

	public class GestureListener implements OnGestureListener {
		private static final String DEBUG_TAG = "GestureListener";
		DrawView view;

		public GestureListener(DrawView view) {
			this.view = view;
		}

		@Override
		public boolean onDown(MotionEvent e) {
			Log.v(DEBUG_TAG, "onDown");
			return true;
		}

		@Override
		public boolean onFling(MotionEvent e1, MotionEvent e2,
				final float velocityX, final float velocityY) {
			// Log.v(DEBUG_TAG, "onFling");
			final float distanceTimeFactor = scaleFactor * 0.4f; // Original
																	// 0.4f
			final float totalDx = (distanceTimeFactor * velocityX / 2);
			final float totalDy = (distanceTimeFactor * velocityY / 2);

			view.onAnimateMove(totalDx, totalDy,
					(long) (1000 * distanceTimeFactor));
			return true;
		}

		public boolean onDoubleTap(MotionEvent e) {
			Log.v(DEBUG_TAG, "onDoubleTap");
			view.onResetLocation();
			return true;
		}

		@Override
		public void onLongPress(MotionEvent e) {
			Log.v(DEBUG_TAG, "onLongPress");
		}

		@Override
		public boolean onScroll(MotionEvent e1, MotionEvent e2,
				float distanceX, float distanceY) {
			// Log.v(DEBUG_TAG, "onScroll scale" + scaleFactor + " " + distanceX
			// + "/" +distanceY);

			// moving a little bit slower depending on the scalefactor: Higher
			// scalefactor -> slower movement
			view.onMove(-distanceX / (1 + (scaleFactor / (10 - scaleFactor))),
					-distanceY / (1 + (scaleFactor / (10 - scaleFactor))));
			return true;
		}

		@Override
		public void onShowPress(MotionEvent e) {
			Log.v(DEBUG_TAG, "onShowPress");
		}

		@Override
		public boolean onSingleTapUp(MotionEvent e) {
			Log.v(DEBUG_TAG, "onSingleTapUp");
			return false;
		}

		public boolean onDoubleTapEvent(MotionEvent e) {
			Log.v(DEBUG_TAG, "onDoubleTapEvent");
			return false;
		}

		public boolean onSingleTapConfirmed(MotionEvent e) {
			Log.v(DEBUG_TAG, "onSingleTapConfirmed");
			return false;
		}

	}

	private List<DrawObject> drawResult() {
		ArrayList<DrawObject> drawables = new ArrayList<DrawObject>();

		Rectangle2D.Double bound = new Rectangle2D.Double();
		if (queryResults == null || queryResults.size() == 0) {
			return null;
		}

		/*
		 * ListExpr listExpr = queryResult.getResultList();
		 * 
		 * if (listExpr == null) { Log.e(TAG, "no result"); return drawables; }
		 */

		for (QueryResult actQueryResult : this.queryResults) {
			if (actQueryResult.isSelected() == false) // don't show QueryResult
				continue;
			Category category = actQueryResult.getCategory();
			if (category == null) {
				Category.getDefaultCat();
			}

			@SuppressWarnings("unused")
			ListExpr listExpr = actQueryResult.getResultList();
			// region
			List<DsplBase> results = actQueryResult.getEntries();

			for (DsplBase obj : results) {
				if (obj instanceof DisplayGraph) {
					DisplayGraph dsplGraph = (DisplayGraph) obj;
					dsplGraph.setCategory(category);
					javamini.awt.Shape shape = null;
					javamini.awt.geom.AffineTransform af = CurrentState.transform;
					shape = dsplGraph.getRenderObject(0, af);

					if (shape == null) {
						// Nothing todo
						continue;
					}
					if (shape instanceof Ellipse2D) {
						drawables.addAll(drawPath(shape, dsplGraph,
								DrawObject.Type.POINT));
					}
					if (shape instanceof Dsplline) {
						Rectangle2D.Double linebound = ((Dsplline) shape)
								.getBounds();
						bound.add(linebound);
						if (shape instanceof GeneralPath) {
							drawables.addAll(drawPath(shape, dsplGraph,
									DrawObject.Type.LINE));
						}
						// drawables.addAll(drawLine(shape));
					}
					if (shape instanceof Area) {
						Area area = (Area) shape;

						drawables.addAll(drawPath(area, dsplGraph, dsplGraph
								.isLineType(0) ? DrawObject.Type.LINE
								: DrawObject.Type.FACE));
					}
					if (shape instanceof GeneralPath) {
						drawables.addAll(drawPath(shape, dsplGraph, dsplGraph
								.isLineType(0) ? DrawObject.Type.LINE
								: DrawObject.Type.FACE));
					}
				}
			}
		}

		return drawables;
	}

	private List<DrawObject> drawPath(Shape shape, DisplayGraph dsplGraph,
			DrawObject.Type lineType) {
		ArrayList<DrawObject> shapeDrawables = new ArrayList<DrawObject>();

		// Ob ein neues Overlay angelegt werden muss oder nicht, ist
		// abhängig, ob die neue Line eine Fläche (geschlossen) ist
		// und ob diese in der anderen Fläche enthalten ist (Ausschnitt)

		// PathOverlay pathOverlay = null;
		PathIterator pathIterator = shape.getPathIterator(null);
		Path path = new Path();

		while (pathIterator.isDone() == false) {
			double[] coordinates = new double[6];
			int type = pathIterator.currentSegment(coordinates);

			if (type == PathIterator.SEG_MOVETO) {
				path.moveTo((float) coordinates[0], (float) coordinates[1]);
			}
			if (type == PathIterator.SEG_LINETO) {
				path.lineTo((float) coordinates[0], (float) coordinates[1]);
			}
			if (type == PathIterator.SEG_CLOSE) {
				path.close();
				Log.i("PathIterator.SEG_CLOSE", "Close");
			}
			if (type == PathIterator.SEG_CUBICTO) {
				path.cubicTo((float) coordinates[0], (float) coordinates[1],
						(float) coordinates[2], (float) coordinates[3],
						(float) coordinates[4], (float) coordinates[5]);
				// Log.i("PathIterator.SEG_CUBICTO", "SEG_CUBICTO: "
				// + coordinates.toString());

			}

			pathIterator.next();
		}

		DrawObject drawObject = new DrawObject();
		drawObject.setCategory(dsplGraph.getCategory());
		drawObject.setPath(path);
		drawObject.setObjectType(lineType);
		drawObject.setBounds(dsplGraph.getBounds());
		drawObject.setSelected(dsplGraph.getSelected());
		shapeDrawables.add(drawObject);

		return shapeDrawables;
	}

	protected void setScaleFactor(float newScaleFactor) {
		this.scaleFactor = newScaleFactor;
	}

	protected void setRotate(float newRotDegree) {
		this.rotDegree = newRotDegree;
	}

	protected void setInvert(float newInvert) {
		this.invert = newInvert;
	}

	public void onAnimateMove(float dx, float dy, long duration) {
		animateStart = new Matrix(translate);
		animateInterpolator = new OvershootInterpolator();
		startTime = System.currentTimeMillis();
		endTime = startTime + duration;
		totalAnimDx = dx;
		totalAnimDy = dy;
		post(new Runnable() {
			@Override
			public void run() {
				onAnimateStep();
			}
		});
	}

	private void onAnimateStep() {
		long curTime = System.currentTimeMillis();
		float percentTime = (float) (curTime - startTime)
				/ (float) (endTime - startTime);
		float percentDistance = animateInterpolator
				.getInterpolation(percentTime);
		float curDx = percentDistance * totalAnimDx;
		float curDy = percentDistance * totalAnimDy;
		translate.set(animateStart);
		onMove(curDx, curDy);

		// Log.v(DEBUG_TAG, "We're " + percentDistance +
		// " of the way there!");
		if (percentTime < 1.0f) {
			post(new Runnable() {
				@Override
				public void run() {
					onAnimateStep();
				}
			});
		}
	}

	public void onMove(float dx, float dy) {
		float movedx = Math.min(Math.abs(dx), stdWidthX / 5); // Only move the
																// half of the
																// showing area
		float movedy = Math.min(Math.abs(dy), stdWidthY / 5);

		if (dx < 0.0f)
			movedx *= -1.0;
		if (dy < 0.0f)
			movedy *= -1.0;
		movedy *= invert;

		translate.postTranslate(movedx, movedy);
		invalidate();
	}

	public void onResetLocation() {
		translate.reset();
		invalidate();
	}

	public void onSetLocation(float dx, float dy) {
		translate.postTranslate(dx, dy);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		scaleGestureDetector.onTouchEvent(event);

		return gestures.onTouchEvent(event);
	}

	public void setNextTime() {

		// Step forward
		setNewTime(progress + 1, -1);
	}

	public void setNewTime(int progress, int maxProgress) {

		if (this.queryResults == null || this.queryResults.size() == 0) {
			return; // No result
		}

		Interval interval = QueryResultHelper.getBoundingInterval(queryResults);
		if (interval == null) // not a Timed Queryresult
			return;

		double actTime = interval.getStart()
				+ (interval.getEnd() - interval.getStart()) * progress / maxProgress;
		CurrentState.setActualTime(actTime);
		actTimeString = LEUtils.convertTimeToString(actTime);

		invalidate();

		this.progress = progress;
	}

}
