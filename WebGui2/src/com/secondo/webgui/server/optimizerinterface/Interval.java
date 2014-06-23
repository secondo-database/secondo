package com.secondo.webgui.server.optimizerinterface;

public class Interval {
	
    private int min=0;
    private int max=0;
	
	public Interval(int x, int y){
         min=x;
	     max = y;
     }

	public int getMin() {
		return min;
	}

	public void setMin(int min) {
		this.min = min;
	}

	public int getMax() {
		return max;
	}

	public void setMax(int max) {
		this.max = max;
	}

}
