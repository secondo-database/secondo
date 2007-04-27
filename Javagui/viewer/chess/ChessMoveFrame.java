package viewer.chess;


import javax.swing.BoxLayout;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JRadioButton;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.SwingConstants;
import javax.swing.border.Border;
import javax.swing.BorderFactory;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridLayout;
import java.awt.event.*;
import sj.lang.ListExpr;
import tools.Reporter;

public class ChessMoveFrame extends ChessObject implements ActionListener {
	private ChessField cf;
	private ChessMoveDates cm = null;
	private JButton repeat,setStart, setEnd;
	private JComboBox movFigBox, capFigBox, proFigBox; 
	private JRadioButton chess, mate;
	private JTextField number, movFig, capFig, proFig;
	private JPanel aPanel;
	private ListExpr val;
	private JLabel invalid;
	
public ChessMoveFrame(ListExpr value){
	this.val = value;
	cm = ChessObject.parseChessMove(val);
	cf = new ChessField();
    //System.out.println(val.writeListExprToString()); 
	GridLayout grid = new GridLayout(7,2);
	grid.setHgap(5);
	grid.setVgap(5);
	aPanel = new JPanel(grid);
	this.setLayout(new BorderLayout());
	//System.out.println("Hier in chessframe its a move" + this.getWidth()+ this.getHeight());
	repeat = new JButton(" repeat ");
	repeat.setBorder(ChessObject.compound5);
	repeat.setToolTipText("show the animation again");
	invalid = new JLabel("invalid move");
	aPanel.add(repeat);
	aPanel.add(invalid);
	invalid.setHorizontalAlignment(SwingConstants.CENTER);
	invalid.setVisible(!cm.checkMove(cf, true));
	invalid.setBorder(BorderFactory.createCompoundBorder(
			BorderFactory.createLineBorder(Color.red), 
			BorderFactory.createMatteBorder(2, 2, 2, 2, getBackground())));
	//buttonPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
	repeat.addActionListener(this);
	setStart =  new JButton(" set start ");
	setStart.setEnabled(true);
	setStart.setBorder(ChessObject.compound5);
	setStart.setToolTipText("set new startfield for move");
	setEnd =  new JButton(" set end ");
	setEnd.setEnabled(true);
	setEnd.setBorder(ChessObject.compound5);
	setEnd.setToolTipText("set new endfield for move");
	number = new JTextField(3);
	number.setText(new Integer(cm.moveNo).toString());
	number.setHorizontalAlignment(JTextField.RIGHT);
	number.setActionCommand("numberChanged");
	number.setEditable(false);
	number.setBorder(ChessObject.linecompound3);
	movFig=new JTextField(10);
	movFig.setText(ChessObject.selection[cm.movFigure]);
	movFig.setHorizontalAlignment(JTextField.RIGHT);
	movFig.setEditable(false);
	movFig.setBorder(ChessObject.linecompound3);
	capFig=new JTextField(10);
	capFig.setText(ChessObject.selection[cm.capFigure]);
	capFig.setHorizontalAlignment(JTextField.RIGHT);
	capFig.setEditable(false);
	capFig.setBorder(ChessObject.linecompound3);
	proFig=new JTextField(10);
	proFig.setText(ChessObject.selection[cm.proFigure]);
	proFig.setHorizontalAlignment(JTextField.RIGHT);
	proFig.setEditable(false);
	proFig.setBorder(ChessObject.linecompound3);
	chess = new JRadioButton("chess given");
	chess.setSelected(cm.chess);
	mate= new JRadioButton("chess mate");
	mate.setSelected(cm.mate);
	movFigBox = new JComboBox(ChessObject.selection);
	movFigBox.setSelectedIndex(cm.movFigure);
	movFigBox.addActionListener(this);
	capFigBox = new JComboBox(ChessObject.selection);
	capFigBox.setSelectedIndex(cm.capFigure);
	capFigBox.addActionListener(this);
	proFigBox = new JComboBox(ChessObject.selection);
	proFigBox.setSelectedIndex(cm.proFigure);
	proFigBox.addActionListener(this);
	movFigBox.setEditable(false);
	capFigBox.setEditable(false);
	proFigBox.setEditable(false);
	movFigBox.setEnabled(true);
	capFigBox.setEnabled(true);
	proFigBox.setEnabled(true);
	aPanel.add(new JLabel("movenumber: "));
	aPanel.add(number);
	aPanel.add(new JLabel("moved figure: "));
	aPanel.add(movFig);
	aPanel.add(new JLabel("captured figure: "));
	aPanel.add(capFig);
	aPanel.add(new JLabel("promoted figure: "));
	aPanel.add(proFig);
	aPanel.add(chess);
	aPanel.add(mate);
	aPanel.add(setStart);
	aPanel.add(setEnd);
	aPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
	Box bBox = new Box(BoxLayout.X_AXIS);
	cf.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
	bBox.add(aPanel);
	bBox.add(cf);
	this.add(bBox);
	this.validate();
	this.paintImmediately(this.getVisibleRect());
	this.showMove();
}
    
