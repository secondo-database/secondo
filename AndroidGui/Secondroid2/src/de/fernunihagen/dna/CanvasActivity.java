package de.fernunihagen.dna;

import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import de.fernunihagen.dna.DrawView;
import de.fernunihagen.dna.hoese.CurrentState;
import de.fernunihagen.dna.hoese.Interval;
import de.fernunihagen.dna.hoese.Layer;
import de.fernunihagen.dna.hoese.QueryResult;
import de.fernunihagen.dna.hoese.QueryResultHelper;
import de.fernunihagen.dna.hoese.Viewer;
import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.graphics.Point;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.Display;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.Window;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

/**
 * CanvasActivity: Holds the DrawViwe for output. All the drawing is done in DrawView 
 * @author Michael Küpper
 *
 */
public class CanvasActivity extends Activity implements OnClickListener, Viewer {
	private static final int INITIAL_PERIOD = 250;

	private static final String TAG = "TestActivity";

	private float scaleFactor = 1.0f;
	private float rotDegree = 0.0f;
	private float invert = 1.0f;
	private int period = INITIAL_PERIOD; // 4 times faster (1000 = normal)

	private static DrawView view;
	private List<QueryResult> queryResults;
	private ScheduledExecutorService scheduleTaskExecutor;
	private boolean running = false;
	private boolean timelineOnTop = true;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		scheduleTaskExecutor = Executors.newScheduledThreadPool(5);

		Intent intent = getIntent();
		QueryResult queryResult = (QueryResult) intent
				.getSerializableExtra(CommandActivity.EXTRA_RESULT);
		queryResults = (List<QueryResult>) intent.getSerializableExtra(CommandActivity.EXTRA_RESULTS);

		queryResult.addViewer(this);
		Layer lay = new Layer(queryResult.getGraphObjects(), null);
		Interval boundingInter = QueryResultHelper.getBoundingInterval(queryResults);
		if (boundingInter != null) {
			CurrentState.setActualTime(boundingInter.getStart()); 
		}

		Display display = getWindowManager().getDefaultDisplay();
		DisplayMetrics outMetrics = new DisplayMetrics();
		display.getMetrics(outMetrics);
		Point size = new Point(outMetrics.widthPixels, outMetrics.heightPixels);

		requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
		setContentView(R.layout.activity_test);
		setProgressBarIndeterminateVisibility(true);
		RelativeLayout ll = (RelativeLayout) findViewById(R.id.CanvasDrawView);

		LayoutParams lp = new LayoutParams(LayoutParams.FILL_PARENT,
				LayoutParams.FILL_PARENT);
		MarginLayoutParams marginLayoutParams = new MarginLayoutParams(lp);
		marginLayoutParams.setMargins(0, 0, 0, 0);

		int pixel = (int) TypedValue.applyDimension(
				TypedValue.COMPLEX_UNIT_DIP, 12, getResources()
						.getDisplayMetrics());

		view = new DrawView(this, queryResults, size);

		ll.addView(view, marginLayoutParams);

		RelativeLayout.LayoutParams lpButtonNext = new RelativeLayout.LayoutParams(
				LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		// align the Seekbar at the bottom of the parent
		lpButtonNext.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
		lpButtonNext.addRule(timelineOnTop ? RelativeLayout.ALIGN_PARENT_TOP : RelativeLayout.ALIGN_PARENT_BOTTOM);
		lpButtonNext.setMargins(0, 0, 0, 0);

		// Create the Play Button for timed Objects
		ImageButton buttonPlay = new ImageButton(this, null,
				android.R.attr.buttonStyleSmall);
		buttonPlay.setAdjustViewBounds(true);
		buttonPlay.setId(R.id.play);
		buttonPlay.setImageResource(R.drawable.mediaplay);
		buttonPlay.setMaxHeight(pixel * 3);
		ll.addView(buttonPlay, lpButtonNext);
		buttonPlay.setOnClickListener(this);

		// Create the "Slower" Button for timed Objects
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
		ll.addView(buttonSlower, lpSlower);
		buttonSlower.setOnClickListener(this);

		// Create the Faster Button for timed Objects
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
		ll.addView(buttonFaster, lpFaster);
		buttonFaster.setOnClickListener(this);

		// Create the Timeline Slider for timed Objects
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
		timeSeekBar.setProgress(0);
		timeSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				// Set the new selected Time
				view.setNewTime(progress, seekBar.getMax());
			}

			public void onStartTrackingTouch(SeekBar seekBar) {
				// Nothing todo
			}

			public void onStopTrackingTouch(SeekBar seekBar) {
				// Nothing todo
			}
		});
		ll.addView(timeSeekBar, lpSeekBar);

		// Hide the Timer Buttons and the Seekbar
		timeSeekBar.setVisibility(boundingInter == null ? ProgressBar.INVISIBLE
				: ProgressBar.VISIBLE);
		buttonPlay.setVisibility(boundingInter == null ? Button.INVISIBLE
				: Button.VISIBLE);
		buttonFaster.setVisibility(boundingInter == null ? Button.INVISIBLE
				: Button.VISIBLE);
		buttonSlower.setVisibility(boundingInter == null ? Button.INVISIBLE
				: Button.VISIBLE);

		setProgressBarIndeterminateVisibility(false);
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

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.canvas, menu);

		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle item selection
		switch (item.getItemId()) {
		case R.id.zoomIn:
			view.setScaleFactor(scaleFactor *= 2f);
			view.invalidate();
			return true;
		case R.id.zoomOut:
			view.setScaleFactor(scaleFactor /= 2f);
			view.invalidate();
			return true;
		case R.id.zoomNormal:
			view.setScaleFactor(scaleFactor = 1.0f);
			view.setInvert(invert = 1.0f);
			view.setRotate(rotDegree = 0.0f);
			view.invalidate();
			return true;
		case R.id.rotateright:
			view.setRotate(rotDegree += 90f);
			view.invalidate();
			return true;
		case R.id.rotateleft:
			view.setRotate(rotDegree -= 90f);
			view.invalidate();
			return true;
		case R.id.invert:
			view.setInvert(invert *= -1.0f);
			view.invalidate();
			return true;
		case R.id.faster:
			period *= 0.5f;
			startTimer();
			return true;
		case R.id.slower:
			period *= 2.0f;
			startTimer();
			return true;
		case R.id.clearResultlist:
			queryResults.clear();

			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
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
			period = INITIAL_PERIOD; // reset
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
		view.invalidate();
	}

	public void updateActivity() {
	}

}
