/*
 * Created on 04.12.2006
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Generation - Code and Comments
 */
package viewer.chess;

/**
 * @author Kathrin
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Generation - Code and Comments
 */
import viewer.chess.FigureSet;
import javax.swing.JComponent;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Color;
import java.awt.Image;
import java.awt.event.*;

import javax.imageio.ImageIO;
import java.io.File;
import java.io.IOException;
import tools.Reporter;
/*
 * this representates the actual chessfield with position
 */
public class ChessField extends JComponent {
	
    private Graphics g;
    private int length;
    private int[][] position;
    private Image[] scalImg = new Image[12];
    private int xMove=0,yMove=0;
    private int movFig = 0;
    private Dimension myDimension;
    private PositionEditor posEdit;
    private MoveEditor moveEdit;
    private MotionListener motionList;
    private static final Image[] CHESS_FIG = new Image[12]; {
	try {
	     CHESS_FIG[0] = ImageIO.read(new File("./res/Pawn_white.png"));
	     CHESS_FIG[1] = ImageIO.read(new File("./res/Bishop_white.png"));
	     CHESS_FIG[2] = ImageIO.read(new File("./res/Knight_white.png"));
	     CHESS_FIG[3] = ImageIO.read(new File("./res/Rook_white.png"));
	     CHESS_FIG[4] = ImageIO.read(new File("./res/Queen_white.png"));
	     CHESS_FIG[5] = ImageIO.read(new File("./res/King_white.png"));
	     CHESS_FIG[6] = ImageIO.read(new File("./res/Pawn_black.png"));
	     CHESS_FIG[7] = ImageIO.read(new File("./res/Bishop_black.png"));
	     CHESS_FIG[8] = ImageIO.read(new File("./res/Knight_black.png"));
	     CHESS_FIG[9] = ImageIO.read(new File("./res/Rook_black.png"));
	     CHESS_FIG[10] = ImageIO.read(new File("./res/Queen_black.png"));
	     CHESS_FIG[11] = ImageIO.read(new File("./res/King_black.png"));
	}
	catch (IOException e) {
		Reporter.showError("error when reading chessfigurefiles"); 
	}
}
    // begin constructor
public ChessField() {
	super();
	int[][] pos = {{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0}};
	position = pos;
	if (this.getHeight()>50 && this.getWidth()>50)
		this.length = Math.round(getHeight()/8);
	else 
		this.length = 40;
	for ( int i =0 ; i< 12; i++)
		scalImg[i]= CHESS_FIG[i].getScaledInstance(length,length,Image.SCALE_DEFAULT);
	this.myDimension = new Dimension(length*8+2,length*8+3);
	//this.paintImmediately(this.getVisibleRect());
}

public ChessField(int[][] game) {
	super();
	//System.out.println("Here chessfield "+this.getHeight());
	position = new int[8][8];
	for (int i =0; i<8;i++) {
		for (int j=0; j<8;j++) {
			//System.out.println("setfield old "+position[i][j]+" "+game[i][j]);
			position[i][j] = game[i][j];
			//System.out.println("setfield new" + position[i][j]+" "+game[i][j]);	
		}
	}
	//this.position = game;
	if (this.getHeight()>50 && this.getWidth()>50)
		this.length = Math.round((getHeight()/8));
	else 
		this.length = 40;
	//System.out.println(this.getHeight()+" "+this.getWidth()+" "+length);
	
	for ( int i =0 ; i< 12; i++)
		scalImg[i]= CHESS_FIG[i].getScaledInstance(length,length,Image.SCALE_DEFAULT);
	this.myDimension = new Dimension(length*8+2,length*8+3);
	//this.paintImmediately(this.getVisibleRect());
        
    } 
    // end of constructor
    
