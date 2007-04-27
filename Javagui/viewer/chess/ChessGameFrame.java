package viewer.chess;

import javax.swing.*;
import javax.swing.border.TitledBorder;
import java.awt.Color;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.*;

import javax.swing.event.*; 
import java.util.LinkedList;
import sj.lang.ListExpr;
import tools.Reporter;

/*
 * 
 */
public class ChessGameFrame extends ChessObject implements ActionListener {
	private Thread mythread;
	private ChessField cf;
	private JSlider slide;
	private JButton autofor,stopp,faster,slower,oneback,onefor;
	private JButton setStart, setEnd;
	private JComboBox proFigBox; 
	private JRadioButton chess, mate;
	private JTextField movFig, capFig, proFig, no;
	private JPanel aPanel;
	private JScrollPane metaPane;
	private JLabel invalid;
	private JTable metaText;
	private long waitTime = 1000;
	private int showElem = 0;
	private boolean paused = false;

	private static final int[][] start = {{10,7,0,0,0,0,1,4},
			{9,7,0,0,0,0,1,3},
			{8,7,0,0,0,0,1,2},
			{11,7,0,0,0,0,1,5},
			{12,7,0,0,0,0,1,6},
			{8,7,0,0,0,0,1,2},
			{9,7,0,0,0,0,1,3},
			{10,7,0,0,0,0,1,4}};
	private LinkedList movesList = new LinkedList();
	private ChessMoveDates cm = null;
	private SlideListener slidelist = new SlideListener();
	
