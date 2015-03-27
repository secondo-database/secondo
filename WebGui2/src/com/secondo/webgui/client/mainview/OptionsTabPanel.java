package com.secondo.webgui.client.mainview;

import java.util.ArrayList;




import java.util.List;

import org.gwtopenmaps.openlayers.client.Projection;
import org.gwtopenmaps.openlayers.client.geometry.Point;
import org.gwtopenmaps.openlayers.client.layer.Vector;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Anchor;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.CheckBox;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.DecoratedTabPanel;
import com.google.gwt.user.client.ui.DecoratorPanel;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.FlexTable.FlexCellFormatter;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.StackPanel;
import com.google.gwt.user.client.ui.Grid;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.google.gwt.user.client.ui.PushButton;
import com.secondo.webgui.utils.config.ModesToShowSymTraj;

public class OptionsTabPanel extends Composite {

	private DecoratedTabPanel optionsTabPanel = new DecoratedTabPanel();

	private FileUploadPanel uploadWidget = new FileUploadPanel();

	ListBox optionsForCreatingSymTraj;
	private Grid gridWithOptionsForCreatingSymTraj;
	private Button createSymTrajButton;

	private StackPanel stackPanel = new StackPanel();

	// elements of the animation panel
	private VerticalPanel animationPanel = new VerticalPanel();
	private HorizontalPanel panelForPlay = new HorizontalPanel();
	private HorizontalPanel panelForPause = new HorizontalPanel();
	private TextBox timeCounter = new TextBox();
	private TimeSlider timeSlider = new TimeSlider();
	private Image playIcon = new Image("resources/images/play-icon2.png");
	private Image forwardIcon = new Image("resources/images/speedup-icon.png");
	private Image rewindIcon = new Image("resources/images/speeddown-icon.png");
	private Image pauseIcon = new Image("resources/images/pause-icon.png");		
	private Image queryIcon = new Image("resources/images/check-icon.png");
	private Anchor playLink = new Anchor("Play");
	private Anchor forwardLink = new Anchor("Speed Up");
	private Anchor rewindLink = new Anchor("Speed Down");
	private Anchor pauseLink = new Anchor("Pause");

	CheckBox checkBoxForVariable = new CheckBox();
	
	ArrayList<HorizontalPanel> arrayOfGrids = new ArrayList<HorizontalPanel>();
	
//	elements for pattern panel
	private Label patternLabel = new Label("");
	private Label warningLabel= new Label("");
	private Label resultOfPatternMatchingLabel = new Label("");
	private ScrollPanel definedPattern = new ScrollPanel();	
	private TextBox textBoxForVariable = new DefaultTextBox("variable");
	private TextBox patternBox = new DefaultTextBox("enter your pattern");	
	private TextBox condition = new DefaultTextBox("condition");
	final private VerticalPanel panelForPattern = new VerticalPanel();
	private ListBox selectOptionsForDisplayMode = new ListBox();
	private ListBox selectOptionsForExistingTrajectories = createBoxWithSelectOptionsForExistingTrajectories();
	private Button getRelationButton = new Button("Get relation");	
	private Button matchButton = new Button("match");
	private Button removeButton = new Button("remove");
	private FlexTable definedPatternWidget;
	private ArrayList<String> variablesForPattern = new ArrayList<String>(); 
	private boolean unsuccessfulVerification=false; 

	private String attributeNameOfMlabelInRelation;
	private String attributeNameOfMPointInRelation;
	private DecoratorPanel decPanelForStackWithSimpleQueries = new DecoratorPanel();
	private SimpleQueriesStackPanel simpleQueriesStackPanel;

	private boolean patternMatchingIsInitiated = false;

	public DecoratedTabPanel getOptionsTabPanel() {
		return optionsTabPanel;
	}

