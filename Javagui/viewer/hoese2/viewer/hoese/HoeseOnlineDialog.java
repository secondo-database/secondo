package viewer.hoese;

import gui.MainWindow;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Date;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.SpinnerDateModel;
import javax.swing.SpinnerNumberModel;

import viewer.HoeseViewer;

/**
 * JDialog allowing the user the configuraiton of the online results Provides
 * getters to retrieve the enterd Data
 * 
 * @author secondo
 *
 */
public class HoeseOnlineDialog extends JDialog {

	public static class LastValues {
		public String relation;
		public String filter;
		public Integer limit;
		public Integer rate;
		public String remotePort;
		public Boolean checkBox;
		public Double speedFactor;
		public Date currentTime;
		public Integer timeOffset;
		public String database;
	};

	public static LastValues lastValues = new LastValues();

	public static String CANCELED = "canceld";
	public static String OK = "ok";

	private JButton CancelBtn;
	private JButton OkBtn;
	private String Result;
	private JLabel relationLabel;
	private JTextField relationTextField;
	private JLabel filterLabel;
	private JTextField filterTextField;
	private JLabel limitLabel;
	private JTextField limitTextField;
	private JLabel rateLabel;
	private JTextField rateTextField;
	private JLabel rtLabel;
	private JTextField remotePortTextField;
	private JLabel remotePortLabel;
	private JCheckBox rtCheckBox;
	private JLabel rtSpeedFactor;
	private SpinnerNumberModel rtSpeedFactorSpinnerModel;
	private JSpinner rtSpeedFactorSpinner;
	private SpinnerDateModel rtCurrentTimeModel;
	private JSpinner rtCurrentTimeSpinner;
	private JLabel rtSimulationTimeLabel;
	private JLabel rtCurrentTimeOffset;
	private SpinnerNumberModel rtCurrentTimeOffsetSpinnerModel;
	private JSpinner rtCurrentTimeOffsetSpinner;

