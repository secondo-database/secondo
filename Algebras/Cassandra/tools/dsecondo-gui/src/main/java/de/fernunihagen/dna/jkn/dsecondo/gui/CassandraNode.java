package de.fernunihagen.dna.jkn.dsecondo.gui;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.event.MouseEvent;
import java.awt.geom.Rectangle2D;

import javax.swing.JFrame;

public class CassandraNode {
	
	protected final static int SIZE = 40;

	protected Point position = null;
	protected CassandraNodeState state = null;
	protected String name;
	protected int tokenRangeCount = 0;
	protected Rectangle2D boundingBox;
	protected final Color defaultBackgroundColor;

	public CassandraNode(final String name) {
		super();
		this.name = name;
		
		// Determine default background color
		final JFrame frame = new JFrame();
		defaultBackgroundColor = frame.getContentPane().getBackground();
	}

	public Point getPosition() {
		return position;
	}

	public void setPosition(Point position) {
		this.position = position;

		boundingBox = new Rectangle2D.Double(
				getPosition().getX() - CassandraNode.SIZE / 2, 
				getPosition().getY() - CassandraNode.SIZE / 2, 
				SIZE, SIZE);
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

	/**
	 * Is the mouse event occoured over this node?
	 * @param event
	 * @return boolean
	 */
	public boolean isMouseOver(MouseEvent event) {
		return boundingBox.contains(event.getPoint());
	}

	/**
	 * Consistency check for this object
	 */
	protected void checkState() {
		if(position == null) {
			throw new IllegalArgumentException("Position is null");
		}
		
		if(state == null) {
			throw new IllegalArgumentException("State is null");
		}
	}
	
	/**
	 * Paint this node on the graphics context
	 * @param g
	 */
	protected void paintComponent(Graphics g) {
		checkState();
		
		final Graphics2D g2d = (Graphics2D) g;
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
			String value = Integer.toString(tokenRangeCount);
			Rectangle2D bounds = g2d.getFontMetrics().getStringBounds(value, g2d);
			int stringLen = (int) bounds.getWidth();
			int stringHight = ((int) bounds.getHeight()) - 2;
			g.drawString(value, position.x - stringLen/2, position.y + stringHight / 2);
		}

		final String description = name;
		
		// Calculate nodename bounding box
		Rectangle2D bounds = g2d.getFontMetrics().getStringBounds(description, g2d);
		int stringLen = (int) bounds.getWidth();
		int stringHeight = ((int) bounds.getHeight());
		
		// Draw nodename background
		g.setColor(defaultBackgroundColor);
        g.fillRect(position.x - stringLen / 2, position.y + SIZE/2 + stringHeight / 2 - 2, stringLen, stringHeight);
		
        // Draw nodename
        g.setColor(Color.BLACK);
		g.drawString(description, position.x - stringLen / 2, position.y + 2 + SIZE/2 + stringHeight);
		
		g.setColor(oldColor);
	}
}