	public void showMove() {
		if (cm != null) {
			long myTime  = System.currentTimeMillis();
			while (System.currentTimeMillis() < myTime+60);
			cm.move(cf);
			//text.setText(cm.getInfo());
		}
	}
    
	public void update(ListExpr value) {
		if (cm != null) {
			val = value;
			cm.clearMove(cf);
			cm = ChessObject.parseChessMove(val);
			number.setText(new Integer(cm.moveNo).toString());
			/* activate this part if added edit moves in relations
			movFigBox.setSelectedIndex(cm.movFigure);
			capFigBox.setSelectedIndex(cm.capFigure);
			proFigBox.setSelectedIndex(cm.proFigure);*/
			invalid.setVisible(!cm.checkMove(cf, true));
			movFig.setText(ChessObject.selection[cm.movFigure]);
			capFig.setText(ChessObject.selection[cm.capFigure]);
			proFig.setText(ChessObject.selection[cm.proFigure]);
			this.validate();
			//this.paintImmediately(this.getVisibleRect());
			this.showMove();
		}
	}   
      
    /**
     * Invoked when an action on the buttons, comboboxes,  occurs.
     */
	public void actionPerformed(ActionEvent arg0) {
		if (arg0.getActionCommand().equals("chess given")) {
			cm.chess = chess.isSelected();
		}
		if (arg0.getActionCommand().equals("chess mate")) {
			cm.mate = mate.isSelected();
		}
		if (arg0.getActionCommand().equals("numberChanged")) {
			try {  // test if no input is a integer
        		int i = Integer.parseInt(number.getText());
        		cm.moveNo = i;
        		//cm.setBack(cf);
        		invalid.setVisible(!cm.checkMove(cf, true));
        		cm.move(cf);
			}
			catch (Exception e) { // if not set to old value
				number.setText(Integer.toString(cm.moveNo));
			}
		}
		if (arg0.getActionCommand().equals(" repeat ")) {
			// repeat showing the move
			if (cm != null) {
				cm.setBack(cf);
				long myTime  = System.currentTimeMillis();
				while (System.currentTimeMillis() < myTime+60);
				invalid.setVisible(!cm.checkMove(cf, true));
				cm.move(cf);
			}
		}
		if (arg0.getActionCommand().equals("comboBoxChanged")) {
			cm.clearMove(cf);
			cm.movFigure = movFigBox.getSelectedIndex();
			cm.capFigure = capFigBox.getSelectedIndex();
			cm.proFigure = proFigBox.getSelectedIndex();
			invalid.setVisible(!cm.checkMove(cf,true));
			movFig.setText(ChessObject.selection[cm.movFigure]);
			capFig.setText(ChessObject.selection[cm.capFigure]);
			proFig.setText(ChessObject.selection[cm.proFigure]);
			if (movFigBox.getSelectedIndex()!= cm.movFigure) {
				movFigBox.removeActionListener(this);
				movFigBox.setSelectedIndex(cm.movFigure);
				movFigBox.addActionListener(this);
			}	
			if (capFigBox.getSelectedIndex()!= cm.capFigure) {
				capFigBox.removeActionListener(this);
				capFigBox.setSelectedIndex(cm.capFigure);
				capFigBox.addActionListener(this);
			}	
			if (proFigBox.getSelectedIndex()!= cm.proFigure) {
				proFigBox.removeActionListener(this);
				proFigBox.setSelectedIndex(cm.proFigure);
				proFigBox.addActionListener(this);
			}
			cm.setBack(cf);
			cm.move(cf);
		}
		if (arg0.getActionCommand().equals(" set start ")) {
			cf.editMove(this, true);
		}
		if (arg0.getActionCommand().equals(" set end ")) {
			cf.editMove(this, false);
		}
	}
	