	/*
	 * constructor of ChessGameFrame
	 */
	public ChessGameFrame(ListExpr value){
		//System.out.println(value.writeListExprToString());
		val = value;
		cf = new ChessField(start);
		metaText = this.parseChessGame(val);
		metaText.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
		metaPane = new JScrollPane();
		metaPane.setBorder(BorderFactory.createCompoundBorder(
				BorderFactory.createEmptyBorder(3,3,3,3),
				BorderFactory.createTitledBorder(
				BorderFactory.createLineBorder(Color.BLACK), "meta-dates",
				TitledBorder.CENTER,TitledBorder.TOP)));
		metaPane.setViewportView(metaText);
		//System.out.println("here in chessgameframe"); 
		JPanel buttonPanel = new JPanel();
		JPanel labelPanel = new JPanel();
		Box aBox = new Box(BoxLayout.Y_AXIS);
		GridLayout grid = new GridLayout(5,2);
		grid.setHgap(5);
		grid.setVgap(5);
		aPanel = new JPanel(grid);
		this.setLayout(new BorderLayout());
		//System.out.println("here in chessframe its a move" + this.getWidth()+ this.getHeight());
		invalid = new JLabel(" INVALID MOVE ");
		invalid.setBackground(Color.orange);
		invalid.setBorder(BorderFactory.createCompoundBorder(
				BorderFactory.createLineBorder(Color.red), 
				BorderFactory.createEmptyBorder(2, 2, 2, 2)));
		invalid.setVisible(false);
		labelPanel.add(invalid);
		setStart =  new JButton(" set start ");
		setStart.setEnabled(false);
		setStart.setBorder(ChessObject.compound3);
		setStart.setToolTipText("set new startfield for move");
		setEnd =  new JButton(" set end ");
		setEnd.setEnabled(false);
		setEnd.setBorder(ChessObject.compound3);
		setEnd.setToolTipText("set new endfield for move");
		movFig=new JTextField(11);
		movFig.setHorizontalAlignment(JTextField.RIGHT);
		movFig.setEditable(false);
		movFig.setBorder(ChessObject.linecompound3);
		capFig=new JTextField(11);
		capFig.setHorizontalAlignment(JTextField.RIGHT);
		capFig.setEditable(false);
		capFig.setBorder(ChessObject.linecompound3);
		proFig=new JTextField(11);
		proFig.setHorizontalAlignment(JTextField.RIGHT);
		proFig.setEditable(false);
		proFig.setBorder(ChessObject.linecompound3);
		chess = new JRadioButton("chess given");
		mate= new JRadioButton("chess mate");
		proFigBox = new JComboBox(ChessObject.selection);
		proFigBox.setEditable(false);
		proFigBox.setEnabled(true);
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
		aPanel.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
		this.setMoveInfos();
		this.setLayout(new BoxLayout(this,BoxLayout.X_AXIS));
		onefor = new JButton(" + ");
		oneback = new JButton(" - ");
		slower = new JButton("<<");
		faster = new JButton(">>");
		autofor = new JButton(" > ");
		stopp = new JButton(" || ");
		autofor.setToolTipText("show the game move by move");
		slower.setToolTipText("show the game slower");
		faster.setToolTipText("show the game faster");
		oneback.setToolTipText("show the previous move");
		onefor.setToolTipText("show the next move");
		stopp.setToolTipText("pause showing the game");
		autofor.setBorder(ChessObject.compound3);
		slower.setBorder(ChessObject.compound3);
		faster.setBorder(ChessObject.compound3);
		oneback.setBorder(ChessObject.compound3);
		onefor.setBorder(ChessObject.compound3);
		stopp.setBorder(ChessObject.compound3);
		autofor.setEnabled(true);
		slower.setEnabled(false);
		faster.setEnabled(false);
		oneback.setEnabled(false);
		onefor.setEnabled(true);
		stopp.setEnabled(false);
		no = new JTextField(2);
		no.setToolTipText("number of actual move");
		no.setHorizontalAlignment(JTextField.RIGHT);
		no.setText("0");
		no.setEditable(true);
		no.setBorder(ChessObject.linecompound2);
		buttonPanel.add(oneback);
		buttonPanel.add(onefor);
		buttonPanel.add(autofor);
		buttonPanel.add(stopp);
		buttonPanel.add(faster);
		buttonPanel.add(slower);
		buttonPanel.add(no);
		//System.out.println("current size " +movesList.size() );
		slide = new JSlider(SwingConstants.HORIZONTAL,0,movesList.size(),0);
		slide.setMinorTickSpacing(1);
		slide.setMajorTickSpacing(10);
		slide.setPaintTicks(true);
		slide.setSnapToTicks(true);
		slide.setEnabled(true);
		oneback.addActionListener(this);
		onefor.addActionListener(this);
		faster.addActionListener(this);
		slower.addActionListener(this);
		autofor.addActionListener(this);
		stopp.addActionListener(this);
		no.addActionListener(new NoListener());
		slide.addChangeListener(slidelist);
		aBox.add(buttonPanel);
		aBox.add(Box.createVerticalStrut(3));
		aBox.add(slide);
		aBox.add(Box.createVerticalStrut(3));
		aBox.add(labelPanel);
		aBox.add(Box.createVerticalStrut(3));
		aBox.add(aPanel);
		aBox.add(Box.createVerticalStrut(3));
		cf.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
		aBox.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
		this.add(aBox, BorderLayout.WEST);
		this.add(cf,BorderLayout.CENTER);
		this.add(metaPane,BorderLayout.EAST);	
		//System.out.println("game building ready" ); 
		//System.out.println(this.getListExpr().writeListExprToString()); 
		this.validate();
	}
	/*
	 * sets the text in the textfields to actual dates and check the move if valid
	 */
	private void setMoveInfos() {
		proFigBox.removeActionListener(this);
		if (cm == null) {
			chess.setSelected(false);
			mate.setSelected(false);
			movFig.setText(ChessObject.selection[0]);
			capFig.setText(ChessObject.selection[0]);
			proFig.setText(ChessObject.selection[0]);
			proFigBox.setSelectedIndex(0);
			invalid.setVisible(false);
			if (aPanel.getComponent(5).equals(proFigBox)) {
				aPanel.remove(proFigBox);
				aPanel.add(proFig,5);
			}
		}
		else {	
			chess.setSelected(cm.chess);
			mate.setSelected(cm.mate);
			movFig.setText(ChessObject.selection[cm.movFigure]);
			capFig.setText(ChessObject.selection[cm.capFigure]);
			proFig.setText(ChessObject.selection[cm.proFigure]);
			proFigBox.setSelectedIndex(cm.proFigure);
			if (editModus && ((cm.movFigure == 1 && cm.rowE == 0) || 
				(cm.movFigure == 7 && cm.rowE == 7))) {
					if (aPanel.getComponent(5).equals(proFig)) {
						aPanel.remove(proFig);
						aPanel.add(proFigBox,5);
					}
					proFigBox.addActionListener(this);
			}
			else {
				if (aPanel.getComponent(5).equals(proFigBox)) {
					aPanel.remove(proFigBox);
					aPanel.add(proFig,5);
				}
			}
			invalid.setVisible(!cm.checkMove(cf, false));
		}		
	}
	/*
	 * its used when in a relation to update a new Chessgame
	 * @see viewer.chess.ChessInterface#update(sj.lang.ListExpr)
	 */
	public void update(ListExpr value) {
		val = value;
		movesList.removeAll(movesList);
		metaText = this.parseChessGame(val);
		metaPane.setViewportView(metaText);
		this.cf.setField(start);
		this.cm = null;
		setMoveInfos();
		this.showElem=0;
		this.slide.setMaximum(movesList.size());
		this.slide.setValue(0);
		this.no.setText(Integer.toString(0));
		this.validate();
		this.repaint();
	}
	/*
	 * involved when buttons or combobox clicked 
	 * @see java.awt.event.ActionListener#actionPerformed(java.awt.event.ActionEvent)
	 */
	public void actionPerformed(ActionEvent arg0) {
		//System.out.println("Actionevent "+ arg0.getActionCommand());
		if (arg0.getActionCommand().equals("chess given")) {
			if (cm != null) {
				cm.chess = chess.isSelected();
				if (cm.movFigure !=0)
					cf.markTheKing(cm.movFigure >6, chess.isSelected());
			}
			else {
				chess.setSelected(false);
				Reporter.showError("no move selected");
			}
		}	
		if (arg0.getActionCommand().equals("chess mate")) {
			if (cm != null) {
				cm.mate = mate.isSelected();
				if (cm.movFigure !=0)
					cf.markTheKing(cm.movFigure >6, mate.isSelected());
			}
			else {
				mate.setSelected(false);
				Reporter.showError("no move selected");
			}
		}	
			if (arg0.getActionCommand().equals(" + ")) {
			// one for
				if (this.showElem < movesList.size()) {
					if (this.showElem > 0) { 
						((ChessMoveDates)movesList.get(this.showElem-1)).unmark(cf);
					}
					showElem++;
					this.cm = ((ChessMoveDates)movesList.get(this.showElem-1));
					setMoveInfos();
					this.cm.move(this.cf);
					this.slide.removeChangeListener(this.slidelist);
					this.slide.setValue(this.showElem);
					this.slide.addChangeListener(this.slidelist);
					this.no.setText(Integer.toString(this.showElem));
    			}
				else { 
					if (editModus) {
						if (this.showElem > 0) 
							((ChessMoveDates)movesList.get(this.showElem-1)).unmark(cf);
						this.showElem++;
						this.cm = new ChessMoveDates(this.showElem);
						this.movesList.add(this.cm);
						setMoveInfos();
						this.slide.removeChangeListener(this.slidelist);
						this.slide.setMaximum(this.showElem);
						this.slide.setValue(this.showElem);
						this.slide.addChangeListener(this.slidelist);
						this.no.setText(Integer.toString(this.showElem));
					}
				}	
				this.oneback.setEnabled(this.showElem > 0);
				this.onefor.setEnabled(this.showElem < this.movesList.size()|| editModus);
			}
			if (arg0.getActionCommand().equals(" - ")) {
			// one back
				if (showElem > 0) {
					this.cm = ((ChessMoveDates)movesList.get(this.showElem-1));
					this.cm.setBack(cf);
					this.setMoveInfos();
					this.showElem--;
				}
				if (showElem ==0){
					this.cf.setField(start);
					this.cm = null;
					this.setMoveInfos();
				}				
				this.slide.removeChangeListener(this.slidelist);
				this.slide.setValue(this.showElem);
				this.slide.addChangeListener(this.slidelist);
				this.no.setText(Integer.toString(this.showElem));
				this.oneback.setEnabled(this.showElem > 0);
				this.onefor.setEnabled(this.showElem < this.movesList.size()|| editModus);
			}
			
			if (arg0.getActionCommand().equals(" > ")) {
			// auto play
				this.slide.removeChangeListener(this.slidelist);
				if (!paused ) {
					//System.out.println("not paused");
					this.showElem=0;
					this.slide.setValue(this.showElem);
					this.no.setText(Integer.toString(this.showElem));
					this.cf.setField(start);
					this.cm = null;
					this.setMoveInfos();
				}
				this.autofor.setEnabled(false);
				this.slower.setEnabled(true);
				this.faster.setEnabled(true);
				this.oneback.setEnabled(false);
				this.onefor.setEnabled(false);
				this.stopp.setEnabled(true);
				this.paused = false;
				this.mythread = new PlayThread(this);
				this.mythread.start();
			}
			if (arg0.getActionCommand().equals(" || ")) {
			// auto play paused
				this.paused = true;
				this.autofor.setEnabled(true);
				this.slower.setEnabled(false);
				this.faster.setEnabled(false);
				this.oneback.setEnabled(this.showElem > 0);
				this.onefor.setEnabled(this.showElem < this.movesList.size()|| editModus);
				this.stopp.setEnabled(false);
				this.slide.addChangeListener(this.slidelist);
				if (this.mythread.isAlive()) this.mythread.interrupt();
			}
			if (arg0.getActionCommand().equals(">>")) {
			// faster
				if ( this.mythread.isAlive()) 
					if (waitTime >= 200) this.waitTime = this.waitTime/2;
			}
			if (arg0.getActionCommand().equals("<<")) {
			// slower
				if ( this.mythread.isAlive()) this.waitTime = (long)this.waitTime*2;
			}
			if (arg0.getActionCommand().equals(" set start ")) {
				if (!(this.cm==null)) {
					cm.setBack(cf);
					this.cf.editMove(this, true);
				}
				else Reporter.showError("no move selected");
			}
			if (arg0.getActionCommand().equals(" set end ")) {
				if (!(this.cm==null)){
					cm.setBack(cf);
					this.cf.editMove(this, false);
				}
				else Reporter.showError("no move selected");
			}
			if (arg0.getActionCommand().equals("comboBoxChanged")) {
				if (this.cm != null) {
					this.cm.setBack(this.cf);
					this.cm.proFigure = this.proFigBox.getSelectedIndex();
					this.invalid.setVisible(!this.cm.checkMove(this.cf, false));
					this.proFig.setText(ChessObject.selection[this.proFigBox.getSelectedIndex()]);
					this.cm.move(this.cf);
				}
			}
		
	}
	