	/**
	 * Default constructor to start this Dialog
	 * 
	 * @param hoese
	 */
	public HoeseOnlineDialog(HoeseViewer hoese) {
		super(hoese.getMainFrame(), true);
		getContentPane().setLayout(new BorderLayout());
		
		lastValues.database = MainWindow.getCurrentDatabase();

		setTitle("Online viewing configuration");

		OkBtn = new JButton("ok");
		CancelBtn = new JButton("cancel");

		JPanel P1 = new JPanel(new GridLayout(9, 2));

		JPanel P2 = new JPanel();
		P2.add(OkBtn);
		P2.add(CancelBtn);

		getContentPane().add(P1, BorderLayout.CENTER);

		relationLabel = new JLabel("Relation Name:");
		relationLabel.setToolTipText("Relation which should be watched for updates");
		P1.add(relationLabel);

		relationTextField = new JTextField(10);
		relationTextField.setText(lastValues.relation);
		P1.add(relationTextField);

		filterLabel = new JLabel("Filter command:");
		filterLabel.setToolTipText("Secondo filter command to reduce the tuples which should be watched");
		P1.add(filterLabel);

		filterTextField = new JTextField(10);
		filterTextField.setText(lastValues.filter);
		P1.add(filterTextField);

		limitLabel = new JLabel("Max. Tuples:");
		limitLabel.setToolTipText("The maximum number of tuples in the result set");
		P1.add(limitLabel);

		limitTextField = new JTextField(10);
		limitTextField.setText(lastValues.limit == null ? "2500"
				: lastValues.limit.toString());
		P1.add(limitTextField);

		rateLabel = new JLabel("Update Rate (ms):");
		rateLabel.setToolTipText("Interval at which the Javagui is updated with newly received tuples");
		P1.add(rateLabel);

		rateTextField = new JTextField(10);
		rateTextField.setText(lastValues.rate == null ? "2500"
				: lastValues.rate.toString());
		P1.add(rateTextField);

		remotePortLabel = new JLabel("Remote Port:");
		remotePortLabel.setToolTipText("The port under which the Secondo-Server delivers updates for the realtion. Should corrospond to the configuration of the webserver.");
		P1.add(remotePortLabel);

		remotePortTextField = new JTextField(10);
		remotePortTextField.setText(lastValues.remotePort == null ? "9000"
				: lastValues.remotePort.toString());
		P1.add(remotePortTextField);

		rtLabel = new JLabel("Realtime Simulation");
		rtLabel.setToolTipText("Enables the realtime simulation mode");
		P1.add(rtLabel);

		rtCheckBox = new JCheckBox();
		rtCheckBox.setSelected(lastValues.checkBox == null ? false
				: lastValues.checkBox);
		P1.add(rtCheckBox);

		rtSpeedFactor = new JLabel("Speed factor:");
		rtSpeedFactor.setToolTipText("Speedfactor of the simulation time. Defining a value of 1 would use playback in realtime speed, 2 would make it two times faster");
		P1.add(rtSpeedFactor);

		rtSpeedFactorSpinnerModel = new SpinnerNumberModel(1.0, 0.1, 10.0, 0.1);
		rtSpeedFactorSpinner = new JSpinner(rtSpeedFactorSpinnerModel);
		if (lastValues.speedFactor != null) {
			rtSpeedFactorSpinner.setValue(lastValues.speedFactor);
		}
		P1.add(rtSpeedFactorSpinner);

		rtSimulationTimeLabel = new JLabel("Simulation time:");
		rtSimulationTimeLabel.setToolTipText("Time used in the simulation");
		P1.add(rtSimulationTimeLabel);

		rtCurrentTimeModel = new SpinnerDateModel();
		rtCurrentTimeSpinner = new JSpinner(rtCurrentTimeModel);
		if (lastValues.currentTime != null) {
			rtCurrentTimeSpinner.setValue(lastValues.currentTime);
		}
		P1.add(rtCurrentTimeSpinner);

		rtCurrentTimeOffset = new JLabel("Time delay:");
		rtCurrentTimeOffset.setToolTipText("Delay used for the simulation mode, playback will start x seconds before the currently defined simulation time");
		P1.add(rtCurrentTimeOffset);

		rtCurrentTimeOffsetSpinnerModel = new SpinnerNumberModel(10, 5,
				Integer.MAX_VALUE, 1);
		rtCurrentTimeOffsetSpinner = new JSpinner(
				rtCurrentTimeOffsetSpinnerModel);
		if (lastValues.timeOffset != null) {
			rtCurrentTimeOffsetSpinner.setValue(lastValues.timeOffset);
		}
		P1.add(rtCurrentTimeOffsetSpinner);

		getContentPane().add(P2, BorderLayout.SOUTH);

		ActionListener ButtonControl = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				Object O = evt.getSource();
				if (O.equals(CancelBtn)) {
					Result = CANCELED;
					setVisible(false);
				}
				if (O.equals(OkBtn)) {
					Result = OK;
					setVisible(false);
				}
			}
		};

		CancelBtn.addActionListener(ButtonControl);
		OkBtn.addActionListener(ButtonControl);

		this.pack();
		this.setSize(300, 200);
		this.validate();
	}

	/**
	 * Returns CANCELD or OK, depending on which Button was pressed by the user
	 * 
	 * @return
	 */
	public String getResult() {
		return Result;
	}

	/**
	 * Returns the entered RelationName
	 * 
	 * @return
	 */
	public String getRelationName() {
		lastValues.relation = relationTextField.getText();
		return lastValues.relation;
	}

	/**
	 * Returns the entered filter Command
	 * 
	 * @return
	 */
	public String getFilterCommand() {
		lastValues.filter = filterTextField.getText();
		return lastValues.filter;
	}

	/**
	 * Returns the enterd Limit. If no Limit was enterd, 0 will be returned.
	 * 
	 * @return
	 */
	public Integer getMaxTuples() {
		String s = limitTextField.getText();
		if (s.isEmpty()) {
			lastValues.limit = 0;
		} else {
			lastValues.limit = Integer.parseInt(s);
		}
		return lastValues.limit;
	}

	/**
	 * Returns the enterd updaterate. If no rate was enterd, 0 will be returned.
	 * 
	 * @return
	 */
	public Integer getUpdateRate() {
		String s = rateTextField.getText();
		if (s.isEmpty()) {
			lastValues.rate = 0;
		} else {
			lastValues.rate = Integer.parseInt(s);
		}
		return lastValues.rate;
	}

	/**
	 * Returns if this run sould be treated as simulation
	 * 
	 * @return
	 */
	public boolean isSimulation() {
		lastValues.checkBox = rtCheckBox.isSelected();
		return lastValues.checkBox;
	}

	/**
	 * Returns the speedFactor entered by the user
	 * 
	 * @return
	 */
	public double getSpeedFactor() {
		lastValues.speedFactor = (Double) rtSpeedFactorSpinner.getValue();
		return lastValues.speedFactor;
	}

	/**
	 * Returns the Time that should be used for the Simulation
	 * 
	 * @return
	 */
	public Date getCurrentTime() {
		lastValues.currentTime = (Date) rtCurrentTimeSpinner.getValue();
		return lastValues.currentTime;
	}

	/**
	 * Returns the TimeOffset used while playing the simulation
	 */
	public int getCurrentTimeOffset() {
		lastValues.timeOffset = (Integer) rtCurrentTimeOffsetSpinner.getValue();
		return lastValues.timeOffset;
	}

	/**
	 * Returns the remote Port
	 */
	public String getRemotePort() {
		lastValues.remotePort = remotePortTextField.getText();
		return lastValues.remotePort;
	}
}