    /**
     * paint the graphic, is paints allways the whole game
     * with all fields, the actual stones  
     * 
     * 
     * @param g
     */
    public void paintComponent(Graphics g) {
    	
    	//System.out.println(this.getHeight()+" "+this.getWidth()+" "+length);
    	if ((length != Math.round(getHeight()/8)/*||length != Math.round(getWidth()/8)  )&& this.getHeight()>50 && this.getWidth()>50*/)) {
    		this.length = Math.round(getHeight()/8);
    		//this.length = Math.min(Math.round(getHeight()/8),Math.round(getWidth()/8));
    		this.myDimension = new Dimension(length*8+2,length*8+3);
    		for (int i=0; i < 12; i++) 
    			scalImg[i]= CHESS_FIG[i].getScaledInstance(length,length,Image.SCALE_DEFAULT);
    		
    	}
    	//System.out.println("Hier in repaint"); 
    	if (isOpaque()) { // paint background
            g.setColor(getBackground());
            g.fillRect(0,0,getWidth(),getHeight());
        }
        for (int x = 0; x < 8; x++) {
           for (int y = 0; y < 8; y++) {
              if (position[x][y] >= 200) { // mark in red if chess
                 g.setColor(Color.RED);
                 g.fillRect(1+x*length,1+y*length,length,length);
                 if ( (x % 2==0 && y% 2 == 0)|| (x % 2 == 1 && y % 2 == 1) )
                       g.setColor(Color.WHITE);
                 else  
                       g.setColor(Color.lightGray);
                       g.fillRect(x*length+4,y*length+4,length-6,length-6);
              }
              else {
                if (position[x][y] >= 100) { // mark in yellow if old field 
                 g.setColor(Color.YELLOW);
                 g.fillRect(1+x*length,1+y*length,length,length);
                 if ( (x % 2==0 && y% 2 == 0)|| (x % 2 == 1 && y % 2 == 1) )
                       g.setColor(Color.WHITE);
                 else  
                       g.setColor(Color.lightGray);
                       g.fillRect(x*length+4,y*length+4,length-6,length-6);
                }
                else { // fill normal
                  if ( (x % 2==0 && y% 2 == 0)|| (x % 2 == 1 && y % 2 == 1) )
                    g.setColor(Color.WHITE);
                  else  
                    g.setColor(Color.lightGray);    
                  g.fillRect(1+x*length,1+y*length,length,length);
                }
              }
	          // draw figures
             if (position[x][y]%100 > 0 && position[x][y]%100 < 13) 
            	 g.drawImage(scalImg[(position[x][y]%100)-1],1+x*length,1+y*length, this);  
              // draw outside of rect
             g.setColor(Color.BLACK);        
             g.drawRect(1+x*length,1+y*length,length,length);
             
           }
       }
       if (movFig > 0 && movFig < 13) { // draw the movefigure
    	   g.drawImage(scalImg[movFig-1],this.xMove,this.yMove, this);
    	   //System.out.println("drawing act xmove " + xMove + " act xmove " + yMove);
       }
    }
    
    
	
	
   	
	/**
	 * allows or disallows to set figures and gives the actual field back
	 * @param d
	 */
	public void editPosition(boolean edit) {
		if (posEdit==null) posEdit = new PositionEditor();
		if (motionList==null) motionList = new MotionListener();
		if (edit){
			this.addMouseListener(posEdit);
			this.addMouseMotionListener(motionList);
		}
		else {
			this.removeMouseListener(posEdit);
			this.removeMouseMotionListener(motionList);
			motionList.unmarkLast();
		}
	}
	
	public void editMove(ChessMoveFrame cm, boolean isStart) {
		moveEdit = new MoveEditor(cm, isStart);
		if (motionList==null) motionList = new MotionListener();
	    this.addMouseMotionListener(motionList);
	    this.addMouseListener(moveEdit);
		
	}
	
	public void editMove(ChessGameFrame cm, boolean isStart) {
		moveEdit = new MoveEditor(cm, isStart);
		if (motionList==null) motionList = new MotionListener();
	    this.addMouseMotionListener(motionList);
	    this.addMouseListener(moveEdit);	
	}
	
	private void unEditMove() {
		this.removeMouseMotionListener(motionList);
	    this.removeMouseListener(moveEdit);
	}
	
	public int[][] getField() {
		int[][] pos = new int[8][8];
		for (int i =0; i<8;i++) {
			for (int j=0; j<8;j++) {
				pos[i][j] = position[i][j]%100;
			}
		}
		return pos;
	}
	
