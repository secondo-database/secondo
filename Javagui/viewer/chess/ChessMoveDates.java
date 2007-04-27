package viewer.chess;

import sj.lang.ListExpr;

class ChessMoveDates {
	protected int colS = -1;
	protected int colE = -1;
	protected int rowS = -1;
	protected int rowE = -1;
		/* for movFigure and capFigure is valid:
		 * 0 - empty field 1 - pawn white 2 - bishop white
		 * 3 - knigth white 4 - rook white 5 - queen white
		 * 6 - king white 7 - pawn black 8 - bishop black
		 * 9 - knigth black 10 - rook black 11 - queen black
		 * 12 - king black */
	protected int movFigure = 0;
	protected int capFigure = 0;
	protected int proFigure = 0;
	protected int castling = 0; 	
		/* 0 - no cast, 1 - white short,2 - white long
			3 - black short, 4 - black long */
	protected boolean chess = false;
	protected boolean mate = false;
	protected int pawnPassCap = 0; // 0- no,-1 left, +1 right
	protected int moveNo=0;
	
	
public ChessMoveDates(int cS, int cE, int rS, int rE, int mfig, int cfig, int pfig, char chess, char cast, int number) {
	//System.out.println( "here class chessmovedates");
	moveNo = number;
	if (mfig >=0 && mfig <=12)
		this.movFigure = mfig;
	else
		this.movFigure =0;
	if (cS >=0 && cS <=7)
		this.colS = cS;
	else
		this.movFigure = 0;
	if (cE >=0 && cE <=7)	
		this.colE = cE;
	else
		this.movFigure = 0;
	if (rS >=0 && rS <=7)
		this.rowS = rS;
	else
		this.movFigure = 0;
	if (rE >=0 && rE <=7)
		this.rowE = rE;
	else
		this.movFigure = 0;
	this.castling = cast;
	if (castling == 'l' && moveNo%2 == 1) { // white castling
		this.castling = 2;
		this.movFigure = 6;
		this.colS = 4; 
		this.rowS = 7;
		this.colE = 2;
		this.rowE = 7;
	}
	if (castling == 'l' && moveNo%2 == 0) { // black castling
		this.castling = 4;
		this.movFigure = 12;
		this.colS = 4;
		this.rowS = 0; 
		this.colE = 2;
		this.rowE = 0;
	}
	if (castling == 's' && moveNo%2 == 1) { // white castling
		this.castling = 1;
		this.movFigure = 6;
		this.colS = 4; 
		this.rowS = 7;
		this.colE = 6; 
		this.rowE = 7;
	}
	if (castling == 's' && moveNo%2 == 0) { // black castling
		this.castling = 3;
		this.movFigure = 12;
		this.colS = 4; 
		this.rowS = 0;
		this.colE = 6;
		this.rowE = 0;
	}
	if (chess == 'c') {
		this.chess = true;
	}
	if (chess == 'm') {
		this.chess = true;
		this.mate = true;
	}
	if (cfig >=0 && cfig <=12) {
		this.capFigure = cfig;
	}
	else
		this.capFigure =0;
	if (pfig >=0 && pfig <=12){
		this.proFigure = pfig;
	}
	else
		this.proFigure =0;
	
}

public ChessMoveDates(int no) {
	moveNo = no;
}

/*
 * checks if a move is valid with the given chessfield
 * it checks:
 * - the move is in rules
 * - the field the figure passes are free
 * - the endfield is ok, captured Figure will be set if detected or check 
 * - if rochade is ok
 * - if promoted figure is valid, set to nothing when not the pawn moved in the rightway
 * it checks not if the chess/mate switches are correct
 * only for pawn move chess-switch will be set if king detected  
 */
	public boolean checkMove(ChessField cf, boolean onlyMove) {
		
		//System.out.println(this.getListExpr().writeListExprToString());
		int [][] pos = cf.getField();
		if (movFigure > 0 && movFigure <= 6) { // must be a white move
			if (rowE != 7 || movFigure != 1) proFigure = 0;
			if (pos[colE][rowE]==0 || (pos[colE][rowE]<12 &&  pos[colE][rowE]>=7)) {
				//if there is something to capture or empty set capFigure
				if (!onlyMove) capFigure = pos[colE][rowE];
				if ((capFigure<=6 && capFigure>=1) || capFigure ==12) {
					//capFigure wrong (own fig or king)
					capFigure = 0;
				} 
				//System.out.println("endfield ok white");
			}	
			else {
			//if ((f <= 6 && f >= 1) || f==12) {
				// if on end field is a own figure or the black king
				//System.out.println("capFig wrong white");
				return false;
			}	
			if (moveNo % 2 == 0) return false;	
		}
		else {	// must be a black move
			if (movFigure >= 7 && movFigure <= 12) { 
				if (rowE != 0 || movFigure != 7) proFigure = 0;
				if (pos[colE][rowE]==0 || (pos[colE][rowE]<6 &&  pos[colE][rowE]>=1)) {
					// if there is something to capture set capFigure
					if (!onlyMove) capFigure = pos[colE][rowE];
					if (capFigure<=12 && capFigure>=6) {
						// if capFigure is wrong (own fig or king)
						capFigure = 0;
					}	
					//System.out.println("capFig ok black");
				}	
				else {
				// if (f<=12 &&  f>=6) {
				// if there is a own figure or the white king
					//System.out.println("endfield wrong black");
					return false;
				}
				if (moveNo % 2 != 0) return false;
			}
			else return false;
		}
		int difRow, difCol, rAdd=0, cAdd=0;
		switch (movFigure) {
		case 0: return false;//0 - empty field
		case 1: //1 - pawn white
			castling = 0; //can't be cast
			if (rowE != 0) proFigure = 0;
			if (rowS == 7 || rowE ==7) return false;
			difRow = rowS-rowE;
			difCol = Math.abs(colE-colS);
			if (difRow == 1) { // gone 1 field strait or cap
				if (difCol==0) { //straight no cap
					capFigure=0;
					if (rowE == 0 && !(proFigure>=1 && proFigure<6))
						return false;
					return true;
				}
				if (difCol==1) { // cross and cap
					if (!(capFigure>=7 && capFigure<12))
						return false;
					else {
						if (!onlyMove && capFigure == 7 && pos[colE][rowE] == 0) {
							if (colS > 0 && pos[colS-1][rowS] == 7) pawnPassCap = -1;
							if (colS < 7 && pos[colS+1][rowS] == 7) pawnPassCap = +1;
						}
						if (rowE == 0 && !(proFigure>=1 && proFigure<6))
							return false;
						else return true;
					}	
				}	
				if (rowE>0) {
					if (colE<7 && pos[colE+1][rowE-1]==12)
						chess = true;
					if (colE>0 && pos[colE-1][rowE-1]==12)
						chess = true;
				}
				else { // if in last row cant give chess
					chess = false;
					mate = false;
				}
				return true;
			}
			if (difRow==2 && difCol==0 && rowS==6 && pos[colS][5]==0) {
			// 2 fields gone, it was startrow, passed field free
					proFigure = 0;
					capFigure = 0;
					return true;
			}
			else return false;
		case 2:	//2 - white bishop
				castling = 0; //can't be cast
				proFigure = 0;
				difRow = rowE-rowS;
				difCol = colE-colS;
				//System.out.println("difrow"+difRow+"difcol"+difCol);
				if (Math.abs(difRow)!=Math.abs(difCol)) return false;
				else { // check if the way is free
					if (difCol>0) cAdd=1;
					else {
						if (difCol<0) cAdd=-1;
						else return false;
					}	
					if (difRow>0) rAdd=1;
					else {
						if (difRow<0) rAdd=-1; 
						else return false;
					}	
					return wayFree(rAdd, cAdd,pos);
				}
		case 3: //3 - knigth white
				castling = 0; //can't be cast
				proFigure = 0;
				difRow = Math.abs(rowE-rowS);
				difCol = Math.abs(colE-colS);
				//System.out.println("difrow"+difRow+"difcol"+difCol);
				if (!((difRow==2 && difCol==1)||(difRow==1 && difCol==2)))
						return false;
				// check endfield
				return true;
		case 4: // 4- white rook
				castling = 0; //can't be cast it's kings move
				proFigure = 0;
				difRow = rowE-rowS;
				difCol = colE-colS;
				//System.out.println("difrow"+difRow+"difcol"+difCol);
				if(difRow!=0 && difCol!=0)
					return false;
				else { // check if the way is free
					if (difCol==0) {
						if (difRow >0) rAdd=1; 
						else rAdd=-1;
					}
					if (difRow==0) {
						if (difCol>0) cAdd=1; 
						else cAdd=-1;
					}
					return wayFree(rAdd,cAdd,pos);
				}
		case 5: //5-white queen
				castling = 0; //can't be cast
				proFigure = 0;
				difRow = rowE-rowS;
				difCol = colE-colS;
				//System.out.println("difrow"+difRow+"difcol"+difCol);
				if(difRow!=0 && difCol!=0 && Math.abs(difCol)!=Math.abs(difRow))
					return false;
				else { // check if the way is free
					if (difCol>0) cAdd=1;
					else {
						if (difCol<0) cAdd=-1;
						else cAdd=0;
					}
					if (difRow>0) rAdd=1;
					else {
						if (difRow<0) rAdd=-1; 
						else rAdd=0;
					}
					return wayFree(rAdd,cAdd,pos);
				}
		case 6: // 6-king white
				proFigure = 0;
				difRow = Math.abs(colE-colS);
				difCol = Math.abs(rowE-rowS);
				//System.out.println("rE "+rowE+" cE "+colE+" rS "+ rowS + " cS " +colS);
				//System.out.println(pos[5][7]+" "+pos[6][7]);
				// check if castling
				if (colS == 4 && rowS == 7 && colE == 6 && rowE == 7 &&
					pos[5][7]==0 && pos[6][7]==0) {
					if (!onlyMove && pos[7][7] != 4) return false;
					castling = 1;
					capFigure = 0;
					System.out.println("short cast");
					return true;
				}	
				if (colS == 4 && rowS == 7 && colE == 2 && rowE == 7 &&
					pos[3][7]==0 && pos[2][7]==0 && pos[1][7]==0) {
					if (!onlyMove && pos[0][7] != 4) return false;
					castling = 2;
					capFigure = 0;
					return true;
				}	
				castling =0;
				if (!(difRow <=1 && difCol <=1 && (difCol>0 || difRow>0)) )
					return false;
				// check endfield
				return true;
		case 7: // 7 - pawn black
				castling = 0; //can't be cast
				if (rowE != 7) proFigure = 0;
				if (rowS == 0 || rowE == 0) return false;
				difRow = rowE-rowS;
				difCol = Math.abs(colE-colS);
				if (difRow == 1) { // gone 1 field strait or captured
					if (difCol==0) { //straight no captured
						capFigure=0;
						if (rowE == 7 && !(proFigure>=8 && proFigure<12))
							return false;
						return true;
					}
					if (difCol==1) { // cross and captured
						if (!(capFigure>=1 && capFigure<6))
							return false;
						else {
							if (!onlyMove && capFigure == 1 && pos[colE][rowE] == 0) {
								if (colS > 0 && pos[colS-1][rowS] == 1) pawnPassCap = -1;
								if (colS < 7 && pos[colS+1][rowS] == 1) pawnPassCap = +1;
							}
							if (rowE == 7 && !(proFigure>=8 && proFigure<12))
								return false;
							else return true;
						}	
					}	
					if (rowE<7) {
						if (colE<7 && pos[colE+1][rowE+1]==6)
							chess = true;
						if (colE>0 && pos[colE-1][rowE+1]==6)
							chess = true;
					}
					else { // if in last row cant give chess
						chess = false;
						mate = false;
					}
					return true;
				}
				if (difRow==2 && difCol==0 && rowS==1 && pos[colS][2]==0) {
				// 2 fields gone, it was startrow, no captured, passed field free
						proFigure = 0;
						capFigure = 0;
						return true;
				}
				else return false;
		case 8: // 8-black bishop
			castling = 0; //can't be cast
			proFigure = 0;
			difRow = rowE-rowS;
			difCol = colE-colS;
			if (Math.abs(difRow)!=Math.abs(difCol)) return false;
			else { // check if the way is free
				if (difCol>0) cAdd=1;
				else
					if (difCol<0) cAdd=-1;
					else return false;
				if (difRow>0) rAdd=1;
				else
					if (difRow<0) rAdd=-1; 
					else return false;
				return wayFree(rAdd, cAdd,pos);
			}
		case 9: //9 - black knigth
			castling = 0; //can't be cast
			proFigure = 0;
			difRow = Math.abs(rowS-rowE);
			difCol = Math.abs(colE-colS);
			if (!((difRow==2 && difCol==1) || (difRow==1 && difCol==2)))
				return false;
			// check endfield
			return true;
		case 10: // 10-black rook
				castling = 0; //can't be cast it's kings move
				difRow = rowE-rowS;
				difCol = colE-colS;
				if(difRow!=0 && difCol!=0)
					return false;
				else { // check if the way is free
					if (difCol==0) {
						if (difRow >0) rAdd=1; else rAdd=-1;
					}
					if (difRow==0) {
						if (difCol>0) cAdd=1; else cAdd=-1;
					}
					
					return wayFree(rAdd, cAdd,pos);
				}	
		case 11: //11-black queen
			castling = 0; //can't be cast
			proFigure = 0;
			difRow = rowE-rowS;
			difCol = colE-colS;
			if(difRow!=0 && difCol!=0 && Math.abs(difCol)!=Math.abs(difRow))
				return false;
			else { // check if the way is free
				if (difCol>0) cAdd=1;
				else {
					if (difCol<0) cAdd=-1;
					else cAdd=0;
				}	
				if (difRow>0) rAdd=1;
				else {
					if (difRow<0) rAdd=-1; 
					else rAdd=0;
				}	
				return wayFree(rAdd,cAdd,pos);
			}
		case 12: // 12-king black
			proFigure = 0;
			difRow = Math.abs(rowE-rowS);
			difCol = Math.abs(colE-colS);
			// check if castling
			if (colS == 4 && rowS == 0 && colE == 6 && rowE == 0 &&
				pos[5][0]==0 && pos[6][0]==0) {
				if (!onlyMove && pos[7][0] != 10) return false;
				castling = 3;
				capFigure = 0;
				return true;
				// can't check for rook it could be a isolated move
			}	
			if (colS == 4 && rowS == 0 && colE == 2 && rowE == 0 &&
				pos[3][0]==0 && pos[2][0]==0 && pos[1][0]==0) {
				if (!onlyMove && pos[0][0] != 10) return false;
				castling = 4;
				capFigure = 0;
				return true;
			}
			castling =0;
			if (!(difRow <=1 && difCol <=1 && (difCol>0 || difRow>0)) )
				return false;
			// check endfield
			return true;
		default: return false;		
		}
	}
	
