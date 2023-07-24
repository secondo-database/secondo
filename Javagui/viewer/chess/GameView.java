package viewer.chess;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

/**
 * This class provides the view of the chessboard and is therefore essential for the ChessViewer. Its super-class
 * is JComponent whose paintComponent method is overwritten. Furthermore it implements the ActionListener interface.
 **/
public class GameView extends JComponent implements ActionListener
{
	/**
	 * image for a black bishop
	 */	
	public static final ImageIcon BBISHOP_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH + ChessToolKit.BBISHOP_FILENAME);

	/**
	 * image for a black rook
	 */
	public static final ImageIcon BROOK_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH+ ChessToolKit.BROOK_FILENAME);

	/**
	 * image for a black knight
	 */
	public static final ImageIcon BKNIGHT_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH + ChessToolKit.BKNIGHT_FILENAME);

	/**
	 * image for a black pawn
	 */
	public static final ImageIcon BPAWN_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH+ ChessToolKit.BPAWN_FILENAME);

	/**
	 * image for a black queen
	 */
	public static final ImageIcon BQUEEN_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH + ChessToolKit.BQUEEN_FILENAME);

	/**
	 * image for a black king
	 */
	public static final ImageIcon BKING_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH + ChessToolKit.BKING_FILENAME);

	/**
	 * image for a white bishop
	 */
	public static final ImageIcon WBISHOP_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH+ ChessToolKit.WBISHOP_FILENAME);

	/**
	 * image for a white rook
	 */
	public static final ImageIcon WROOK_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH + ChessToolKit.WROOK_FILENAME);

	/**
	 * image for a white knight
	 */
	public static final ImageIcon WKNIGHT_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH + ChessToolKit.WKNIGHT_FILENAME);

	/**
	 * image for a white pawn
	 */
	public static final ImageIcon WPAWN_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH + ChessToolKit.WPAWN_FILENAME);

	/**
	 * image for a white queen
	 */
	public static final ImageIcon WQUEEN_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH + ChessToolKit.WQUEEN_FILENAME);

	/**
	 * image for a white king
	 */
	public static final ImageIcon WKING_IMAGE = new ImageIcon(ChessToolKit.IMAGES_PATH + ChessToolKit.WKING_FILENAME);
	
	/**
	 * background color for a dark square on the chessboard
	 */
	public static final Color DARK_SQUARE_COLOR = Color.GRAY;

	/**
	 * background color for a light square of the board
	 */
	public static final Color LIGHT_SQUARE_COLOR = Color.LIGHT_GRAY;

	/**
	 * border color for the square where the moves starts
	 */
	public static final Color START_SQUARE_COLOR = Color.GREEN;

	/**
	 * border color for the square where the move ends, if capture while moving
	 */
	public static final Color TARGET_SQUARE_COLOR = Color.RED;

	/**
	 * border color for a square if the mouse edits the square
	 */
	public static final Color MOUSE_SQUARE_COLOR = Color.BLUE;
	 

	/**
	 * currently shown position
	 */
	private PositionData currentPosition;

	/**
	 * list of already shown positions
	 */
        private ArrayList positionsHistory;

/**
 * hashmap to organize the images in. Makes it faster to find the needed image
 */	
	private HashMap images;

/**
 * index number of the currently shown position
 */
	private int lastPositionIndex = -1;

/**
 * indexes of the startRow, startFile, targetRow and targetFile to make them visible if necessary
 */
	private int startRow, startFile, targetRow, targetFile;

/**
 * boolean to check whether a capture took place in the current position or not 
 */
	private boolean showCapture = false;

/**
 *button group for the popup menu
 */
	private ButtonGroup bg;

/**
 * the popup menu of the view, needed for editing
 */
	private JPopupMenu popUpMenu;

/**
 * current mouse position
 */
	private Point mousePosition;

/**
 * location of the viewer 
 */
	private Point location;

/**
 * readiobuttons in the popup-menu
 */
	private JRadioButtonMenuItem[] menuButtons;
/**
 * current dimension of the GameView
 */
	private Dimension dimension;