	/*
	 * when new start- or end-pos for a move is choosen,
	 * this sets new captured or moved figure changes the texts and show the move one time 
	 */
	public void execSet(int x, int y, boolean isStart) {
		if (isStart)  {
			this.cm.movFigure = (cf.getField())[x][y];
			this.movFig.setText(ChessObject.selection[this.cm.movFigure]);
			this.cm.colS= x;
			this.cm.rowS= y;	
		}
		else { // is End
			this.cm.capFigure = (cf.getField())[x][y];
			this.capFig.setText(ChessObject.selection[this.cm.capFigure]);
			this.cm.colE= x;
			this.cm.rowE= y;
		}
		this.cf.markField(x, y, Color.YELLOW);
		this.invalid.setVisible(!this.cm.checkMove(this.cf, false));
		this.proFig.setText(ChessObject.selection[this.cm.proFigure]);
		this.cm.move(this.cf);	
	}
	
	/**
	* parses the listexpr to a chessgame
	*/
	private JTable parseChessGame( ListExpr value){
		JTable info = new JTable(11,2);
		info.setRowMargin(1);
		info.setEnabled(false);
		info.setTableHeader(null);
		if ( value.listLength() == 3 ) {
			ListExpr value1 = value.first();
			//System.out.println("my 1.game expr "+ value1.writeListExprToString());
			if (value1.listLength()==6) {
				if ( value1.first().isAtom() && value1.first().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("Event", 0,0);
					info.setValueAt(value1.first().stringValue(),0,1);
				}	
				if ( value1.second().isAtom() && value1.second().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("Site", 1,0);
					info.setValueAt(value1.second().stringValue(),1,1);
				}
				if ( value1.third().isAtom() && value1.third().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("Date", 2,0);
					info.setValueAt(value1.third().stringValue(),2,1);
				}
				if ( value1.fourth().isAtom() && value1.fourth().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("Round", 3,0);
					info.setValueAt(value1.fourth().stringValue(),3,1);
				}
				if ( value1.fifth().isAtom() && value1.fifth().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("White", 4,0);
					info.setValueAt(value1.fifth().stringValue(),4,1);
				}
				if ( value1.sixth().isAtom() && value1.sixth().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("Black", 5,0);
					info.setValueAt(value1.sixth().stringValue(),5,1);
				}
			}
			else Reporter.showError("invalid chessgame object");
			
			ListExpr value2 = value.second();
			//System.out.println("my 2.game expr "+ value2.writeListExprToString());
			if (value2.listLength()==5) {
				if ( value2.first().isAtom() && value2.first().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("Result", 6,0);
					info.setValueAt(value2.first().stringValue(),6,1);
				}
				if ( value2.second().isAtom() && value2.second().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("WhiteElo", 7,0);
					info.setValueAt(value2.second().stringValue(),7,1);
				}
				if ( value2.third().isAtom() && value2.third().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("BlackElo", 8,0);
					info.setValueAt(value2.third().stringValue(),8,1);
				}
				if ( value2.fourth().isAtom() && value2.fourth().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("EventDate", 9,0);
					info.setValueAt(value2.fourth().stringValue(),9,1);
				}
				if ( value2.fifth().isAtom() && value2.fifth().atomType() == ListExpr.STRING_ATOM) {
					info.setValueAt("ECO", 10,0);
					info.setValueAt(value2.fifth().stringValue(),10,1);
				}
			}
			else Reporter.showError("invalid chessgame object"); 
			
			ListExpr value3 = value.third().first();
			while (!value3.isEmpty()) {
			// System.out.println("current value " + value3.first().writeListExprToString());
				this.movesList.add(ChessObject.parseChessMove(value3.first())); 
			//	System.out.println("current size " + movesList.size()  );
				value3 = value3.rest();
			//	System.out.println("gameslist3 length "+ value3.listLength());
			} 
		}
		else Reporter.showError("invalid chessgame object"); 
		//System.out.println("parse game ready");
		return info;
	}
	
