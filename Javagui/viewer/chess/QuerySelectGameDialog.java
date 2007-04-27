package viewer.chess;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.border.TitledBorder;
import sj.lang.ListExpr;
import tools.Reporter;

/*
 * the  QuerySelectDialog is used for create Querys when a Game is selected 
 * to choose an element of Chessgameobject
 */
public class QuerySelectGameDialog extends JDialog implements ActionListener {
	static String matString = "actual material";
    static String posString = "actual position";
    static String moveString = "actual move";
    static String eventString = "event";
    static String siteString = "site";
    static String dateString = "date";
    static String roundString = "round";
    static String whiteString = "white";
    static String blackString = "black";
    static String resultString = "result";
    static String whiteEloString = "whiteElo";
    static String blackEloString = "blackElo";
    //static String eventDateString = "eventDate";
    static String ecoString = "ECO";
    
	private int selection = -1;
	private Vector vectSql, vectSec;
	private ChessGameFrame game;
	
    /*
     * constructor of QuerySelectDialog
     */
    public QuerySelectGameDialog(Vector querys, Vector secQuerys, ChessGameFrame cg) {
        super();
        this.vectSql = querys;
        this.vectSec = secQuerys;
        this.game = cg;
//  Create the radio buttons.
    JRadioButton matButton = new JRadioButton(matString);
    matButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    matButton.setActionCommand(matString);
    matButton.setSelected(false);
         
    JRadioButton posButton = new JRadioButton(posString);
    posButton.setActionCommand(posString);
    posButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    posButton.setSelected(false);
    
    JRadioButton moveButton = new JRadioButton(moveString);
    moveButton.setActionCommand(moveString);
    moveButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    moveButton.setSelected(false);
    
    JRadioButton eventButton = new JRadioButton(eventString);
    eventButton.setActionCommand(eventString);
    eventButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    eventButton.setSelected(false);
    
    JRadioButton siteButton = new JRadioButton(siteString);
    siteButton.setActionCommand(siteString);
    siteButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    siteButton.setSelected(false);
    
    JRadioButton dateButton = new JRadioButton(dateString);
    dateButton.setActionCommand(dateString);
    dateButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    dateButton.setSelected(false);
    
    JRadioButton roundButton = new JRadioButton(roundString);
    roundButton.setActionCommand(roundString);
    roundButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    roundButton.setSelected(false);
    
    JRadioButton whiteButton = new JRadioButton(whiteString);
    whiteButton.setActionCommand(whiteString);
    whiteButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    whiteButton.setSelected(false);
    
    JRadioButton blackButton = new JRadioButton(blackString);
    blackButton.setActionCommand(blackString);
    blackButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    blackButton.setSelected(false);
    
    JRadioButton resultButton = new JRadioButton(resultString);
    resultButton.setActionCommand(resultString);
    resultButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    resultButton.setSelected(false);
    
    JRadioButton whiteEloButton = new JRadioButton(whiteEloString);
    whiteEloButton.setActionCommand(whiteEloString);
    whiteEloButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    whiteEloButton.setSelected(false);
    
    JRadioButton blackEloButton = new JRadioButton(blackEloString);
    blackEloButton.setActionCommand(blackEloString);
    blackEloButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    blackEloButton.setSelected(false);
    
    /*JRadioButton eventDateButton = new JRadioButton(eventDateString);
    eventDateButton.setActionCommand(eventDateString);
    eventDateButton.setSelected(false);
    */
    
    JRadioButton ecoButton = new JRadioButton(ecoString);
    ecoButton.setActionCommand(ecoString);
    ecoButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    ecoButton.setSelected(false);
    
	//	Put the radio buttons in a column in a panel.
    JPanel radioPanel1 = new JPanel(new GridLayout(0, 3));
    JPanel radioPanel2 = new JPanel(new GridLayout(4, 3));
    radioPanel1.add(matButton);
    radioPanel1.add(posButton);
    radioPanel1.add(moveButton);
    radioPanel2.add(eventButton);
    radioPanel2.add(siteButton);
    radioPanel2.add(dateButton);
    radioPanel2.add(roundButton);
    radioPanel2.add(resultButton);
    radioPanel2.add(ecoButton);
    radioPanel2.add(whiteButton);
    radioPanel2.add(blackButton);
    radioPanel2.add(new JPanel());
    radioPanel2.add(whiteEloButton);
    radioPanel2.add(blackEloButton);
    radioPanel2.add(new JPanel());
    //radioPanel2.add(eventDateButton);
    
    radioPanel1.setBorder(BorderFactory.createCompoundBorder(
    		BorderFactory.createEmptyBorder(5,5,5,5),
			BorderFactory.createLineBorder(Color.BLACK)));
    radioPanel2.setBorder(BorderFactory.createCompoundBorder(
    		BorderFactory.createEmptyBorder(4,4,4,4),
			BorderFactory.createTitledBorder(
			BorderFactory.createLineBorder(Color.BLACK), "meta-dates",
			TitledBorder.CENTER,TitledBorder.TOP)));
    //Group the radio buttons.
    ButtonGroup group = new ButtonGroup();
    group.add(matButton);
    group.add(posButton);
    group.add(moveButton);
    group.add(eventButton);
    group.add(siteButton);
    group.add(dateButton);
    group.add(roundButton);
    group.add(whiteButton);
    group.add(blackButton);
    group.add(resultButton);
    group.add(whiteEloButton);
    group.add(blackEloButton);
    //group.add(eventDateButton);
    group.add(ecoButton);
    
    //Register a listener for the radio buttons.
    matButton.addActionListener(this);
    posButton.addActionListener(this);
    moveButton.addActionListener(this);
    eventButton.addActionListener(this);
    siteButton.addActionListener(this);
    dateButton.addActionListener(this);
    roundButton.addActionListener(this);
    whiteButton.addActionListener(this);
    blackButton.addActionListener(this);
    resultButton.addActionListener(this);
    whiteEloButton.addActionListener(this);
    blackEloButton.addActionListener(this);
    //eventDateButton.addActionListener(this);
    ecoButton.addActionListener(this);

    //  Create the JOptionPane
	JPanel optionPanel = new JPanel();
	JButton okButton = new JButton(" OK ");
	JButton cancelButton = new JButton(" Cancel ");
	cancelButton.setBorder(ChessObject.compound5);
	okButton.setBorder(ChessObject.compound5);
	optionPanel.add(okButton);
	optionPanel.add(cancelButton);
	cancelButton.addActionListener(this);
	okButton.addActionListener(this);
	JPanel title = new JPanel();
	title.add(new JLabel("select a element you wish to add to selection"));
	title.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
	Box aBox = new Box(BoxLayout.Y_AXIS);
	aBox.add(radioPanel1);
	aBox.add(radioPanel2);
	//radioPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
	optionPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
	this.getContentPane().setLayout(new BorderLayout()); 
	this.getContentPane().add(title, BorderLayout.NORTH);
	this.getContentPane().add(aBox, BorderLayout.CENTER);
	this.getContentPane().add(optionPanel, BorderLayout.SOUTH);
	this.pack();
    }
    
