package viewer.chess;
import sj.lang.*;
import java.lang.*;
import java.lang.reflect.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.Dimension;
import java.awt.event.*;
import java.awt.*;
import java.util.*;

/**
 * This class provides the functionality to create a secondo or optimizer query from a PositionData object and the meta data of a chessgame. As a subclass of JPanel 
 * also it provides the view for editing the query. 
 * Furthermore this class sends small queries to secondo to find out which relations contain a chessgame. Since this functioniality needs another viewer method invoking and object creation  are done via the java.lang.reflect package. Thus even if the other viewer is not available the QueryBuilder class would still be compilable without errors
 */
public class QueryBuilder extends JPanel implements ChangeListener
{
	/**
	 * class name needed to get a secondo interface
	 */
	private static final String SECONDO_INTERFACE_PROVIDER_CLASS = "gui.MainWindow";

	/**
	 * method name  to get the secondo interface
	 */
	private static final String GET_SECONDOINTERFACE_METHOD = "getUpdateInterface";

	/**
	 * class name of the secondo interface
	 */
	private static final String SECONDO_INTERFACE_CLASS = "sj.lang.UpdateInterface";

	/**
	 * method name of the secondo interface class to submit a query to it
	 */
	private static final String SECONDO_QUERY_METHOD = "secondo";

	/**
	 * names of the parameter classes needed to invoke the secondo query method
	 */
	private static final String[] PARAMETER_CLASSES = {"java.lang.String","sj.lang.ListExpr","sj.lang.IntByReference", "sj.lang.IntByReference", "java.lang.StringBuffer"};

	/**
	 * class of the secondo interface
	 */
	private Class secondoInterfaceClass;

	/**
	 * the secondo interface as an Object
	 */
	private Object secondoInterface;

	/**
	 * the method to execute the query
	 */
	private Method commandExecutionMethod;

	/**
	 * parameter classes for the query method
	 */
	private Class[] parameters;

	/**
	 * text fields to enter the relations and attributes if it is not possible to get them from secondo
	 */
	private JTextField relationsText, attributesText;

	/**
	 * the textfield where the query is displayed
	 */
	private JTextArea queryText ;

	/**
	 * two lists containing the chessObjects and the indexes in the current database if queries to secondo can be done
	 */
	private ArrayList chessObjects, indices;

	/**
	 * list containing possible relations and chessgame attributes if these can be obtained from secondo
	 */
	private JList relationsList, attributesList;

	/**
	 * layout boxes
	 */
	private Box withSecInterface, withoutSecInterface, upper, lower, content, upper2;

	/**
	 * true if queries to secondo can be send and return other values than empty lists, false otherwise
	 */
	private boolean hasSecondoInterface;

	/**
	 * selection
	 */
	private ButtonGroup selection;

	/**
	 * shows the two kinds of possible queries: secondo and optimizer
	 */
	private JTabbedPane tabPane;

	/**
	 * layout box
	 */
	private Box positionOptions;

	/**
	 * checkboxes giving the possibilty to create a query on the meta data, the position or both
	 */
	private JCheckBox metaSelection, positionSelection;

	/**
	 * check for only similiar positions
	 */
	private JRadioButton similiar;

	/**
	 * check for exactly matching positions
	 */
	private JRadioButton equal;

	/**
	 * textfields for entering specified position numbers for queries
	 */ 
	private JTextField from;
	private JTextField to;

	/**
	 * checkbox to decide whether the query should look for positions after a specified move number
	 */
	private JCheckBox afterMove;

	/**
	 * PositionDeliverer to get the position used for the query from
	 */
	private PositionDeliverer positionDeliverer;

	/**
	 * MetaDataDeliverer to get the meta data used for querying from
	 */
	private MetaDataDeliverer metaDeliverer;
	