	private class PlayThread extends Thread {
		ChessGameFrame c;
		public PlayThread(ChessGameFrame c) {
			super();
			this.c= c;
			
		}
		
		public void run() {
			boolean interrupt = false;
			c.slide.removeChangeListener(c.slidelist);
			while (c.showElem < c.movesList.size() && !interrupt){
				try {
					c.showElem++;
					c.cm = ((ChessMoveDates)movesList.get(c.showElem-1));
					c.setMoveInfos();
					c.cm.move(c.cf);
					c.slide.setValue(c.showElem);
					c.no.setText(Integer.toString(c.showElem));
					paintImmediately(c.cf.getVisibleRect());
					//System.out.println("Wartezeit " + waitTime);
					sleep(c.waitTime);
					c.cm.unmark(c.cf);
				} catch (InterruptedException e) {
					interrupt = true;
				}
			}
			c.autofor.setEnabled(true);
			c.slower.setEnabled(false);
			c.faster.setEnabled(false);
			c.oneback.setEnabled(c.showElem > 0);
			c.onefor.setEnabled(c.showElem < c.movesList.size()|| editModus);
			c.stopp.setEnabled(false);
			c.slide.addChangeListener(c.slidelist);
		}
	}
	
	/*
	 * invloved when changes no-field 
	 */
	private class NoListener implements ActionListener {
		public void actionPerformed(ActionEvent arg0) {
			System.out.println(" NoListener");
			try {  // test if no input is a integer
        		int i = Integer.parseInt(no.getText());
				if (i > movesList.size()) {
					i = movesList.size();	
				}
				if (i <= 0) {
					i = 0;
					showElem=0;
					cf.setField(start);	
				}
				while (i != showElem) {
					if (showElem > i) {
						((ChessMoveDates)movesList.get(showElem-1)).setBack(cf);
						showElem--;
						
					}
					if (showElem < i) {
						if (showElem != 0) ((ChessMoveDates)movesList.get(showElem-1)).unmark(cf);
						showElem++;
						((ChessMoveDates)movesList.get(showElem-1)).setFor(cf);
					}
				}
				if (i==0) cm = null;
				else cm = ((ChessMoveDates)movesList.get(i-1));
				setMoveInfos();
				no.setText(Integer.toString(i));
				slide.removeChangeListener(slidelist);
				slide.setValue(i);
				slide.addChangeListener(slidelist);
				oneback.setEnabled(showElem > 0);
				onefor.setEnabled(showElem < movesList.size()|| editModus);
			} catch (Exception e) { // if not set to old value
				no.setText(Integer.toString(showElem));
			}
		}
	}
	
