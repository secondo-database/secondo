package com.secondo.webgui.client.mainview;

import java.io.UnsupportedEncodingException;
import java.util.Properties;

import javax.mail.Message;
import javax.mail.MessagingException;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.AddressException;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DecoratorPanel;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.TextArea;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.FlexTable.FlexCellFormatter;

public class SupportDialog {

	
	private DialogBox supportDialogBox = new DialogBox();
	private FlowPanel supportDialogContents = new FlowPanel();
	private Button closeButton = new Button("Close");
	private Button sendButton = new Button("Send");
	
	private String nameLabel = "Name: ";
    private String emailLabel = "Email: ";
    private String messageLabel = "Message: ";    
    private TextBox name = new TextBox();
    private TextBox email = new TextBox();    
    private TextArea message = new TextArea();
    private FlexTable layout = new FlexTable();
    private FlexTable layoutWithButton = new FlexTable();
    private DecoratorPanel decPanel = new DecoratorPanel();
    
    
public SupportDialog(){
		
		supportDialogBox.setText("Send a message to support");
		supportDialogBox.setWidth("300px");
		

	    // Create a table to layout the content
	    supportDialogContents.getElement().getStyle().setPadding(5, Unit.PX);	    
	    supportDialogBox.setWidget(supportDialogContents);
	    
	    layout.setCellSpacing(6);
        FlexCellFormatter cellFormatter = layout.getFlexCellFormatter();

        // Add username and password fields       
        
        
        layout.setHTML(0, 0, nameLabel);
        cellFormatter.setHorizontalAlignment(0, 0, HasHorizontalAlignment.ALIGN_LEFT);
        layout.setWidget(1, 0, name);
        name.setWidth("100%");
        layout.setHTML(2, 0, emailLabel);
        cellFormatter.setHorizontalAlignment(2, 0, HasHorizontalAlignment.ALIGN_LEFT);
        layout.setWidget(3, 0, email);
        email.setWidth("100%");
        layout.setHTML(4, 0, messageLabel);
        cellFormatter.setHorizontalAlignment(4, 0, HasHorizontalAlignment.ALIGN_LEFT); 
        message.setHeight("150px");
        message.setWidth("100%");
        layout.setWidget(5, 0, message);
        layout.setWidth("97%");
        cellFormatter.setHorizontalAlignment(5, 0, HasHorizontalAlignment.ALIGN_RIGHT);
        
       
        // Wrap the content in a DecoratorPanel
        decPanel.setWidget(layout);
        decPanel.setWidth("100%");
                
        supportDialogContents.add(decPanel);     

        
	    // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  cleanSupportDialogBox();
	        	  supportDialogBox.hide();
	        	  }
	        });
	    closeButton.setStyleName("right-floated-button");
	    closeButton.getElement().setAttribute("margin-top", "3px");	    
	    layoutWithButton.setWidget(0, 1, closeButton);
	    
	 
	    sendButton.setStyleName("right-floated-button");
	    sendButton.getElement().setAttribute("margin-top", "3px");
	    layoutWithButton.setWidget(0, 0, sendButton);
	    layoutWithButton.setStyleName("floatRight");
	    
	    
	    supportDialogContents.add(layoutWithButton);	
	    
	}


public DialogBox getSupportDialogBox() {
	return supportDialogBox;
}

public void cleanSupportDialogBox(){
	name.setText("");
	email.setText("");
	message.setText("");
}


public Button getSendButton() {
	return sendButton;
}

	public String getMessage() {
		String message = "<h3>Support Request</h3> <p>"
				+ "<b>Sender's Name: </b>" + name.getText()
				+ "<p><b>Sender's Email: </b>" + email.getText()
				+"<p>" + this.message.getText();
		return message;
	}

}