	/**
	 * 
	 */
	public OptionsTabPanel() {
		attributeNameOfMlabelInRelation="";
		optionsTabPanel.setWidth("300px");
		optionsTabPanel.getElement().getStyle().setMarginBottom(10.0, Unit.PX);
		optionsTabPanel.setVisible(true);
		optionsTabPanel.setStyleName("maxWidth");

		optionsForCreatingSymTraj = new ListBox();

		final VerticalPanel vPanel0 = new VerticalPanel();
		vPanel0.setStyleName("minHeight");
		vPanel0.setWidth("300px");
		vPanel0.setSpacing(5);

		Image closeImage = new Image(
				"resources/images/Action-arrow-blue-down-icon.png");
		closeImage.setStyleName("centered");
		closeImage.setTitle("Show");
		Image openImage = new Image(
				"resources/images/Action-arrow-blue-up-icon.png");
		openImage.setStyleName("centered");
		openImage.setTitle("Hide");

		final PushButton openArrowButtonForTab1 = new PushButton(new Image(
				"resources/images/Action-arrow-blue-down-icon.png"));
		openArrowButtonForTab1.setTitle("Show");
		openArrowButtonForTab1.setStyleName("pushButtonWithTransparentBackground");

		final PushButton closeArrowButtonForTab1 = new PushButton(new Image(
				"resources/images/Action-arrow-blue-up-icon.png"));
		closeArrowButtonForTab1.setStyleName("pushButtonWithTransparentBackground");
		closeArrowButtonForTab1.setVisible(true);
		closeArrowButtonForTab1.setTitle("Hide");

		openArrowButtonForTab1.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {
				openArrowButtonForTab1.setVisible(false);
				closeArrowButtonForTab1.setVisible(true);
				uploadWidget.setVisible(true);
				if (gridWithOptionsForCreatingSymTraj.isVisible()) {
					gridWithOptionsForCreatingSymTraj.setVisible(true);
				}
			}
		});
		openArrowButtonForTab1.setVisible(false);

		vPanel0.add(openArrowButtonForTab1);

		vPanel0.add(uploadWidget);

		closeArrowButtonForTab1.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {

				openArrowButtonForTab1.setVisible(true);
				closeArrowButtonForTab1.setVisible(false);
				uploadWidget.setVisible(false);
				gridWithOptionsForCreatingSymTraj.setVisible(false);
				vPanel0.setHeight("30px");

			}
		});

		gridWithOptionsForCreatingSymTraj = new Grid(2, 3);
		gridWithOptionsForCreatingSymTraj.setCellSpacing(3);
		gridWithOptionsForCreatingSymTraj.getCellFormatter().setStyleName(0, 2,
				"lastCellInGrid");
		gridWithOptionsForCreatingSymTraj.getCellFormatter().setStyleName(1, 2,
				"lastCellInGrid");

		Label selectOptionForSymTraj = new Label(
				"Select an option for symbolic trajectory:");
		selectOptionForSymTraj.setStyleName("centered");
		selectOptionForSymTraj.setStyleName("labelTextInOneLine");
		gridWithOptionsForCreatingSymTraj.setWidget(0, 0,
				selectOptionForSymTraj);

		/**
		 * comboBox should be visible if gpx file was successfully uploaded
		 * */
		optionsForCreatingSymTraj.addItem("speed mode");
		optionsForCreatingSymTraj.addItem("directions");
		optionsForCreatingSymTraj.addItem("distance");// with personal location
		optionsForCreatingSymTraj.addItem("administrative districts");
		optionsForCreatingSymTraj.setVisibleItemCount(1);
		optionsForCreatingSymTraj.setWidth("130px");

		createSymTrajButton = new Button("Create trajectory");
		createSymTrajButton.setWidth("100px");

		gridWithOptionsForCreatingSymTraj.setWidget(0, 1,
				optionsForCreatingSymTraj);
		gridWithOptionsForCreatingSymTraj.setWidget(1, 1, createSymTrajButton);
		gridWithOptionsForCreatingSymTraj.setVisible(false);

		vPanel0.add(gridWithOptionsForCreatingSymTraj);
		vPanel0.add(closeArrowButtonForTab1);

		optionsTabPanel.add(vPanel0, "Create trajectory");

		final VerticalPanel vPanel2 = new VerticalPanel();
		vPanel2.setStyleName("minHeight");
		vPanel2.setWidth("300px");
		vPanel2.setSpacing(5);

		final FlexTable gridForExistingTrajectory = new FlexTable();

		gridForExistingTrajectory.setWidth("290px");
		gridForExistingTrajectory.setCellSpacing(3);

		Label labelForSelectTrajectory = createLabel("select relation:");
		labelForSelectTrajectory.ensureDebugId("labelForSelectSymbTraj");
		labelForSelectTrajectory.setStyleName("labelTextInOneLine");
		gridForExistingTrajectory.setWidget(0, 0, labelForSelectTrajectory);

		selectOptionsForExistingTrajectories
				.ensureDebugId("listBoxForSelectExistingSymbTraj");
		selectOptionsForExistingTrajectories.setWidth("130px");
		gridForExistingTrajectory.setWidget(0, 1,
				selectOptionsForExistingTrajectories);

		Label labelForSelectModesToDisplayTrajectory = createLabel("select display mode:");
		labelForSelectModesToDisplayTrajectory
				.ensureDebugId("labelForSelectModesToDisplayTrajectory");
		labelForSelectModesToDisplayTrajectory
				.setStyleName("labelTextInOneLine");
		gridForExistingTrajectory.setWidget(1, 0,
				labelForSelectModesToDisplayTrajectory);

		selectOptionsForDisplayMode.addItem(ModesToShowSymTraj.NoModes
				.getValue());
		selectOptionsForDisplayMode.addItem(ModesToShowSymTraj.SHOWwithLabel
				.getValue());
		selectOptionsForDisplayMode.addItem(ModesToShowSymTraj.SHOWwithPopup
				.getValue());
		selectOptionsForDisplayMode.addItem(ModesToShowSymTraj.SHOWwithColor
				.getValue());
		selectOptionsForDisplayMode.setVisibleItemCount(1);
		selectOptionsForDisplayMode.ensureDebugId("listBoxForDisplayMode");
		selectOptionsForDisplayMode.setWidth("130px");
		gridForExistingTrajectory.setWidget(1, 1, selectOptionsForDisplayMode);

		getRelationButton.setWidth("100px");
		gridForExistingTrajectory.setWidget(2, 1, getRelationButton);