	/*
	 * Listens for changes from jslider
	 */
	private class SlideListener implements ChangeListener {
		public void stateChanged(ChangeEvent arg0) {
			//System.out.println(" slideListener");
			int i = slide.getValue();
			if (i!=showElem) {
				if (i==0) {
					showElem=0;
					cf.setField(start);
					cm = null;
					setMoveInfos();
				}
				else {
					while (i != showElem) {
						if (showElem > i) {
							((ChessMoveDates)movesList.get(showElem-1)).setBack(cf);
							showElem--;
						}
						if (showElem < i) {
							if (showElem != 0) ((ChessMoveDates)movesList.get(showElem-1)).unmark(cf);
							showElem++;
							((ChessMoveDates)movesList.get(showElem-1)).setFor(cf);
						}
					}
					if (i==0) {
						showElem=0;
						cf.setField(start);
						cm = null;
						setMoveInfos();
					}
					else {
						cm = (ChessMoveDates)movesList.get(i-1);
						cm.setBack(cf);
						setMoveInfos();
						cm.setFor(cf);
					}
				}
				no.setText(Integer.toString(showElem));
				oneback.setEnabled(showElem > 0);
				onefor.setEnabled(showElem < movesList.size() || editModus);
			}
		} 
	}
	/*
	 * returns type of this object
	 * @see viewer.chess.ChessInterface#getType()
	 */
	public String getType() {
		return "chessgame";
	}
	/*
	 * change to edit-modus or leave edit-modus 
	 * @see viewer.chess.ChessInterface#changeToEdit(boolean)
	 */
	public boolean changeToEdit(boolean edit) {
		editModus= edit;
		if (edit) {
			setStart.setEnabled(true);
			setEnd.setEnabled(true);
			setStart.addActionListener(this);
			setEnd.addActionListener(this);
			chess.addActionListener(this);
			mate.addActionListener(this);
			metaText.setEnabled(true);
			onefor.setEnabled(true);
		}
		else {
			setStart.setEnabled(false);
			setEnd.setEnabled(false);
			proFigBox.removeActionListener(this);
			setStart.removeActionListener(this);
			chess.removeActionListener(this);
			mate.removeActionListener(this);
			setEnd.removeActionListener(this);
			metaText.setEnabled(false);
			onefor.setEnabled(showElem < movesList.size());
		}
		this.validate();
		return true;
	}
	/*
	 * returns the ListExpr of this game, 
	 * without all invalid chessmoves and the chessmoves 
	 * after detected a invalid chessmove
	 *  @see viewer.chess.ChessInterface#getListExpr()
 	*/
	public ListExpr getListExpr() {
		ListExpr value1 = ListExpr.sixElemList(
				ListExpr.stringAtom((String)metaText.getValueAt(0,0)), 
				ListExpr.stringAtom((String)metaText.getValueAt(1,0)), 
				ListExpr.stringAtom((String)metaText.getValueAt(2,0)), 
				ListExpr.stringAtom((String)metaText.getValueAt(3,0)), 
				ListExpr.stringAtom((String)metaText.getValueAt(4,0)), 
				ListExpr.stringAtom((String)metaText.getValueAt(5,0)));
		ListExpr value2 = ListExpr.fiveElemList(
				ListExpr.stringAtom((String)metaText.getValueAt(6,0)), 
				ListExpr.stringAtom((String)metaText.getValueAt(7,0)), 
				ListExpr.stringAtom((String)metaText.getValueAt(8,0)), 
				ListExpr.stringAtom((String)metaText.getValueAt(9,0)), 
				ListExpr.stringAtom((String)metaText.getValueAt(10,0)));
		ListExpr value3=ListExpr.theEmptyList(), rest;
		boolean correct = false;
		if (!movesList.isEmpty()) {
			ChessField myCF = new ChessField(start);
			ChessMoveDates cm;
			int i =0; // find the first valid move
			do {
				cm = (ChessMoveDates)movesList.get(i);
				correct = cm.checkMove(myCF, false);
				cm.setFor(myCF);
				i++;
			}
			while (!correct && i < movesList.size());
			value3 = ListExpr.oneElemList(cm.getListExpr());
			rest = value3;
			for (int j = i; j< movesList.size() && correct; j++) {
				cm = (ChessMoveDates)movesList.get(j);
				correct = cm.checkMove(myCF, false);
				cm.setFor(myCF);
				if (correct) // append if valid
					rest = ListExpr.append(rest,cm.getListExpr());
			}
		}	
		ListExpr value =  ListExpr.threeElemList(value1, value2,ListExpr.oneElemList(value3));
		System.out.println(value.writeListExprToString());
		return value;
	}
	/*
	 *  returns a true this object can be edited by the viewer
	 *  false otherwise
	 */
	public boolean canEdit() {
		return true; 
	}

