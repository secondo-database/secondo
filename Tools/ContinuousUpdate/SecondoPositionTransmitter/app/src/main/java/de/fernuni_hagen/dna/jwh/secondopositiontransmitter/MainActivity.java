package de.fernuni_hagen.dna.jwh.secondopositiontransmitter;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.text.Editable;
import android.text.TextWatcher;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.ToggleButton;


public class MainActivity extends Activity {

    private SharedPreferences prefs;
    private BroadcastReceiver receiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        loadPreferences();

        final ToggleButton toggleButton = (ToggleButton) findViewById(R.id.toggleButton);
        toggleButton.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    savePreferences();
                    startService(new Intent(getBaseContext(), PositionTransmissionService.class));
                } else {
                    stopService(new Intent(getBaseContext(), PositionTransmissionService.class));
                }
            }
        });

        addTextWatcher();

        receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String s = intent.getStringExtra("message");
                TextView tv = (TextView) findViewById(R.id.textViewLogging);
                tv.setText(s + "\n" + tv.getText());
            }
        };
    }

    private void addTextWatcher() {
        TextWatcher watcher = new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                ToggleButton tg = (ToggleButton) findViewById(R.id.toggleButton);
                tg.setChecked(false);
            }
        };

        EditText editText = (EditText) findViewById(R.id.editTextHost);
        editText.addTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextPort);
        editText.addTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextInterval);
        editText.addTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextRelation);
        editText.addTextChangedListener(watcher);
    }

    private void loadPreferences() {
        prefs = getSharedPreferences("configuration", MODE_PRIVATE);
        ((EditText) findViewById(R.id.editTextHost)).setText(prefs.getString("host", "192.168.159.130"));
        ((EditText) findViewById(R.id.editTextPort)).setText(prefs.getString("port", "8081"));
        ((EditText) findViewById(R.id.editTextRelation)).setText(prefs.getString("relation", "orte"));
        ((EditText) findViewById(R.id.editTextInterval)).setText(Integer.toString(prefs.getInt("updateInterval", 60)));
    }

    @Override
    protected void onStart() {
        super.onStart();
        LocalBroadcastManager.getInstance(this).registerReceiver((receiver), new IntentFilter(PositionTransmissionService.class.getName()));
    }

    @Override
    protected void onStop() {
        super.onStop();
        LocalBroadcastManager.getInstance(this).unregisterReceiver(receiver);
    }

    @Override
    protected void onResume() {
        super.onResume();
        loadPreferences();
    }

    @Override
    protected void onPause() {
        super.onPause();
        savePreferences();
    }

    public void addToLog(String log) {
        TextView tv = (TextView) findViewById(R.id.textViewLogging);
        tv.setText(log + "\n" + tv.getText());
    }

    private void savePreferences() {
        SharedPreferences.Editor ed = prefs.edit();
        ed.putString("host", ((EditText) findViewById(R.id.editTextHost)).getText().toString());
        ed.putString("port", ((EditText) findViewById(R.id.editTextPort)).getText().toString());
        ed.putString("relation", ((EditText) findViewById(R.id.editTextRelation)).getText().toString());
        ed.putInt("updateInterval", Integer.parseInt(((EditText) findViewById(R.id.editTextInterval)).getText().toString()));
        ed.commit();
    }
}
