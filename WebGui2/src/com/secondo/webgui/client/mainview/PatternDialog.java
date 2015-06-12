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
 * This class represents a dialog containing the help information about patterns
 * 
 * @author Irina Russkaya
 *
 */
public class PatternDialog {

	private DialogBox helpDialogBox = new DialogBox();
	private FlowPanel dialogContents = new FlowPanel();
	private ScrollPanel scrollContent = new ScrollPanel();
	private HTML patternInfo;
	private Button closeButton = new Button("Close");

	public PatternDialog() {

		helpDialogBox.setText("Information about Pattern Matching");

		// Create a table to layout the content
		dialogContents.getElement().getStyle().setPadding(5, Unit.PX);
		dialogContents.setSize("410px", "410px");
		helpDialogBox.setWidget(dialogContents);

		patternInfo = new HTML(				
						"In Pattern queries you can define: <p>"
						+ "<b>unit pattern</b> which has one of the forms ( t l ), (_ l ), ( t _), or (), where <b>t</b> is a time interval, "
						+ "<b>l</b> is a label, and <b>_</b> is a wildcard symbol.<p>"
						+ "For time intervals you can use one of the following time symbols:"
						+"<ul>"
						+"<li>a year, month or day, written as 2010, 2010-07, 2010-07-05</li>"
						+"<li>an hour, minute or second on a particular day, e.g. 2010-07-05-14:30</li>"
						+"<li>a range of dates, e.g. 2010~2011, 2010-07~2011-03</li>"
						+"<li>a range of times, e.g. 2010-07-05~14:30∼2010-07-09-14</li>"
						+"<li>a halfopen range, e.g. 2005-05~ or ~2010-12-06</li>"
						+"<li>a day of the week, i.e. one of {sunday, monday, tuesday, ..., saturday}</li>"
						+"<li>a month of the year, i.e. one of {january, ..., december}</li>"
						+"<li>a time of day such as {morning, afternoon, evening, night}</li>"
						+"<li>a time of the day given by a time interval such as 14:30~16, 17~</li>"
						+"</ul>"
						+ "<b>sequence pattern</b> which has one of the forms *, +, [p], [p1 | p2], [p]+, [p]*, or [p]?, where p, p1, p2 are simple patterns <p>"
						+ "<b>variable</b> to bind a specified pattern element <p>"
						+ "<b>condition</b> which has a form <variable.attribute> with attributes: label(s), time, start, end, leftclosed, rightclosed"
						+ "<h3>Sample Pattern</h3><p>"
						+ "A (_\"SouthEast\") + B (_ \"SouthWest\")<br>"
						+ "// (B.end - A.start) < (20 * minute)  <p>"
						+ "<p><a href=\"http://dna.fernuni-hagen.de/papers/SymbolicTrajectories369.pdf\">More info</a></p>");

		// Add the text to the dialog
		scrollContent.add(patternInfo);
		scrollContent.setSize("390px", "375px");
		dialogContents.add(scrollContent);

		// Add a close button at the bottom of the dialog
		closeButton.addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				helpDialogBox.hide();
			}
		});
		closeButton.setStyleName("right-floated-button");
		dialogContents.add(closeButton);
	}

	/**
	 * Returns the dialog box containing the optimizer info text
	 * 
	 * @return The dialog box containing the optimizer info text
	 * */
	public DialogBox getHelpDialogBox() {
		return helpDialogBox;
	}

}