/**
 * height of the GameView, width of the GameView, difference between the needed width for the chessboard and the total width, difference between the needed height for the chessboard and the total height, width/height of a chessboard square, current mouse position by index of square on chessboard in x and in y
 */
	private int height, width, width_difference, height_difference, squareSize,xbox, ybox;
	
	/**
	 * default constructor which initializes all the fields.
	 */
	public GameView()
	{
		location = new Point();
		mousePosition = new Point();
		images = new HashMap(12);
		fillImagesMap();
		currentPosition = new PositionData();
		currentPosition.startPositions();
		positionsHistory = new ArrayList();
		positionsHistory.add(currentPosition);
		startRow = -1;
		startFile = -1;
		targetRow = -1;
		targetFile = -1;
		this.initPopUpMenu();
		this.addMouseListener(new MouseAdapter() //mouseListener for the GameView
				{
										
					public void mousePressed(MouseEvent e)
					{
						if(e.getButton() == MouseEvent.BUTTON3 ) //only the third button is interesting
						{
							
							location.setLocation(getX(), getY());
							mousePosition.setLocation(e.getX(), e.getY()); //get mouse position
							if(mousePosition.x>(width_difference+squareSize) && mousePosition.y>(height_difference+squareSize) && mousePosition.x<(width_difference+(9*squareSize)) && mousePosition.y<(height_difference+(9*squareSize))) //shown only if mouse is on chessboard
							{
								xbox = (mousePosition.x - width_difference)/squareSize;
								ybox = (mousePosition.y - height_difference)/squareSize;
								char agent = currentPosition.getAgentAt(7-(ybox-1),xbox-1); //get the current agent on this sqaure
								for(int i=0;i<menuButtons.length;i++)
								{
									if(menuButtons[i].getActionCommand().toCharArray()[0] == agent) //select the agent in the popupmenu
									{
										menuButtons[i].setSelected(true);
										break;
									}
								}
								popUpMenu.show(e.getComponent(),e.getX(),e.getY());//show popup menu						
							}
							else
							{
								xbox = -1;
							}
						}
						else
						{
							xbox = -1;
						}
						repaint();					
					}
				});
	}
	
	/**
	 * actionPerformed method for the JRadioButtons of the popupmenu
	 */
	public void actionPerformed(ActionEvent e)
	{
		currentPosition.changePosition(7-(ybox-1),( xbox-1), e.getActionCommand().toCharArray()[0]);
		repaint();
	}

	/**
	 * initialising the popup-menu
	 */
	private void initPopUpMenu()
	{
		popUpMenu = new JPopupMenu("edit square");
		bg = new ButtonGroup();
		menuButtons = new JRadioButtonMenuItem[13];
		menuButtons[0] = new JRadioButtonMenuItem("White King"); 
		menuButtons[0].setActionCommand(""+ChessToolKit.WHITE_KING);
		menuButtons[1] = new JRadioButtonMenuItem("White Queen");
		menuButtons[1].setActionCommand(""+ChessToolKit.WHITE_QUEEN);
		menuButtons[2] = new JRadioButtonMenuItem("White Bishop");
		menuButtons[2].setActionCommand(""+ChessToolKit.WHITE_BISHOP);
		menuButtons[3] = new JRadioButtonMenuItem("White Knight");
		menuButtons[3].setActionCommand(""+ChessToolKit.WHITE_KNIGHT);
		menuButtons[4] = new JRadioButtonMenuItem("White Rook");
		menuButtons[4].setActionCommand(""+ChessToolKit.WHITE_ROOK);
		menuButtons[5] = new JRadioButtonMenuItem("White Pawn");
		menuButtons[5].setActionCommand(""+ChessToolKit.WHITE_PAWN);
		menuButtons[6] = new JRadioButtonMenuItem("Black King");
		menuButtons[6].setActionCommand(""+ChessToolKit.BLACK_KING);
		menuButtons[7] = new JRadioButtonMenuItem("Black Queen");
		menuButtons[7].setActionCommand(""+ChessToolKit.BLACK_QUEEN);
		menuButtons[8] = new JRadioButtonMenuItem("Black Bishop");
		menuButtons[8].setActionCommand(""+ChessToolKit.BLACK_BISHOP);
		menuButtons[9] = new JRadioButtonMenuItem("Black Knight");
		menuButtons[9].setActionCommand(""+ChessToolKit.BLACK_KNIGHT);
		menuButtons[10] = new JRadioButtonMenuItem("Black Rook");
		menuButtons[10].setActionCommand(""+ChessToolKit.BLACK_ROOK);
		menuButtons[11] = new JRadioButtonMenuItem("Black Pawn");
		menuButtons[11].setActionCommand(""+ChessToolKit.BLACK_PAWN);
		menuButtons[12] = new JRadioButtonMenuItem("none");
		menuButtons[12].setActionCommand(""+ChessToolKit.NONE);
		popUpMenu.add(new JMenuItem("keep current"));
		for (int i=0; i< menuButtons.length;i++)
		{
			bg.add(menuButtons[i]);
			popUpMenu.add(menuButtons[i]);
			menuButtons[i].addActionListener(this);
		}
		popUpMenu.setVisible(false);	
		this.add(popUpMenu);	
	}

	/**
	 * this method fills the hashmap with a Character object as key and the image 
	 */
	private void fillImagesMap()
	{
		images.put(Character.valueOf(ChessToolKit.WHITE_KING), WKING_IMAGE);
		images.put(Character.valueOf(ChessToolKit.WHITE_QUEEN), WQUEEN_IMAGE);
		images.put(Character.valueOf(ChessToolKit.WHITE_BISHOP), WBISHOP_IMAGE);
		images.put(Character.valueOf(ChessToolKit.WHITE_ROOK), WROOK_IMAGE);
		images.put(Character.valueOf(ChessToolKit.WHITE_KNIGHT), WKNIGHT_IMAGE);
		images.put(Character.valueOf(ChessToolKit.WHITE_PAWN), WPAWN_IMAGE);
		images.put(Character.valueOf(ChessToolKit.BLACK_KING), BKING_IMAGE);
		images.put(Character.valueOf(ChessToolKit.BLACK_QUEEN), BQUEEN_IMAGE);
		images.put(Character.valueOf(ChessToolKit.BLACK_BISHOP), BBISHOP_IMAGE);
		images.put(Character.valueOf(ChessToolKit.BLACK_KNIGHT), BKNIGHT_IMAGE);
		images.put(Character.valueOf(ChessToolKit.BLACK_ROOK), BROOK_IMAGE);
		images.put(Character.valueOf(ChessToolKit.BLACK_PAWN), BPAWN_IMAGE);		
	}
	
	protected void paintComponent(Graphics g)
	{
		dimension = this.getSize();
		height = (int)dimension.getHeight();
		width = (int)dimension.getWidth(); //get current size of the GameView
		if(height > width)
		{
			squareSize = ((width-6)/9); 
		}
		else
		{
			squareSize = ((height-6)/9); //calculate size of a square
		}
		width_difference = (width - ((squareSize * 8)+(squareSize*2)))/2; //width and height difference
		height_difference = (height - ((squareSize * 8)+(squareSize*2)))/2;
		Graphics myG = g.create();
		myG.setColor(Color.WHITE);
		myG.fillRect(0,0,width,height); //draw background
		myG.translate(width_difference, height_difference); //new origin of graphics context to paint the chess board
		for(int i=0; i< 9; i++)
		{
			for(int j=0;j<9;j++)
			{		
				myG.setColor(Color.BLACK); //paint the row numbers and file-names
				if(i == 0 || j== 0)
				{
					if(j == 0 && i != 0)
					{
						myG.drawString(""+(9-i),j*squareSize+((squareSize/3)*2),i*squareSize+((squareSize/3)*2));		
					}
					if (i == 0 && j!=0)
					{
						myG.drawString(""+ChessToolKit.fileForNumber(j),j*squareSize+((squareSize/3)),i*squareSize+(squareSize-4));
					}
				}
				else
				{			
					myG.drawRect(j*squareSize, i*squareSize, squareSize, squareSize); //paint the squares
					if(i%2 == 0)
					{
						if(j%2 == 0)
						{
							myG.setColor(GameView.LIGHT_SQUARE_COLOR);
							myG.fillRect((j*squareSize)+1,(i*squareSize)+1,squareSize-1,squareSize-1);
						}
						else
						{
							myG.setColor(GameView.DARK_SQUARE_COLOR);
							myG.fillRect((j*squareSize)+1,(i*squareSize)+1,squareSize-1,squareSize-1);
						}
					}
					else
					{
						if(j%2 ==0)
						{
							myG.setColor(GameView.DARK_SQUARE_COLOR);
							myG.fillRect((j*squareSize)+1,(i*squareSize)+1,squareSize-1,squareSize-1);
						}
						else
						{
							myG.setColor(GameView.LIGHT_SQUARE_COLOR);
							myG.fillRect((j*squareSize)+1,(i*squareSize)+1,squareSize-1,squareSize-1);
						}
					
					}
					if((startRow == 7-(i-1) && startFile == j-1) || (targetRow == 7-(i-1) && targetFile ==j-1 )) //paint borders if necessary
					{
						if(targetFile == j-1 && targetRow == 7-(i-1)&& showCapture)
							myG.setColor(GameView.TARGET_SQUARE_COLOR);
						else
							myG.setColor(GameView.START_SQUARE_COLOR);
						myG.drawRect((j*squareSize) +1,( i*squareSize)+1, squareSize-2, squareSize-2);
						myG.drawRect((j*squareSize) +2,( i*squareSize)+2, squareSize-4, squareSize-4);
					}
					if(xbox == j && ybox == i) //paint mouse border if necessary
					{
						myG.setColor(GameView.MOUSE_SQUARE_COLOR);
						myG.drawRect((j*squareSize) +1,( i*squareSize)+1, squareSize-2, squareSize-2);
						myG.drawRect((j*squareSize) +2,( i*squareSize)+2, squareSize-4, squareSize-4);

					}
					ImageIcon currentImage = ((ImageIcon)images.get(Character.valueOf(currentPosition.getAgentAt(7-(i-1),(j-1))))); //paint the icon
					if (currentImage != null)
						myG.drawImage(currentImage.getImage(), (j*squareSize)+2,(i*squareSize)+2, squareSize-4, squareSize-4 ,this);	
				}
			}
		}
		myG.dispose(); //clean up
			
	}

	
