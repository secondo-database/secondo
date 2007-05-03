package viewer.chess;

import javax.swing.*;
import javax.swing.table.*;
import java.util.*;
import java.awt.*;

/***
 * This class provides a view for the meta data of a chessgame object. Furthermore it displays relation values that are not chessgames, if there are any. It is a direct sub class of JPanel
 */
public class InformationPanel extends JPanel
{
	/**
	 * the textfields to show the standard meta data tags.
	 */
	private JTextField event, site, date, round, white, black, result;

	/**
	 * boxes to layout. main contains upper and lower, while upper contains the meta tags and lower contains the other relation values
	 */
        private Box upper,lower, main;	
	
	/**
	 * method to add a tag to the Box b. t is the JTextField which takes the value, title ist the key of the tag.
	 */
	private void addTagToBox(JTextField t, String title, Box b)
	{
		JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT, 0 ,0));
		JLabel l = new JLabel(title);	
		l.setFont(l.getFont().deriveFont(Font.PLAIN));
		p.add(l);
		b.add(p);
		p=new JPanel(new FlowLayout(FlowLayout.LEFT, 0 ,0));
		t.setMinimumSize(new Dimension(200,24));
		t.setPreferredSize(new Dimension(200, 24));
		p.add(t);
		b.add(p);
		b.add(Box.createVerticalStrut(4));
	}
	
	/**
	 * default constructor. After this an empty InformationPanel is shown, which contains the seven standard meta tags of a chessgame. 
	 */
	public InformationPanel()
	{
		upper = new Box(BoxLayout.Y_AXIS);
		event = new JTextField();
		site = new JTextField();
		date = new JTextField();
		round = new JTextField();
		white = new JTextField();
		black = new JTextField();
		result = new JTextField();
		addTagToBox(event, "event:", upper);
		addTagToBox(site, "site:", upper);
		addTagToBox(date, "date:", upper);
		addTagToBox(round, "round:", upper);
		addTagToBox(white, "white:", upper);
		addTagToBox(black, "black:", upper);
		addTagToBox(result, "result:", upper);
		main = new Box(BoxLayout.Y_AXIS);
		lower = new Box(BoxLayout.Y_AXIS);
		JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT, 0,0));
		JLabel lbl = new JLabel("meta tags:");
		p.add(lbl);
		main.add(p);
		main.add(Box.createVerticalStrut(4));
		main.add(upper);
		main.add(Box.createVerticalStrut(10));
		main.add(lower);
		this.add(main);
	}

	/**
	 * delivers the values of the standard meta tags in a HashMap.
	 */
	public HashMap getValues()
	{
		HashMap map = new HashMap();
		if (event != null && !event.getText().equals(""))
			map.put(ChessToolKit.EVENT_KEY,event.getText());
		if(site != null && ! site.getText().equals(""))
			map.put(ChessToolKit.SITE_KEY,site.getText());
		if(date != null && !date.getText().equals(""))
			map.put(ChessToolKit.DATE_KEY,date.getText());
		if(round != null && ! round.getText().equals(""))
			map.put(ChessToolKit.ROUND_KEY,round.getText());
		if(white != null && !white.getText().equals(""))
			map.put(ChessToolKit.WHITE_KEY,white.getText());
		if(black != null && !black.getText().equals(""))
			map.put(ChessToolKit.BLACK_KEY,black.getText());
		if(result != null && !result.getText().equals(""))
			map.put(ChessToolKit.RESULT_KEY, result.getText());
		return map;
	}
	
	/**
	 * method to clear the InformationPanel and restore the initial look with the seven empty TextFields representing the seven standard tags. 
	 */
	public void clearAll()
	{
		clearMetaTable();
		clearRelationValues();
		repaint();	
	}

	/**
	 * clears the upper part of the Information Panel. Empties all the standard textfields and removes all further tags from the InformtaionPanel
	 */
	public void clearMetaTable()
	{
		upper.removeAll(); //has to be done since there may be more tags than the usual seven
		addTagToBox(event, "event:", upper); //add seven standard tags again
		addTagToBox(site, "site:", upper);
		addTagToBox(date, "date:", upper);
		addTagToBox(round, "round:", upper);
		addTagToBox(white, "white:", upper);
		addTagToBox(black, "black:", upper);
		addTagToBox(result, "result:", upper);

		event.setText("");//empty the textfields
		site.setText("");
		date.setText("");
		round.setText("");
		white.setText("");
		black.setText("");
		result.setText("");
	}

	/**
	 * shows the meta data values denoted in the parameters. 
	 * for example the event-parameter is the value of the event tag, the site-parameter contains the value of the site tag and so on. The HashMap metaData contains further meta values, if there are any
	 */
	public void showMetaData(String event, String site, String date, String round, String white, String black, String result, HashMap metaData)
	{
		this.clearMetaTable(); //first clear everything
		this.event.setText(event); //set the values
		this.site.setText(site);
		this.date.setText(date);
		this.round.setText(round);
		this.white.setText(white);
		this.black.setText(black);
		this.result.setText(result);
		Object[] keys = metaData.keySet().toArray(); //get the keys of the further tags
		JTextField t;
		for(int i=0; i< keys.length;i++)
		{
			t = new JTextField();
			addTagToBox(t,(String)keys[i],upper); //add a label containing the key and a textfield containing value to the metadata
			t.setText((String)metaData.get(keys[i]));
			t.setToolTipText((String)metaData.get(keys[i]));
		}
		upper.repaint(); 
	}

	/**
	 * this method shows further relation values in the InformationPanel. The hashmap contains a pair of attribute name and attribute value of the relations attributes. 
	 */
	public void showRelationValues(HashMap relValues)
	{
		this.clearRelationValues(); //clear everything first
		JLabel lbl = new JLabel("other values in relation:");
		JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT, 0,0));
		p.add(lbl);
		lower.add(p);
		lower.add(Box.createVerticalStrut(4));
		Object[] keys = relValues.keySet().toArray(); //get the keys
		JTextField t;
		for(int i=0; i< keys.length;i++)
		{
			t = new JTextField();
			addTagToBox(t,(String)keys[i],lower); //add the key and the value to the box
			t.setText((String)relValues.get(keys[i]));
			t.setToolTipText((String)relValues.get(keys[i]));
		}
		lower.repaint();	
	}

	/**
	 * clears the lower part of the InformationPanel containing the relation values
	 */
	public void clearRelationValues()
	{
		lower.removeAll();
	}
}