    /*
     * listens when Button pressed or radiobuttons changed
     * @see java.awt.event.ActionListener#actionPerformed(java.awt.event.ActionEvent)
     */
	public void actionPerformed(ActionEvent e) {
		if (e.getActionCommand().equals(" OK ")) {
			StringBuffer buf = new StringBuffer();
			StringBuffer message = new StringBuffer();
			StringBuffer name = new StringBuffer();
			boolean ok = false;
			switch (selection) {
			case 0: // event
					buf.append("(match getkey \"event\") = ");
					buf.append(game.getMeta(selection));
					
					vectSql.add(buf.toString());
					Reporter.showInfo("event to query-list added");
					break;
			case 1: //site
					buf.append("(match getkey \"site\") = ");
					buf.append(game.getMeta(selection));
					
					vectSql.add(buf.toString());
					Reporter.showInfo("site to query-list added");
					break;
			case 2: // date
				buf.append("(match getkey \"date\") = ");
				buf.append(game.getMeta(selection));
				vectSql.add(buf.toString());
				Reporter.showInfo("date to query-list added");
				break;
			case 3: // round
				buf.append("(match getkey \"round\") = ");
				buf.append(game.getMeta(selection));
				Reporter.showInfo("round to query-list added");
				vectSql.add(buf.toString());
				break;
			case 4: //white
				buf.append("(match getkey \"name_w\") = ");
				buf.append(game.getMeta(selection));
				vectSql.add(buf.toString());
				Reporter.showInfo("name_w to query-list added");
				break;
			case 5: // black
				buf.append("(match getkey \"name_b\") = ");
				buf.append(game.getMeta(selection));
				vectSql.add(buf.toString());
				Reporter.showInfo("name_b to query-list added");
				break;
			case 6: // result
				buf.append("(match getkey \"result\") = ");
				buf.append(game.getMeta(selection));
				vectSql.add(buf.toString());
				Reporter.showInfo("result to query-list added");
				break;
			case 7: // whiteElo
				buf.append("(getkey \"rating_w\") = ");
				buf.append(game.getMeta(selection));
				vectSql.add(buf.toString());
				Reporter.showInfo("rating_w to query-list added");
				break;
			case 8: // black elo
				buf.append("(match getkey \"rating_b\") = ");
				buf.append(game.getMeta(selection));
				vectSql.add(buf.toString());
				Reporter.showInfo("rating_b to query-list added");
				break;
			case 9: // eventdate
				/*buf.append("(match getkey \"eventdate\") = ");
				buf.append(game.getMeta(selection));
				vect.add(buf.toString());*/
				break;
			case 10: // eco
				buf.append("(match getkey \"eco_code\") = ");
				buf.append(game.getMeta(selection));
				vectSql.add(buf.toString());
				Reporter.showInfo("eco_code to query-list added");
				break;
			case 11: // mat
				ListExpr leMat = game.getListExprMaterial();
				ok = ChessObject.saveObject(message, name, "chessmaterial", leMat.writeListExprToString());
				if (ok && name.length()!=0 ) {
					buf.append("(pieces(pos)) = ");
					buf.append(name);
					vectSql.add(buf.toString());
					Reporter.showInfo((message.insert(0,"\nchessmaterial to query-list added")).toString());
				}
				else 
					Reporter.showWarning((message.insert(0,"\nnothing added")).toString());
				break;
			case 12: // pos
				ListExpr lePos = game.getListExprPos();
				ok = ChessObject.saveObject(message, name, "chessposition", lePos.writeListExprToString());
				if (ok && name.length()!=0) {
					buf.append(name);
					buf.append(" includes pos");
					vectSql.add(buf.toString());
					Reporter.showInfo((message.insert(0,"\nchessposition to query-list added")).toString());
				}
				else 
					Reporter.showWarning((message.insert(0,"\nnothing added")).toString());
				break;
			case 13: // move
					boolean error = false;
					for (int i =0; i < 6; i++) {
						String s = game.getQueryMoves(i);
						if (s!=null)
							vectSql.add(s);
						else {
							error = true;
							break;
						}
					}
					if (error)
						Reporter.showWarning("nothing added \n no move selected");
					else
						Reporter.showInfo("chessmove to query-list added");
					break;
			default:Reporter.showWarning("no selection\nnothing added ");
			}
			this.setVisible(false);
		}
		if (e.getActionCommand().equals(" Cancel ")) {
			selection = -1;
			this.setVisible(false);
			Reporter.showWarning("nothing added");
		}
		if (e.getActionCommand().equals(matString)) {
			selection = 11;	
		}
		if (e.getActionCommand().equals(posString)) {
			selection = 12;	
		}
		if (e.getActionCommand().equals(moveString)) {
			selection = 13;	
		}
		if (e.getActionCommand().equals(eventString)) {
			selection = 0;	
		}
		if (e.getActionCommand().equals(siteString)) {
			selection = 1;	
		}
		if (e.getActionCommand().equals(dateString)) {
			selection = 2;	
		}
		if (e.getActionCommand().equals(roundString)) {
			selection = 3;	
		}	
		if (e.getActionCommand().equals(whiteString)) {
			selection = 4;	
		}
		if (e.getActionCommand().equals(blackString)) {
			selection = 5;	
		}	
		if (e.getActionCommand().equals(resultString)) {
			selection = 6;	
		}	
		if (e.getActionCommand().equals(whiteEloString)) {
			selection = 7;	
		}	
		if (e.getActionCommand().equals(blackEloString)) {
			selection = 8;	
		}
		/*if (e.getActionCommand().equals(eventDateString)) {
			selection = 9;	
		}*/
		if (e.getActionCommand().equals(ecoString)) {
			selection = 10;	
		}
	}
	
	/*
	 * set the actual ChessGameFrame-referenz 
	 */
	public void setComp(ChessGameFrame frame) {
		this.game = frame;
	}

}
