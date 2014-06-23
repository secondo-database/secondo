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

package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.ScrollPanel;

/**
*  This class contains the dialog for the secondo syntax in the help menu.
*  
*  @author Kristina Steiger
*  
**/
public class SecondoSyntaxDialog {
	
	private DialogBox helpDialogBox = new DialogBox();
    private FlowPanel dialogContents = new FlowPanel();
    private ScrollPanel scrollContent = new ScrollPanel();
	private HTML secondoSyntax;
	private Button closeButton = new Button("Close");
	
	public SecondoSyntaxDialog(){
		
		helpDialogBox.setText("Info about Secondo Syntax"); //Title

	    // Create a table to layout the content
	    dialogContents.getElement().getStyle().setPadding(5, Unit.PX);
	    helpDialogBox.setWidget(dialogContents);
		
		secondoSyntax = new HTML("<h3>Inquiries</h3><p> " +
				"Inquiry commands are used to inspect the actual system and database configuration. They can be called even without a database. " +
				"<ul><li>list type constructors</li></ul><br>" +
				"Displays all names of type constructors together with their specification and an example in a formatted mode on the screen." +
				"<ul><li>list operators</li></ul><br>Nearly the same as the command above, but information about operations is presented instead." +
				"<ul><li>list algebras</li></ul><br>" +
				"Displays a list containing all names of active algebra modules." +
				"<ul><li>list algebra <identifier></li></ul><br>" +
				"Displays type constructors and operators of the specified algebra." +
				"<ul><li>list databases</li></ul><br>" +
				"Displays a list of names for all known databases." +
				"<ul><li>list types</li></ul><br>" +
				"Displays a list of type names defined in the currently opened database." +
				"<ul><li> list objects</li></ul><br>" +
				"Displays a list of objects present in the currently opened database." +
				"<h3>Basic Commands</h3><p>" +
				"These are the fundamental commands executed by SECONDO. They provide creation and manipulation of types and objects as well as querying, within an open database." +
				"<ul><li>type (identifier) = (type expression)</li></ul><br>" +
				"Creates a new type named (identifier) for the given type expression." +
				"<ul><li>delete type (identifier)</li></ul><br>" +
				"Deletes the user defined type named<identifier>." +
				"<ul><li>create (identifier) : (type expression)</li></ul><br>" +
				"Creates an object called (identifier) of the type given by (type expression). The value is still undefined." +
				"<ul><li>update (identifier) := (value expression)</li></ul><br>" +
				"Assigns the result value of the right hand side to the object (identifier)." +
				"<ul><li>let (identifier) = (value expression)</li></ul><br>" +
				"Assign the result value right hand side to a new object called (identifier). The object is not allowed to exist yet; it is created by this command and its type is defined as the" +
				" one of the value expression. The main advantage vs. using create and update is that the type is determined automatically." +
				"<ul><li>derive (identifier) = (value expression)</li></ul><br>" +
				"This is a variant of the let command which can be useful to construct objects which use other objects as input and have no external list representation, e.g. indexes. When restoring a database those objects are reconstructed automatically." +
				"<ul><li>delete (identifier)</li></ul><br>" +
				"Destroys the object whose name is (identifier)." +
				"<ul><li> query (value expression)</li></ul><br>" +
				"Evaluates the given value expression and returns the result object. If the user interface provides no special display functions for the objects type it will be displayed as a nested list." +
				"<ul><li>kill (identifier)</li></ul><br>" +
				"Removes the object whose name is (identifier) from the database catalog without removing its datastructures. Generally, the delete command should be used to remove database " +
				"objects, but this command may be useful if delete would crash the database due to corrupted persistent data structures for this object." +
				"<ul><li>if (value expr) then (command1) [ else (command2)] endif</li></ul><br>" +
				"This is the conditional command. (bool value expression) is a value expression of typembool. It is evaluated by the query processor. This is only possible, if a database is currently open. " +
				"If the result is (bool TRUE),command1 is executed. If the result is (bool FALSE), nothing happens unless the optional else-part is used in which case command2 is executed. If the evaluation of" +
				" the predicate does not return a defined bool value, the complete commandfails. The result of the executed command is forwarded as the conditional commands reult." +
				"<ul><li>while (value expression) do (command) endwhile</li></ul><br>" +
				"This implements a loop construct. As long as the evaluation of (value expression) returns (bool TRUE) (evaluation is only possible, if a database is currently open), the command is executed. If the expression value is (bool FALSE), the loop is terminated. " +
				"For any other result, the loop command terminates with an error, but all effects of commands executed within the loop remain valid. The result of the command is a list " +
				"(resultsequence ((result)[(result)*]))with each result of the executed command." +
				"<ul><li>{(command) [ |(command)]*}</li></ul><br>" +
				"This construct is the sequence command. It consists of an arbitrary number of commands separated by | (the pipe character) enclosed in round paranthesis. The commands are executed from left to right. When a command fails, the execution is stopped (the following " +
				"commands in this sequence are ignored). The result of the command is a list (resultsequence ((result)[(result)*])) with the results of the executed commands.<p> " +
				"The available commands allow for complete programs being written as command scripts. " +
				"Using operators from the FTextAlgebra and the ImExAlgebra, it is possible to write scripts for convenient data import or export. Here some example commands:" +
				"<ul><li>type myrel = rel(tuple([Name: string, Age: int]))</li>" +
				"<li>create x : int</li>" +
				"<li>update x := 5</li>" +
				"<li>let place = Hagen </li>" +
				"<li>let rel2 = [const rel(tuple([Name: string, Age: int]))</li>" +
				"<li>value ((Peter 17)(Klaus 31))]</li>" +
				"<li>derive rel2_Age = rel2 createbtree[Age]</li>" +
				"<li>query (x * 7) + 5</li>" +
				"<li>query rel2 feed filter[.Age > 20] project[Name] consume</li>" +
				"<li>delete type myrel</li>" +
				"<li>delete rel2</li>" +
				"<li>{open database opt | query deleteObject('twenty') | let twenty =intstream(1,20) namedtransformstream[No] consume | query twenty}{query deleteObject('mycounter') | " +
				"let mycounter = 0 | while mycounter <10 do { update mycounter := mycounter + 1 | query mycounter } endwhile }</li></ul><p>" +
				"<h3>Querying System Tables</h3><p>" +
				"There are some system tables which provide internal information. The most interesting are SEC2COMMANDS and SEC2OPERATORINFO. The first one contains a command history and the " +
				"second contains descriptions of the operators. Since they are relation objects they can by queried by the means of the relational algebra, e.g.query SEC2OPERATORINFO feed " +
				"filter[.Signature contains \"stream(tuple\"] consume displays all operators which process a stream of tuples.<p>" +
				"<h3>Transactions</h3><p>" +
				"Each of the basic commands of SECONDO is encapsulated into its own transaction and committed automatically. If you want to put several commands into one single transaction the following commands have to be used." +
				"<ul><li>begin transaction</li></ul><br>" +
				"Starts a new transaction; all commands until the next commit command are managed as one common unit of work." +
				"<ul><li>commit transaction</li></ul>" +
				"Commits a running transaction; all changes to the database will be effective." +
				"<ul><li>abort transaction</li></ul>" +
				"Aborts a running transaction; all changes to the database will be revoked."
                 );
		
		// Add the text to the dialog
	    secondoSyntax.setSize("490px", "490px");
	    scrollContent.setSize("510px", "510px");
	    scrollContent.add(secondoSyntax);
	    dialogContents.add(scrollContent);

	    // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	            helpDialogBox.hide();}
	        });
	    dialogContents.add(closeButton);	
	}

	/**Returns the dialog box containing the secondo info text
	 * 
	 * @return The dialog box containing the secondo info text
	 * */
	public DialogBox getHelpDialogBox() {
		return helpDialogBox;
	}
}