	/**
     * set figure or mark on the given coordinates
	 * 2xx red marked field  1xx yellow marked field
	 * xx int between 0 and 12 
	 * 0 - empty field. 1 - pawn white 2 - bishop white
	 * 3 - knigth white 4 - rook white 5 - queen white
	 * 6 - king white 7 - pawn black 8 - bishop black
	 * 9 - knigth black 10 - rook black 11 - queen black
	 * 12 - king black 
	 * @param d
	 */
	public void setFigure(int x, int y, int fig) {
		if (x!=-1 && y!=-1) {
			this.position[x][y] = fig;
			int xup=0, yup =0, xdown=0,ydown=0;
			if ( x > 0 ) xup = 10;
			if ( y > 0 ) yup = 10;
			if ( x < 7 ) xdown = 10;
			if ( y < 7 ) ydown = 10;
			this.paintImmediately(1+x*length-xup,1+y*length-yup,length+xup+xdown,length+yup+ydown);
		}
	}	
	/**
     * mark field on the given coordinates
	 * @param d
	 */
	public void markField(int x, int y, Color c){
	if (x!=-1 && y!=-1) {	
	  if (c == Color.YELLOW || c == Color.yellow) { 
	  	this.position[x][y] = this.position[x][y] % 100 + 100;
	  }
	  else {
		  if (c == Color.RED || c == Color.red) {
			  this.position[x][y] = this.position[x][y] % 100 + 200;
		  }
		  else  {// any color so unmark
			  this.position[x][y] = this.position[x][y] % 100;  
			  //System.out.println("here markfield "+ x+y+c+" "+position[x][y]);
			  
		  }
	  }
	  this.paintImmediately(1+x*length,1+y*length,length,length);
	}
	}
	/**
     * set new field and paint
	 * @param d
	 */
	public void setField(int[][] pos){
		for (int i =0; i<8;i++) {
			for (int j=0; j<8;j++) {
				position[i][j] = pos[i][j];
			}
		}
		this.paintImmediately(this.getVisibleRect());
	}
	
	public void markTheKing (boolean white, boolean markRed) {
		if (white) {
			for (int x =0; x< 8; x++ ){
				for (int y =0; y< 8; y++ ){
					if (position[x][y] == 6 || position[x][y] == 106 || position[x][y] == 206) {
						if (markRed) position[x][y] = 206;
						else position[x][y] = 6; // remove marking
						this.paintImmediately(1+x*length,1+y*length,length,length);
						break;
					}
				}
			}
		}
		else {
			for (int x =0; x< 8; x++ ){
				for (int y =0; y< 8; y++ ){
					if (position[x][y] == 12 || position[x][y] == 112 || position[x][y] == 212) {
						if (markRed) position[x][y] = 212;
						else position[x][y] = 12; // remove marking
						this.paintImmediately(1+x*length,1+y*length,length,length);
						break;
					}
				}
			}
		}
	}
	/**
	* moves a figure from field xs,ys to field xe,ye
	* @param d
	*/
	public void moveFigure(int xs ,int ys, int xe, int ye, int fig){
	if (xs!=-1 && ys!=-1 && xe!=-1 && ye!=-1) {
		this.setFigure(xs, ys, 0);
		this.markField(xs, ys, Color.YELLOW);
		this.xMove = xs*length;
		this.yMove = 1+(ys*length);
		this.movFig = fig;
		int xadd=0, yadd=0;
		if (ye > ys) yadd = 10;
		else 
			if (ye < ys) yadd = -10;
			//else ye ==ys yadd =0;
		if (xe > xs) xadd = 10;
		else 
			if (xe < xs) xadd = -10;
			//else xe ==xs xadd =0;
		//System.out.println("act xmove " + xMove + " act xmove " + yMove);
		while (Math.abs(1+xe*length-xMove) > 10 || Math.abs(1+ye*length-yMove) > 10) {
			//System.out.println("act xdiff " + Math.abs(xe*length-xMove) + " act xdiff " + Math.abs(ye*length-yMove));
			long myTime  = System.currentTimeMillis();
			if (Math.abs(1+xe*length-xMove) > 10) xMove = xMove + xadd;
			else xMove = 1+xe*length;
			if (Math.abs(1+(ye*length)-yMove) > 10) yMove = yMove + yadd;
			else yMove = 1+ye*length;
			//this.paintImmediately(this.getVisibleRect());
			this.paintImmediately(1+(Math.min(xMove,xMove-xadd)),1+(Math.min(yMove,yMove-yadd)),Math.max(length-xadd,length+xadd),Math.max(length-yadd,length+yadd));
			/// wait 60 ms
			while (System.currentTimeMillis() < myTime+60);
			//System.out.println("act xmove " + xMove + " act xmove " + yMove);
		}
		/*yMove = 1+ye*length;
		xMove = xe*length;
		this.paintImmediately(Math.min(xMove,xMove-xadd),1+(Math.min(yMove,yMove-yadd)),Math.max(length-xadd,length+xadd),Math.max(length-yadd,length+yadd));*/
		movFig =0;
		this.setFigure(xe, ye, fig);
	}	
	}
	/**
     * overwrites the getPreferredSize method of JComponent
     * gives the dimension of this field
     * @return Dimension
     */
    public Dimension getPreferredSize() {
        return myDimension;
    }
    /**
     * overwrites the getMinimumSize method of JComponent
     * gives the dimension of this field
     * @return Dimension
     */
    public Dimension getMinimumSize() {
        return myDimension;
    }
    private class MotionListener implements MouseMotionListener {
    	private int markedX = -1;
    	private int markedY = -1;
    	public void unmarkLast() {
    		if (markedX>=0 && markedX <=7 && markedY>=0 && markedY<=7) { 
    			markField(markedX,markedY,Color.WHITE);
    			markedX = -1;
    			markedY = -1;
    		}
    	}
    	public void mouseDragged(MouseEvent e) {
    		// TODO Auto-generated method stub
    		
    	}