	/*
	 * check if the passed field of a moved figure are free
	 */
	private boolean wayFree(int rAdd, int cAdd, int[][] pos) {
		int rTemp=rowS+rAdd;
		int cTemp=colS+cAdd;
		while (rTemp!=rowE && cTemp!=colE) {
			//System.out.println("check field "+ cTemp +" "+ rTemp);
			if (pos[cTemp][rTemp]!=0) {
				return false;
			}
			cTemp=cTemp+cAdd;
			rTemp=rTemp+rAdd;
		}
		//System.out.println("Weg ist frei");
		return true;
	}
	
	
	/*
	 * returns the ListExpr of the value this chessmove
	 */
	public ListExpr getListExpr() {
		StringBuffer buffer = new StringBuffer(); 
		String fig = buffer.append(ChessObject.parseToChar(movFigure)).toString();
		buffer = new StringBuffer();
		//System.out.println("colS "+ChessObject.parseCol(colS));
		//System.out.println("rowS "+ChessObject.parseRow(rowS));
		buffer.append(ChessObject.parseCol(colS));
		buffer.append(ChessObject.parseRow(rowS));
		String start = new String(buffer);
		buffer = new StringBuffer(); 		 
		buffer.append(ChessObject.parseCol(colE));
		buffer.append(ChessObject.parseRow(rowE));
		String end = new String(buffer);
		buffer = new StringBuffer(); 
		if (this.mate) buffer.append('m');
		else 
			if (this.chess)buffer.append('c');
			else buffer.append('-');
		buffer.append(ChessObject.parseToChar(this.proFigure));
		if (this.castling == 2 || this.castling == 4) {
			buffer.append('l');
		}
		else 
			if (castling == 1 || castling == 3) {
				buffer.append('s');
			}
			else buffer.append('-');
		String action = new String(buffer);
		buffer = new StringBuffer();
		buffer.append(ChessObject.parseToChar(this.capFigure));
		String caption = new String(buffer);
		ListExpr val = 	ListExpr.sixElemList(ListExpr.stringAtom(fig),
				ListExpr.stringAtom(start),
				ListExpr.stringAtom(end),
				ListExpr.stringAtom(action),
				ListExpr.stringAtom(caption),
				ListExpr.intAtom(this.moveNo));
		//System.out.println("my move list"+ val.writeListExprToString());
		return val;
	}
	