	/**
	 * constructor which needs a PositionDeliverer and a MetaDataDeliverer to get the position and the meta data. 
	 */
	public QueryBuilder(PositionDeliverer positionDeliverer, MetaDataDeliverer metaDeliverer)
	{
		try //try if it is possible to get a secondo interface
		{
			Class secondoProvider = Class.forName(SECONDO_INTERFACE_PROVIDER_CLASS);
      Class[] paramTypes = new Class[0];
			Method secondoInterfaceMethod = secondoProvider.getMethod(GET_SECONDOINTERFACE_METHOD, paramTypes);
      Object[] params = new Object[0];
			secondoInterface = secondoInterfaceMethod.invoke(null, params);
			secondoInterfaceClass = Class.forName(SECONDO_INTERFACE_CLASS);
			parameters = new Class[PARAMETER_CLASSES.length];
			for (int i=0;i< PARAMETER_CLASSES.length;i++)
			{ 
				parameters[i] = Class.forName(PARAMETER_CLASSES[i]);
			}
			commandExecutionMethod = secondoInterfaceClass.getMethod(SECONDO_QUERY_METHOD, parameters);
		}
		catch (Exception e)
		{}
		finally
		{
			this.metaDeliverer = metaDeliverer;
			this.positionDeliverer = positionDeliverer;
			this.initView();		
		}
	}

	/**
	 * this method tries to obtain the objects in the current database containing a chessgame. Returns true if successful false otherwise. optimizerUsed indicates if the optimizer should be used to create a query. If so there are special filters needed to get only the StandardChessRel relations.
	 */
	private boolean  getChessObjects(boolean optimizerUsed)
	{
		try
		{
			ListExpr result = tryExecuteQuery("list objects", new ListExpr(), new StringBuffer());
			if(result == null || result.isEmpty() ) //if there is no restul returns false
				return false;
			ArrayList objects = new ArrayList();
			this.getObjects(objects, result);
			if(objects.isEmpty())
				return false;
			indices = filterNonIndexes(objects); //get the filters
			objects = this.filterNonRelations(objects); //filter objects not being a relation
			objects = this.filterNonChessGames(objects); //filter relations not containing a chessgame
			if(optimizerUsed) 
			{
				objects = this.filterNonStandardChessRelations(objects); //filter relations that are not standard chess relations (optimizer query)
			}
			chessObjects = objects;
			if(chessObjects.isEmpty()) //if there are no more objects left
				return false; //return false
			return true; //true otherwise
		}
		catch(Exception e)
		{
			return false;
		}
		
	}

	/**
	 * this method is called to show the queryBuilderInterface.
	 */
	public void showQueryBuilder()
	{
		Box showBox = tabPane.getSelectedIndex()==0?upper:upper2; 
		Box showNoBox = tabPane.getSelectedIndex()==0?upper2:upper;
		showNoBox.removeAll(); //remove everything from the tab not seen
		showNoBox.validate();
		try
		{
			if(!this.getChessObjects(tabPane.getSelectedIndex()!=0)) //try to get the chess objects if fails then an Exception is thrown
				throw new Exception();
			relationsList.setListData(getChessRelationNames()); //if fails not the lists are put on the upper part of the box
			relationsList.setSelectedIndex(0);
			showBox.removeAll();
			showBox.add(withSecInterface);
			showBox.validate();
			hasSecondoInterface = true; 
		}
		catch (Exception e)
		{
			showBox.removeAll(); //if it is not possible to obtain objects from secondo the plainer view is shown 
			showBox.add(withoutSecInterface);
			showBox.validate();
			hasSecondoInterface = false;
		}
	}

	/**
	 * returns the names of the relations containing a chessgame
	 */
	private String[] getChessRelationNames() throws Exception
	{
		if (chessObjects != null) 
		{
			Object[] objects = chessObjects.toArray();
			String[] names = new String[objects.length];
			for (int i=0; i< objects.length;i++)
			{
				names[i] = ((ListExpr)objects[i]).second().writeListExprToString().trim();	
			}
			return names;
		}
		return null;
	}

