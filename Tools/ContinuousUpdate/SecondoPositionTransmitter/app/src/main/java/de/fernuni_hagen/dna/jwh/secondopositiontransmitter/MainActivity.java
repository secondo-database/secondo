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

import java.util.ArrayList;


public class MainActivity extends Activity {

    private SharedPreferences prefs;
    private BroadcastReceiver receiver;
    private DatabaseManager db;

    /**
     * Is used for stopping the Service, if it's running, and updating preferences
     */
    private TextWatcher watcher = new TextWatcher() {
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

    /**
     * Used to stop/start the service
     */
    private CompoundButton.OnCheckedChangeListener checkedChangeListener = new CompoundButton.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            if (isChecked) {
                savePreferences();
                startService(new Intent(getBaseContext(), PositionTransmissionService.class));
            } else {
                stopService(new Intent(getBaseContext(), PositionTransmissionService.class));
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        db = new DatabaseManager(getApplicationContext());
        loadMessagesFromDB();
        loadPreferences();

        // Create BroadcastReceiver to trigger the db read
        receiver = new BroadcastReceiver() {

            @Override
            public void onReceive(Context context, Intent intent) {
                loadMessagesFromDB();
            }
        };
    }

    /**
     * Loads the new messages form the db and deletes old entries
     */
    private void loadMessagesFromDB() {
        TextView tv = (TextView) findViewById(R.id.textViewLogging);
        ArrayList<String> list = db.getAll();
        tv.setText(null);
        for (String line : list) {
            tv.setText(line + "\n" + tv.getText());
        }
        db.deleteOld(90);
    }

    /**
     * Adds the Textwatcher to all the fields
     */
    private void addWatches() {
        final ToggleButton toggleButton = (ToggleButton) findViewById(R.id.toggleButton);
        toggleButton.setOnCheckedChangeListener(checkedChangeListener);

        EditText editText = (EditText) findViewById(R.id.editTextHost);
        editText.addTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextPort);
        editText.addTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextSendInterval);
        editText.addTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextLogInterval);
        editText.addTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextRelation);
        editText.addTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextUser);
        editText.addTextChangedListener(watcher);
        ToggleButton tButton = (ToggleButton) findViewById(R.id.toggleMoving);
        tButton.addTextChangedListener(watcher);
    }

    /**
     * Removes the TextWatcher from all fields
     */
    private void removeWatches() {
        final ToggleButton toggleButton = (ToggleButton) findViewById(R.id.toggleButton);
        toggleButton.setOnCheckedChangeListener(null);

        EditText editText = (EditText) findViewById(R.id.editTextHost);
        editText.removeTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextPort);
        editText.removeTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextSendInterval);
        editText.removeTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextLogInterval);
        editText.removeTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextRelation);
        editText.removeTextChangedListener(watcher);
        editText = (EditText) findViewById(R.id.editTextUser);
        editText.removeTextChangedListener(watcher);
        ToggleButton tButton = (ToggleButton) findViewById(R.id.toggleMoving);
        tButton.removeTextChangedListener(watcher);
    }

    private void loadPreferences() {
        prefs = getSharedPreferences("configuration", MODE_PRIVATE);
        ((EditText) findViewById(R.id.editTextHost)).setText(prefs.getString("host", "192.168.159.130"));
        ((EditText) findViewById(R.id.editTextPort)).setText(prefs.getString("port", "8081"));
        ((EditText) findViewById(R.id.editTextRelation)).setText(prefs.getString("relation", "orte"));
        ((EditText) findViewById(R.id.editTextUser)).setText(prefs.getString("user", ""));
        ((ToggleButton) findViewById(R.id.toggleMoving)).setChecked(prefs.getBoolean("goodLocation", true));
        ((EditText) findViewById(R.id.editTextSendInterval)).setText(Integer.toString(prefs.getInt("sendInterval", 60)));
        ((EditText) findViewById(R.id.editTextLogInterval)).setText(Integer.toString(prefs.getInt("logInterval", 60)));
    }

    @Override
    protected void onStart() {
        super.onStart();
        LocalBroadcastManager.getInstance(this).registerReceiver((receiver), new IntentFilter(PositionTransmissionService.class.getName()));
        loadMessagesFromDB();
    }

    @Override
    protected void onStop() {
        super.onStop();
        LocalBroadcastManager.getInstance(this).unregisterReceiver(receiver);
        removeWatches();
        db.close();
    }

    @Override
    protected void onResume() {
        super.onResume();
        loadPreferences();

        final ToggleButton toggleButton = (ToggleButton) findViewById(R.id.toggleButton);
        toggleButton.setChecked(PositionTransmissionService.isRunning);

        addWatches();
        loadMessagesFromDB();
    }

    @Override
    protected void onPause() {
        super.onPause();
        savePreferences();
        removeWatches();
    }

    private void savePreferences() {
        SharedPreferences.Editor ed = prefs.edit();
        ed.putString("host", ((EditText) findViewById(R.id.editTextHost)).getText().toString());
        ed.putString("port", ((EditText) findViewById(R.id.editTextPort)).getText().toString());
        ed.putString("user", ((EditText) findViewById(R.id.editTextUser)).getText().toString());
        ed.putString("relation", ((EditText) findViewById(R.id.editTextRelation)).getText().toString());
        ed.putBoolean("goodLocation", ((ToggleButton) findViewById(R.id.toggleMoving)).isChecked());
        ed.putInt("sendInterval", Integer.parseInt(((EditText) findViewById(R.id.editTextSendInterval)).getText().toString()));
        ed.putInt("logInterval", Integer.parseInt(((EditText) findViewById(R.id.editTextLogInterval)).getText().toString()));
        ed.commit();
    }
}
