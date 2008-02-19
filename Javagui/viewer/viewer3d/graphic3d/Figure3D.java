


package viewer.viewer3d.graphic3d;

import viewer.viewer3d.graphic2d.Figure2D;
import gui.idmanager.*;

public abstract class Figure3D{

protected ID myID;

public abstract BoundingBox3D getBoundingBox();

public abstract Figure2D project(FM3DGraphic fm);

public ID getID(){ return myID; }

public void setID(ID aID) { myID.equalize(aID); }


public abstract Figure3D copy();


}