	/**
	 * returns the names of the indexes for the 'relation relationsName'. The indexes' names have to be according to the name convention of indexes. E.g an index for 'attribute' of the relation 'RELATION' would have to be calles 'rELATION_attribute' with a leading lower case character and a underscore between relation name and attribute. 
	 */
	private String[] getRelationsIndexNames(String relationsName) throws Exception
	{
		ArrayList out = new ArrayList();
		String[] indexes = getIndicesNames();
		boolean equalAttributes = true;
		String name = relationsName;
		char[] nameChar = name.toCharArray();
		nameChar[0] = Character.toLowerCase(nameChar[0]);
		name = new String(nameChar)+"_";
		for (int i= 0;i<indexes.length;i++)
		{
			if(indexes[i].startsWith(name)) //if it starts with the correct name
			{
				equalAttributes = true;
				String[] ind_attributes = getAttributeNames(indexes[i],indices);
				String[] rel_attributes = getAttributeNames(relationsName, chessObjects);
				for(int j=0;j<ind_attributes.length;j++) //check if the attributes are equal
				{
					equalAttributes = equalAttributes &&(ind_attributes[j].equals(rel_attributes[j]));
				}
				if(equalAttributes)
					out.add(indexes[i]);
			}
		}
		String[] names = new String[out.size()]; //create the array
		for(int i=0;i<names.length;i++)
			names[i]=(String)out.get(i);
		return names;
	}


	/**
	 * returns the names of all standardChessRealtions in this database
	 */
	private String[] getStandardChessRelationNames() throws Exception
	{
		String[] relations = getChessRelationNames();
		boolean event,site,date,result,white,black,round;
		ArrayList out = new ArrayList();
		for (int i=0; i< relations.length;i++)
		{
			//check if relation really has indexes with the right names
			site = false;
			date = false;
			result = false;
			white = false;
			black = false;
			round = false;
			event = false;
			String name = relations[i];
			char[] nameChar = name.toCharArray();
			nameChar[0] = Character.toLowerCase(nameChar[0]);
			name = new String(nameChar);
			String[] indexes = getRelationsIndexNames(relations[i]);
			for(int j=0;j<indexes.length;j++)
			{
				if(indexes[j].equals(name+"_event"))
					event = true;
				if(indexes[j].equals(name+"_site"))
					site = true;
				if(indexes[j].equals(name+"_date"))
					date = true;
				if(indexes[j].equals(name+"_result"))
					result = true;
				if(indexes[j].equals(name+"_white"))
					white = true;
				if(indexes[j].equals(name+"_black"))
					black = true;
				if(indexes[j].equals(name+"_round"))
					round = true;
			}
			if(event&&site&&date&&result&&white&&black&&round)
				out.add(relations[i]);
		}
		String[] names = new String[out.size()];
		for(int i=0;i<names.length;i++)
			names[i]=(String)out.get(i);
		return names;
	}
	
	/**
	 * returns the names of all indexes in the database
	 */
	private String[] getIndicesNames() throws Exception
	{
		if (indices != null)
		{
			Object[] objects = indices.toArray();
			String[] names = new String[objects.length];
			for (int i=0; i< objects.length;i++)
			{
				names[i] = ((ListExpr)objects[i]).second().writeListExprToString().trim();	
			}
			return names;
		}
		return null;
	}
	
	/**
	 *returns the names of the chessgame - attributes in the relation 'relationName'
	 */
	private String[] getChessAttributeNames(String relationName) throws Exception
	{
		if(chessObjects != null)
		{
			String[] allNames = this.getChessRelationNames();
			int i=0;
			while(i< allNames.length && !allNames[i].equals(relationName))
			{
				i++;
			}
			ArrayList attributeNames = new ArrayList();
			ListExpr relationList = (ListExpr)chessObjects.toArray()[i];
			ListExpr attributes = relationList.fourth().first().second().second();
			while(!attributes.isEmpty())
			{
				if(attributes.first().second().writeListExprToString().trim().equals("chessgame"))
					attributeNames.add(attributes.first().first().writeListExprToString().trim());
				attributes = attributes.rest();
			}
			Object[] nameList = attributeNames.toArray();
			String[] names = new String[nameList.length];
			for(int j=0; j< names.length;j++)
				names[j] =( String)nameList[j];
			return names;
		}
		return null;
	}

	/**
	 * returns the names of the attributes of relationName.
	 */
	private String[] getAttributeNames(String relationName, ArrayList chessObjects) throws Exception
	{
		if(chessObjects != null)
		{
			String[] allNames = this.getChessRelationNames();
			int i=0;
			while(i< allNames.length && !allNames[i].equals(relationName))
			{
				i++;
			} // find relationName in all of the other relatiions
			ArrayList attributeNames = new ArrayList();
			ListExpr relationList = (ListExpr)chessObjects.toArray()[i];
			ListExpr attributes = relationList.fourth().first().second().second();
			while(!attributes.isEmpty())
			{
				attributeNames.add(attributes.first().first().writeListExprToString().trim());
				attributes = attributes.rest();
			}
			Object[] nameList = attributeNames.toArray();
			String[] names = new String[nameList.length];
			for(int j=0; j< names.length;j++)
				names[j] =( String)nameList[j];
			return names;
		}
		return null;
	}
	
