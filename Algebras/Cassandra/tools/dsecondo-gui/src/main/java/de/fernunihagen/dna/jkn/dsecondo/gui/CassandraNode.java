package de.fernunihagen.dna.jkn.dsecondo.gui;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.event.MouseEvent;
import java.awt.geom.Rectangle2D;

public class CassandraNode {
	
	protected final static int SIZE = 40;

	protected Point position = null;
	protected CassandraNodeState state = null;
	protected String name;
	protected int tokenRangeCount = 0;
	
	public CassandraNode(final String name) {
		super();
		this.name = name;
	}

	public Point getPosition() {
		return position;
	}

	public void setPosition(Point position) {
		this.position = position;
	}

	public CassandraNodeState getState() {
		return state;
	}

	public void setState(CassandraNodeState state) {
		this.state = state;
	}
	
	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}
	
	public int getTokenRangeCount() {
		return tokenRangeCount;
	}

	public void setTokenRangeCount(int tokenRangeCount) {
		this.tokenRangeCount = tokenRangeCount;
	}

	public boolean isMouseOver(MouseEvent event) {
		if(event.getX() > getPosition().getX() - CassandraNode.SIZE / 2 && 
		   event.getX() < getPosition().getX() + CassandraNode.SIZE / 2 &&
		   event.getY() > getPosition().getY() - CassandraNode.SIZE / 2 &&
		   event.getY() < getPosition().getY() + CassandraNode.SIZE / 2) {
			return true;
		}
		
		return false;
	}

	protected void checkState() {
		if(position == null) {
			throw new IllegalArgumentException("Position is null");
		}
		
		if(state == null) {
			throw new IllegalArgumentException("State is null");
		}
	}
	
	protected void paintComponent(Graphics g) {
		checkState();
		
		final Color oldColor = g.getColor();
		
		switch(state) {
		case ACTIVE:
			g.setColor(Color.GREEN);
			break;
		case MISSING:
			g.setColor(Color.YELLOW);
			break;
		case INACTIVE:
			g.setColor(Color.RED);
			break;
		default:
			break;
		}
		
		g.fillRoundRect(position.x - SIZE/2, position.y - SIZE/2, SIZE, SIZE, SIZE, SIZE);
		g.setColor(Color.BLACK);
		g.drawOval(position.x - SIZE/2, position.y - SIZE/2, SIZE, SIZE);
		
		if(state == CassandraNodeState.ACTIVE || state == CassandraNodeState.INACTIVE) {
			Graphics2D g2d = (Graphics2D) g;
			String value = Integer.toString(tokenRangeCount);
			Rectangle2D bounds = g2d.getFontMetrics().getStringBounds(value, g2d);
			int stringLen = (int) bounds.getWidth();
			int stringHight = ((int) bounds.getHeight()) - 2;
			
			g.drawString(value, position.x - stringLen/2, position.y + stringHight / 2);
		}
		
		g.setColor(oldColor);
	}
	
	
}