	/*
	 * sets a move back on the given chessfield
	 */
	public void setBack(ChessField cf) {
		//System.out.println("Here setBack");
			switch (castling) {
				case 0: 
					if ( movFigure != 0 && !(rowE == -1 || rowS == -1 || colE == -1 || colS == -1)) {
						
						cf.setFigure(colS,rowS,movFigure);
						if (pawnPassCap != 0) 
							cf.setFigure(colS+pawnPassCap,rowS,capFigure);	
						else 
							cf.setFigure(colE,rowE,capFigure);
					}	
					break;
				case 1: //set the white rook back
					cf.setFigure(4, 7, 6);
					cf.setFigure(6, 7, 0);
					cf.setFigure(7, 7, 4);
					cf.setFigure(5, 7, 0);
					break;
				case 2: // set the white rook back
					cf.setFigure(4, 7, 6);
					cf.setFigure(2, 7, 0);
					cf.setFigure(0, 7, 4);
					cf.setFigure(3, 7, 0);
					break;
				case 3:// set the black rook back
					cf.setFigure(4, 0, 12);
					cf.setFigure(6, 0, 0);
					cf.setFigure(7, 0, 10);
					cf.setFigure(5, 0, 0);
					break;
				case 4://set the black rook back
					cf.setFigure(4, 7, 12);
					cf.setFigure(2, 7, 0);
					cf.setFigure(0, 0, 10);
					cf.setFigure(3, 0, 0);
					break;
				default:
			}
			if (chess) 
			// if movFigure > 6 it was a black turn so mark the white king
			// else it was a white turn so mark the black king
				cf.markTheKing(movFigure>6, false);	
	}
	