	/**
	 * filters the relations which do not contain a chessgame from objects
	 */
	private ArrayList filterNonChessGames(ArrayList objects) throws Exception
	{
		ArrayList out = new ArrayList();
		Object[] theObjects = objects.toArray();
		ListExpr current, attributes, currentAttribute;
		boolean chessgame;
		for(int i=0;i<theObjects.length;i++)
		{
			current = (ListExpr) theObjects[i];
			chessgame = false;
			attributes = current.fourth().first().second().second();
			while(!attributes.isEmpty())
			{
				currentAttribute = attributes.first();
				if (currentAttribute.second().writeListExprToString().trim().equals("chessgame"))
					chessgame = true;
				attributes = attributes.rest();
			}
			if(chessgame)
			{
				out.add(current);
			}
		}
		return out;
	}

	/**
	 * filters the relations which are no Standard chess relations from objects. A standard chess relation must at least contain the following attributes:
	 * one string attribute for the event tag value
	 *one string attribute for the site tag value
	 *one string attribute for the date tag value
	 *one string attribute for the result tag value
	 *one string attribute for the round tag value
	 *one string attribute for the white tag value
	 *one string attribute for the black tag value
	 * Furthermore it hast to contain a chessgame
	 */ 
	private ArrayList filterNonStandardChessRelations(ArrayList objects) throws Exception
	{
		ArrayList out = new ArrayList();
		Object[] theObjects = objects.toArray();
		ListExpr current, attributes, currentAttribute;
		boolean event, site, date, result, white, black, round;
		for(int i=0;i<theObjects.length;i++)
		{
			current = (ListExpr) theObjects[i];
			event = false;
			site = false;
			date = false;
			result = false;
			white = false;
			black = false;
			round = false;
			attributes = current.fourth().first().second().second();
			while(!attributes.isEmpty())
			{
				currentAttribute = attributes.first();
				String currentAttributeName = currentAttribute.first().writeListExprToString().trim().toLowerCase();
				if(currentAttribute.second().writeListExprToString().trim().toLowerCase().equals("string"))
				{	
					if (currentAttributeName.equals(ChessToolKit.EVENT_KEY))
						event = true;
					if (currentAttributeName.equals(ChessToolKit.SITE_KEY))
						site = true;
					if (currentAttributeName.equals(ChessToolKit.DATE_KEY))
						date = true;
					if (currentAttributeName.equals(ChessToolKit.RESULT_KEY))
						result = true;
					if (currentAttributeName.equals(ChessToolKit.WHITE_KEY))
						white = true;
					if (currentAttributeName.equals(ChessToolKit.BLACK_KEY))
						black = true;
					if (currentAttributeName.equals(ChessToolKit.ROUND_KEY))
						round = true;
				}
				attributes = attributes.rest();
			}
			if(event && site && date && result && white && black && round)
			{
				out.add(current);
			}
		}
		return out;
	}

	/**
	 * this method fills the objects ArrayList with all the objects contained in the ListExpr
	 */
	private void getObjects(ArrayList objects, ListExpr list) throws Exception
	{
		if(!list.isEmpty() && list.listLength()>=2)
			list = list.second();
		if(list != null && !list.isEmpty()&&list.listLength()>=2)
			list = list.second();
	        if(list != null && !list.isEmpty()&&list.listLength()>=2)
			list = list.rest(); 
		while(list != null && !list.isEmpty())
		{
			objects.add(list.first());
			list = list.rest();
		}
	}