//		gridForExistingTrajectory.getFlexCellFormatter().setColSpan(3, 0, 2);
//		gridForExistingTrajectory.getFlexCellFormatter()
//				.setHorizontalAlignment(3, 0,
//						HasHorizontalAlignment.ALIGN_RIGHT);
//		gridForExistingTrajectory.setWidget(3, 0, createAnimationItem());

		final PushButton openArrowButtonForTab2 = new PushButton(closeImage);
		final PushButton closeArrowButtonForTab2 = new PushButton(openImage);
		openArrowButtonForTab2
				.setStyleName("pushButtonWithTransparentBackground");
		closeArrowButtonForTab2
				.setStyleName("pushButtonWithTransparentBackground");

		openArrowButtonForTab2.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {
				openArrowButtonForTab2.setVisible(false);
				gridForExistingTrajectory.setVisible(true);
				panelForPattern.setVisible(true);
				decPanelForStackWithSimpleQueries.setVisible(true);
				closeArrowButtonForTab2.setVisible(true);

			}
		});
		openArrowButtonForTab2.setVisible(false);
		openArrowButtonForTab2.getElement().setAttribute("align", "center");

		vPanel2.add(openArrowButtonForTab2);
		vPanel2.add(gridForExistingTrajectory);

		String animationHeader = getHeaderString("Animation", createPatternIcon());
		stackPanel.add(createAnimationItem(), animationHeader, true);
		String patternHeader = getHeaderString("Pattern queries", createPatternIcon());
		stackPanel.add(createPanelForPattern(), patternHeader, true);
		String simpleQueriesHeader = getHeaderString("Simple queries",
				createPatternIcon());
		simpleQueriesStackPanel= new SimpleQueriesStackPanel();
		simpleQueriesStackPanel.setWidth("100%");
		simpleQueriesStackPanel.ensureDebugId("nestedStack");
		
		
		
		stackPanel.add(simpleQueriesStackPanel, simpleQueriesHeader,
				true);
		stackPanel.getElement().setAttribute("width", "100%");
		

		decPanelForStackWithSimpleQueries.getElement().setAttribute("width",
				"290px");
		decPanelForStackWithSimpleQueries.setWidget(stackPanel);
		vPanel2.add(decPanelForStackWithSimpleQueries);
		

		closeArrowButtonForTab2.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {

				openArrowButtonForTab2.setVisible(true);
				gridForExistingTrajectory.setVisible(false);
				decPanelForStackWithSimpleQueries.setVisible(false);
				closeArrowButtonForTab2.setVisible(false);
				vPanel2.setHeight("30px");

			}
		});
		closeArrowButtonForTab2.getElement().setAttribute("align", "center");
		vPanel2.add(closeArrowButtonForTab2);

		optionsTabPanel.add(vPanel2, "Try trajectory");

		// Return the content
		optionsTabPanel.selectTab(0);
		optionsTabPanel.ensureDebugId("cwTabPanel");
		
	}

	/**
	 * adds pattern components to panel 
	 * @return pattern queries panel
	 */
	private VerticalPanel createPanelForPattern() {
		Label labelForVariable = createLabel("define your pattern");			
		panelForPattern.add(labelForVariable);
		
		HorizontalPanel hpForPattern = createHorizPanelForPattern();
		panelForPattern.add(hpForPattern);

		HorizontalPanel hpForCondition = createHorizPanelForCondition();
		panelForPattern.add(hpForCondition);

		definedPatternWidget= createDefinedPatternWidget();		
		panelForPattern.add(definedPatternWidget);
		
		return panelForPattern;
	}

	/**
	 * creates icon for pattern matching and simple queries panel with specified image and style
	 * @return Image
	 */
	private Image createPatternIcon() {
		Image patternIcon = new Image("resources/images/checked-icon.png");
		patternIcon.setSize("20px", "20px");
		patternIcon.getElement().getStyle().setPadding(5, Unit.PX);
		return patternIcon;
	}

	/**
	 * The method creates HorizontalPanel containing a text box for condition and "add condition" button
	 * @return panel with text box for condition and "add condition" button
	 */
	private HorizontalPanel createHorizPanelForCondition() {
		HorizontalPanel hpForCondition = new HorizontalPanel();
		hpForCondition.setStyleName("elementWithMargin");
		hpForCondition.setSpacing(4);
		
		condition.setWidth("200px");
		hpForCondition.add(condition);
		final Image addConditionButton = new Image(
				"resources/images/plus-green.png");
		addConditionButton.getElement().setAttribute("background",
				"transparent");
		addConditionButton.setTitle("add condition");
		addConditionButton.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {
				
				if (condition.getText().contains("//")) {
					setTextInPatternLabel(condition.getText());
				} else {
					setTextInPatternLabel("//" + condition.getText());
				}
				verifyConditionAndPrintWarningIfNeeded(condition.getText());
				condition.setText("condition");				
				
				if (!patternLabel.getText().equals("")) {
					definedPatternWidget.setVisible(true);
				}

			}
		});
		hpForCondition.add(addConditionButton);
		return hpForCondition;
	}

	private void verifyConditionAndPrintWarningIfNeeded(String text) {
		if(patternLabel.getText().isEmpty()){
			printWarning("Conditions should be specified after pattern. Provide a pattern at first");
		}
		ArrayList<Integer> points=defineIndexOfPoint(text);
		for(int i:points){
			
		if(!variablesForPattern.contains(text.toUpperCase().charAt(i-1)+"")){
			printWarning("Variables in condition should be similar to variables from pattern");
		}	
		
		}
		
		if(!attributeFromUserVerified(text, points)){
			printWarning("Variable can have only special defined attributes");
		}
		if(points.isEmpty()){
			printWarning("Condition should contain \"variable.attribute\"");
		}
		
		
	}
	
	private boolean attributeFromUserVerified(String text,
			ArrayList<Integer> indexes) {
		boolean positivVerified=false;
		int count = 0;
		
		List<Boolean> verified=new ArrayList<Boolean>();
		ArrayList<String> attributes = new ArrayList<String>();
		attributes.add("label");
		attributes.add("time");
		attributes.add("card");
		attributes.add("start");
		attributes.add("end");
		attributes.add("leftclosed");
		attributes.add("rightclosed");		
		for (int i=0; i<indexes.size(); i++) {
			String attrFromUser;
			if (count+1 >= indexes.size()) {
				attrFromUser = text.substring(indexes.get(i)+1, text.length());
			} else {
				count=count+1;
				attrFromUser = text.substring(indexes.get(i)+1, indexes.get(count));
			}				
				
			for(String attr:attributes){
				
				if(attrFromUser.matches("(?i)"+attr+".*")){
					verified.add(i, true);				
					
				}
					
			}
			if(verified.size()==i){
				verified.add(i, false);
			}
				
			 
			
		}
		if(!verified.contains(false)){
			positivVerified=true;
		}
		return positivVerified;

	}

	private ArrayList<Integer> defineIndexOfPoint(String text){
		int index = text.indexOf(".");
		ArrayList<Integer> indexes= new ArrayList<Integer>(); 
		while (index >= 0) {
		    System.out.println(index);
		    indexes.add(index);
		    index = text.indexOf(".", index + 1);
		}
		return indexes;
	}

	/**
	 * Widget contains label with the defined by user pattern (with/without
	 * conditions), a warning label, a label for the result of pattern matching, two buttons:
	 * "match", "remove"
	 * 
	 * @return FlexTable
	 */
	private FlexTable createDefinedPatternWidget() {
		final FlexTable definedPatternWidget = new FlexTable();
		FlexCellFormatter formatter = definedPatternWidget
				.getFlexCellFormatter();

		formatter.setHorizontalAlignment(1, 0,
				HasHorizontalAlignment.ALIGN_RIGHT);
		formatter.setHorizontalAlignment(1, 1,
				HasHorizontalAlignment.ALIGN_RIGHT);
		
		VerticalPanel patternAndResultOfPatternLabels = new VerticalPanel();
		patternLabel.setAutoHorizontalAlignment(HasHorizontalAlignment.ALIGN_JUSTIFY);
		warningLabel.setAutoHorizontalAlignment(HasHorizontalAlignment.ALIGN_JUSTIFY);
		resultOfPatternMatchingLabel.setAutoHorizontalAlignment(HasHorizontalAlignment.ALIGN_JUSTIFY);
		warningLabel.setStyleName("textInRed");
		resultOfPatternMatchingLabel.setStyleName("textInRed");
		
		patternAndResultOfPatternLabels.add(patternLabel);		
		patternAndResultOfPatternLabels.add(warningLabel);
		patternAndResultOfPatternLabels.add(resultOfPatternMatchingLabel);
		
		definedPattern.add(patternAndResultOfPatternLabels);
		definedPatternWidget.setVisible(false);
		definedPattern.setSize("280", "70");
		definedPatternWidget.setWidget(0, 0, definedPattern);
		
		removeButton.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {
				cleanTextInPatternLabel();
				cleanWarningLabel();
				cleanTextInResultOfPatternMatchingLabel();
				definedPatternWidget.setVisible(false);
				variablesForPattern.clear();
			}
		});

		matchButton.getElement().setAttribute("float", "right");
		removeButton.getElement().setAttribute("float", "right");
		
		definedPatternWidget.setWidget(1, 0, matchButton);
		definedPatternWidget.setWidget(1, 1, removeButton);
		return definedPatternWidget;
	}

	/**
	 * cleans a warning label and resets unsuccessfulVerification to false
	 */
	protected void cleanWarningLabel() {
		warningLabel.setText("");
		unsuccessfulVerification=false;
		
	}

	/**
	 * @return
	 */
	private HorizontalPanel createHorizPanelForPattern() {
		final HorizontalPanel hzForPattern = new HorizontalPanel();
		hzForPattern.setSpacing(4);
		checkBoxForVariable.setEnabled(true);
		checkBoxForVariable.addClickHandler(new ClickHandler() {

			@SuppressWarnings("deprecation")
			@Override
			public void onClick(ClickEvent event) {
				if (checkBoxForVariable.isChecked()) {
					textBoxForVariable.setEnabled(true);
				}

			}
		});

		textBoxForVariable.setMaxLength(1);
		textBoxForVariable.setWidth("20px");
		textBoxForVariable.setEnabled(false);

		patternBox.setEnabled(true);
		patternBox.setWidth("167px");
		
		Image addPatternButton = new Image("resources/images/plus.png");
		addPatternButton.getElement().setAttribute("background", "transparent");
		addPatternButton.setTitle("add new pattern part");
		addPatternButton.addClickHandler(new ClickHandler() {

			@SuppressWarnings("deprecation")
			@Override
			public void onClick(ClickEvent event) {
				if (checkBoxForVariable.isChecked()) {
					checkBoxForVariable.setChecked(false);
				}
				if(!textBoxForVariable.getText().equals("variable")){
					setTextInPatternLabel(Character.toUpperCase(textBoxForVariable.getText().charAt(0))+"");					
					
					verifyVariableAndPrintWarningIfNeeded(textBoxForVariable.getText(), patternBox.getText());
					((DefaultTextBox) textBoxForVariable)
					.setDefaultTextAndDisable("variable");
				}				

				if (!patternBox.getText().equals("enter your pattern")) {
					if(patternLabel.getText().contains("//")){
						printWarning("You can not provide pattern after defined condition");
					}else{
						setTextInPatternLabel(patternBox.getText());						
					}
					
					patternBox.setText("enter your pattern");
				}

				if (!patternLabel.getText().equals("")) {
					definedPatternWidget.setVisible(true);
				}
			}
		});

		hzForPattern.add(checkBoxForVariable);
		hzForPattern.add(textBoxForVariable);
		hzForPattern.add(patternBox);
		hzForPattern.add(addPatternButton);
		return hzForPattern;
	}
	
	private void verifyVariableAndPrintWarningIfNeeded(String var,
			String pattern) {
		if(!Character.isLetter(var.charAt(0))){
			printWarning("A variable should be only a letter! Please remove and then provide a new one.");
		}
		if (variablesForPattern.contains(var)) {
			printWarning("All variables occuring in a pattern should be distict! Please remove and then select a new variable");
		}

		if (pattern.replace("enter your pattern", "").isEmpty()) {
			printWarning("A varibale should be always associated with a patter. Please remove and then define a pattern");

		}
		addToListWithVariablesForPattern(var);

	}
	
	

	private void printWarning(String warningText) {
		warningLabel.setText(warningText);	
		unsuccessfulVerification=true;
	}

	/**
	 * creates simple queries stack panel
	 */
	private StackPanel createSimpleQueriesStackPanel() {
		StackPanel simpleQueriesPanel = new StackPanel();
		String passesHeader = getHeaderStringLevel2("passes", queryIcon);
		VerticalPanel passesPanel = new SimpleQueryPanel(
				"Does the trip pass through ...(i.e. specified street, southeast, moderate tempo)?",
				"passes", "i.e. Baker St");
		simpleQueriesPanel.add(passesPanel, passesHeader, true);		

		String atinstantHeader = getHeaderStringLevel2("atinstant", queryIcon);
		VerticalPanel atinstantPanel = new SimpleQueryPanel(
				"Through what does the trip pass at defined time?",
				"atinstant", "i.e. 2012-01-01-01:15");
		simpleQueriesPanel.add(atinstantPanel, atinstantHeader, true);

		String deftimeHeader = getHeaderStringLevel2("deftime", queryIcon);
		VerticalPanel deftimePanel = new SimpleQueryPanel(
				"Determine the time intervals when the trip was at ... (i.e. specified street, southeast, moderate tempo)",
				"deftime", "i.e. Baker St");
		simpleQueriesPanel.add(deftimePanel, deftimeHeader, true);
		return simpleQueriesPanel;
	}

	/**
	 * creates passes panel for simple queries stack
	 */
	private VerticalPanel createPanelForOneQueryFromSimple(
			String textForHelpInfoLabel, String textForButton,
			String defaultText) {
		VerticalPanel queryPanel = new VerticalPanel();
		queryPanel.setSpacing(4);
		Label helpInfoLabel = new Label(textForHelpInfoLabel);
		helpInfoLabel.setStylePrimaryName("labelForPasses");
		queryPanel.add(helpInfoLabel);
		DefaultTextBox labelTextForQuery = new DefaultTextBox(defaultText);
		labelTextForQuery.setWidth("90%");
		queryPanel.add(labelTextForQuery);
		Button queryButton = new Button(textForButton);
		queryButton.setStyleName("floatRight");
		queryPanel.add(queryButton);
		Label resultInfoLabel = new Label();
		queryPanel.add(resultInfoLabel);
		return queryPanel;
	}

	/**
	 * @return drop-dawn list with options to select
	 */
	private ListBox createBoxWithSelectOptionsForExistingTrajectories() {
		ListBox selectOptionsForExistingTrajectories = new ListBox();
		selectOptionsForExistingTrajectories.addItem("");
		selectOptionsForExistingTrajectories.addItem("geotrips_part");
//		selectOptionsForExistingTrajectories.addItem("geotrips");
//		selectOptionsForExistingTrajectories.addItem("geolife");
		selectOptionsForExistingTrajectories.addItem("geolife_part");
//		selectOptionsForExistingTrajectories.addItem("geotrips_part2");
		selectOptionsForExistingTrajectories.setVisibleItemCount(1);
		// selectOptionsForExistingTrajectories.setWidth("150px");
		return selectOptionsForExistingTrajectories;
	}

	private Label createLabel(String textForLabel) {
		Label label = new Label(textForLabel);
		// label.setStyleName("centered");
		 label.setStyleName("labelTextInOneLine");
		return label;
	}

	/**
	 * @return the optionsForCreatingSymTraj
	 */
	public ListBox getOptionsForCreatingSymTraj() {
		return optionsForCreatingSymTraj;
	}

	/**
	 * @return the createSymTrajButton
	 */
	public Button getCreateSymTrajButton() {
		return createSymTrajButton;
	}

	/**
	 * returns the name of uploaded file
	 * 
	 * @return the nameOfUploadedFile
	 */
	public String getNameOfUploadedFile() {
		return this.uploadWidget.getNameOfUploadedFile();
	}

	private VerticalPanel createAnimationItem() {

		animationPanel.setSpacing(4);

		HorizontalPanel playpanel = new HorizontalPanel();
		playpanel.add(playIcon);
		playpanel.add(playLink);

		animationPanel.add(getPanelForPlay());

		animationPanel.add(timeCounter);
		timeCounter.setReadOnly(true);

		animationPanel.add(timeSlider.getMainPanel());

		return animationPanel;
	}

	/**
	 * @return
	 */
	private HorizontalPanel createRewindPanel() {
		HorizontalPanel rewindpanel = new HorizontalPanel();
		rewindpanel.add(rewindIcon);
		rewindpanel.add(rewindLink);
		return rewindpanel;
	}

	/**
	 * @return
	 */
	private HorizontalPanel createForwardPanel() {
		HorizontalPanel forwardpanel = new HorizontalPanel();
		forwardpanel.add(forwardIcon);
		forwardpanel.add(forwardLink);
		return forwardpanel;
	}

	/**
	 * @return
	 */
	private HorizontalPanel createPausePanel() {
		HorizontalPanel pausepanel = new HorizontalPanel();
		pausepanel.add(pauseIcon);
		pausepanel.add(pauseLink);
		return pausepanel;
	}

	/**
	 * @return the uploadWidget
	 */
	public FileUploadPanel getUploadWidget() {
		return uploadWidget;
	}

	public boolean isUnsuccessfulVerification() {
		return unsuccessfulVerification;
	}

	private HorizontalPanel createPlayPanel() {
		HorizontalPanel playpanel = new HorizontalPanel();
		playpanel.add(playIcon);
		playpanel.add(playLink);
		return playpanel;
	}

	/** Resets the animation panel to the default values */
	public void resetAnimationPanel() {

		animationPanel.remove(0);
		animationPanel.insert(panelForPlay, 0);
		timeSlider.resetSlider();
		timeCounter.setText("Press Play to start animation");		
	}

	/**
	 * @return the selectOptionsForDisplayMode
	 */
	public ListBox getSelectOptionsForDisplayMode() {
		return selectOptionsForDisplayMode;
	}

	/**
	 * @return the playLink
	 */
	public Anchor getPlayLink() {
		return playLink;
	}

	/**
	 * @return the animationPanel
	 */
	public VerticalPanel getAnimationPanel() {
		return animationPanel;
	}

	/**
	 * @return the timeSlider
	 */
	public TimeSlider getTimeSlider() {
		return timeSlider;
	}

	/**
	 * @return the timeCounter
	 */
	public TextBox getTimeCounter() {
		return timeCounter;
	}

	/**
	 * @return the pauseLink
	 */
	public Anchor getPauseLink() {
		return pauseLink;
	}

	/**
	 * @return the animateButton
	 */
	public Button getAnimateButton() {
		return getRelationButton;
	}

	/**
	 * @return the forwardLink
	 */
	public Anchor getForwardLink() {
		return forwardLink;
	}

	/**
	 * @return the rewindLink
	 */
	public Anchor getRewindLink() {
		return rewindLink;
	}

	/**
	 * @return the panelForPause
	 */
	public HorizontalPanel getPanelForPause() {
		panelForPause.add(createPausePanel());
		panelForPause.add(createForwardPanel());
		panelForPause.add(createRewindPanel());
		return panelForPause;
	}

	/**
	 * @return the panelForPlay
	 */
	public HorizontalPanel getPanelForPlay() {
		panelForPlay.add(createPlayPanel());
		panelForPlay.add(createForwardPanel());
		panelForPlay.add(createRewindPanel());
		return panelForPlay;
	}

	private void setTextInPatternLabel(String text) {
		if (!text.equals("enter your pattern") && !text.equals("//condition")) {
			if(patternLabel.getText().contains("//")){
				text.replace("//", ",");
			}
			patternLabel.setText(patternLabel.getText() + " " + text);
		}
	}

	private void cleanTextInPatternLabel() {
		patternLabel.setText("");
	}

	public boolean isPatternMatchingIsInitiated() {
		return patternMatchingIsInitiated;
	}

	public void setPatternMatchingIsInitiated(boolean patternMatchingIsInitiated) {
		this.patternMatchingIsInitiated = patternMatchingIsInitiated;
	}

	public void setTextInResultOfPatternMatchingLabel(String text) {
		resultOfPatternMatchingLabel.setText(text);
	}

	private void cleanTextInResultOfPatternMatchingLabel() {
		resultOfPatternMatchingLabel.setText("");
	}

	/**
	 * @param attributeNameOfMlabelInRelation
	 *            the attributeNameOfMpointInRelation to set
	 */
	public void setAttributeNameOfMLabelInRelation(
			String attributeNameOfMlabelInRelation) {
		this.attributeNameOfMlabelInRelation = attributeNameOfMlabelInRelation;
	}

	/**
	 * @param attributeNameOfMPointInRelation
	 */
	public void setAttributeNameOfMPointInRelation(
			String attributeNameOfMPointInRelation) {
		this.attributeNameOfMPointInRelation = attributeNameOfMPointInRelation;
	}

	/**
	 * returns query for pattern matching to be send to secondo; empty query if no relation was selected
	 * @return
	 */
	public String getCommandForPatternMatching() {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command = "";
		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {
			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed filtermatches["
					+ attributeNameOfMlabelInRelation + ",";
			command = command + " '" + patternLabel.getText() + "'] consume";
		}
		return command;
	}
	
	public String getCommandForSimpleQueryPasses(){
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command="";
		String label = simpleQueriesStackPanel.getPassesPanel().getLabelTextForQuery().getText();
		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {
			
			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed extract["
					+ attributeNameOfMlabelInRelation + "] passes tolabel(\""+label+"\")";
		
		}
		
		return command;
	}
	
	public String getCommandForSimpleQueryDeftime(){
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command="";
		String label = simpleQueriesStackPanel.getDeftimePanel().getLabelTextForQuery().getText();
		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {
			
			command = "query deftime ("
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed extract["
					+ attributeNameOfMlabelInRelation + "] at tolabel(\""+label+"\"))";
		
		}
		
		return command;
	}
	
	public String getCommandForSimpleQueryAtinstant(){
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command="";
		String label = simpleQueriesStackPanel.getAtinstantPanel().getLabelTextForQuery().getText();
		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {
			
			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed extract["
					+ attributeNameOfMlabelInRelation + "] atinstant [const instant value\""+label+"\"]";
		
		}
		
		return command;
	}
	
	
	/**
	 * generates command with the coordinates of the last rectangle on the map (draw layer)
	 * @param drawLayer
	 * @return command to be sent to SECONDO
	 */
	public String getCommandForSimpleQueryPassesThroughRegion(Vector drawLayer) {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		System.out.println("Length of "+drawLayer.getFeatures().length);
		removePreviousFeatures(drawLayer);

		Point[] listOfPoints = drawLayer.getFeatures()[drawLayer.getFeatures().length - 1]
				.getGeometry().getVertices(false);

		double[] coordinatesForPasses = coordinateValues(listOfPoints);

		simpleQueriesStackPanel
				.getPassesThroughRegionPanel()
				.getLabelTextForQuery()
				.setText(
						coordinatesForPasses[0] + " " + coordinatesForPasses[1]
								+ " " + coordinatesForPasses[2] + " "
								+ coordinatesForPasses[3]);
		String command = "";

		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {

			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed extract["
					+ attributeNameOfMPointInRelation
					+ "] passes [const rect value(" + coordinatesForPasses[0]
					+ " " + coordinatesForPasses[1] + " "
					+ coordinatesForPasses[2] + " " + coordinatesForPasses[3]
					+ ")]";

		}

		return command;
	}
	
	private void removePreviousFeatures(Vector layer) {
		if (layer.getFeatures().length > 1) {
			System.out.println("Length "+layer.getFeatures().length);
			for (int i = 0; i < layer.getFeatures().length - 2; i++) {
				layer.removeFeature(layer.getFeatures()[i]);
				System.out.println("Feature removed");
			}
		}
	}
	
	private static double [] coordinateValues(Point[] listOfPoints) {		
		Projection OSM_PROJECTION = new Projection(
				"EPSG:4326");
		Projection GEO_PROJECTION = new Projection(
				"EPSG:900913");
		for(Point each:listOfPoints){
			each.transform(GEO_PROJECTION, OSM_PROJECTION);
		}
		
		double maxX = listOfPoints[0].getX();
		double minX=listOfPoints[0].getX();
		double maxY=listOfPoints[0].getY();
		double minY=listOfPoints[0].getY();
		for (int ktr = 0; ktr < listOfPoints.length; ktr++) {
			if (listOfPoints[ktr].getX() > maxX) {
				maxX = listOfPoints[ktr].getX();
			}
			if(listOfPoints[ktr].getX()<minX){
				minX=listOfPoints[ktr].getX();
			}
			if (listOfPoints[ktr].getY() > maxY) {
				maxY = listOfPoints[ktr].getY();
			}
			if(listOfPoints[ktr].getY()<minY){
				minY=listOfPoints[ktr].getY();
			}
			
		}
		double [] result=new double[]{minX,maxX, minY, maxY};
		return result;
	}

	public String getCommandForQueryRelation() {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command = "";
		if (selectedInd != -1) {
			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
		}
		return command;

	}
	
	

	/**
	 * @return the removeButton
	 */
	public Button getMatchButton() {
		return matchButton;
	}

	/**
	 * @return the gridWithOptionsForCreatingSymTraj
	 */
	public Grid getGridWithOptionsForCreatingSymTraj() {
		return gridWithOptionsForCreatingSymTraj;
	}

	/**
	 * @return the selectOptionsForExistingTrajectories
	 */
	public ListBox getSelectOptionsForExistingTrajectories() {
		return selectOptionsForExistingTrajectories;
	}

	/**
	 * Gets a string representation of the header that includes an image and
	 * some text.
	 * 
	 * @param text
	 *            the header text
	 * @param image
	 *            the Image to add next to the header
	 * @return the header as a string
	 */
	private String getHeaderString(String text, Image image) {
		// Add the image and text to a horizontal panel
		HorizontalPanel hPanel = new HorizontalPanel();
		hPanel.setSpacing(0);
		hPanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
		hPanel.add(image);
		HTML headerText = new HTML(text);
		hPanel.add(headerText);

		// Return the HTML string for the panel
		return hPanel.getElement().getString();
	}

	/**
	 * Gets a string representation of the header that includes an image and
	 * some text.
	 * 
	 * @param text
	 *            the header text
	 * @param image
	 *            the Image to add next to the header
	 * @return the header as a string
	 */
	private String getHeaderStringLevel2(String text, Image image) {
		// Add the image and text to a horizontal panel
		HorizontalPanel hPanel = new HorizontalPanel();
		hPanel.setSpacing(0);
		hPanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
		image.setSize("16px", "16px");
		image.setWidth("20px");
		hPanel.add(image);
		HTML headerText = new HTML(text);
		headerText.getElement().setAttribute("float", "left");
		hPanel.add(headerText);
		hPanel.setStyleName("customizedStackPanel");

		// Return the HTML string for the panel
		return hPanel.getElement().getString();
	}
	
	

	private void addToListWithVariablesForPattern(String var) {
		if (!unsuccessfulVerification) {
			variablesForPattern.add(var.toUpperCase());
		}

	}

	public SimpleQueriesStackPanel getSimpleQueriesStackPanel() {
		return simpleQueriesStackPanel;
	}

}
