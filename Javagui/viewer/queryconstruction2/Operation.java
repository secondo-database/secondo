//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
package viewer.queryconstruction2;

import java.awt.Color;	///nur zum Text grauen

import java.awt.Graphics;
import java.util.Arrays;
import javax.swing.BorderFactory;


/**
 * This class is respresenting the operations in the query 
 * and the operation objects in the OperationsPane.
 */
public class Operation extends ObjectView { 
    
    //name of the operator shown in the OperationsPane
    private String name;
    //objects, the operator is applicable to
    private String[] objects;
    //parameters, the operator needs
    private String[] parameter;
    //result type after using the operator
    private String result;
    //siganture of the operator
    private String signature;
    //object component belonging to the operation
    private ObjectView view;

    private boolean active = true;  ///
    
    /**
     * Constructor of an operation.
     * @param name name of the operation
     * @param objects type of objects which the operation needs
     * @param signature signature of the operation
     * @param parameter type of parameter which the operation needs
     * @param result type of the result, when needed
     */
    public Operation(String name, String[] objects, String signature, 
            String[] parameter, String result){
        this.setOpaque(false);
        this.name = name;
        this.signature = signature;
        this.result = result;
        this.parameter = parameter;
        this.objects = objects;
        Arrays.sort(this.objects);
        
        /* initialize the related object component */
        view = new ObjectView(name, ObjectView.OPERATION);
        view.setSignature(signature);
    }  
    
    /**
     * Returns a copy of this operation object.
     * @return 
     */
    protected Operation copy() {
        return new Operation(this.name, this.objects, this.signature, 
                this.parameter, this.result);
    }
    
    /**
     * Returns the number of objects the operator needs.
     * @return 
     */
    protected int countObjects() {
        return objects.length;
    }
    
    /**
     * 
     * @return objects array
     */
    protected String[] getObjects() {
        return this.objects;
    }
    
    /**
     * Returns the Name of the object, that is shown in the viewer.
     * @return 
     */
    protected String getOperationName() {
        return getView().getOnlyName();
    }
    
    /**
     * 
     * @return parameter array
     */
    protected String[] getParameter() {
        return this.parameter;
    }
    
    /**
     * Returns the type, the query has after using the operation. 
     * @return 
     */
    protected String getResultType() {
        return result;
    }
    
    /**
     * Returns the signature of this operation.
     * @return 
     */
    protected String getSignature() {
        return signature;
    }
    
    /**
     * Get the ObjectView object of this operation.
     * @return 
     */
    protected ObjectView getView() {
        return this.view;
    }
///
    protected void setInactive() {
	active = false;
    }
///    
    public void paintComponent(Graphics g) {
        this.setBorder(BorderFactory.createEtchedBorder());

        String s = name;

	if (active == false) {        
	    g.setColor(Color.blue);	///nur zum Text grauen lightGrey
	}
	g.drawString(s, 20, this.getSize().height/2 + 5);
    }
}