	/**
	 * this method returns an arraylist that contains those values of 'objects' which are relations 
	 */
	private ArrayList filterNonRelations(ArrayList objects) throws Exception
	{
		ArrayList out = new ArrayList();
		Object[] theObjects = objects.toArray();
		ListExpr current, relation;
		boolean remove;
		for (int i=0; i<theObjects.length;i++)
		{
			remove = false;
			current = (ListExpr)theObjects[i];
			if (current.listLength() != 4 )
			{
				remove = true;
			}
			else
			{
				relation = current.fourth().first();
				if(relation.listLength() != 2 ||
						!relation.first().isAtom() ||
						relation.first().atomType() != ListExpr.SYMBOL_ATOM ||
						!(relation.first().symbolValue().equals("rel")) ||
						relation.second().listLength() != 2 ||
						!relation.second().first().isAtom() ||
						relation.second().first().atomType() != ListExpr.SYMBOL_ATOM ||
						!(relation.second().first().symbolValue().equals("tuple")))
					remove = true;
				
			}
			if (!remove)
				out.add(current);
		}
		return out;
	}

	/**
	 * This method returns an ArrayList which contains the indexes as ListExpr contained in the objects ArrayList
	 */
	private ArrayList filterNonIndexes(ArrayList objects) throws Exception
	{
		ArrayList out = new ArrayList();
		Object[] theObjects = objects.toArray();
		ListExpr current, relation;
		boolean remove;
		for (int i=0; i<theObjects.length;i++)
		{
			remove = false;
			current = (ListExpr)theObjects[i];
			if (current.listLength() != 4 )
			{
				remove = true;
			}
			else
			{
				relation = current.fourth().first();
				if(relation.listLength() != 2 ||
						!relation.first().isAtom() ||
						relation.first().atomType() != ListExpr.SYMBOL_ATOM ||
						!(relation.first().symbolValue().equals("btree")) ||
						relation.second().listLength() != 2 ||
						!relation.second().first().isAtom() ||
						relation.second().first().atomType() != ListExpr.SYMBOL_ATOM ||
						!(relation.second().first().symbolValue().equals("tuple")))
					remove = true;
				
			}
			if (!remove)
				out.add(current);
		}
		return out;
	}

