package viewer;

import java.awt.GridLayout;
import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;

import java.net.Socket;
import java.net.ConnectException;
import java.net.UnknownHostException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.Iterator;

import javax.swing.BoxLayout;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ListSelectionModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

import gui.SecondoObject;

import tools.Reporter;

import sj.lang.MyDataInputStream;
import sj.lang.MyDataOutputStream;

/**
 * This class provides a viewer for the CStreamAlgebra to display tuplestreams
 */
public class CStreamViewer extends SecondoViewer {
	
	private TupleStreamReceiver tsr;
	private HashMap<String, LinkedHashSet<String>> tuplemap;
	private ArrayList<String> attributes, allAttributes, opList, containerAttrs;
	private String tupledescription, unknownAttr, unknownOp;
	
	private String hostname;
	private int tupleport, streamport, numOfTuples;
	private boolean connect = false;
    private int idxHelper;
    private boolean cutAllowed = false;
	
	private JTextField filterField, numOfTypes, hostnameField, tupleportField, streamportField;;
	private JTextArea tupletextarea;
	private JSlider slider;
	private JLabel sliderlabel;
	private JButton confirmBtn, showBtn;
	
	private final String TUPLE_TYPE_PROTOCOL_MSG = "RequestDone()";
	private final String TUPLE_STREAM_PROTOCOL_MSG = "StreamDone()";
	