    	public void mouseMoved(MouseEvent e) {
    		// TODO Auto-generated method stub
    		double mouseX = e.getX();
    		double mouseY = e.getY();
    		int y = (int)((mouseY-1.0)/length);
    		int x = (int)((mouseX-1.0)/length);
    		if (x!=markedX || y!=markedY) {
    			if (markedX != -1 && markedY != -1) {
    				markField(markedX,markedY,null);
    				//System.out.println("feld "+ markedX +" " +markedY +" old move ");
    			}
    			if (x>=0 && x <=7 && y>=0 && y<=7) { 
    				markedX = x;
    				markedY = y;
    				markField(x,y,Color.YELLOW); 
    				//System.out.println("feld "+ x +" " +y +" move "+markedX+markedY);
    			}
    		}
    			
    	}
    }
    
    private class PositionEditor implements MouseListener {
    	FigureSet fig_dlg = new FigureSet();
    	/** 
         * Components that display logical rows or columns should compute the
         * scroll increment that will completely expose one new row or column,
         * depending on the value of orientation. Only used if mouselistener is addad by method 
         * allowClick()
         */
    	public void mouseClicked(MouseEvent arg0) {
    		double mouseX = arg0.getX();
    		double mouseY = arg0.getY();
            //System.out.println("Koord" + mouseX + " " +mouseY);
    		int y = (int)((mouseY-1)/length);
    		int x = (int)((mouseX-1)/length);
            //System.out.println("feld "+ p.getX() +" " +p.getY() +"geklickt");
    		this.fig_dlg.setLocationRelativeTo(null);
    		this.fig_dlg.setModal(true);
    		this.fig_dlg.setVisible(true);
    		boolean isWhiteField= (x % 2==0 && y% 2 == 0) || (x % 2 == 1 && y % 2 == 1);
    		if (!fig_dlg.getCancel()) {
    		// check if logical position 
    			if (checkPos(this.fig_dlg.getFigure(),isWhiteField))
    				setFigure(x,y,this.fig_dlg.getFigure());
    			else
    				Reporter.showError("invalid chessposition");
    		}
    		//System.out.println("feld "+ x +" " +y +"geklickt"+fig_dlg.getFigure());
    	}
    	
