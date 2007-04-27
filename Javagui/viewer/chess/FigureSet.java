package viewer.chess;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridLayout;

import javax.swing.border.Border;
import javax.swing.*;

/*
 * class for setting figures on a chesspositionobject
 */
public class FigureSet extends JDialog implements ActionListener{
	private int select = 0;
	private boolean canceled;
    
public FigureSet() {
	super();
	this.setTitle("choose a figure");
	JRadioButton noneButton = new JRadioButton(ChessObject.selection[0]);
    noneButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    noneButton.setActionCommand(ChessObject.selection[0]);
    noneButton.setSelected(true);
         
    JRadioButton pawnWButton = new JRadioButton(ChessObject.selection[1]);
    pawnWButton.setActionCommand(ChessObject.selection[1]);
    pawnWButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    pawnWButton.setSelected(false);
    
    JRadioButton bishopWButton = new JRadioButton(ChessObject.selection[2]);
    bishopWButton.setActionCommand(ChessObject.selection[2]);
    bishopWButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    bishopWButton.setSelected(false);
    
    JRadioButton knightWButton = new JRadioButton(ChessObject.selection[3]);
    knightWButton.setActionCommand(ChessObject.selection[3]);
    knightWButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    knightWButton.setSelected(false);
    
    JRadioButton rookWButton = new JRadioButton(ChessObject.selection[4]);
    rookWButton.setActionCommand(ChessObject.selection[4]);
    rookWButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    rookWButton.setSelected(false);
    
    JRadioButton queenWButton = new JRadioButton(ChessObject.selection[5]);
    queenWButton.setActionCommand(ChessObject.selection[5]);
    queenWButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    queenWButton.setSelected(false);
    
    JRadioButton kingWButton = new JRadioButton(ChessObject.selection[6]);
    kingWButton.setActionCommand(ChessObject.selection[6]);
    kingWButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    kingWButton.setSelected(false);
    
    JRadioButton pawnBButton = new JRadioButton(ChessObject.selection[7]);
    pawnBButton.setActionCommand(ChessObject.selection[7]);
    pawnBButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    pawnBButton.setSelected(false);
    
    JRadioButton bishopBButton = new JRadioButton(ChessObject.selection[8]);
    bishopBButton.setActionCommand(ChessObject.selection[8]);
    bishopBButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    bishopBButton.setSelected(false);
    
    JRadioButton knightBButton = new JRadioButton(ChessObject.selection[9]);
    knightBButton.setActionCommand(ChessObject.selection[9]);
    knightBButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    knightBButton.setSelected(false);
    
    JRadioButton rookBButton = new JRadioButton(ChessObject.selection[10]);
    rookBButton.setActionCommand(ChessObject.selection[10]);
    rookBButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    rookBButton.setSelected(false);
    
    JRadioButton queenBButton = new JRadioButton(ChessObject.selection[11]);
    queenBButton.setActionCommand(ChessObject.selection[11]);
    queenBButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    queenBButton.setSelected(false);
    
    JRadioButton kingBButton = new JRadioButton(ChessObject.selection[12]);
    kingBButton.setActionCommand(ChessObject.selection[12]);
    kingBButton.setBorder(BorderFactory.createEmptyBorder(3,3,3,3));
    kingBButton.setSelected(false);
    
    
	//	Put the radio buttons in a column in a panel.
    JPanel radioPanel = new JPanel(new GridLayout(5, 3));
    radioPanel.add(new JPanel());
    radioPanel.add(noneButton);
    radioPanel.add(new JPanel());
    radioPanel.add(pawnWButton);
    radioPanel.add(bishopWButton);
    radioPanel.add(knightWButton);
    radioPanel.add(rookWButton);
    radioPanel.add(queenWButton);
    radioPanel.add(kingWButton);
    radioPanel.add(pawnBButton);
    radioPanel.add(bishopBButton);
    radioPanel.add(knightBButton);    
    radioPanel.add(rookBButton);
    radioPanel.add(queenBButton);
    radioPanel.add(kingBButton);
    radioPanel.setBorder(BorderFactory.createCompoundBorder(
    		BorderFactory.createEmptyBorder(5,5,5,5),
			BorderFactory.createRaisedBevelBorder()));
    ButtonGroup group = new ButtonGroup();
    group.add(noneButton);
    group.add(pawnWButton);
    group.add(bishopWButton);
    group.add(knightWButton);
    group.add(rookWButton);
    group.add(queenWButton);
    group.add(kingWButton);
    group.add(pawnBButton);
    group.add(bishopBButton);
    group.add(knightBButton);
    group.add(rookBButton);
    group.add(queenBButton);
    group.add(kingBButton);
    
    //Register a listener for the radio buttons.
    noneButton.addActionListener(this);
    pawnWButton.addActionListener(this);
    bishopWButton.addActionListener(this);
    knightWButton.addActionListener(this);
    rookWButton.addActionListener(this);
    queenWButton.addActionListener(this);
    kingWButton.addActionListener(this);
    pawnBButton.addActionListener(this);
    bishopBButton.addActionListener(this);
    knightBButton.addActionListener(this);
    rookBButton.addActionListener(this);
    queenBButton.addActionListener(this);
    kingBButton.addActionListener(this);

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
	this.getContentPane().setLayout(new BorderLayout()); 
	this.getContentPane().add(radioPanel, BorderLayout.CENTER);
	this.getContentPane().add(optionPanel, BorderLayout.SOUTH);
	this.pack();
	//System.out.println("Here figureset");    
}
    
    /* (non-Javadoc)
	 * @see java.awt.event.ActionListener#actionPerformed(java.awt.event.ActionEvent)
	 */
public void actionPerformed(ActionEvent arg0) {
	
	if (arg0.getActionCommand().equals(ChessObject.selection[0])) {
		select = 0;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[1])) {
		select = 1;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[2])) {
		select = 2;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[3])) {
		select = 3;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[4])) {
		select = 4;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[5])) {
		select = 5;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[6])) {
		select = 6;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[7])) {
		select = 7;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[8])) {
		select = 8;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[9])) {
		select = 9;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[10])) {
		select = 10;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[11])) {
		select = 11;
	}
	if (arg0.getActionCommand().equals(ChessObject.selection[12])) {
		select = 12;
	}
	if (arg0.getActionCommand().equals(" OK ")) {
		canceled = false;
		this.setVisible(false);
	}
	if (arg0.getActionCommand().equals(" Cancel ")) {
		canceled = true;
		this.setVisible(false);
	}
}

	/**
     * gives if cancel pressed
     * 
     * @return boolean 
     */
    public boolean getCancel() {
        return canceled;
    }
    /**
     * gives back which figure to set
     * 0 - none
     * 1 - white Pawn			7 - black Pawn
     * 2 - white Bishop			8 - black Bishop
     * 3 - white knight			9 - black kinght
     * 4 - white rook			10 - black rook
     * 5 - white queen			11 - black queen
     * 6 - white king			12 - black king  
     * @return int 
     */
    public int getFigure() {
        return select;
    }
}