	public void execSet(int x, int y, boolean isStart) {
		// clear old
		/*int [][] empty = {{0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0}}; */
		cm.clearMove(cf);
		// update fields
		if (isStart)  {
			cm.colS= x;
			cm.rowS= y;
		}
		else {
			cm.colE= x;
			cm.rowE= y;
		}
		// set new
		cf.setFigure(cm.colS, cm.rowS, cm.movFigure);
		cf.setFigure(cm.colE, cm.rowE, cm.capFigure);
		// check
		invalid.setVisible(!cm.checkMove(cf, true));
		// update the infos
		movFig.setText(ChessObject.selection[cm.movFigure]);
		capFig.setText(ChessObject.selection[cm.capFigure]);
		proFig.setText(ChessObject.selection[cm.proFigure]);
		movFigBox.removeActionListener(this);
		capFigBox.removeActionListener(this);
		proFigBox.removeActionListener(this);
		movFigBox.setSelectedIndex(cm.movFigure);
		capFigBox.setSelectedIndex(cm.capFigure);
		proFigBox.setSelectedIndex(cm.proFigure);
		movFigBox.addActionListener(this);
		capFigBox.addActionListener(this);
		proFigBox.addActionListener(this);
		// show move
		cm.setBack(cf);
		cm.move(cf);	
	}
	/*
	 * @see viewer.chess.ChessInterface#getType()
	 */
	public String getType() {
		return "chessmove";
	}

	/*
	 * @see viewer.chess.ChessInterface#changeToEdit(boolean)
	 */
	public boolean changeToEdit(boolean edit) {
		editModus = edit;
		if (edit) {
			aPanel.remove(movFig);
			aPanel.add(movFigBox,5);
			aPanel.remove(capFig);
			aPanel.add(capFigBox,7);
			aPanel.remove(proFig);
			aPanel.add(proFigBox,9);
			setStart.addActionListener(this);
			setEnd.addActionListener(this);
			chess.addActionListener(this);
			mate.addActionListener(this);
			number.addActionListener(this);
			number.setEditable(true);
		}
		else {
			aPanel.remove(movFigBox);
			aPanel.add(movFig,5);
			aPanel.remove(capFigBox);
			aPanel.add(capFig,7);
			aPanel.remove(proFigBox);
			aPanel.add(proFig,9);
			setStart.removeActionListener(this);
			setEnd.removeActionListener(this);
			chess.removeActionListener(this);
			mate.removeActionListener(this);
			number.removeActionListener(this);
			number.setEditable(false);
		}
		this.validate();
		return true;
	}

	public ListExpr getListExpr() {		
		return cm.getListExpr();
	}

	public boolean canEdit() {
		return true;
	}

	public boolean canExport() {
		return false;
	}
	
	public String getQuery(int i) {
		return ChessObject.getQueryMoves(i, cm);
	}
}