    /* (non-Javadoc)
     * @see java.awt.event.MouseListener#mouseEntered(java.awt.event.MouseEvent)
     */
    public void mouseEntered(MouseEvent arg0) {
        // TODO Auto-generated method stub
    }

    /* (non-Javadoc)
     * @see java.awt.event.MouseListener#mouseExited(java.awt.event.MouseEvent)
     */
    public void mouseExited(MouseEvent arg0) {
        // TODO Auto-generated method stub
    }

    /* (non-Javadoc)
     * @see java.awt.event.MouseListener#mousePressed(java.awt.event.MouseEvent)
     */
    public void mousePressed(MouseEvent arg0) {
        // TODO Auto-generated method stub
    }

    /* (non-Javadoc)
     * @see java.awt.event.MouseListener#mouseReleased(java.awt.event.MouseEvent)
     */
    public void mouseReleased(MouseEvent arg0) {
        // TODO Auto-generated method stub
    }
    
    private boolean checkPos(int fig, boolean isWhite) {
		int max = 0;
		int count = 0;
		if (fig != 0) {
			if (fig == 2 || fig == 8) { // when bishop
				max = 1;
				for (int i =0; i< 8; i++) {
					for (int j=0; j<8; j++) {
						 if (position[i][j]==fig) { // decide if another is on black or white field
							 if ((isWhite && ((i%2==0 && j%2 == 0) || (i%2 == 1 && j%2 == 1))) ||
								 (!isWhite && ((i%2==1 && j%2 == 0) || (i%2 == 0 && j%2 == 1)))) {
						 		setFigure(i,j,0);
						 		return true;
							 }
						 }	 
					 }
				}
				return true;
			}
			if (fig == 1 || fig == 7)
				 max = 8; // eigth pawn
			else
				 if (fig==6 || fig==5 || fig==11 || fig==12)
					 max = 1; // one king,queen
				 else 
					 if (fig==3 || fig==9 || fig==4 || fig==10)
						max =2; // two knigth, rook
					 else 
						 max = 0;
			
			 for (int i =0; i< 8; i++) {
				for (int j=0; j<8; j++) {
					 if (position[i][j]==fig) 
						 count++;
					 if (count==max) {
						 setFigure(i,j,0);
						 count--;
					 }
				}
			}
		}
		else return true;
	return (count<max);
	}
     
    }
    private class MoveEditor implements MouseListener {
    	ChessMoveFrame cm = null;
    	ChessGameFrame cg = null;
    	boolean isStart;
    	
    	MoveEditor(ChessMoveFrame cm, boolean isStart) {
    		this.cm = cm;
    		this.cg = null;
    		this.isStart = isStart;
    	}
    	MoveEditor(ChessGameFrame cm, boolean isStart) {
    		this.cg = cm;
    		this.cm = null;
    		this.isStart = isStart;
    	}
    	
		public void mouseClicked(MouseEvent arg0) {
			double mouseX = arg0.getX();
	        double mouseY = arg0.getY();
	        int y = (int)((mouseY-1)/length);
	        int x = (int)((mouseX-1)/length);
	        if (cm != null) {
	        	unEditMove();
	        	cm.execSet(x, y, isStart);        
	        }
	        if (cg != null) {
	        	//System.out.println("feld "+ x +" " +y +"geklickt"+ position[x][y]);
	        	if (!isStart || (isStart && position[x][y]%100!=0)) {
	        		//System.out.println("feld "+ x +" " +y +"geklickt"+ position[x][y]);
	        		unEditMove();
	        		cg.execSet(x, y, isStart);
	        	}	
	        }	
		}

		public void mouseEntered(MouseEvent e) {
			// TODO Auto-generated method stub			
		}

		public void mouseExited(MouseEvent e) {
			// TODO Auto-generated method stub	
		}

		public void mousePressed(MouseEvent e) {
			// TODO Auto-generated method stub	
		}

		public void mouseReleased(MouseEvent e) {
			// TODO Auto-generated method stub
		}
    	
    }
}    
    
