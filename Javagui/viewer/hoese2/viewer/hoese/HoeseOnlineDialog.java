package viewer.hoese;

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
import javax.swing.JSeparator;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.SpinnerDateModel;
import javax.swing.SpinnerNumberModel;
import javax.swing.SwingConstants;

import viewer.HoeseViewer;

/**
 * JDialog allowing the user the configuraiton of the online results
 * Provides getters to retrieve the enterd Data
 * @author secondo
 *
 */
public class HoeseOnlineDialog extends JDialog {

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
	 * @param hoese
	 */
	public HoeseOnlineDialog(HoeseViewer hoese) {
		super(hoese.getMainFrame(), true);
		getContentPane().setLayout(new BorderLayout());

		setTitle("Online viewing configuration");
		
		OkBtn = new JButton("ok");
		CancelBtn = new JButton("cancel");
		
		JPanel P1 = new JPanel(new GridLayout(8, 2));
		
		JPanel P2 = new JPanel();
		P2.add(OkBtn);
		P2.add(CancelBtn);
		
		getContentPane().add(P1, BorderLayout.CENTER);
		
		relationLabel = new JLabel("Relation Name:");
		P1.add(relationLabel);
		
		relationTextField = new JTextField(10);
		P1.add(relationTextField);
		
		filterLabel = new JLabel("Filter command:");
		P1.add(filterLabel);
		
		filterTextField = new JTextField(10);
		P1.add(filterTextField);
		
		limitLabel = new JLabel("Max. Tuples:");
		P1.add(limitLabel);
		
		limitTextField = new JTextField(10);
		limitTextField.setText("2500");
		P1.add(limitTextField);
		
		rateLabel = new JLabel("Update Rate (ms):");
		P1.add(rateLabel);
		
		rateTextField = new JTextField(10);
		rateTextField.setText("2500");
		P1.add(rateTextField);
		
		rtLabel = new JLabel("RealTime Simulation");
		P1.add(rtLabel);
		
		rtCheckBox = new JCheckBox();
		P1.add(rtCheckBox);
		
		rtSpeedFactor = new JLabel("Geschwindigkeitsfaktor:");
		P1.add(rtSpeedFactor);
		
		rtSpeedFactorSpinnerModel = new SpinnerNumberModel(1.0, 0.1, 10.0, 0.1);
		rtSpeedFactorSpinner = new JSpinner(rtSpeedFactorSpinnerModel);
		P1.add(rtSpeedFactorSpinner);
		
		rtSimulationTimeLabel = new JLabel("Simulationszeit:");
		P1.add(rtSimulationTimeLabel);
		
		rtCurrentTimeModel = new SpinnerDateModel();
		rtCurrentTimeSpinner = new JSpinner(rtCurrentTimeModel);
		P1.add(rtCurrentTimeSpinner);
		
		rtCurrentTimeOffset = new JLabel("Zeitversatz:");
		P1.add(rtCurrentTimeOffset);
		
		rtCurrentTimeOffsetSpinnerModel = new SpinnerNumberModel(10, 5, Integer.MAX_VALUE, 1);
		rtCurrentTimeOffsetSpinner = new JSpinner(rtCurrentTimeOffsetSpinnerModel);
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
	 * @return
	 */
	public String getResult() {
		return Result;
	}
	
	/**
	 * Returns the entered RelationName
	 * @return
	 */
	public String getRelationName(){
		return relationTextField.getText();
	}
	
	/**
	 * Returns the entered filter Command
	 * @return
	 */
	public String getFilterCommand(){
		return filterTextField.getText();
	}
	
	/** 
	 * Returns the enterd Limit. If no Limit was enterd, 0 will be returned. 
	 * @return
	 */
	public Integer getMaxTuples(){
		String s =limitTextField.getText();
		if(s.isEmpty()){
			return 0;
		}else{
			return Integer.parseInt(s);
		}
	}
	
	/** 
	 * Returns the enterd updaterate. If no rate was enterd, 0 will be returned. 
	 * @return
	 */
	public Integer getUpdateRate(){
		String s =rateTextField.getText();
		if(s.isEmpty()){
			return 0;
		}else{
			return Integer.parseInt(s);
		}
	}
	
	/**
	 * Returns if this run sould be treated as simulation 
	 * @return
	 */
	public boolean isSimulation(){
		return rtCheckBox.isSelected();
	}
	
	/**
	 * Returns the speedFactor entered by the user
	 * @return
	 */
	public double getSpeedFactor(){
		return (double)rtSpeedFactorSpinner.getValue();
	}
	
	/**
	 * Returns the Time that should be used for the Simulation
	 * @return
	 */
	public Date getCurrentTime(){
		return (Date)rtCurrentTimeSpinner.getValue();
	}
	
	/**
	 * Returns te TimeOffset used while playing the simulation
	 */
	public int getCurrentTimeOffset(){
		return (int)rtCurrentTimeOffsetSpinner.getValue();
	}
}
