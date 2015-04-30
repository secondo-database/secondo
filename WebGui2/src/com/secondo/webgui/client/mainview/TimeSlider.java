package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.TextAlign;
import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.user.client.ui.DecoratorPanel;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Label;

/**
 * This class represents a timeslider which displays a slider animation for a
 * range of given dates
 * 
 * @author Irina Russkaya
 * 
 **/
public class TimeSlider {

	/** The main panel of the timeslider */
	private FlowPanel mainPanel = new FlowPanel();

	/** The decorator panel to display the slider in a nice outfit */
	private DecoratorPanel sliderDecPanel = new DecoratorPanel();

	/** The panel for the slider */
	private HorizontalPanel sliderPanel = new HorizontalPanel();
	private FlowPanel leftSlider = new FlowPanel();
	private FlowPanel rightSlider = new FlowPanel();
	private FlowPanel sliderHandle = new FlowPanel();

	/** The panel for the ticks of the slider */
	private HorizontalPanel sliderTicks = new HorizontalPanel();
	private Label tick1 = new Label("min");
	private Label tick2 = new Label("max");

	/** The number of values for the timeslider */
	private int numberOfTimeValues = 0;

	public TimeSlider() {

		sliderDecPanel.setSize("179px", "10px");

		leftSlider.setSize("5px", "10px");
		leftSlider.getElement().getStyle().setBackgroundColor("orange");
		rightSlider.setSize("169px", "10px");
		rightSlider.getElement().getStyle().setBackgroundColor("#F1F1F1");
		sliderHandle.setSize("5px", "10px");
		sliderHandle.getElement().getStyle().setBackgroundColor("black");
		sliderHandle.setTitle("No Animation started");
		sliderPanel.add(leftSlider);
		sliderPanel.add(sliderHandle);
		sliderPanel.add(rightSlider);
		sliderDecPanel.add(sliderPanel);

		sliderTicks.setSize("179px", "10px");
		tick1.getElement().getStyle().setPaddingLeft(10, Unit.PX);
		tick1.setWidth("80px");
		tick1.getElement().getStyle().setFontSize(10, Unit.PX);
		sliderTicks.add(tick1);

		tick2.getElement().getStyle().setPaddingLeft(10, Unit.PX);
		tick2.setWidth("80px");
		tick2.getElement().getStyle().setTextAlign(TextAlign.RIGHT);
		tick2.getElement().getStyle().setFontSize(10, Unit.PX);

		sliderTicks.add(tick2);

		mainPanel.add(sliderDecPanel);
		mainPanel.add(sliderTicks);
	}

	/**
	 * Sets the value of the ticks to the given string
	 * 
	 * @paran t1 The new value for the minimum tick
	 * @param t2
	 *            The new value for the maximum tick
	 * */
	public void setTicks(String t1, String t2) {
		this.tick1.setText(t1);
		this.tick2.setText(t2);
	}

	/**
	 * Sets the number of time values to the given integer number
	 * 
	 * @param number
	 *            The new number of time values
	 * */
	public void setNumberOfTimeValues(int number) {
		this.numberOfTimeValues = number;
	}

	/**
	 * Moves the slider to the given counter position
	 * 
	 * @param counter
	 *            The position of the slider
	 * @param currentTime
	 *            The current time of the slider position
	 * */
	public void moveSlider(int counter, String currentTime) {

		Double maxTime = new Double(numberOfTimeValues);

		double position = (179 / maxTime);
		position = position * counter;
		int handlePosition = new Double(Math.round(position)).intValue();

		if (handlePosition < 174) {
			leftSlider.setWidth(handlePosition + "px");
			rightSlider.setWidth(179 - handlePosition - 5 + "px");
		}
		sliderHandle.setTitle(currentTime);
	}

	/** Resets the slider to the default position */
	public void resetSlider() {
		leftSlider.setWidth("5px");
		rightSlider.setWidth("169px");
		sliderHandle.setTitle("No Animation started");
		this.tick1.setText("min");
		this.tick2.setText("max");
	}

	/**
	 * Returns the main panel of the slider
	 * 
	 * @return The main panel of the slider
	 * */
	public FlowPanel getMainPanel() {
		return mainPanel;
	}
}
