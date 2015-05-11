package com.secondo.webgui.client.mainview;

import java.util.ArrayList;
import java.util.List;

import org.gwtopenmaps.openlayers.client.Projection;
import org.gwtopenmaps.openlayers.client.geometry.Point;
import org.gwtopenmaps.openlayers.client.layer.Vector;

import com.google.gwt.dom.client.Element;
import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.dom.client.KeyCodes;
import com.google.gwt.event.dom.client.KeyPressEvent;
import com.google.gwt.event.dom.client.KeyPressHandler;
import com.google.gwt.user.client.Window;
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
import com.google.gwt.user.client.ui.TabBar;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.google.gwt.user.client.ui.PushButton;
import com.secondo.webgui.utils.config.ModesToShowSymTraj;

/**
 * This class represents tab panel with all the functions for creating symbolic
 * trajectory, its visualization, pattern queries and simple queries
 * 
 * @author Irina Russkaya
 *
 */
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
	private Anchor playLink = new Anchor("Play");
	private Anchor forwardLink = new Anchor("Speed Up");
	private Anchor rewindLink = new Anchor("Speed Down");
	private Anchor pauseLink = new Anchor("Pause");

	CheckBox checkBoxForVariable = new CheckBox();

	ArrayList<HorizontalPanel> arrayOfGrids = new ArrayList<HorizontalPanel>();

	// elements for pattern panel
	private Label patternLabel = new Label("");
	private Label warningLabel = new Label("");
	private Label resultOfPatternMatchingLabel = new Label("");
	private ScrollPanel definedPattern = new ScrollPanel();
	private TextBox textBoxForVariable = new DefaultTextBox("variable");
	private TextBox patternBox = new DefaultTextBox("enter your pattern");
	private TextBox condition = new DefaultTextBox("condition");
	final private VerticalPanel panelForPattern = new VerticalPanel();
	private ListBox selectOptionsForDisplayMode = new ListBox();
	private HelpDialog helpDialog;
	private HelpDialog helpDialog2;
	private ListBox selectOptionsForExistingTrajectories = createBoxWithSelectOptionsForExistingTrajectories();
	private Button getRelationButton = new Button("Get relation");
	private Button retrieveButton = new Button("retrieve");
	private Button countButton = new Button("count");
	private Button removeButton = new Button("remove");
	private Button removeLastInPatternButton = new Button("remove last");
	private FlexTable definedPatternWidget;
	private ArrayList<String> variablesForPattern = new ArrayList<String>();
	private boolean unsuccessfulVerification = false;

	private String attributeNameOfMlabelInRelation;
	private String attributeNameOfMPointInRelation;
	private DecoratorPanel decPanelForStackWithSimpleQueries = new DecoratorPanel();
	private SimpleQueriesStackPanel simpleQueriesStackPanel;

	private FlexTable labelForInfoAboutOpenedRelation;
	private Label numberOfTuplesInSampleRelation = new Label("");
	private Label numberOfShownTuplesInSampleRelation = new Label("");
	private HorizontalPanel numberOfTrajectoriesInPatternToShowPanel = new HorizontalPanel();
	private ListBox numberOfTrajectoriesInPatternToShow = new ListBox();

	private boolean patternMatchingIsInitiated = false;
	private boolean simpleQueryForPassesIsInitiated = false;
	private boolean simpleQueryForPassesTrhoughRegionsInitiated = false;

	public OptionsTabPanel() {
		initWidget(optionsTabPanel);

		attributeNameOfMlabelInRelation = "";
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
		openArrowButtonForTab1
				.setStyleName("pushButtonWithTransparentBackground");

		final PushButton closeArrowButtonForTab1 = new PushButton(new Image(
				"resources/images/Action-arrow-blue-up-icon.png"));
		closeArrowButtonForTab1
				.setStyleName("pushButtonWithTransparentBackground");
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
		// HTML labelForFirstTab= new
		// HTML("<label id=\"FirstTab\">Create trajectory</label>");
		// optionsTabPanel.add(vPanel0, labelForFirstTab, true);

		// LabelElement labelOfFirstTab =
		// (LabelElement)Document.get().getElementById("FirstTab");

		final VerticalPanel vPanel2 = new VerticalPanel();
		vPanel2.setStyleName("minHeight");
		vPanel2.setWidth("300px");
		vPanel2.setSpacing(5);

		final FlexTable gridForExistingTrajectory = new FlexTable();

		gridForExistingTrajectory.setWidth("290px");
		gridForExistingTrajectory.setCellSpacing(3);

		Label labelForSelectTrajectory = createLabel("open relation:");
		labelForSelectTrajectory.ensureDebugId("labelForSelectSymbTraj");
		labelForSelectTrajectory.setStyleName("labelTextInOneLine");
		gridForExistingTrajectory.setWidget(0, 0, labelForSelectTrajectory);

		selectOptionsForExistingTrajectories
				.ensureDebugId("listBoxForSelectExistingSymbTraj");
		selectOptionsForExistingTrajectories.setWidth("130px");
		gridForExistingTrajectory.setWidget(0, 1,
				selectOptionsForExistingTrajectories);
		gridForExistingTrajectory.setWidget(0, 2, getHelp());

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
		gridForExistingTrajectory.setWidget(1, 2, getHelp2());

		getRelationButton.setWidth("100px");
		gridForExistingTrajectory.setWidget(2, 1, getRelationButton);

		labelForInfoAboutOpenedRelation = createLabelForInfoAboutOpenedRelation();
		gridForExistingTrajectory.setWidget(3, 0,
				labelForInfoAboutOpenedRelation);

		gridForExistingTrajectory.getFlexCellFormatter().setColSpan(3, 0, 2);
		gridForExistingTrajectory
				.getFlexCellFormatter()
				.setHorizontalAlignment(3, 0, HasHorizontalAlignment.ALIGN_LEFT);
		// gridForExistingTrajectory.setWidget(3, 0, createAnimationItem());

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

		String animationHeader = getHeaderString("Animation",
				createPatternIcon());
		stackPanel.add(createAnimationItem(), animationHeader, true);
		String patternHeader = getHeaderString("Pattern queries",
				createPatternIcon());
		stackPanel.add(createPanelForPattern(), patternHeader, true);
		String simpleQueriesHeader = getHeaderString("Simple queries",
				createPatternIcon());
		simpleQueriesStackPanel = new SimpleQueriesStackPanel();
		simpleQueriesStackPanel.setWidth("100%");
		simpleQueriesStackPanel.ensureDebugId("nestedStack");

		stackPanel.add(simpleQueriesStackPanel, simpleQueriesHeader, true);
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
	 * Creates a label with info about opened relation like how many tuples in
	 * the relation
	 * 
	 * @return The flex table with label
	 */
	private FlexTable createLabelForInfoAboutOpenedRelation() {
		FlexTable labelForInfoAboutOpenedRelation = new FlexTable();

		Label numberOfTuplesInRelationLabel = new Label("The relation has:");
		numberOfTuplesInRelationLabel
				.setStyleName("labelTextInOneLineWithItalic");

		numberOfTuplesInSampleRelation
				.setStyleName("labelTextInOneLineWithItalic");
		numberOfShownTuplesInSampleRelation
				.setStyleName("labelTextInOneLineWithItalic");

		Label numberOfShownTuplesLabel = new Label("On the map shown:");
		numberOfShownTuplesLabel.setStyleName("labelTextInOneLineWithItalic");

		labelForInfoAboutOpenedRelation.setWidget(0, 0,
				numberOfTuplesInRelationLabel);
		labelForInfoAboutOpenedRelation.setWidget(0, 1,
				numberOfTuplesInSampleRelation);
		labelForInfoAboutOpenedRelation.setWidget(1, 0,
				numberOfShownTuplesLabel);
		labelForInfoAboutOpenedRelation.setWidget(1, 1,
				numberOfShownTuplesInSampleRelation);
		labelForInfoAboutOpenedRelation.setVisible(false);

		return labelForInfoAboutOpenedRelation;

	}

	/**
	 * Returns the help functionality to support creating trajectory process
	 * 
	 * @return The image in the form of question mark with the help info
	 */
	private Image getHelp() {
		Image help = new Image("resources/images/question-icon.png");
		help.setWidth("18px");
		help.ensureDebugId("helpButton");
		help.setVisible(true);
		HTML helpInfo = new HTML(
				"<h3>You can load and experiment with the following relations:</h3>"
						+ "<dl>"
						+ "<dt><h4 style=\"color:#009DD8\">Geotrips</h4></dt>"
						+ "<dd>trajectories set with symbolic information containing road names (\"Alte Teichstraﬂe\", \"Grotenbachstraﬂe\")</dd>"
						+ "<dt><h4 style=\"color:#009DD8\">Geolife</h4></dt>"
						+ "<dd>trajectories with transportation modes (\"bus\", \"taxi\", \"subway\") based on a real data set collected in the Geolife project by 182 users during a period of over five years</dd>"
						+"<dt><h4 style=\"color:#009DD8\">Animals</h4></dt>"
						+"<dd>trajectories based on roe deer GPS data with symbolic information representing either a home range (labels H0, H1, and H2), an excursion (label E0), or a stopover (labels S0 and S1).</dd>"
						+ "</dl>");
		helpDialog = new HelpDialog("Sample relations to open", helpInfo);

		help.addClickHandler(new ClickHandler() {
			@Override
			public void onClick(ClickEvent event) {
				helpDialog.getHelpDialogBox().center();
				helpDialog.getHelpDialogBox().show();
			}
		});
		return help;
	}
	
	/**
	 * Returns the help functionality to support visualization for symbolic trajectory
	 * 
	 * @return The image in the form of question mark with the help info
	 */
	private Image getHelp2() {
		Image help = new Image("resources/images/question-icon.png");
		help.setWidth("18px");
		help.ensureDebugId("helpButton");
		help.setVisible(true);
		HTML helpInfo = new HTML(
				"<h3>You can one of three options to visualize a symbolic trajectroy:</h3>"
						+ "<dl>"
						+ "<dt><h4 style=\"color:#009DD8\">Label</h4></dt>"
						+ "<dd>animate trajectory to see that animated point contains an always visible label representing symbolic information</dd>"
						+ "<dt><h4 style=\"color:#009DD8\">Popup</h4></dt>"
						+ "<dd>click on the shown on the map trajectory and popup will appear. To hide the popup click on any place of the map</dd>"
						+"<dt><h4 style=\"color:#009DD8\">Color</h4></dt>"
						+"<dd>each component of symbolic trajectory gets a separate color for display. The meaning of the color notation in \"legend\" (menu)  or in the popup. The left click of the mouse on the trajectory initiates a popup</dd>"
						+ "</dl>");
		helpDialog2 = new HelpDialog("Visualization options", helpInfo);

		help.addClickHandler(new ClickHandler() {
			@Override
			public void onClick(ClickEvent event) {
				helpDialog2.getHelpDialogBox().center();
				helpDialog2.getHelpDialogBox().show();
			}
		});
		return help;
	}

	/**
	 * Adds pattern components to panel
	 * 
	 * @return The pattern queries panel
	 */
	private VerticalPanel createPanelForPattern() {
		Label labelForVariable = createLabel("define your pattern");
		panelForPattern.add(labelForVariable);

		HorizontalPanel hpForPattern = createHorizPanelForPattern();
		panelForPattern.add(hpForPattern);

		HorizontalPanel hpForCondition = createHorizPanelForCondition();
		panelForPattern.add(hpForCondition);

		definedPatternWidget = createDefinedPatternWidget();
		panelForPattern.add(definedPatternWidget);

		return panelForPattern;
	}

	/**
	 * Creates icon for pattern matching and simple queries panel with specified
	 * image and style
	 * 
	 * @return The image
	 */
	private Image createPatternIcon() {
		Image patternIcon = new Image("resources/images/checked-icon.png");
		patternIcon.setSize("20px", "20px");
		patternIcon.getElement().getStyle().setPadding(5, Unit.PX);
		return patternIcon;
	}

	/**
	 * The method creates HorizontalPanel containing a text box for condition
	 * and "add condition" button
	 * 
	 * @return The panel with text box for condition and "add condition" button
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
				verifyAndAddCondition();
			}
		});
		condition.addKeyPressHandler(new KeyPressHandler() {
			@Override
			public void onKeyPress(KeyPressEvent event) {
				boolean enterPressed = KeyCodes.KEY_ENTER == event
						.getNativeEvent().getKeyCode();
				if (enterPressed) {
					verifyAndAddCondition();
				}
			}
		});
		hpForCondition.add(addConditionButton);
		return hpForCondition;
	}

	/**
	 * 
	 */
	private void verifyAndAddCondition() {
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

	/**
	 * Verifies a condition and prints a warning if needed
	 * 
	 * @param The
	 *            condition text
	 */
	private void verifyConditionAndPrintWarningIfNeeded(String text) {
		if (patternLabel.getText().isEmpty()) {
			printWarning("Conditions should be specified after pattern. Provide a pattern at first");
		}
		ArrayList<Integer> points = defineIndexOfPoint(text);
		for (int i : points) {

			if (!variablesForPattern.contains(text.toUpperCase().charAt(i - 1)
					+ "")) {
				printWarning("Variables in condition should be similar to variables from pattern");
			}
		}

		if (!attributeFromUserVerified(text, points)) {
			printWarning("Variable can have only special defined attributes");
		}
		if (points.isEmpty()) {
			printWarning("Condition should contain \"variable.attribute\"");
		}
	}

	/**
	 * Validates the attributes in user defined conditions
	 * 
	 * @param The
	 *            condition text
	 * @param The
	 *            indexes of point to decide where the variable is
	 * @return The result of attribute validation
	 */
	private boolean attributeFromUserVerified(String text,
			ArrayList<Integer> indexes) {
		boolean positivVerified = false;
		int count = 0;

		List<Boolean> verified = new ArrayList<Boolean>();
		ArrayList<String> attributes = new ArrayList<String>();
		attributes.add("label");
		attributes.add("time");
		attributes.add("card");
		attributes.add("start");
		attributes.add("end");
		attributes.add("leftclosed");
		attributes.add("rightclosed");
		for (int i = 0; i < indexes.size(); i++) {
			String attrFromUser;
			if (count + 1 >= indexes.size()) {
				attrFromUser = text
						.substring(indexes.get(i) + 1, text.length());
			} else {
				count = count + 1;
				attrFromUser = text.substring(indexes.get(i) + 1,
						indexes.get(count));
			}

			for (String attr : attributes) {

				if (attrFromUser.matches("(?i)" + attr + ".*")) {
					verified.add(i, true);
				}
			}
			if (verified.size() == i) {
				verified.add(i, false);
			}
		}
		if (!verified.contains(false)) {
			positivVerified = true;
		}
		return positivVerified;
	}

	/**
	 * The service method supporting the verifying of condition
	 * 
	 * @param The
	 *            condition text
	 * @return The index of point dividing variable from attribute
	 */
	private ArrayList<Integer> defineIndexOfPoint(String text) {
		int index = text.indexOf(".");
		ArrayList<Integer> indexes = new ArrayList<Integer>();
		while (index >= 0) {
			indexes.add(index);
			index = text.indexOf(".", index + 1);
		}
		return indexes;
	}

	/**
	 * Widget contains label with the defined by user pattern (with/without
	 * conditions), a warning label, a label for the result of pattern matching,
	 * two buttons: "match", "remove"
	 * 
	 * @return The Flex Table
	 */
	private FlexTable createDefinedPatternWidget() {
		final FlexTable definedPatternWidget = new FlexTable();
		FlexCellFormatter formatter = definedPatternWidget
				.getFlexCellFormatter();

		formatter.setHorizontalAlignment(1, 0,
				HasHorizontalAlignment.ALIGN_RIGHT);
		formatter.setHorizontalAlignment(1, 1,
				HasHorizontalAlignment.ALIGN_RIGHT);
		formatter.setHorizontalAlignment(1, 2,
				HasHorizontalAlignment.ALIGN_RIGHT);
		formatter.setHorizontalAlignment(1, 3,
				HasHorizontalAlignment.ALIGN_RIGHT);

		VerticalPanel patternAndResultOfPatternLabels = new VerticalPanel();
		patternLabel
				.setAutoHorizontalAlignment(HasHorizontalAlignment.ALIGN_JUSTIFY);
		warningLabel
				.setAutoHorizontalAlignment(HasHorizontalAlignment.ALIGN_JUSTIFY);
		resultOfPatternMatchingLabel
				.setAutoHorizontalAlignment(HasHorizontalAlignment.ALIGN_JUSTIFY);
		warningLabel.setStyleName("textInRed");
		resultOfPatternMatchingLabel.setStyleName("textInRed");

		patternAndResultOfPatternLabels.add(patternLabel);
		patternAndResultOfPatternLabels.add(warningLabel);
		patternAndResultOfPatternLabels.add(resultOfPatternMatchingLabel);

		definedPattern.add(patternAndResultOfPatternLabels);
		definedPatternWidget.setVisible(false);
		definedPattern.setSize("280", "70");
		definedPatternWidget.setWidget(0, 0, definedPattern);
		definedPatternWidget.getFlexCellFormatter().setColSpan(0, 0, 3);

		removeButton.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {
				cleanTextInPatternLabel();
				cleanWarningLabel();
				cleanTextInResultOfPatternMatchingLabel();
				definedPatternWidget.setVisible(false);
				numberOfTrajectoriesInPatternToShowPanel.setVisible(false);
				variablesForPattern.clear();
			}
		});

		removeLastInPatternButton.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {
				String currentText = patternLabel.getText();
				if (currentText.length() != 0) {
					String withoutLast = currentText.substring(0,
							currentText.lastIndexOf(" "));
					patternLabel.setText(withoutLast);
				}

				cleanWarningLabel();
			}
		});

		retrieveButton.getElement().setAttribute("float", "right");
		retrieveButton.setStyleName("SpecialWidth");
		countButton.getElement().setAttribute("float", "right");
		countButton.setStyleName("SpecialWidth");
		removeButton.getElement().setAttribute("float", "right");
		removeButton.setStyleName("SpecialWidth");
		removeLastInPatternButton.getElement().setAttribute("float", "right");
		removeLastInPatternButton.setStyleName("SpecialWidth");

		Label numberOfTrajectoriesToShowBeforeLabel = new Label("Show up to ");
		Label numberOfTrajectoriesToShowAfterLabel = new Label(
				"     trajectories");

		numberOfTrajectoriesInPatternToShow.addItem(" ");
		numberOfTrajectoriesInPatternToShow.addItem("3");
		numberOfTrajectoriesInPatternToShow.addItem("5");
		numberOfTrajectoriesInPatternToShow.addItem("7");

		numberOfTrajectoriesInPatternToShowPanel
				.add(numberOfTrajectoriesToShowBeforeLabel);
		numberOfTrajectoriesInPatternToShowPanel
				.add(numberOfTrajectoriesInPatternToShow);
		numberOfTrajectoriesInPatternToShowPanel
				.add(numberOfTrajectoriesToShowAfterLabel);
		numberOfTrajectoriesInPatternToShowPanel.getElement().setAttribute(
				"cellpadding", "5px");
		numberOfTrajectoriesInPatternToShowPanel.getElement().setAttribute(
				"padding-left", "10px");
		numberOfTrajectoriesInPatternToShowPanel.getElement().setAttribute(
				"color", "#808080");

		definedPatternWidget.setWidget(2, 0,
				numberOfTrajectoriesInPatternToShowPanel);
		numberOfTrajectoriesInPatternToShowPanel.setVisible(false);
		definedPatternWidget.getFlexCellFormatter().setColSpan(2, 0, 3);

		retrieveButton.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {
				numberOfTrajectoriesInPatternToShowPanel.setVisible(true);
			}
		});

		definedPatternWidget.setWidget(1, 0, retrieveButton);
		definedPatternWidget.setWidget(1, 1, countButton);
		definedPatternWidget.setWidget(1, 2, removeButton);
		definedPatternWidget.setWidget(1, 3, removeLastInPatternButton);
		return definedPatternWidget;
	}

	/**
	 * Cleans a warning label and resets unsuccessfulVerification to false
	 */
	protected void cleanWarningLabel() {
		warningLabel.setText("");
		unsuccessfulVerification = false;

	}

	/**
	 * Creates the pattern panel for the main tab panel
	 * 
	 * @return The pattern panel
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

		final Image addPatternButton = new Image("resources/images/plus.png");
		addPatternButton.getElement().setAttribute("background", "transparent");
		addPatternButton.setTitle("add new pattern part");
		addPatternButton.addClickHandler(new ClickHandler() {
			@Override
			public void onClick(ClickEvent event) {
				verifyAndAddPattern();
			}
		});

		patternBox.addKeyPressHandler(new KeyPressHandler() {
			@Override
			public void onKeyPress(KeyPressEvent event) {
				boolean enterPressed = KeyCodes.KEY_ENTER == event
						.getNativeEvent().getKeyCode();
				if (enterPressed) {
					verifyAndAddPattern();
				}
			}
		});
		hzForPattern.add(checkBoxForVariable);
		hzForPattern.add(textBoxForVariable);
		hzForPattern.add(patternBox);
		hzForPattern.add(addPatternButton);
		return hzForPattern;
	}

	/**
	 * Verifies a variable and prints a warning if needed
	 * 
	 * @param The
	 *            variable
	 * @param The
	 *            pattern
	 */
	private void verifyVariableAndPrintWarningIfNeeded(String var,
			String pattern) {
		if (!Character.isLetter(var.charAt(0))) {
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

	/**
	 * Verifies pattern and condition and adds them to the visible label
	 */
	@SuppressWarnings("deprecation")
	private void verifyAndAddPattern() {
		if (checkBoxForVariable.isChecked()) {
			checkBoxForVariable.setChecked(false);
		}
		if (!textBoxForVariable.getText().equals("variable")) {
			setTextInPatternLabel(Character.toUpperCase(textBoxForVariable
					.getText().charAt(0)) + "");

			verifyVariableAndPrintWarningIfNeeded(textBoxForVariable.getText(),
					patternBox.getText());
			((DefaultTextBox) textBoxForVariable)
					.setDefaultTextAndDisable("variable");
		}

		if (!patternBox.getText().equals("enter your pattern")) {
			if (patternLabel.getText().contains("//")) {
				printWarning("You can not provide pattern after defined condition");
			} else {
				setTextInPatternLabel(patternBox.getText());
			}
			patternBox.setText("enter your pattern");
		}
		if (!patternLabel.getText().equals("")) {
			definedPatternWidget.setVisible(true);
		}
	}

	/**
	 * Sets the text to the warning label and checks verification as
	 * unsuccessful
	 * 
	 * @param The
	 *            warningText
	 */
	private void printWarning(String warningText) {
		warningLabel.setText(warningText);
		unsuccessfulVerification = true;
	}

	/**
	 * Returns the drop-dawn list with options to select
	 * 
	 * @return The drop-dawn list with options to select
	 */
	private ListBox createBoxWithSelectOptionsForExistingTrajectories() {
		ListBox selectOptionsForExistingTrajectories = new ListBox();
		selectOptionsForExistingTrajectories.addItem("");
		selectOptionsForExistingTrajectories.addItem("geotrips_part");
		selectOptionsForExistingTrajectories.addItem("geotrips");
		selectOptionsForExistingTrajectories.addItem("geolife");
		selectOptionsForExistingTrajectories.addItem("animals");
		selectOptionsForExistingTrajectories.setVisibleItemCount(1);
		return selectOptionsForExistingTrajectories;
	}

	/**
	 * Creates a label
	 * 
	 * @param The
	 *            textForLabel
	 * @return The created label
	 */
	private Label createLabel(String textForLabel) {
		Label label = new Label(textForLabel);
		label.setStyleName("labelTextInOneLine");
		return label;
	}

	/**
	 * Returns the list box with options for creating symbolic trajectory
	 * 
	 * @return the optionsForCreatingSymTraj
	 */
	public ListBox getOptionsForCreatingSymTraj() {
		return optionsForCreatingSymTraj;
	}

	/**
	 * Returns the "create sym traj" button
	 * 
	 * @return The createSymTrajButton
	 */
	public Button getCreateSymTrajButton() {
		return createSymTrajButton;
	}

	/**
	 * Returns the name of uploaded file
	 * 
	 * @return The nameOfUploadedFile
	 */
	public String getNameOfUploadedFile() {
		return this.uploadWidget.getNameOfUploadedFile();
	}

	/**
	 * Creates the animation panel
	 * 
	 * @return The animation panel
	 */
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
	 * Creates the rewind panel
	 * 
	 * @return The rewind panel
	 */
	private HorizontalPanel createRewindPanel() {
		HorizontalPanel rewindpanel = new HorizontalPanel();
		rewindpanel.add(rewindIcon);
		rewindpanel.add(rewindLink);
		return rewindpanel;
	}

	/**
	 * Creates the forward panel
	 * 
	 * @return The forward panel
	 */
	private HorizontalPanel createForwardPanel() {
		HorizontalPanel forwardpanel = new HorizontalPanel();
		forwardpanel.add(forwardIcon);
		forwardpanel.add(forwardLink);
		return forwardpanel;
	}

	/**
	 * Creates a pause panel
	 * 
	 * @return The pause panel
	 */
	private HorizontalPanel createPausePanel() {
		HorizontalPanel pausepanel = new HorizontalPanel();
		pausepanel.add(pauseIcon);
		pausepanel.add(pauseLink);
		return pausepanel;
	}

	/**
	 * Returns the file upload panel
	 * 
	 * @return The upload widget
	 */
	public FileUploadPanel getUploadWidget() {
		return uploadWidget;
	}

	/**
	 * Shows whether Verification of specified pattern with condition was
	 * successful or not
	 * 
	 * @return The variable indicating the result of verification
	 */
	public boolean isUnsuccessfulVerification() {
		return unsuccessfulVerification;
	}

	/**
	 * @return
	 */
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
	 * Returns the selectOptionsForDisplayMode
	 * 
	 * @return the selectOptionsForDisplayMode
	 */
	public ListBox getSelectOptionsForDisplayMode() {
		return selectOptionsForDisplayMode;
	}

	/**
	 * Returns the play link
	 * 
	 * @return the playLink
	 */
	public Anchor getPlayLink() {
		return playLink;
	}

	/**
	 * Returns the panel for animation
	 * 
	 * @return The animationPanel
	 */
	public VerticalPanel getAnimationPanel() {
		return animationPanel;
	}

	/**
	 * Returns the time slider
	 * 
	 * @return The time slider
	 */
	public TimeSlider getTimeSlider() {
		return timeSlider;
	}

	/**
	 * Returns the time counter
	 * 
	 * @return The time counter
	 */
	public TextBox getTimeCounter() {
		return timeCounter;
	}

	/**
	 * Returns the pause link
	 * 
	 * @return The pauseLink
	 */
	public Anchor getPauseLink() {
		return pauseLink;
	}

	/**
	 * Returns the animateButton to start animation
	 * 
	 * @return the animateButton
	 */
	public Button getAnimateButton() {
		return getRelationButton;
	}

	/**
	 * Returns the forward link
	 * 
	 * @return The forwardLink
	 */
	public Anchor getForwardLink() {
		return forwardLink;
	}

	/**
	 * Returns the rewind link
	 * 
	 * @return The rewindLink
	 */
	public Anchor getRewindLink() {
		return rewindLink;
	}

	/**
	 * Returns the panel for pause the animation
	 * 
	 * @return The panelForPause
	 */
	public HorizontalPanel getPanelForPause() {
		panelForPause.add(createPausePanel());
		panelForPause.add(createForwardPanel());
		panelForPause.add(createRewindPanel());
		return panelForPause;
	}

	/**
	 * Returns the panel for play
	 * 
	 * @return The panelForPlay
	 */
	public HorizontalPanel getPanelForPlay() {
		panelForPlay.add(createPlayPanel());
		panelForPlay.add(createForwardPanel());
		panelForPlay.add(createRewindPanel());
		return panelForPlay;
	}

	/**
	 * Sets the text in the label containing defined pattern
	 * 
	 * @param The
	 *            text in the label containing defined pattern
	 */
	private void setTextInPatternLabel(String text) {
		if (!text.equals("enter your pattern") && !text.equals("//condition")) {
			if (patternLabel.getText().contains("//")) {
				text.replace("//", ",");
			}
			patternLabel.setText(patternLabel.getText() + " " + text);
		}
	}

	/**
	 * Cleans the text in the label containing defined pattern
	 */
	private void cleanTextInPatternLabel() {
		patternLabel.setText("");
	}

	/**
	 * Returns the variable indicating whether query for pattern matching is
	 * initiated or not
	 * 
	 * @return The variable indicating whether query for pattern matching is
	 *         initiated or not
	 */
	public boolean isPatternMatchingIsInitiated() {
		return patternMatchingIsInitiated;
	}

	/**
	 * Sets the variable indicating whether query for pattern matching is
	 * initiated or not
	 * 
	 * @param The
	 *            patternMatchingIsInitiated
	 */
	public void setPatternMatchingIsInitiated(boolean patternMatchingIsInitiated) {
		this.patternMatchingIsInitiated = patternMatchingIsInitiated;
	}

	/**
	 * Returns the variable indicating whether query for operator "passes" is
	 * initiated or not
	 * 
	 * @return The variable indicating whether query for operator "passes" is
	 *         initiated or not
	 */
	public boolean isSimpleQueryForPassesIsInitiated() {
		return simpleQueryForPassesIsInitiated;
	}

	/**
	 * Sets the variable indicating whether query for operator "passes" is
	 * initiated or not
	 * 
	 * @param The
	 *            query for operator "passes" is initiated or not
	 */
	public void setSimpleQueryForPassesIsInitiated(
			boolean simpleQueryIsInitiated) {
		this.simpleQueryForPassesIsInitiated = simpleQueryIsInitiated;
	}

	/**
	 * Returns the variable indicating whether query for passes through regions
	 * is initiated or not
	 * 
	 * @return The variable indicating whether query for passes through regions
	 *         is initiated or not
	 */
	public boolean isSimpleQueryForPassesTrhoughRegionsInitiated() {
		return simpleQueryForPassesTrhoughRegionsInitiated;
	}

	/**
	 * Sets the variable indicating whether query for passes through regions is
	 * initiated or not
	 * 
	 * @param The
	 *            simpleQueryForPassesTrhoughRegionsInitiated
	 */
	public void setSimpleQueryForPassesTrhoughRegionsInitiated(
			boolean simpleQueryForPassesTrhoughRegionsInitiated) {
		this.simpleQueryForPassesTrhoughRegionsInitiated = simpleQueryForPassesTrhoughRegionsInitiated;
	}

	/**
	 * Sets the text to the label showing the result of pattern matching
	 * 
	 * @param The
	 *            text to the label
	 */
	public void setTextInResultOfPatternMatchingLabel(String text) {
		resultOfPatternMatchingLabel.setText(text);
	}

	/**
	 * Cleans the label showing the result of pattern matching
	 */
	private void cleanTextInResultOfPatternMatchingLabel() {
		resultOfPatternMatchingLabel.setText("");
	}

	/**
	 * Sets the attribute name of Mlabel in loaded relation (used while building
	 * query to be send to secondo)
	 * 
	 * @param attributeNameOfMlabelInRelation
	 *            The attribute name of Mlabel in loaded relation to set
	 */
	public void setAttributeNameOfMLabelInRelation(
			String attributeNameOfMlabelInRelation) {
		this.attributeNameOfMlabelInRelation = attributeNameOfMlabelInRelation;
	}

	/**
	 * Sets the attribute name of MPoint in loaded relation (used while building
	 * query to be send to secondo)
	 * 
	 * @param attributeNameOfMPointInRelation
	 */
	public void setAttributeNameOfMPointInRelation(
			String attributeNameOfMPointInRelation) {
		this.attributeNameOfMPointInRelation = attributeNameOfMPointInRelation;
	}

	/**
	 * Returns query for pattern matching to be send to secondo; empty query if
	 * no relation was selected
	 * 
	 * @return The command to be sent to SECONDO
	 */
	public String getCommandForPatternMatching() {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String numberOfTrajToShow = getNumberOfTrajectoriesToShow().getValue(
				getNumberOfTrajectoriesToShow().getSelectedIndex());
		String command = "";
		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {
			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed filtermatches["
					+ attributeNameOfMlabelInRelation + ",";
			command = command + " '" + patternLabel.getText() + "'] head["
					+ numberOfTrajToShow + "] consume";
		}
		return command;
	}

	/**
	 * Generates command for counting result of pattern matching
	 * 
	 * @return The command to be sent to SECONDO
	 */
	public String getCommandForCountPatternMatching() {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command = "";
		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {
			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed filtermatches["
					+ attributeNameOfMlabelInRelation + ",";
			command = command + " '" + patternLabel.getText() + "'] count";
		}
		return command;
	}

	/**
	 * Generates command (for retrieving result) containing the operator passes
	 * 
	 * @return The command to be sent to SECONDO
	 */
	public String getCommandForSimpleQueryPasses() {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		ListBox numberOfTrajectoriesToBeShown = simpleQueriesStackPanel
				.getPassesPanel().getNumberOfTrajectoriesToBeShown();
		String numberOfTrajToShow = numberOfTrajectoriesToBeShown
				.getValue(numberOfTrajectoriesToBeShown.getSelectedIndex());
		String command = "";
		String label = simpleQueriesStackPanel.getPassesPanel()
				.getLabelTextForQuery().getText();
		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {

			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed filter[."
					+ attributeNameOfMlabelInRelation + " passes tolabel(\""
					+ label + "\")] head[" + numberOfTrajToShow + "] consume";
		}
		return command;
	}

	/**
	 * Generates command (for counting result) containing the operator passes
	 * 
	 * @return The command to be sent to SECONDO
	 */
	public String getCommandForSimpleQueryPassesCount() {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command = "";
		String label = simpleQueriesStackPanel.getPassesPanel()
				.getLabelTextForQuery().getText();
		if (selectedInd != -1 && !(attributeNameOfMlabelInRelation == null)) {

			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed filter[."
					+ attributeNameOfMlabelInRelation + " passes tolabel(\""
					+ label + "\")] count";

		} else {
			Window.alert("Please select relation and load it");
		}
		return command;
	}

	/**
	 * Generates command containing the operator deftime
	 * 
	 * @return The command to be sent to SECONDO
	 */
	public String getCommandForSimpleQueryDeftime() {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command = "";
		String label = simpleQueriesStackPanel.getDeftimePanel()
				.getLabelTextForQuery().getText();
		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {

			command = "query deftime ("
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed extract["
					+ attributeNameOfMlabelInRelation + "] at tolabel(\""
					+ label + "\"))";

		}

		return command;
	}

	/**
	 * Generates command containing the operator atinstant
	 * 
	 * @return The command to be sent to SECONDO
	 */
	public String getCommandForSimpleQueryAtinstant() {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command = "";
		String label = simpleQueriesStackPanel.getAtinstantPanel()
				.getLabelTextForQuery().getText();
		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {

			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			command = command + " feed extract["
					+ attributeNameOfMlabelInRelation
					+ "] atinstant [const instant value\"" + label + "\"]";
		}
		return command;
	}

	/**
	 * Generates command with the coordinates of the last rectangle on the map
	 * (draw layer)
	 * 
	 * @param The
	 *            draw layer
	 * @return The command to be sent to SECONDO
	 */
	public String getCommandForSimpleQueryPassesThroughRegion(Vector drawLayer,
			String typeOfRequest) {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		System.out.println("Length of " + drawLayer.getFeatures().length);
		removePreviousFeatures(drawLayer);

		Point[] listOfPoints = drawLayer.getFeatures()[drawLayer.getFeatures().length - 1]
				.getGeometry().getVertices(false);

		double[] coordinatesForPasses = coordinateValues(listOfPoints);

		String command = "";
		String commandForCount = "";

		ListBox numberOfTrajectoriesToBeShown = simpleQueriesStackPanel
				.getPassesThroughRegionPanel()
				.getNumberOfTrajectoriesToBeShown();
		String numberOfTrajToShow = numberOfTrajectoriesToBeShown
				.getValue(numberOfTrajectoriesToBeShown.getSelectedIndex());

		if (selectedInd != -1 && !attributeNameOfMlabelInRelation.isEmpty()) {

			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd);
			commandForCount = command + " feed filter[."
					+ attributeNameOfMPointInRelation
					+ " passes [const rect value(" + coordinatesForPasses[0]
					+ " " + coordinatesForPasses[1] + " "
					+ coordinatesForPasses[2] + " " + coordinatesForPasses[3]
					+ ")]] count";

			command = command + " feed filter[."
					+ attributeNameOfMPointInRelation
					+ " passes [const rect value(" + coordinatesForPasses[0]
					+ " " + coordinatesForPasses[1] + " "
					+ coordinatesForPasses[2] + " " + coordinatesForPasses[3]
					+ ")]] head[" + numberOfTrajToShow + "] consume";

		}

		if (typeOfRequest.equals("count")) {
			return commandForCount;
		} else {

			return command;
		}
	}

	/**
	 * Removes all features from the specified layer
	 * 
	 * @param The
	 *            specified layer
	 */
	private void removePreviousFeatures(Vector layer) {
		if (layer.getFeatures().length > 1) {
			System.out.println("Length " + layer.getFeatures().length);
			for (int i = 0; i < layer.getFeatures().length - 2; i++) {
				layer.removeFeature(layer.getFeatures()[i]);
				System.out.println("Feature removed");
			}
		}
	}

	/**
	 * Orders and transforms user defined coordinates
	 * 
	 * @param The
	 *            list of user defined points
	 * @return The ordered and transformed list of user defined coordinates
	 */
	private static double[] coordinateValues(Point[] listOfPoints) {
		Projection OSM_PROJECTION = new Projection("EPSG:4326");
		Projection GEO_PROJECTION = new Projection("EPSG:900913");
		for (Point each : listOfPoints) {
			each.transform(GEO_PROJECTION, OSM_PROJECTION);
		}

		double maxX = listOfPoints[0].getX();
		double minX = listOfPoints[0].getX();
		double maxY = listOfPoints[0].getY();
		double minY = listOfPoints[0].getY();
		for (int ktr = 0; ktr < listOfPoints.length; ktr++) {
			if (listOfPoints[ktr].getX() > maxX) {
				maxX = listOfPoints[ktr].getX();
			}
			if (listOfPoints[ktr].getX() < minX) {
				minX = listOfPoints[ktr].getX();
			}
			if (listOfPoints[ktr].getY() > maxY) {
				maxY = listOfPoints[ktr].getY();
			}
			if (listOfPoints[ktr].getY() < minY) {
				minY = listOfPoints[ktr].getY();
			}
		}

		for (Point each : listOfPoints) {
			each.transform(OSM_PROJECTION, GEO_PROJECTION);
		}
		double[] result = new double[] { minX, maxX, minY, maxY };
		return result;
	}

	/**
	 * Returns the command for querying a sample relation
	 * 
	 * @return The command for querying a sample relation
	 */
	public String getCommandForQuerySampleRelation() {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command = "";
		if (selectedInd != -1) {
			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd) + " feed head[3] consume";
			numberOfShownTuplesInSampleRelation.setText("3 tuples");
		}
		return command;
	}

	/**
	 * Returns the command for count a sample relation
	 * 
	 * @return The command for count a sample relation
	 */
	public String getCommandForCountTuplesInSampleRelation() {
		int selectedInd = selectOptionsForExistingTrajectories
				.getSelectedIndex();
		String command = "";
		if (selectedInd != -1) {
			command = "query "
					+ selectOptionsForExistingTrajectories
							.getItemText(selectedInd) + " count";

		}
		return command;
	}

	/**
	 * Returns the retrieve button
	 * 
	 * @return The retrieve button
	 */
	public Button getRetrieveButton() {
		return retrieveButton;
	}

	/**
	 * Returns the grid with options for creating symbolic trajectory
	 * 
	 * @return The gridWithOptionsForCreatingSymTraj
	 */
	public Grid getGridWithOptionsForCreatingSymTraj() {
		return gridWithOptionsForCreatingSymTraj;
	}

	/**
	 * Returns the list box with list of ExistingTrajectories available for
	 * select
	 * 
	 * @return The selectOptionsForExistingTrajectories
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
	 * Adds a current variable to the list with all variable
	 * 
	 * @param The
	 *            user defined variable for pattern query
	 */
	private void addToListWithVariablesForPattern(String var) {
		if (!unsuccessfulVerification) {
			variablesForPattern.add(var.toUpperCase());
		}
	}

	/**
	 * Returns a stack panel with simple queries
	 * 
	 * @return The stack panel with simple queries
	 */
	public SimpleQueriesStackPanel getSimpleQueriesStackPanel() {
		return simpleQueriesStackPanel;
	}

	/**
	 * Returns the label for info about opened relation
	 * 
	 * @return The label for info about opened relation
	 */
	public FlexTable getLabelForInfoAboutOpenedRelation() {
		return labelForInfoAboutOpenedRelation;
	}

	/**
	 * Returns the number of tuples in sample relation
	 * 
	 * @return The number of tuples in sample relation
	 */
	public Label getNumberOfTuplesInSampleRelation() {
		return numberOfTuplesInSampleRelation;
	}

	/**
	 * Returns the label showing the number of shown on the map trajectories
	 * from the sample relation
	 * 
	 * @return The number of shown on the map trajectories
	 */
	public Label getNumberOfShownTuplesInSampleRelation() {
		return numberOfShownTuplesInSampleRelation;
	}

	/**
	 * Returns the "count" button
	 * 
	 * @return The "count" button
	 */
	public Button getCountButton() {
		return countButton;
	}

	/**
	 * Returns user defined number of trajectories
	 * 
	 * @return The user defined number of trajectories
	 */
	public ListBox getNumberOfTrajectoriesToShow() {
		return numberOfTrajectoriesInPatternToShow;
	}

	/**
	 * Returns the options tab panel object
	 * 
	 * @return The options tab panel
	 */
	public DecoratedTabPanel getOptionsTabPanel() {
		return optionsTabPanel;
	}

	public TabBar getTabBar() {
		return optionsTabPanel.getTabBar();
	}

}