/**
 * sets the current position to p and shows it in the viewer
 */
	public void showPosition(PositionData p)
	{
		this.currentPosition = p;
		this.repaint();
	}

/**
 * shows the move m in the viewer, position is the index of m in all the moves of the game. 
 * @throws IllegalArgumentException if position fulfills not the following condition: position <= count of already shown moves+1
 */
	public void showMove(MoveData m, int position) throws IllegalArgumentException
	{
		if(position<positionsHistory.size()) //if position has already been shown before
		{
			this.currentPosition = (PositionData)positionsHistory.get((position));	 //show the position again from the history
			lastPositionIndex = position;
		}
		else
		{
			if(position == positionsHistory.size()) //if m is the next move after current move
			{
				currentPosition = new PositionData(); //create new PositionData object
				currentPosition.copyPositions(((PositionData)positionsHistory.get(positionsHistory.size()-1)).getPositions());
				currentPosition.doMove(m);
				positionsHistory.add(currentPosition); //ad move to history
				lastPositionIndex = positionsHistory.size()-1;
			}
			else
			{
				throw new IllegalArgumentException(); //if move is not directly following the current move
			}
		}
		if (position != 0) //if not the initial position is shown
		{
			startRow = m.getStartRow()-1; //get the indexes that have to be  made visible
			targetRow = m.getTargetRow()-1;
			startFile = ChessToolKit.fileToNumber(m.getStartFile());
			targetFile = ChessToolKit.fileToNumber(m.getTargetFile());
			showCapture = m.getCaptured() != ChessToolKit.NONE;
		}
		else //no special square borders have to be painted
		{
			startRow =-1;
			startFile = -1;
			targetRow = -1;
			targetFile = -1;	
		}
	}

	/**
	 * delivers the shown position
	 */
	public PositionData getCurrentPosition()
	{
		return currentPosition;
	}

	/**
	 * get the index of the current shown move/position
	 */
	public int getLastPositionIndex()
	{
		return lastPositionIndex;
	}

	/**
	 * reset the viewer no chessmen will be shown on the chess board if called
	 */
	public void reset()
	{
		currentPosition.clear();
		this.positionsHistory.clear();
		this.positionsHistory.add(currentPosition);
		this.repaint();
		lastPositionIndex = -1;
		startRow = -1;
		startFile = -1;
		targetRow = -1;
		targetFile = -1;
	}

	/**
	 * same as reset but the chessmen are shown in their starting positions
	 */
	public void clear()
	{
		this.currentPosition.startPositions();
		this.positionsHistory.clear();
		this.positionsHistory.add(currentPosition);
		this.repaint();
		lastPositionIndex = -1;
		startRow = -1;
		startFile = -1;
		targetRow = -1;
		targetFile = -1;
	}

}