	/** The constructor which layouts the viewer */
	public CStreamViewer(){
		setLayout(new BorderLayout());

		/* north */
		JPanel filterpanel = new JPanel();
		filterpanel.setLayout(new BoxLayout(filterpanel, BoxLayout.LINE_AXIS));
		JLabel filterlabel = new JLabel("Filter : ");
		filterField = new JTextField();
		filterpanel.add(filterlabel);
		filterpanel.add(filterField);

		/* center */
		JPanel textpanel = new JPanel();
		textpanel.setLayout(new BorderLayout());
		tupletextarea = new JTextArea();
		tupletextarea.setEditable(false);
		textpanel.add(new JScrollPane(tupletextarea), BorderLayout.CENTER);
		
		/* west */
		JPanel westpanel = new JPanel();
		westpanel.setLayout(new GridLayout(2,0));
		JPanel configpanel = new JPanel();
		configpanel.setLayout(new BoxLayout(configpanel, BoxLayout.PAGE_AXIS));
		JLabel numOfTypesLabel = new JLabel("Number of Tupletypes");
		numOfTypes = new JTextField();
		JLabel hostlabel = new JLabel("Hostname");
		hostnameField = new JTextField();
		JLabel tuplelabel = new JLabel("Port for Tupletypes");
		tupleportField = new JTextField();
		JLabel streamlabel = new JLabel("Port for Tuplestream");
		streamportField = new JTextField();
		confirmBtn = new JButton();
		confirmBtn.setText("receive tupletypes");
		
		configpanel.add(numOfTypesLabel);
		configpanel.add(numOfTypes);
		configpanel.add(hostlabel);
		configpanel.add(hostnameField);
		configpanel.add(tuplelabel);
		configpanel.add(tupleportField);
		configpanel.add(streamlabel);
		configpanel.add(streamportField);
		configpanel.add(confirmBtn);
		
		JPanel sliderpanel = new JPanel();
		sliderpanel.setLayout(new BoxLayout(sliderpanel, BoxLayout.PAGE_AXIS));
		sliderlabel = new JLabel("show 5 Tuples");
		slider = new JSlider(JSlider.HORIZONTAL, 1, 30, 5);
		
		showBtn = new JButton();
		showBtn.setText("receive tuplestream");
		
		sliderpanel.add(slider);
		sliderpanel.add(sliderlabel);
		sliderpanel.add(showBtn);
		
		westpanel.add(configpanel);
		westpanel.add(sliderpanel);

		add(filterpanel, BorderLayout.NORTH);
		add(textpanel, BorderLayout.CENTER);
		add(westpanel, BorderLayout.WEST);
		
		tsr = new TupleStreamReceiver();
		tuplemap = new HashMap<String, LinkedHashSet<String>>();
		attributes = new ArrayList<String>();
		allAttributes = new ArrayList<String>();
		opList = new ArrayList<String>();
		containerAttrs = new ArrayList<String>();
		tupledescription = new String();
		
		fillOpList();

        confirmBtn.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent action) {
				showTupleTypes();
			}
		});

        slider.addChangeListener(new ChangeListener(){

			@Override
			public void stateChanged(ChangeEvent c) {
                sliderlabel.setText("show "+slider.getValue()+" Tuples");
			}
		});
		
        showBtn.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent action) {
				if( !connect ){
					showTuples();
				} else {
					tsr.stopThread();
				}
			}
		});
	}
	
	/** This function is not needed in this viewer */
    public boolean addObject(SecondoObject o){
        return false;
    }

    /** This function is not needed in this viewer */
    public boolean isDisplayed(SecondoObject o){
        return false;
    } 

    /** This function is not needed in this viewer */
    public void removeObject(SecondoObject o){ }

    /** This function is not needed in this viewer */
    public void removeAll(){ }    

    /** This function is not needed in this viewer */
    public boolean canDisplay(SecondoObject o){
        return false;
    }

    /** Returns the name of this viewer */
    public String getName(){
        return "CStreamViewer";
    }
   
    /** This function is not needed in this viewer */
    public boolean selectObject(SecondoObject O){
        return false;
    }
    
    /** Fills the operatorlist with available operators for the filter */
    private void fillOpList(){
        opList.add("+");
        opList.add("-");
        opList.add("*");
        opList.add("/");
        opList.add("<");
        opList.add("<=");
        opList.add("=");
        opList.add(">=");
        opList.add(">");
        opList.add("#");
        opList.add("startsWith");
        opList.add("contains");
        opList.add("count");
        opList.add("mod");
        opList.add("div");
    }

	/** Checks the userinputs for the serverconnections */
	private boolean checkEntries(){
        boolean tpOK = false;
        boolean spOK = false;
        boolean ntOK = false;
        
        try {
			tupleport = Integer.parseInt(tupleportField.getText());
			tpOK = true;
        } catch ( NumberFormatException tp ) {
            Reporter.showInfo("Invalid entry for the tupleport.\nPlease select a number between 0 and 65535.");
        }
        
        try {
			streamport = Integer.parseInt(streamportField.getText());
			spOK = true;
        } catch ( NumberFormatException sp ) {
            Reporter.showInfo("Invalid entry for the streamport.\nPlease select a number between 0 and 65535.");
        }
        
        try {
            numOfTuples = Integer.parseInt(numOfTypes.getText());
            ntOK = true;
        } catch ( NumberFormatException np ) {
            Reporter.showInfo("Invalid entry for number of tupletypes.\nPlease select a number greater than zero.");
        }
        
        if(!tpOK || !spOK || !ntOK ){
            return false;
        }
        
        hostname = hostnameField.getText();

        if ( hostname.isEmpty() || tupleport < 0 || tupleport > 65535 ||
            streamport < 0 || streamport > 65535 || numOfTuples < 1){
            hostnameField.setText("127.0.0.1");
            tupleportField.setText("1024");
            streamportField.setText("1024");
            numOfTypes.setText("1");
            Reporter.showInfo("One or more entries are invalid (out of bounds)");
            return false;
        } else {
            return true;
        }
	}
	
	/** This function starts the serverconnection to get the
	 * tupletypes from the server which provides tupletypes
	 */
	private void showTupleTypes(){
		tuplemap.clear();
		
		if ( !checkEntries() ){
            return;
        }
		
		String tupletypes = new String();
		TupleTypeReceiver ttr = new TupleTypeReceiver();
		ttr.startThread();

		/* Wait one second, to receive a tupledescription from the
		 * server before the thread will be killed */
		int loop = 10;
		while( !ttr.canInterrupt() && loop > 0){
            try {
                Thread.sleep(100);
                loop--;
            } catch ( InterruptedException ie ){ 
                Reporter.debug(ie);
            }
        }

        tupletypes = ttr.getTupleTypes();
		ttr.stopThread();

		if ( tupletypes.equals("()RequestDone()") ){
            Reporter.showInfo("An empty tupledescription received! Abort.");
            return;
        } else if ( tupletypes.equals("") ){
            Reporter.showInfo("no tupledescription received.");
            return;
        }
        
        tupletypes = tupletypes.replace(") (",")("); //
		splitList(tupletypes);
		
		if ( !tuplemap.isEmpty() ){
			new TupleDialog();
			
            if ( tupledescription.equals("()") || tupledescription.equals("") ){
                Reporter.showInfo("An empty or no description was choosed. Abort.");
                return;
            }

            if( !attributes.isEmpty() ){
                buildProjectTupleDescription();
                for(int i = 0; i < containerAttrs.size(); i++){
                    eraseEmptyContainers();
                }
            }
		}
	}
	
	/** An algorithm to extract tupletypes and their attributes */
	private void splitList(String tupletypes){
        if ( tupletypes.isEmpty() ){
            return;
        }
		
		if ( tupletypes.indexOf(TUPLE_TYPE_PROTOCOL_MSG) != -1 ){
            tupletypes = tupletypes.replace(TUPLE_TYPE_PROTOCOL_MSG,"");
        } else {
            Reporter.showInfo("Protocol error : No "+TUPLE_TYPE_PROTOCOL_MSG+" received.");
            return;
        }
		
        ArrayList<String> tupledescrs = new ArrayList<String>();
		int bracketcounter = 0;
		int idxOld = 0;
		for(int i = 0; i < tupletypes.length(); i++){
			if ( tupletypes.charAt(i) == '(' ){
				bracketcounter++;
			} else if ( tupletypes.charAt(i) == ')' ){
				bracketcounter--;
				if ( bracketcounter == 0 ){
					String sub = tupletypes.substring(idxOld, i + 1);
					idxOld = i + 1;
					tupledescrs.add(sub);
				}
			}
		}
		tuplemap = getTupleTypes(tupledescrs);
	}

	/** Extracts all tupletypes and attributes */
	private HashMap<String, LinkedHashSet<String>> getTupleTypes(final ArrayList<String> tupletypes){
		HashMap<String, LinkedHashSet<String>> map = new HashMap<String, LinkedHashSet<String>>();
		
		for(int i = 0; i < tupletypes.size(); i++){
			LinkedHashSet<String> attrs = getAttributes(tupletypes.get(i));
			String temp = tupletypes.get(i);
			map.put(temp, attrs);
		}
		return map;
	}
    
    /** Extracts all attributes from a giving tupledescription */
	private LinkedHashSet<String> getAttributes(String tupledesc){
		LinkedHashSet<String> attrs = new LinkedHashSet<String>();
        int begin= -1;
        int end = 0;
		while(begin != end){
            begin = end;
            begin = getStartIndex(tupledesc, end);
            end = getEndIndex(tupledesc, begin);
            if ( end == tupledesc.length() || begin == end ){
                break;
            }
            
            String sub = tupledesc.substring(begin - 1, end + 1);
            if( !(sub.contains("record") || sub.contains("vector")) ){
            	attrs.add(sub);
            } else {
                int blank = 0;
                for(int i = 1; i < sub.length(); i++){
                    if(sub.charAt(i) == ' '){
                        blank = i;
                        break;
                    }
                }
                sub = sub.substring(1, blank);
                containerAttrs.add("("+sub+")");
                containerAttrs.add("("+sub+" )");
                containerAttrs.add("("+sub+"  )");
            }
        }
        return attrs;
	}
	
	/** Finds the startindex where an attributes begins */
	private int getStartIndex(String tupledesc, int end){
        
        int start = end;
        for(int i = end; i < tupledesc.length(); i++){
            if ( Character.isUpperCase(tupledesc.charAt(i)) ){
                start = i;
                break;
            }
        }
        return start;
    }

    /** Finds the endindex where the attributes ends or a container begins */
	private int getEndIndex(String tupledesc, int start){
	
        int end = start;
        for(int i = start; i < tupledesc.length(); i++){
            if ( tupledesc.charAt(i) == '(' ){
            	return i + "vector".length();
            } else if ( tupledesc.charAt(i) == ')' ){
            	end = i;
            	break;
            } else if ( i == tupledesc.length() - 1 ){
                return tupledesc.length();
            }
        }
        return end;
	}
	
	/** This function builds a projected tupledescription with the
	 * choosen attributes from the tupledialog
	 */
    private void buildProjectTupleDescription(){
    	for(int i = 0; i < attributes.size(); i++){
    		tupledescription = tupledescription.replace(attributes.get(i), "");
    	}
	}
	
    /** deletes an empty vector or record */
	private void eraseEmptyContainers(){
        tupledescription = tupledescription.replace(" ()","");
        tupledescription = tupledescription.replace("()","");
        tupledescription = tupledescription.replace("(vector)","");
        tupledescription = tupledescription.replace("(record)","");
        tupledescription = tupledescription.replace("(vector )","");
        tupledescription = tupledescription.replace("(record )","");
        
        for(int i = 0; i < containerAttrs.size(); i++){
            tupledescription = tupledescription.replace(containerAttrs.get(i),"");
        }
    }
        
	/** This function starts the serverconnection to get the
	 * tuplestreams from the server which distributes a tuplestream
	 */
	private void showTuples(){
	
        if ( !checkEntries() ){
            return;
        }

		String filter = checkFilterContent();
		if ( filter.equals("error") ){
            Reporter.showInfo("Invalid entry for the filtercondition\ne.g. '.No >= 5'\ne.g. 'fun(t : tuple([No : int])) attr(t,No) >= 5'.");
            return;
		} else if ( filter.equals("errAttr") ){
            Reporter.showInfo("Unknown attribute for the filter :'"+unknownAttr+"'");
            return;
        } else if ( filter.equals("errOp") ){
            Reporter.showInfo("Unknown operator for the filter :'"+unknownOp+"'");
            return;
        }

		tsr = new TupleStreamReceiver();
		tsr.setFilter(filter);
		tsr.startThread();
	}

	/** Checks the filterinput */ 
	private String checkFilterContent(){
		if ( filterField.getText().isEmpty() ){
			return new String("()");
		} else {
			return buildOpTreeString();
		}
	}
	
    /** This function builds an 'operatortreestring' */
    private String buildOpTreeString(){
        String fkt = filterField.getText();
    	if ( fkt.length() < 3){
    		return new String("error");
    	}

        if(fkt.startsWith(".")){
            try {
                return shortForm(fkt);
            } catch ( StringIndexOutOfBoundsException se ){
                return new String("error");
            }
        } else if (fkt.startsWith("fun")){
            try {
                return longForm(fkt);
            } catch ( StringIndexOutOfBoundsException se ){
                return new String("error");
            }
        } else {
            return new String("error");
        }
    }
    
    private boolean examineAttributeFromFilter(String filterattr){
        for(int i = 0; i < allAttributes.size(); i++){
            if(allAttributes.get(i).contains(filterattr)){
                return true;
            }
        }
        unknownAttr = filterattr;
        return false;
    }
    
    private boolean examineOperatorFromFilter(String filterop){
        for(int i = 0; i < opList.size(); i++){
            if(filterop.equals(opList.get(i)) ){
                return true;
            }
        }
        unknownOp = filterop;
        return false;
    }

    /** Creates an operatortree from a longform */
	private String longForm(String funtxt) throws StringIndexOutOfBoundsException {
        String filterattr = new String();
        String filterop = new String();
        String filterval = new String();
        
        funtxt = funtxt.trim();
        
        int idx = funtxt.indexOf("attr");
        funtxt = funtxt.substring(idx, funtxt.length());

        idx = funtxt.indexOf(",");
        funtxt = funtxt.substring(idx + 1, funtxt.length());
        
        idx = funtxt.indexOf(")");
        filterattr = funtxt.substring(0, idx);
        
        if ( !examineAttributeFromFilter(filterattr) ){
            return new String("errAttr");
        }
        
        funtxt = funtxt.substring(idx + 1, funtxt.length());
        
        funtxt = funtxt.trim();
        idx = funtxt.indexOf(" ");
        filterop = funtxt.substring(0, idx);
        
        if ( !examineOperatorFromFilter(filterop) ){
            return new String("errOp");
        }
        
        funtxt = funtxt.substring(idx + 1, funtxt.length());
        
        /* One more condition */
        if ( funtxt.contains(" ")){
            String filterattr2 = new String();
            String filterop2 = new String();
            String filterval2 = new String();
            String con = new String();
            
            if ( funtxt.contains(" and ")){
                con = "and";
            } else if ( funtxt.contains(" or ")){
                con = "or";;
            }

            idx = funtxt.indexOf(" ");
            filterval = funtxt.substring(0, idx);
            funtxt = funtxt.substring(idx + 1, funtxt.length());

            idx = funtxt.indexOf("attr");
            funtxt = funtxt.substring(idx, funtxt.length());
            idx = funtxt.indexOf(",");
            funtxt = funtxt.substring(idx + 1, funtxt.length());
            
            idx = funtxt.indexOf(")");
            filterattr2 = funtxt.substring(0, idx);
            
            if ( !examineAttributeFromFilter(filterattr2) ){
                return new String("errAttr");
            }
            
            funtxt = funtxt.substring(idx + 1, funtxt.length());
            funtxt = funtxt.trim();
            idx = funtxt.indexOf(" ");
            filterop2 = funtxt.substring(0, idx);
            
            if ( !examineOperatorFromFilter(filterop2) ){
                return new String("errOp");
            }
            
            funtxt = funtxt.substring(idx + 1, funtxt.length());
            filterval2 = funtxt.substring(0, funtxt.length() - 1);
            
            return "(fun(t(tuple"+tupledescription+"))("+con+"("+filterop+"(attr t "+filterattr+")"+filterval+")("+filterop2+"(attr t "+filterattr2+")"+filterval2+")))";
        } else {
        	/* no second condition */
            filterval = funtxt.substring(0, funtxt.length());
    		return "(fun(t(tuple"+tupledescription+"))("+filterop+"(attr t "+filterattr+")"+filterval+"))";
        }
	}
	
	/** Creates an operatortree from a shortform */
	private String shortForm(String funtxt) throws StringIndexOutOfBoundsException {
        String filterattr = new String();
        String filterop = new String();
        String filterval = new String();
        String con = new String();
        
        boolean two = false;
        funtxt = funtxt.trim();
        funtxt = funtxt.replace("(","");
        funtxt = funtxt.replace(")","");

        /* examine, if there is a second filtercondition */
        if ( funtxt.contains(" and ")){
            con = "and";
            two = true;
        } else if ( funtxt.contains(" or ")){
            con = "or";
            two = true;
        }

        funtxt = funtxt.substring(1, funtxt.length());
            
        int pos = 0;
        for(int i = 0; i < funtxt.length(); i++){
            if(funtxt.charAt(i) == ' '){
                pos = i;
                break;
                }
        }
        
        filterattr = funtxt.substring(0, pos);
        
        if ( !examineAttributeFromFilter(filterattr) ){
            return new String("errAttr");
        }
        
        funtxt = funtxt.substring(pos + 1, funtxt.length());
            
        for(int i = 0; i < funtxt.length(); i++){
            if(funtxt.charAt(i) == ' '){
                pos = i;
                break;
            }
        }
            
        filterop = funtxt.substring(0, pos);
        
        if ( !examineOperatorFromFilter(filterop) ){
            return new String("errOp");
        }
        
        funtxt = funtxt.substring(pos + 1, funtxt.length());
            
        if (!two){
            filterval = funtxt;
            String opTree = "(fun(t(tuple"+tupledescription+"))("+filterop+"(attr t "+filterattr+")"+filterval+"))";
            return opTree;
        }
            
        for(int i = 0; i < funtxt.length(); i++){
            if(funtxt.charAt(i) == ' '){
                pos = i;
                break;
            }
        }
            
        filterval = funtxt.substring(0, pos);
        funtxt = funtxt.substring(pos + 1, funtxt.length());
            
        String filterval2 = new String();
        String filterop2 = new String();
        String filterattr2 = new String();

        for(int i = funtxt.length() - 1; i > 0; i--){
            if(funtxt.charAt(i) == ' ' ){
                pos = i;
                    break;
            }
        }
            
        filterval2 = funtxt.substring(pos + 1, funtxt.length());
        funtxt = funtxt.substring(0, pos);
            
        for(int i = funtxt.length() - 1; i > 0; i--){
            if(funtxt.charAt(i) == ' ' ){
                pos = i;
                break;
            }
        }
        
        filterop2 = funtxt.substring(pos + 1, funtxt.length());
        
        if ( !examineOperatorFromFilter(filterop2) ){
            return new String("errOp");
        }
        
        funtxt = funtxt.substring(0, pos);
            
        for(int i = funtxt.length() - 1; i > 0; i--){
            if(funtxt.charAt(i) == '.' ){
                pos = i;
                break;
            }
        }
            
        filterattr2 = funtxt.substring(pos + 1,funtxt.length());
        if ( !examineAttributeFromFilter(filterattr2) ){
            return new String("errAttr");
        }

        String opTree = "(fun(t(tuple"+tupledescription+"))("+con+"("+filterop+"(attr t "+filterattr+")"+filterval+")("+filterop2+"(attr t "+filterattr2+")"+filterval2+")))";
        return opTree;
	}

	/** Updates the textarea which shows the tuplestream */
	private void updateTupleTextArea(final String text){
        tupletextarea.insert(text, 0);
    }

	/**
	 * A helperclass to choose tupletypes and optional attributes from this tupletype
	 */
	private class TupleDialog extends JDialog {
		
		private DefaultListModel<String> tupletypelistmodel, attributelistmodel;
		private JList<String> tupletypelist, attributelist;
		private JButton ok;
		private ArrayList<Integer> negativeIndices;
		
		/** The constructor */
		private TupleDialog(){
			setModal(true);
			setLayout(new GridLayout(2,0));
			
			negativeIndices = new ArrayList<Integer>();
			
			JPanel typepanel = new JPanel();
			typepanel.setLayout(new BoxLayout(typepanel, BoxLayout.PAGE_AXIS));
			JLabel typelabel = new JLabel("Available Tupletypes");
			tupletypelistmodel = new DefaultListModel<String>();
			tupletypelist = new JList<String>(tupletypelistmodel);
			tupletypelist.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
			tupletypelist.setSelectedIndex(0);
			tupletypelist.setVisibleRowCount(10);
			
			typepanel.add(typelabel);
			typepanel.add(new JScrollPane(tupletypelist));
			
			JPanel attrpanel = new JPanel();
			attrpanel.setLayout(new BoxLayout(attrpanel, BoxLayout.PAGE_AXIS));
			JLabel attrlabel = new JLabel("choose Attributes");
			attributelistmodel = new DefaultListModel<String>();
			attributelist = new JList<String>(attributelistmodel);
			attributelist.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
			attributelist.setSelectedIndex(0);
			attributelist.setVisibleRowCount(10);

			ok = new JButton("confirm");
			ok.setEnabled(false);
			
			attrpanel.add(attrlabel);
			attrpanel.add(new JScrollPane(attributelist));
			attrpanel.add(ok);
			
			add(typepanel, BorderLayout.WEST);
			add(attrpanel, BorderLayout.CENTER);
			
            tupletypelist.addListSelectionListener(new ListSelectionListener(){

				@Override
				public void valueChanged(ListSelectionEvent l) {
					if ( l.getValueIsAdjusting() ){
                        if ( tupletypelist.isSelectionEmpty() ){
                            ok.setEnabled(false);
                        } else {
                            ok.setEnabled(true);
                        }
						final String tupledesc = tupletypelist.getSelectedValue();
						addAttributesInList(tupledesc);
					}
				}
			});
			
            attributelist.addListSelectionListener(new ListSelectionListener(){

				@Override
				public void valueChanged(ListSelectionEvent l) {
					if ( l.getValueIsAdjusting() ){
                        negativeIndices.clear();
                        for(int i = 0; i < attributelistmodel.getSize(); i++){
                            if(!attributelist.isSelectedIndex(i)){
                                negativeIndices.add(i);
                            }
                        }
					}
				}
			});
			
            ok.addActionListener(new ActionListener(){

				@Override
				public void actionPerformed(ActionEvent action) {
                    collectAttributes();
					if ( collectTupleDescription() ){
                        clearEntries();
                        dispose();
                    }
				}
			});

			addTupleTypesInList();
			setSize(640, 480);
			setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
            setVisible(true);
		}
		
		/** Adds all tupletypes in the upper list */
		private void addTupleTypesInList(){
			for(Iterator<String> it = tuplemap.keySet().iterator(); it.hasNext();){
				String type = it.next();
				tupletypelistmodel.addElement(type);
			}
		}
		
		/** Adds all attributes from the choosen tupletype in the lower list */
		private void addAttributesInList(String tupledesc){
            allAttributes.clear();
            attributelistmodel.clear();
			for(Iterator<String> at = tuplemap.get(tupledesc).iterator(); at.hasNext();){
				String attr = at.next();
				attributelistmodel.addElement(attr);
				allAttributes.add(attr);
			}
		}
		
		/** Saves the choosen tupletype from the upper list */
		private boolean collectTupleDescription(){
			tupledescription = tupletypelistmodel.getElementAt(tupletypelist.getSelectedIndex());
			
			if( tupledescription.equals("()") ){
                Reporter.showInfo("An empty tupledescription was choosed");
                return false;
            }
            return true;
		}
		
		/** Saves all choosen attributes from the lower list, if no attributes is selected
		 * the original tupledescription will be choosen
		 */
		private void collectAttributes(){
            attributes.clear();
            
            if(negativeIndices.isEmpty()){
                return;
            }

			for(int i = 0; i < negativeIndices.size(); i++){
				attributes.add(attributelistmodel.getElementAt(negativeIndices.get(i)));
			}
		}
		
		/** Clears all entries from the tuplemap and the two list */
		private void clearEntries(){
			tuplemap.clear();
			tupletypelistmodel.clear();
			attributelistmodel.clear();
		}
	}
	
	/**
	 * A helperclass to receive the tupletypes from the server 
	 */
	private class TupleTypeReceiver extends Thread {
		
		private String tupletypes;
		private boolean allowInterrupt;
		private Socket socket;
		private MyDataInputStream input;
		private MyDataOutputStream output;
		private BufferedInputStream reader;
		private BufferedOutputStream writer;
		
		/** The constructor */
		private TupleTypeReceiver(){
			tupletypes = new String();
			allowInterrupt = false;
		}
		
		/** Starts the thread from outside */
		public void startThread(){
			start();
		}
		
		/** Interrupts the thread from outside */
		public void stopThread(){
			try {
				writer.close();
				reader.close();
				output.close();
				input.close();
				socket.close();
			} catch ( Exception e ){
				Reporter.debug(e);
			} finally {
                interrupt();
            }
		}
		
		/** Serverconnection */
		@Override
		public void run(){
			try {
				socket = new Socket(hostname, tupleport);
            } catch ( ConnectException ce ){
                allowInterrupt = true;
                return;
            } catch ( UnknownHostException uhe ){
                allowInterrupt = true;
                return;
            } catch ( IOException ioe ) {
                allowInterrupt = true;
                return;
            }
            
            try {
				reader = new BufferedInputStream(socket.getInputStream());
				writer = new BufferedOutputStream(socket.getOutputStream());
				input = new MyDataInputStream(reader);
				output = new MyDataOutputStream(writer);
				
                output.writeString("RequestTupleTypes("+numOfTuples+")\n");
				output.flush();
				
                String line = null;
                while( (line = input.readLine()) != null){
                    tupletypes += line;
                }
                
                allowInterrupt = true;
                
            } catch ( IOException ioe ){
                Reporter.debug(ioe);
                allowInterrupt = true;
            }
		}
		
		/** Get the tupletypes */
		public String getTupleTypes(){
			return tupletypes;
		}
		
		public boolean canInterrupt(){
            return allowInterrupt;
        }
	}
	
	/**
	 * A helperclass to receive and display a tuplestream from the server
	 */
	private class TupleStreamReceiver extends Thread {
		
		private Socket socket;
		private MyDataInputStream input;
		private MyDataOutputStream output;
		private BufferedInputStream reader;
		private BufferedOutputStream writer;
		private String filter;
		
		/** The constructor */
		private TupleStreamReceiver(){
			filter = new String();
		}
		
		public void setFilter(String filter){
			this.filter = filter;
		}
		
		/** Starts the thread from outside */
		public void startThread(){
            tupletextarea.setText("");
			start();
		}
		
		/** Interrupts the thread from outside */
		public void stopThread(){
			try {
				writer.close();
				reader.close();
				output.close();
				input.close();
				socket.close();
			} catch ( Exception e ){
				Reporter.debug(e);
			} finally {
                interrupt();
                connect = false;
                showBtn.setText("receive tuplestream");
                showBtn.setEnabled(true);
                slider.setEnabled(true);
                confirmBtn.setEnabled(true);
            }
		}
		
		/** Serverconnection */
		@Override
		public void run(){
			try {
				socket = new Socket(hostname, streamport);
            } catch ( ConnectException ce ){
                Reporter.showInfo("Could not connect to\nServer '"+hostname+"'\nPort '"+streamport+"'");
                interrupt();
                return;
            } catch ( UnknownHostException uhe ){
                interrupt();
                return;
            } catch ( IOException ioe ) {
                interrupt();
                return;
            }
            
            try {
				reader = new BufferedInputStream(socket.getInputStream());
				writer = new BufferedOutputStream(socket.getOutputStream());
				input = new MyDataInputStream(reader);
				output = new MyDataOutputStream(writer);
                
				connect = true;
				showBtn.setText("stop tuplestream");
				slider.setEnabled(false);
				confirmBtn.setEnabled(false);
				tupledescription = tupledescription.replace(")(",") ("); //
				output.writeString("RequestStream("+tupledescription+","+filter+",false)\n");
				output.flush();

                String in = input.readLine();
                if ( in.equals("ConfirmStream(OK)") ){
                ArrayList<String> lastXTuples = new ArrayList<String>();
                    while ( !isInterrupted() ) {
                        in = input.readLine();
                        if ( in.equals(TUPLE_STREAM_PROTOCOL_MSG) ) {
                            stopThread();
                            return;
                        } else {
                            String extract = in.substring("TupleMessage".length(), in.length());
                            extract = formatter(extract);
                            showlastXTuples(lastXTuples, extract);
                        }
                    }
                } else {
                    Reporter.showInfo("Protocol error : "+in);
                }
            } catch ( IOException ioe ){
                Reporter.debug(ioe);
            } finally {
                stopThread();
            }
        }
        
        /** shows the last X tuples, where X is the slidervalue */
        private void showlastXTuples(ArrayList<String> last, String tuple){

            if ( last.size() < slider.getValue() ){
                last.add(tuple);
            } else {
                last.remove(0);
                last.add(tuple);
            }
            
            tupletextarea.setText("");
            for(int i = 0; i < last.size(); i++){
                updateTupleTextArea(last.get(i)+"\n");
            }
        }
                
        /** this function formattes the received tuples for 
          * the tupletextarea */
        private String formatter(String tuplevalues){
            StringBuilder sb = new StringBuilder();
            sb.append(tuplevalues.substring(0, 1));
            sb.append("\n\t");
            tuplevalues = tuplevalues.substring(1, tuplevalues.length() - 1);
		
            int start = -1;
            int end = 0;
            while(start != end){
                start = end;
                for(int i = start; i < tuplevalues.length(); i++){
                    if(Character.isDigit(tuplevalues.charAt(i))){
                        String number = numberOrBoolFormatter(tuplevalues, i);
                        if(number.isEmpty()){
                            return new String();
                        } else {
                            end = i + number.length() - 2;
                            sb.append(number+"\n\t");
                            break;
                        }
                    } else if (Character.isLetter(tuplevalues.charAt(i)) ){
                        String bool = numberOrBoolFormatter(tuplevalues, i);
                        if(bool.isEmpty()){
                            return new String();
                        } else if (bool.contains("true") || bool.contains("TRUE") ||
                                    bool.contains("false") || bool.contains("FALSE") ){
                            end = i + bool.length() - 2;
                            sb.append(bool+"\n\t");
                            break;
                        }
                    } else if (tuplevalues.charAt(i) == '\"' ){
                        String s = stringFormatter(tuplevalues, i);
                        if(s.isEmpty()){
                            return new String();
                        } else {
                            end = i + s.length() - 2;
                            sb.append(s+"\n\t");
                            break;
                        }
                    } else if (tuplevalues.charAt(i) == '\'' ){
                        String t = textFormatter(tuplevalues, i);
                        if(t.isEmpty()){
                            return new String();
                        } else {
                            end = i + t.length() - 2;
                            sb.append(t+"\n\t");
                            break;
                        }
                    } else if (tuplevalues.charAt(i) == '(' ){
                        String nf = nestedFormatter(tuplevalues, i);
                        if(nf.isEmpty()){
                            return new String();
                        } else {
                            end = i + nf.length() - 2;
                            sb.append(nf+"\n\t");
                            break;
                        }
                    }		
                }
            }
		
            sb.deleteCharAt(sb.length() - 1);
            sb.append(")");
            return sb.toString();
        }

        private String stringFormatter(String tuplevalues, int start){
            /* search for a string */
            int end = 0;
            for(int i = start + 1; i < tuplevalues.length(); i++){
                if(tuplevalues.charAt(i) == '\"' ){
                    end = i + 1;
                    break;
                } else if (i == tuplevalues.length() - 1){
                    end = i + 1;
                    break;
                }
            }

            try {
                String word = tuplevalues.substring(start, end);
                word = word.replace(word, "("+word+")");
                return word;
            } catch ( StringIndexOutOfBoundsException e ){
                return new String();
            }
        }
	
        private String textFormatter(String tuplevalues, int start){
            /* search for a text */
            int end = 0;
            for(int i = start + 1; i < tuplevalues.length(); i++){
                if(tuplevalues.charAt(i) == '\'' ){
                    end = i + 1;
                    break;
                } else if (i == tuplevalues.length() - 1){
                    end = i + 1;
                    break;
                }
            }

            try {
                String word = tuplevalues.substring(start, end);
                word = word.replace(word, "("+word+")");
                return word;
            } catch ( StringIndexOutOfBoundsException e ){
                return new String();
            }
        }
	
        private String numberOrBoolFormatter(String tuplevalues, int start){
            /* search for a numeric */
            int end = 0;
            for(int i = start; i < tuplevalues.length(); i++){
                if(tuplevalues.charAt(i) == ' ' ){
                    end = i;
                    break;
                } else if (i == tuplevalues.length() - 1){
                    end = i + 1;
                    break;
                }
            }

            try {
                String word = tuplevalues.substring(start, end);
                word = word.replace(word, "("+word+")");
                return word;
            } catch ( StringIndexOutOfBoundsException e ){
                return new String();
            }
        }

        private String nestedFormatter(String tuplevalues, int start){
            /* search for a nested form (vector or record) */
            int end = 0;
            int brackets = 1;
            for(int i = start + 1; i < tuplevalues.length(); i++){
                if( tuplevalues.charAt(i) == '(' ){
                    brackets++;
                } else if( tuplevalues.charAt(i) == ')' ){
                    brackets--;
                    if( brackets == 0){
                        end = i + 1;
                        break;
                    } else {
                        continue;
                    }
                } else if ( i == tuplevalues.length() - 1){
                    end = i + 1;
                    break;
                }
            }

            try {
                String word = tuplevalues.substring(start, end);
                word = word.replace(word, "("+word+")");
                return word;
            } catch ( StringIndexOutOfBoundsException e ){
                return new String();
            }
        }
    }
}
