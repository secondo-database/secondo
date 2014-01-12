package de.fernunihagen.dna;

import de.fernunihagen.dna.hoese.Category;
import yuku.ambilwarna.AmbilWarnaDialog;
import yuku.ambilwarna.AmbilWarnaDialog.OnAmbilWarnaListener;
import android.os.Bundle;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Intent;
import android.text.InputFilter;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class CategoryActivity extends Activity implements OnClickListener {
	private Category cat;

	@SuppressLint("UseValueOf")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		final Intent intent = getIntent();

		cat = (Category) intent
				.getSerializableExtra(ResultsActivity.EXTRA_CATEGORY);
		// queryResult = (QueryResult)
		// intent.getSerializableExtra(ResultsActivity.EXTRA_QUERYRESULT);

		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_category);

		Button button = (Button) findViewById(R.id.pickColor);
		button.setOnClickListener(this);
		button.setBackgroundColor(cat.getColor());

		Button selColorButton = (Button) findViewById(R.id.pickSelectedColor);
		selColorButton.setOnClickListener(this);
		selColorButton.setBackgroundColor(cat.getSelectedColor());

		EditText thickness = (EditText) findViewById(R.id.thickness);
		thickness.setText(new Integer(cat.getStrokeWidth()).toString());
		thickness.setFilters(new InputFilter[] { new InputFilterMinMax("1",
				"99") });

		EditText alpha = (EditText) findViewById(R.id.alpha);
		alpha.setText(new Integer(cat.getAlpha()).toString());
		alpha.setFilters(new InputFilter[] { new InputFilterMinMax("0", "255") });

		Button okayButton = (Button) findViewById(R.id.okayButton);
		okayButton.setOnClickListener(this);

		Button cancelButton = (Button) findViewById(R.id.cancelButton);
		cancelButton.setOnClickListener(this);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.category, menu);
		return true;
	}

	public void onBackPressed() {
		saveCategory();

		finish();
		return;
	}

	private void saveCategory() {
		EditText thickness = (EditText) findViewById(R.id.thickness);
		EditText alpha = (EditText) findViewById(R.id.alpha);
		cat.setStrokeWidth(Integer.parseInt(thickness.getText().toString()));
		cat.setAlpha(Integer.parseInt(alpha.getText().toString()));

		Intent returnIntent = new Intent();
		returnIntent.putExtra(ResultsActivity.EXTRA_CATEGORY, cat);
		setResult(RESULT_OK, returnIntent);
	}

	@Override
	public void onClick(View view) {
		switch (view.getId()) {
		case R.id.pickColor:
			// initialColor is the initially-selected color to be shown in the
			// rectangle on the left of the arrow.
			// for example, 0xff000000 is black, 0xff0000ff is blue. Please be
			// aware of the initial 0xff which is the alpha.
			int initialColor = cat.getColor();

			AmbilWarnaDialog dialog = new AmbilWarnaDialog(this, initialColor,
					new OnAmbilWarnaListener() {
						@Override
						public void onOk(AmbilWarnaDialog dialog, int color) {
							Button button = (Button) findViewById(R.id.pickColor);

							button.setBackgroundColor(color);
							cat.setColor(color);
						}

						@Override
						public void onCancel(AmbilWarnaDialog dialog) {
							// cancel was selected by the user
						}
					});

			dialog.show();
			break;
		case R.id.pickSelectedColor:
			// initialColor is the initially-selected color to be shown in the
			// rectangle on the left of the arrow.
			// for example, 0xff000000 is black, 0xff0000ff is blue. Please be
			// aware of the initial 0xff which is the alpha.
			int selectedColor = cat.getSelectedColor();

			AmbilWarnaDialog selectedDialog = new AmbilWarnaDialog(this,
					selectedColor, new OnAmbilWarnaListener() {
						@Override
						public void onOk(AmbilWarnaDialog dialog, int color) {
							Button button = (Button) findViewById(R.id.pickSelectedColor);

							button.setBackgroundColor(color);
							cat.setSelectedColor(color);
						}

						@Override
						public void onCancel(AmbilWarnaDialog dialog) {
							// cancel was selected by the user
						}
					});

			selectedDialog.show();
			break;
		case R.id.cancelButton:
			finish();
			break;
		case R.id.okayButton:
			saveCategory();

			finish();
			break;
		}
		

	}

}