	/*
	 * clears the marks of this move 
	 * after a move was show always the startfield is marked in yellow and 
	 * if chess or mate the kingsfield is marked in red
	 */
	public void unmark(ChessField cf) {
		if (chess || mate) cf.markTheKing(movFigure>6, false);
		switch (castling) {
		case 0: 
			if ( movFigure != 0 && !(rowE == -1 || rowS == -1 || colE == -1 || colS == -1)) {
				cf.markField(colS,rowS,null);
			}	
			break;
		case 1: //set the white rook back
			cf.markField(4,7,null);
			cf.markField(7,7,null);
			break;
		case 2: // set the white rook back
			cf.markField(4,7,null);
			cf.markField(0,7,null);
			break;
		case 3:// set the black rook back
			cf.markField(4,0,null);
			cf.markField(7,0,null);
			break;
		case 4://clears the black rook and black king
			cf.markField(4,0,null);
			cf.markField(0,0,null);
			break;
		default:
	}
		
	}
	
	/* 
	 * clears the last move from given chessfield
	 */
	public void clearMove(ChessField mycf) {
		//System.out.println("Hier clear");
		if (!(rowE == -1 || rowS == -1 || colE == -1 || colS == -1)){
			mycf.setFigure(colS,rowS,0);
			mycf.setFigure(colE,rowE,0);
		}
		if (chess || mate) mycf.markTheKing(movFigure>6, false);
		switch(this.castling) {
			case 0: break;
			case 1: //set the white rook back
				mycf.setFigure(4, 7, 0);
				mycf.setFigure(6, 7, 0);
				mycf.setFigure(7, 7, 0);
				mycf.setFigure(5, 7, 0);
				break;
			case 2: // set the white rook back
				mycf.setFigure(4, 7, 0);
				mycf.setFigure(2, 7, 0);
				mycf.setFigure(0, 7, 0);
				mycf.setFigure(3, 7, 0);
				break;
			case 3:// set the black rook back
				//System.out.println("Hier clear castling black short");
				mycf.setFigure(4, 0, 0);
				mycf.setFigure(6, 0, 0);
				mycf.setFigure(7, 0, 0);
				mycf.setFigure(5, 0, 0);
				break;
			case 4://set the black rook back
				mycf.setFigure(4, 7, 0);
				mycf.setFigure(2, 7, 0);
				mycf.setFigure(0, 0, 0);
				mycf.setFigure(3, 0, 0);
				break;
			default:
		}
	}
	