	/*
	 *  returns a true this object can write into a PGN-File
	 *  false otherwise
	 */
	public boolean canExport() {
		return true;  
	}

	/*
	 *  returns a ListExpr of actual shown chessposition
	 */
	public ListExpr getListExprPos() {
		StringBuffer fieldString = new StringBuffer();
		int[][] field = cf.getField();
		for (int i=0; i<8;i++) {
			for (int j=0; j<8;j++) {
				fieldString.append(ChessObject.parseToChar(field[j][i]));
			}
		}
		ListExpr ll = ListExpr.twoElemList
			(ListExpr.textAtom(fieldString.toString()),
			ListExpr.intAtom(Integer.parseInt(no.getText())));
//		System.out.println("resultList "+ commandList.writeListExprToString());
		return ll;
	}
	
	/*
	 * returns a ListExpr of actual shown chessmove
	 */
	public ListExpr getListExprMove() {
		if (cm!=null)
			return cm.getListExpr();
		else return null;
	}
	
	/*
	 * returns a ListExpr of chessmaterial of actual shown position 
	 */
	public ListExpr getListExprMaterial() {
		int [][] myMat = new int[2][6];
		int [][] myfield = cf.getField();
		for (int i=0; i<8;i++) {
			for (int j=0; j<8;j++) {
				switch (myfield[i][j]) {
				case 0: break;
				case 1: myMat[0][0]++; break;
				case 2: myMat[0][1]++; break;
				case 3: myMat[0][2]++; break;
				case 4: myMat[0][3]++; break;
				case 5: myMat[0][4]++; break;
				case 6: myMat[0][5]++; break;
				case 7: myMat[1][0]++; break;
				case 8: myMat[1][1]++; break;
				case 9: myMat[1][2]++; break;
				case 10: myMat[1][3]++; break;
				case 11: myMat[1][4]++; break;
				case 12: myMat[1][5]++; break;
				default: 
				}
			}
		}
		ListExpr myList = ListExpr.sixElemList(
				ListExpr.twoElemList(ListExpr.intAtom(myMat[0][0]), ListExpr.intAtom(myMat[1][0])),
				ListExpr.twoElemList(ListExpr.intAtom(myMat[0][1]), ListExpr.intAtom(myMat[1][1])),
				ListExpr.twoElemList(ListExpr.intAtom(myMat[0][2]), ListExpr.intAtom(myMat[1][2])),
				ListExpr.twoElemList(ListExpr.intAtom(myMat[0][3]), ListExpr.intAtom(myMat[1][3])),
				ListExpr.twoElemList(ListExpr.intAtom(myMat[0][4]), ListExpr.intAtom(myMat[1][4])),
				ListExpr.twoElemList(ListExpr.intAtom(myMat[0][5]), ListExpr.intAtom(myMat[1][5])));
		return myList; 
	}
	
	/*
	 * returns a string with the nThattribut of Meta-dates 
	 */
	public String getMeta(int selection) {
		return "\""+(String)metaText.getValueAt(selection,1)+"\"";
	}
	
	/*
	 * returns a string with the Query of nTh Attribut operator for getting inquirys of chessmoves 
	 */
	public String getQueryMoves(int i) {
		return ChessObject.getQueryMoves(i, this.cm);
	}	
}