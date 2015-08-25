package viewer.hoese.algebras.continuousupdate;

import java.awt.event.ActionListener;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Vector;

import javax.swing.Timer;

import viewer.HoeseViewer;
import viewer.hoese.DateTime;
import viewer.hoese.TextWindow;
import components.LongScrollBar;

public class AnimSimulationCtrlListener implements ActionListener {

	boolean isRunning = true;
	LongScrollBar TimeSlider;
	Timer AnimTimer;
	Calendar simulationTime;
	double speedFactor;
	SimpleDateFormat format;
	
	public void stop(){
		AnimTimer.stop();
	}

	public AnimSimulationCtrlListener(final HoeseViewer hoese, Date time,
			int offset, double factor) {
		simulationTime = Calendar.getInstance();
		simulationTime.setTime(time);
		simulationTime.add(Calendar.SECOND, -1 * offset);
		speedFactor = factor;
		format = new SimpleDateFormat("yyyy-MM-dd-HH:mm:ss");

		AnimTimer = new Timer(1000, new ActionListener() {

			public void actionPerformed(java.awt.event.ActionEvent evt) {

				simulationTime.add(Calendar.MILLISECOND,
						(int) (1000 * speedFactor));

				if (isRunning) {

					hoese.setTime(DateTime.getDateTime(format
							.format(simulationTime.getTime())));
				}
			}
		});

		AnimTimer.start();
	}

	public void actionPerformed(java.awt.event.ActionEvent evt) {
		switch (Integer.parseInt(evt.getActionCommand())) {
		case 0: // play
			isRunning = true;
			break;
		case 5: // stop
			isRunning = false;
			break;
		}
	}

}