	/*
	 * show the move on the given chesssfield with animation
	 */
	public void move(ChessField cf) {
		//System.out.println("Here move");
		
			switch (castling) {
				case 0: 
					if ( movFigure != 0 && !(rowE == -1 || rowS == -1 || colE == -1 || colS == -1)) {
						cf.setFigure(colS,rowS,movFigure);
						cf.moveFigure(colS, rowS, colE, rowE, movFigure);
						if (proFigure != 0)
							cf.setFigure(colE, rowE, proFigure);
						if (pawnPassCap != 0) cf.setFigure(colS+pawnPassCap,rowS,0);		
					}
					break;
				case 1: //move the white rook short
					cf.moveFigure(4, 7, 6, 7, 6);
					cf.moveFigure(7, 7, 5, 7, 4);
					break;
				case 2: //move the white rook long
					cf.moveFigure(4, 7, 2, 7, 6);
					cf.moveFigure(0, 7, 3, 7, 4);
					break;
				case 3://move the black rook short
					cf.moveFigure(4, 0, 6, 0, 12);
					cf.moveFigure(7, 0, 5, 0, 10);
					break;
				case 4://move the black rook long
					cf.moveFigure(4, 0, 2, 0, 12);
					cf.moveFigure(0, 0, 3, 0, 10);
					break;
				default:
			}		
			if (chess) {
			// if movFigure > 6 it was a black turn so mark the white king
			// else it was a white turn so mark the black king
				cf.markTheKing(movFigure>6, true);
			}
		}
	
