package de.fernunihagen.dna;

import de.fernunihagen.dna.hoese.QueryResult;
import sj.lang.IntByReference;
import sj.lang.ListExpr;
import android.app.Activity;
import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.EditText;
import android.widget.Toast;

/**
 * QueryAsyncTask is the Thread Class handle the database connection
 * doInBackground executes a database query an take the result
 * @author Michael Küpper
 *
 */
public class QueryAsyncTask extends AsyncTask<String, Integer, QueryResult> {
	private static final String TAG = QueryAsyncTask.class.getName();
	private Context context;
	private EditText resultView;
	private SecondoServerService secondoServerService;
	private Activity activity;

	public QueryAsyncTask(Activity activity) {
		this.activity = activity;
	}
	

	public void setContext(Context context) {
		this.context = context;
		if (this.secondoServerService != null)
			this.secondoServerService.setContext(this.context);
	}

	public void setResultView(EditText resultView) {
		this.resultView = resultView;
	}

	public void setSecondoService(SecondoServerService secondoServerService) {
		this.secondoServerService = secondoServerService;
		this.secondoServerService.setContext(this.context);
	}

	protected QueryResult doInBackground(String... command) {
		QueryResult qr = new QueryResult();

		if (!secondoServerService.isConnected()) {
			secondoServerService.reconnect();
			secondoServerService.reopenDatabase();
			if (command[0].toUpperCase().contains("OPEN") && command[0].toUpperCase().contains("DATABASE")) {
				qr.setCommand(command[0]);
				qr.setResultString(null); // resultList.toString());
				return qr;
			}
		}

		IntByReference errorCode = new IntByReference();
		IntByReference errorPos = new IntByReference();
		StringBuffer errorMessage = new StringBuffer();
		publishProgress(0);
		String fileName = "";
		try {
//			resultList = secondoServerService.execute(command[0], errorCode,
//				errorPos, errorMessage);
			fileName = secondoServerService.executeInFile(command[0], errorCode,
					errorPos, errorMessage, false);
			if (command[0].toUpperCase().contains("CLOSE") && command[0].toUpperCase().contains("DATABASE")) {
				secondoServerService.disconnect();
			}
			
		} catch (Exception ex) {
			Log.e(TAG,
					"Fehler beim Ausführen des Commandos " + command[0],
					ex);
			qr.setError(ex.getMessage());
			return qr;
		}
		publishProgress(100);


		if (errorMessage.length() > 0) {
			// TODO ErrorObect erzeugen
			qr.setError(errorMessage.toString());
			return qr;
		}

		qr.setCommand(command[0]);
		qr.setResultString(null); // resultList.toString());

		qr.setFileName(fileName);
		return qr;

	}

	protected void onProgressUpdate(Integer... progress) {
		if (resultView != null) {
			if (progress[0] == 0) {
				resultView.setText(this.context.getString(R.string.running));
			}

			if (progress[0] == 100) {
				resultView.setText(this.context.getString(R.string.ready));
			}

		}
	}
	
	protected void onPreExecute() {
		activity.setProgressBarIndeterminateVisibility(true);
	}

	protected void onPostExecute(QueryResult result) {
		if (result.getError() != null) {
			Toast.makeText(this.context, "Fehler: " + result.getError(), Toast.LENGTH_LONG)
					.show();
			resultView.setText(result.getError());

			return;
		}
		
		ListExpr resultList = result.getResultList();
		
		if (resultList == null || resultList.isEmpty()) {
//			Toast.makeText(this.context, "kein Ergebnis", Toast.LENGTH_LONG)
//					.show();
		} else {
			String error = result.getError();
			if (error != null && !"".equals(error)) {
				resultView.setText(error);
				Toast.makeText(
						this.context,
						"Fehler beim Ausführen: "
								+ result.getError(),
						Toast.LENGTH_LONG).show();
				return;
			}

			if (resultView != null) {
				if (result.getCommand().startsWith("list ")) { // list Objects, list algebras
					resultView.setText(result.getResultString());
					resultView.setTag(null);
				} else {
					resultView.setText(this.context.getString(R.string.queryexecuted));
					resultView.setTag(result);
				}
			}
		}
		activity.setProgressBarIndeterminateVisibility(false);
		if (activity instanceof CommandActivity) {
			if (!result.getCommand().startsWith("open ") && !result.getCommand().startsWith("close ")) {
				((CommandActivity) activity).postAsyncTask();
			}
		}
}
}