	/**
	 * This method initializes the upper box of the view containing the selection of the relation name and attribute name. In this case this is done via a list since the possible relations cann be obtained from secondo
	 */
	private void initWithSecInterface()
	{
		withSecInterface = new Box(BoxLayout.Y_AXIS);
		
		JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0));
		JLabel lbl = new JLabel("choose a chess-relation:");
		lbl.setFont(lbl.getFont().deriveFont(Font.PLAIN));
		p.add(lbl);
		
		JPanel p2 = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0)); //create the listst
		attributesList = new JList();
		attributesList.setFixedCellWidth(200);
		attributesList.setVisibleRowCount(3);
		attributesList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		attributesList.setFixedCellHeight(24);
		JScrollPane scroller1 = new JScrollPane(attributesList);
		
		relationsList = new JList();
		relationsList.setFixedCellWidth(200);
		relationsList.setVisibleRowCount(3);
		relationsList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		relationsList.setFixedCellHeight(24);
		relationsList.addListSelectionListener(new ListSelectionListener()
			{
				public void valueChanged(ListSelectionEvent e)
				{
					String relationName = null;
					try
					{
						relationName = (String)relationsList.getSelectedValue();
						if(relationName != null)
						{
							try
							{
								attributesList.setListData(getChessAttributeNames(relationName));
								attributesList.setSelectedIndex(0);
							}
							catch(Exception ex)
							{
							}
						}
					}
					catch(Exception ex)
					{
						attributesList.setListData(new Vector());
					}
				}
				
			});
		JScrollPane scroller2 = new JScrollPane(relationsList);
		p2.add(scroller2);

		withSecInterface.add(p);
		withSecInterface.add(p2);
		withSecInterface.add(Box.createVerticalStrut(4));
		p = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0));
		lbl = new JLabel("choose a chess-attribute:");
		lbl.setFont(lbl.getFont().deriveFont(Font.PLAIN));
		p.add(lbl);
		p2 = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0));
		p2.add(scroller1);
		withSecInterface.add(p);
		withSecInterface.add(p2);
	}

	/**
	 * Initializes the upper part of the panel if there is no secondo interface to obtain relation names. This means the user has to enter the relation name and the attribute containing a chessgame
	 */
	private void initWithoutSecInterface()
	{
		withoutSecInterface = new Box(BoxLayout.Y_AXIS);
		JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0));
		JLabel lbl = new JLabel("enter relation-name:");
		lbl.setFont(lbl.getFont().deriveFont(Font.PLAIN));
		p.add(lbl);
		
		relationsText = new JTextField();
		relationsText.setMinimumSize(new Dimension(200,24));
		relationsText.setPreferredSize(new Dimension(200,24));
		
		withoutSecInterface.add(p);
		withoutSecInterface.add(relationsText);

		p = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0));
		lbl = new JLabel("enter chessgame attribute:");
		lbl.setFont(lbl.getFont().deriveFont(Font.PLAIN));
		p.add(lbl);
		withoutSecInterface.add(p);
		
		attributesText = new JTextField();
		attributesText.setMinimumSize(new Dimension(200,24));
		attributesText.setPreferredSize(new Dimension(200, 24));
		
		withoutSecInterface.add(attributesText);
	}

	/**
	 * This method initializes a panel containing further options what kind of query as far as position querying goes should be created
	 */
	private void initPositionOptions()
	{
		positionOptions = new Box(BoxLayout.Y_AXIS);
		selection = new ButtonGroup();
		JLabel lbl = new JLabel("chosse matching type:");
		lbl.setFont(lbl.getFont().deriveFont(Font.PLAIN));
		JPanel p = new JPanel(new GridLayout(3,1));
		p.add(lbl);
		similiar = new JRadioButton("similiar positions");
		equal = new JRadioButton("exactly matching positions");
		selection.add(similiar);
		selection.add(equal);
		selection.setSelected(similiar.getModel(),true);
		p.add(similiar);
		p.add(equal);
		positionOptions.add(p);

		from = new JTextField(3);
		to = new JTextField(3);
		from.setEnabled(false);
		to.setEnabled(false);
		afterMove = new JCheckBox("only positions from ");
		afterMove.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						if(!afterMove.isSelected())
						{
							from.setText("");
							to.setText("");
						}
						from.setEnabled(afterMove.isSelected());
						to.setEnabled(afterMove.isSelected());
						
					}
				});
		lbl = new JLabel(" to ");
		p = new JPanel(new FlowLayout(FlowLayout.LEFT, 0,0));
		p.add(afterMove);
		JPanel p2 = new JPanel(new FlowLayout(FlowLayout.RIGHT, 0,0));
		p2.add(from);
		p2.add(lbl);
		p2.add(to);
		positionOptions.add(p);
		positionOptions.add(p2);
	
	}

	/**
	 * Init the box contained in the 'secondo'- tab
	 */
	private void initSecondoView(Box view)
	{
		upper = new Box(BoxLayout.Y_AXIS);
		JPanel p = new JPanel(new GridLayout(2,1));
		initPositionOptions();
		final Box position_options_holder = new Box(BoxLayout.Y_AXIS);
		positionSelection = new JCheckBox("look up position");
		positionSelection.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if(((JCheckBox)e.getSource()).isSelected())
						position_options_holder.add(positionOptions);
					else
						position_options_holder.removeAll();
					validate();
				}
			});
		
		metaSelection = new JCheckBox("look up meta data");
	        p.add(metaSelection);
		p.add(positionSelection);
			
		view.add(upper);

		view.add(p);
		view.add(position_options_holder);

	}

	/**
	 * Initialize the box contained in the 'optimizer' - tab
	 */
	private void initOptimizerView(Box view)
	{
		upper2 = new Box(BoxLayout.Y_AXIS);
		JCheckBox metaSel = new JCheckBox("look up meta data");
		JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0));
		p.add(metaSel);
		metaSel.setSelected(true);
		metaSel.setEnabled(false);
		view.add(upper2);
		view.add(p);
	}

	/**
	 * Initialize the lower part of the query builder containing the create query button, the textarea for the query and a reset button
	 */
	private void initLower()
	{
		JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT, 0,0));
		JButton create = new JButton("create query");
		create.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						createQueryButtonPressed();				
					}
				});
		create.setPreferredSize(new Dimension(200, 24));
		lower.add(Box.createVerticalStrut(2));
		p.add(create);
		lower.add(p);
		lower.add(Box.createVerticalStrut(10));
		p = new JPanel(new FlowLayout(FlowLayout.LEFT,0,0));
		JLabel lbl = new JLabel("query:");
		lbl.setFont(lbl.getFont().deriveFont(Font.PLAIN));
		p.add(lbl);
		queryText = new JTextArea(10,18);
		queryText.setLineWrap(true);
		queryText.setWrapStyleWord(true);
		queryText.setAutoscrolls(true);
		JScrollPane scroller = new JScrollPane(queryText);
		scroller.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
		scroller.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
		JButton reset = new JButton("reset");
		reset.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						resetView();
					}
				});
		lower.add(p);
		lower.add(scroller);
		lower.add(Box.createVerticalStrut(4));
		Box b2 = new Box(BoxLayout.X_AXIS);
		p = new JPanel(new FlowLayout(FlowLayout.RIGHT,0,0));
		b2.add(reset);
		p.add(b2);
		lower.add(p);
	}

	/**
	 * this is what happens when the create query button is pressed
	 */
	private void createQueryButtonPressed()
	{
		if(tabPane.getSelectedIndex() == 0) // if a secondo query is requested
		{
			if(afterMove.isSelected()) //if range was specified for the position index
				createSecondoQuery(selection.getSelection().equals(equal.getModel()),positionSelection.isSelected(), metaSelection.isSelected(), from.getText(), to.getText());
			else 
				createSecondoQuery(selection.getSelection().equals(equal.getModel()),positionSelection.isSelected(), metaSelection.isSelected(), null, null);
		}
		else
		{
			createOptimizerQuery(); //optimizer query was requested
		}
	}

	/**
	 * this method resets the view of the QueryBuilder to its initial form
	 */
	private void resetView()
	{
		relationsList.setSelectedIndex(0);
		relationsText.setText("");
		attributesText.setText("");
		queryText.setText("");
		afterMove.setSelected(false);
		selection.setSelected(similiar.getModel(), true);
		from.setText("");
		to.setText("");
		from.setEnabled(false);
		to.setEnabled(false);					
		if(positionSelection.isSelected())
			positionSelection.doClick();
		metaSelection.setSelected(false);

	}

	/**
	 * Initialize the whole view
	 */
	private void initView()
	{
		tabPane = new JTabbedPane();

		Box optimizerView = new Box(BoxLayout.Y_AXIS);
		Box secondoView = new Box(BoxLayout.Y_AXIS);
		
		content = new Box(BoxLayout.Y_AXIS);
		lower = new Box(BoxLayout.Y_AXIS);
		
		tabPane.addTab("secondo", secondoView);
		tabPane.addTab("optimizer", optimizerView);
		tabPane.addChangeListener(this);
		
		initSecondoView(secondoView);
		initOptimizerView(optimizerView);
		initLower();
		
		content.add(tabPane);
		content.add(Box.createVerticalStrut(4));

		content.add(lower);
		this.add(content);	
		initWithoutSecInterface();
		initWithSecInterface();
		upper.add(withSecInterface);//has to be done or else there will be nothing shown in one of the tabs
		upper2.add(withoutSecInterface);	
	}

	/**
	 * This method tries to execute a query in secondo with the help of the secondo Interface. Returns the result as a ListExpr otherwise throws an Exception
	 */
	private ListExpr tryExecuteQuery(String command, ListExpr result, StringBuffer errorList) throws Exception
	{
		if (parameters[0].isInstance(command) && parameters[1].isInstance(result) && parameters[4].isInstance(errorList))
		{
			Object[] params = new Object[parameters.length];
			params[0] = command;
			params[1] = result;
			params[2] = parameters[2].newInstance();
			params[3] = parameters[3].newInstance();
			params[4] = errorList;
			commandExecutionMethod.invoke(secondoInterface, params);	
			return result;
		}
		throw new Exception();
	}

	/**
	 * This method appends the part of the position to the query in the StringBuffer 'query'
	 */
	private void createPositionQueryPart(boolean exact, String from, String to, StringBuffer query, String attribute) throws Exception
	{
		int fromVal = -1;
		int toVal = -1;
		if(from != null && to != null)
		{
			fromVal = (Integer.valueOf(from)).intValue();
			toVal = (Integer.valueOf(to)).intValue();
		}
		String position = createConstPosition(); //get the position as a list expression
		query.append("extendstream[tempPos: .");
		query.append(attribute);
		query.append(" positions] ");
		if(fromVal != -1 && toVal != -1) // if a range was specified for the position to be in
		{
			query.append(" filter [.tempPos moveNo <= ");
			query.append(to);
			query.append("] filter [.tempPos moveNo >= ");
			query.append(from);
			query.append("] ");
		}
		query.append("filter [.tempPos includes[ ");
		query.append(position.toString());
		query.append("]]");
		if(exact)
		{
			query.append( " filter ["); //if exact matching is needed
			query.append(position.toString());
			query.append(" includes [.tempPos]] ");
		}
		
		
		query.append("remove[tempPos] sort rdup ");
	}

	/**
	 * appends the part of the query where the meta tags are compared to each other
	 */
	private void createMetaQueryPart(String attribute, StringBuffer query)
	{
		HashMap metaVals = metaDeliverer.getMetaVals();
		Object[] keys = metaVals.keySet().toArray();
		for (int i=0;i< keys.length; i++)
		{
			query.append("filter [.");
			query.append(attribute);
			query.append(" getkey[\"");
			query.append(ChessToolKit.convertKeyStringToListFormat((String)keys[i]));
			query.append("\"] = \"");
			query.append((String)metaVals.get(keys[i]));
			query.append("\"] ");
		}
	}

	/**
	 * this method returns the constant position  needed in the query 
	 */
	private String createConstPosition()
	{

		PositionData currentPosition = positionDeliverer.getCurrentPosition();
		StringBuffer position = new StringBuffer();
		position.append("[const position value");
		position.append('(');
		position.append('0');
		position.append(currentPosition.toListString());
		position.append(')');
		position.append(']');
		return position.toString();
	}

	/**
	 * appends the where clause of the optimizer query to the StringBuffer
	 */
	private void createMetaQueryPartOptimizer(StringBuffer query)
	{
		HashMap metaVals = metaDeliverer.getMetaVals();
		Object[] keys = metaVals.keySet().toArray();
		if(keys.length > 0)
		{
			query.append("where [");
			for (int i=0; i< keys.length;i++)
			{
				query.append(keys[i]);
				query.append("=\"");
				query.append((String)metaVals.get(keys[i]));
				query.append('\"');
				if((i+1) < keys.length)
					query.append(", ");
			}
			query.append(']');
		}
	}
	
	/**
	 * This method creates an Optimizer query on the edited meta data and shows it in the QueryBuilder
	 */
	public void createOptimizerQuery()
	{
		StringBuffer query = new StringBuffer();
		String relation, attribute;
		if(hasSecondoInterface)
		{
			relation = ((String)relationsList.getSelectedValue()).toLowerCase();
			attribute = ((String)attributesList.getSelectedValue()).toLowerCase();
		}
		else
		{
			relation = relationsText.getText().toLowerCase();
			attribute = attributesText.getText().toLowerCase();
		}
		query.append("sql select ");
		query.append(attribute);
		query.append(" from ");
		query.append(relation);
		query.append(' ');
		createMetaQueryPartOptimizer(query);
		query.append('.');
		queryText.setText(query.toString());
	}
	
	/**
	 * This method creates a Secondo query. if position is true the query is based on the edited position on the chess board in the QueryBuilder. if meta is true the query is based on the edited meta tags in the QueryBuilder (if both are true the query is based on both). If exact is true the position has to be exactly matching, if from and to are not null there are only those positions requested between from and to. 
	 */
	public void createSecondoQuery(boolean exact, boolean position,boolean meta,  String from, String to)
	{
		StringBuffer query = new StringBuffer();
		String relation, attribute;
		if(hasSecondoInterface)
		{
			relation = (String)relationsList.getSelectedValue();
			attribute = (String)attributesList.getSelectedValue();
		}
		else
		{
			relation = relationsText.getText();
			attribute = attributesText.getText();
		}

		query.append("query ");
		query.append(relation);
		query.append(" feed ");
		if (position)
		{
			try
			{
				createPositionQueryPart(exact, from, to, query, attribute);
			}
			catch(Exception e)
			{
				queryText.setText("Error: you entered a wrong text!");
				return;
			}
				
		}
		if(meta)
		{
			createMetaQueryPart(attribute, query);
		}
		query.append("consume");
		queryText.setText(query.toString());
	}

	public void stateChanged(ChangeEvent e)
	{
			this.showQueryBuilder();		
	}
}