	/*
	 * show the move on the given chesssfield without animation
	 */
	public void setFor(ChessField cf) {
		System.out.println("Here setfor");
		
			switch (castling) {
				case 0:
					if ( movFigure != 0 && !(rowE == -1 || rowS == -1 || colE == -1 || colS == -1)) {
						cf.setFigure(colS, rowS, 100);
						if (proFigure != 0)
							cf.setFigure(colE, rowE, proFigure);
						else
							cf.setFigure(colE, rowE, movFigure);
						if (pawnPassCap != 0) 
							cf.setFigure(colS+pawnPassCap,rowS,capFigure);	
					}	
					break;
				case 1: //move the white rook short
					cf.setFigure(4, 7, 100);
					cf.setFigure(6, 7, 6);
					cf.setFigure(7, 7, 100);
					cf.setFigure(5, 7, 4);
					break;
				case 2: //move the white rook long
					cf.setFigure(4, 7, 100);
					cf.setFigure(2, 7, 6);
					cf.setFigure(0, 7, 100);
					cf.setFigure(3, 7, 4);
					break;
				case 3://move the black rook short
					cf.setFigure(4, 0, 100);
					cf.setFigure(6, 0, 12);
					cf.setFigure(7, 0, 100);
					cf.setFigure(5, 0, 10);
					break;
				case 4://move the black rook long
					cf.setFigure(4, 0, 100);
					cf.setFigure(2, 0, 12);
					cf.setFigure(0, 0, 100);
					cf.setFigure(3, 0, 10);
					break;
				default:
			}
			
			if (chess) {
			// if movFigure > 6 it was a black turn so mark the white king
			// else it was a white turn so mark the black king
				cf.markTheKing(movFigure>6, true);
			}
		}
}
	
