package com.secondo.webgui.client.mainview;



import java.util.ArrayList;

import com.google.gwt.core.client.GWT;
import com.google.gwt.dom.client.Element;
import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.dom.client.KeyPressEvent;
import com.google.gwt.event.dom.client.KeyPressHandler;
import com.google.gwt.event.logical.shared.SelectionEvent;
import com.google.gwt.event.logical.shared.SelectionHandler;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Anchor;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.CheckBox;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.DecoratedPopupPanel;
import com.google.gwt.user.client.ui.DecoratedTabPanel;
import com.google.gwt.user.client.ui.FileUpload;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.FlexTable.FlexCellFormatter;
import com.google.gwt.user.client.ui.FormPanel;
import com.google.gwt.user.client.ui.FormPanel.SubmitCompleteEvent;
import com.google.gwt.user.client.ui.FormPanel.SubmitEvent;
import com.google.gwt.user.client.ui.ScrollPanel;
import com.google.gwt.user.client.ui.SuggestOracle.Suggestion;
import com.google.gwt.user.client.ui.Grid;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.MultiWordSuggestOracle;
import com.google.gwt.user.client.ui.SuggestBox;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.google.gwt.user.client.ui.PushButton;
import com.secondo.webgui.utils.config.ModesToShowSymTraj;


public class OptionsTabPanel extends Composite{

	private DecoratedTabPanel optionsTabPanel= new DecoratedTabPanel(); 
	Button uploadButton = new Button("Upload file");
	FileUpload fileUpload = new FileUpload();
	FormPanel formPanel = new FormPanel();
	
//	private Button helpButton = new Button("<img src='resources/images/help-icon-16.png' height='25px'/>");
	ListBox optionsForCreatingSymTraj;
	Grid gridWithOptions;
	Grid gridForUploadComponents;
	Button createSymTrajButton;
	TextBox fileUploadBox;
	String nameOfUploadedFile;
	private SecondoSyntaxDialog secondoSyntaxDialog = new SecondoSyntaxDialog();
	private int count;
	
	//elements of the animation panel
    private VerticalPanel animationPanel = new VerticalPanel();
    private HorizontalPanel panelForPlay = new HorizontalPanel();
    private HorizontalPanel panelForPause = new HorizontalPanel();   
	private TextBox timeCounter = new TextBox();
	private TimeSlider timeSlider = new TimeSlider(); 
	private Image animationIcon = new Image("resources/images/play-icon.png");
    private Image playIcon = new Image("resources/images/play-icon2.png");
    private Image forwardIcon = new Image("resources/images/speedup-icon.png");
    private Image rewindIcon = new Image("resources/images/speeddown-icon.png");
    private Image pauseIcon = new Image("resources/images/pause-icon.png");
	private Anchor playLink = new Anchor("Play");
	private Anchor forwardLink = new Anchor("Speed Up");
	private Anchor rewindLink = new Anchor("Speed Down");
	private Anchor pauseLink = new Anchor("Pause");
	
	  CheckBox checkBoxForVariable = new CheckBox();
	 private TextBox textBoxForVariable = new DefaultTextBox("variable");
	 ArrayList<HorizontalPanel> arrayOfGrids = new ArrayList<HorizontalPanel>();
	 SuggestBox optionsForTimeInPattern;
	 SuggestBox optionsForLabelInPattern ;
	 private Label patternLabel = new Label("");
	 private ScrollPanel definedPattern= new ScrollPanel();
	 Pattern pattern = new Pattern();
	 private TextBox patternBox= new DefaultTextBox	("enter your pattern");
	 final private FlexTable flexTable = new FlexTable();
	 final private VerticalPanel panelForPattern = new VerticalPanel();
	 private ListBox optionsWithSequencePattern = new ListBox(false);
	 private ListBox selectOptionsForDisplayMode = new ListBox(); 
	 private ListBox selectOptionsForExistingTrajectories = createBoxWithSelectOptionsForExistingTrajectories();
	 private Button animateButton = new Button("Get relation");
	 final Image addPatternButton = new Image("resources/images/plus.png");
	 private Button matchButton= new Button("match");
	 
	 private String attributeNameOfMpointInRelation="";
	
	
	
	public DecoratedTabPanel getOptionsTabPanel() {
		return optionsTabPanel;
	}

	/**
	 * 
	 */
	public OptionsTabPanel(){	
				
		optionsTabPanel.setWidth("300px");
		optionsTabPanel.getElement().getStyle().setMarginBottom(10.0, Unit.PX);
		optionsTabPanel.setVisible(true);
		optionsTabPanel.setStyleName("maxWidth");
		
		
		
		
		formPanel.setAction(GWT.getModuleBaseURL()+"uploadService");	
		formPanel.setEncoding(FormPanel.ENCODING_MULTIPART);
		formPanel.setMethod(FormPanel.METHOD_POST);
		

		optionsForCreatingSymTraj = new ListBox();		
		
	    
	 
	    
	    final VerticalPanel vPanel0 = new VerticalPanel();  	    
        vPanel0.setStyleName("minHeight");  
        vPanel0.setWidth("300px");
        vPanel0.setSpacing(5);                 
        
        
        
        Image closeImage=new Image("/resources/images/Action-arrow-blue-down-icon.png");
        closeImage.setStyleName("centered");
        Image openImage=new Image("/resources/images/Action-arrow-blue-up-icon.png");
        openImage.setStyleName("centered");
        
        
        final PushButton openArrowButton = new PushButton(new Image("/resources/images/Action-arrow-blue-down-icon.png"));
        openArrowButton.setStyleName("pushButtonWithTransparentBackground"); 
//        openArrowButton.setHeight("20px");
        
        final PushButton closeArrowButton = new PushButton(new Image("/resources/images/Action-arrow-blue-up-icon.png"));
        closeArrowButton.setStyleName("pushButtonWithTransparentBackground");
//        closeArrowButton.setHeight("20px");
        closeArrowButton.setVisible(true);
        
        gridForUploadComponents= new Grid(2,3);
        gridForUploadComponents.ensureDebugId("gridForUploadComponents");
        
//        gridForUploadComponents.getCellFormatter().setStyleName(1,2,"lastCellInGrid");
        
        
        final Label lblSelectAgpx = new Label("Select a .gpx track: ");        
        lblSelectAgpx.setStyleName("labelTextInOneLine");
        
        fileUploadBox= new TextBox();
        fileUploadBox.ensureDebugId("textBoxForUpload");
        fileUploadBox.setWidth("130px");
        fileUploadBox.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {
				fileClick(fileUpload.getElement());				
//				fileUploadBox.setText(fileUpload.getFilename());
//				fileUploadBox.setEnabled(false);
				
			}});
        
      
        
        fileUpload.setName("upload");
        fileUpload.ensureDebugId("fileUploadHidden");
        fileUpload.setVisible(false);
        fileUpload.setStyleName("fileUploadInOneLine");  
        
        
        uploadButton.setWidth("100px");
     // Add a button to upload the file        
        uploadButton.addClickHandler(new ClickHandler() {
          public void onClick(ClickEvent event) { 
        	  System.out.println("File selected "+fileUpload.getFilename());
            	formPanel.submit();              
            
          }
        });
        
        
        
         	
        openArrowButton.addClickHandler(new ClickHandler() {
			
			@Override
			public void onClick(ClickEvent event) {
				openArrowButton.setVisible(false);				
				closeArrowButton.setVisible(true);
				gridForUploadComponents.setVisible(true);	
				if(gridWithOptions.isVisible()){
					gridWithOptions.setVisible(true);
				}
							}
		});
        openArrowButton.setVisible(false);        
        
        vPanel0.add(openArrowButton);   
        
        Image help = new Image("resources/images/help-icon-16.png");
        help.setWidth("18px");
        help.ensureDebugId("helpButton");
        help.setVisible(true);
//        help.setStyleName("transparentButton");
        help.addClickHandler(new ClickHandler() {
			
			@Override
			public void onClick(ClickEvent event) {
				secondoSyntaxDialog.getHelpDialogBox().center();
		        secondoSyntaxDialog.getHelpDialogBox().show();
				
			}
		});
        
        gridForUploadComponents.setCellSpacing(3);
        gridForUploadComponents.setWidget(0, 0, lblSelectAgpx);
        gridForUploadComponents.setWidget(0, 1, fileUploadBox);
        gridForUploadComponents.setWidget(0, 2, help);
        gridForUploadComponents.setWidget(1, 0, fileUpload);
        gridForUploadComponents.setWidget(1, 1, uploadButton);        
        formPanel.add(gridForUploadComponents);
        
        vPanel0.add(formPanel);
        
        closeArrowButton.addClickHandler(new ClickHandler() {
			
			@Override
			public void onClick(ClickEvent event) {
				
				openArrowButton.setVisible(true);				
				closeArrowButton.setVisible(false);
				gridForUploadComponents.setVisible(false);
				gridWithOptions.setVisible(false);
				vPanel0.setHeight("30px");				

				
				
			}
		});
        
        
     // This event is fired just before the form is submitted. We can take
        // this opportunity to perform validation.
        formPanel.addSubmitHandler(new FormPanel.SubmitHandler() {
          

		@Override
		public void onSubmit(SubmitEvent event) {
			String filename = fileUpload.getFilename();
            if (filename.length() == 0) {
              Window.alert("Please select a file");
              event.cancel();
            }
            if(!filename.endsWith(".gpx")){
            	Window.alert("You can upload only gpx file");
            	event.cancel();
            }			
			
		}
        });
        
        formPanel.addSubmitCompleteHandler(new FormPanel.SubmitCompleteHandler() {
    		

			@Override
			public void onSubmitComplete(SubmitCompleteEvent event) {
					if(event.getResults().contains("File name:")) {
						nameOfUploadedFile=event.getResults().substring(event.getResults().lastIndexOf("File name:"));
					}			
				gridWithOptions.setVisible(true);
				
			}
    	});
        
        
        gridWithOptions = new Grid(2,3);
        gridWithOptions.setCellSpacing(3);
        gridWithOptions.getCellFormatter().setStyleName(0,2,"lastCellInGrid");
        gridWithOptions.getCellFormatter().setStyleName(1,2,"lastCellInGrid");       
        
        Label selectOptionForSymTraj = new Label("Select an option for symbolic trajectory:");        
        selectOptionForSymTraj.setStyleName("centered");
        selectOptionForSymTraj.setStyleName("labelTextInOneLine");       
        gridWithOptions.setWidget(0,0, selectOptionForSymTraj);
        
        /**
         * comboBox should be visible if gpx file was successfully uploaded
         * */        
        optionsForCreatingSymTraj.addItem("speed mode");
        optionsForCreatingSymTraj.addItem("directions");
        optionsForCreatingSymTraj.addItem("personal location");//operator geocode
        optionsForCreatingSymTraj.setVisibleItemCount(1);   
        optionsForCreatingSymTraj.setWidth("130px");
        
        
        createSymTrajButton= new Button("Create trajectory");
        createSymTrajButton.setWidth("100px");
        
        gridWithOptions.setWidget(0, 2, help);
        gridWithOptions.setWidget(0, 1, optionsForCreatingSymTraj);
        gridWithOptions.setWidget(1, 1, createSymTrajButton);
        gridWithOptions.setVisible(false);
        
        
        vPanel0.add(gridWithOptions);  
        vPanel0.add(closeArrowButton);
        
        optionsTabPanel.add(vPanel0, "Create trajectory");
        
	    
	    final VerticalPanel vPanel2 = new VerticalPanel();
	    vPanel2.setStyleName("minHeight");
	    vPanel2.setWidth("300px");
        vPanel2.setSpacing(5); 
        
        final FlexTable gridForExistingTrajectory= new FlexTable();
        
        gridForExistingTrajectory.setWidth("290px");
        gridForExistingTrajectory.setCellSpacing(3);
        
        Label labelForSelectTrajectory = createLabel("select relation:");
        labelForSelectTrajectory.ensureDebugId("labelForSelectSymbTraj");
        labelForSelectTrajectory.setStyleName("labelTextInOneLine");
        gridForExistingTrajectory.setWidget(0,0,labelForSelectTrajectory);
        
        
        selectOptionsForExistingTrajectories.ensureDebugId("listBoxForSelectExistingSymbTraj");
        selectOptionsForExistingTrajectories.setWidth("130px");
        gridForExistingTrajectory.setWidget(0, 1, selectOptionsForExistingTrajectories);
        
        Label labelForSelectModesToDisplayTrajectory = createLabel("select display mode:");
        labelForSelectModesToDisplayTrajectory.ensureDebugId("labelForSelectModesToDisplayTrajectory");
        labelForSelectModesToDisplayTrajectory.setStyleName("labelTextInOneLine");
        gridForExistingTrajectory.setWidget(1,0,labelForSelectModesToDisplayTrajectory);
        
               
        selectOptionsForDisplayMode.addItem(ModesToShowSymTraj.NoModes.getValue());
        selectOptionsForDisplayMode.addItem(ModesToShowSymTraj.SHOWwithLabel.getValue());
        selectOptionsForDisplayMode.addItem(ModesToShowSymTraj.SHOWwithPopup.getValue());
        selectOptionsForDisplayMode.addItem(ModesToShowSymTraj.SHOWwithColor.getValue());
        selectOptionsForDisplayMode.setVisibleItemCount(1);
        selectOptionsForDisplayMode.ensureDebugId("listBoxForDisplayMode");
        selectOptionsForDisplayMode.setWidth("130px");
        gridForExistingTrajectory.setWidget(1, 1, selectOptionsForDisplayMode);
        
        
        
        animateButton.setWidth("100px");
        gridForExistingTrajectory.setWidget(2, 1, animateButton);
        
        gridForExistingTrajectory.getFlexCellFormatter().setColSpan(3, 0, 2);
        gridForExistingTrajectory.getFlexCellFormatter().setHorizontalAlignment(3, 0, HasHorizontalAlignment.ALIGN_RIGHT);
        gridForExistingTrajectory.setWidget(3, 0, createAnimationItem());
        
        
        
        
        
        final PushButton openArrowButtonForTab2 = new PushButton(closeImage);
        final PushButton closeArrowButtonForTab2 = new PushButton(openImage);
        openArrowButtonForTab2.setStyleName("pushButtonWithTransparentBackground"); 
        closeArrowButtonForTab2.setStyleName("pushButtonWithTransparentBackground");
        
        openArrowButtonForTab2.addClickHandler(new ClickHandler() {
			
			@Override
			public void onClick(ClickEvent event) {
//				optionsTabPanel.remove(vPanelForOpening);
//				optionsTabPanel.add(vPanel0, "Create trajectory");
				openArrowButtonForTab2.setVisible(false);
				gridForExistingTrajectory.setVisible(true);
//				flexTable.setVisible(true);
				panelForPattern.setVisible(true);				
				
				closeArrowButtonForTab2.setVisible(true);
				
				
			}
		});
        openArrowButtonForTab2.setVisible(false);
        openArrowButtonForTab2.getElement().setAttribute("align", "center");
        
        vPanel2.add(openArrowButtonForTab2);
        vPanel2.add(gridForExistingTrajectory);
        
        
        
        flexTable.ensureDebugId("tableForPattern");
        flexTable.addStyleName("tableWithFixedWidth");        
        flexTable.setWidth("290px");
        panelForPattern.setWidth("290px");
        panelForPattern.setSpacing(4);
        
        FlexCellFormatter cellFormatter = flexTable.getFlexCellFormatter();
                
        flexTable.setCellSpacing(5);
        flexTable.setCellPadding(0);
        cellFormatter.setHorizontalAlignment(
        	      0, 0, HasHorizontalAlignment.ALIGN_LEFT); 
        cellFormatter.setHorizontalAlignment(
      	      0, 2, HasHorizontalAlignment.ALIGN_LEFT);
        
        
//        flexTable.getColumnFormatter().setWidth(0, "30px");
//        flexTable.getColumnFormatter().setWidth(1, "25px");
    
        flexTable.getFlexCellFormatter().setColSpan(0, 0, 4);
        flexTable.getFlexCellFormatter().setColSpan(1, 0, 2);
        flexTable.getFlexCellFormatter().setColSpan(2, 0, 3);
        flexTable.getFlexCellFormatter().setColSpan(3, 0, 4);
        
        Label labelForVariable= createLabel("Define your pattern");
        labelForVariable.setStyleName("labelTextInOneLine");
//        Label labelForTime = createLabel("time");
//        Label labelForLabel = createLabel("label");
//        Label labelForSequence = createLabel("sequence");
        flexTable.setWidget(0, 0, labelForVariable);   
        panelForPattern.add(labelForVariable);
        
//        flexTable.setWidget(0, 2, labelForTime);
//        flexTable.setWidget(0, 3, labelForLabel);
//        flexTable.setWidget(0, 4, labelForSequence);
        
                
        addPatternButton.getElement().setAttribute("background", "transparent");
        addPatternButton.setTitle("add new pattern part");
       
        
        final Image addConditionButton = new Image("resources/images/plus-green.png");
        addConditionButton.getElement().setAttribute("background", "transparent");
        addConditionButton.setTitle("add condition");
        
        
        
        
//        HorizontalPanel gridForInputValuesForPattern = createGridForInputValuesForPattern();
//        arrayOfGrids.add(gridForInputValuesForPattern);
       
       
       
//       createGridForInputValuesForPattern();
       
//       flexTable.setWidget(1, 0, gridForInputValuesForPattern);    
         
        
        
       flexTable.setWidget(1, 3, addPatternButton);     
       flexTable.setWidget(2, 3, addConditionButton);
       
       final DefaultTextBox condition = new DefaultTextBox("condition");
       condition.setWidth("225px");

       flexTable.setWidget(2,0, condition);
       
       final HorizontalPanel hzForPattern = new HorizontalPanel();  
       hzForPattern.setSpacing(4);
       checkBoxForVariable.setEnabled(true);        
       checkBoxForVariable.addClickHandler(new ClickHandler() {
			
			@SuppressWarnings("deprecation")
			@Override
			public void onClick(ClickEvent event) {
				if(checkBoxForVariable.isChecked()){
					textBoxForVariable.setEnabled(true);
				}
				
			}
		});
       
       textBoxForVariable.setMaxLength(1);        
       textBoxForVariable.setWidth("50px");
       textBoxForVariable.setEnabled(false);
       
       patternBox.setEnabled(true);      
       patternBox.setWidth("160px");
       
       hzForPattern.add(checkBoxForVariable);
       hzForPattern.add(textBoxForVariable);
       hzForPattern.add(patternBox);       
       hzForPattern.add(addPatternButton);
//       flexTable.setWidget(1, 0, panelForVariable);
       panelForPattern.add(hzForPattern);
       
       HorizontalPanel hpForCondition= new HorizontalPanel(); 
       hpForCondition.setStyleName("elementWithMargin");
       hpForCondition.setSpacing(4);
       hpForCondition.add(condition);       
       hpForCondition.add(addConditionButton);
       panelForPattern.add(hpForCondition);
       
       addConditionButton.addClickHandler(new ClickHandler() {
		
		@Override
		public void onClick(ClickEvent event) {
			setTextInPatternLabel(condition.getText());
			condition.setText("");
			
			
		}
	});
       
       
       final FlexTable definedPatternWidget = new FlexTable(); 
       FlexCellFormatter formatter = definedPatternWidget.getFlexCellFormatter();
       
       formatter.setHorizontalAlignment(
    	        1, 0, HasHorizontalAlignment.ALIGN_RIGHT);
       definedPattern.add(patternLabel);
       definedPatternWidget.setVisible(false);
       definedPattern.setSize("280", "70");
       definedPatternWidget.setWidget(0, 0, definedPattern);
       
       
       definedPatternWidget.setWidget(1, 0, matchButton);
       flexTable.setWidget(3, 0, definedPatternWidget);
       panelForPattern.add(definedPatternWidget);
       
//       setCount(getCount());
       
       addPatternButton.addClickHandler(new ClickHandler() {
		
		@SuppressWarnings("deprecation")
		@Override
		public void onClick(ClickEvent event) {
			if(checkBoxForVariable.isChecked()){
				checkBoxForVariable.setChecked(false);
			}
			setTextInPatternLabel(textBoxForVariable.getText());			
			((DefaultTextBox) textBoxForVariable).setDefaultTextAndDisable("variable");
			
			setTextInPatternLabel(patternBox.getText());			
			patternBox.setText("enter your pattern");
			
			if(!patternLabel.getText().equals("")){
				definedPatternWidget.setVisible(true);
			}
			
//			optionsForTimeInPattern.setValue("");
//			optionsForLabelInPattern.setValue("");
//			optionsWithSequencePattern.setSelectedIndex(0);
			
			
		}
	});
       
       
       
       
//       vPanel2.add(flexTable);
       vPanel2.add(panelForPattern);
       
       
        
        closeArrowButtonForTab2.addClickHandler(new ClickHandler() {
			
			@Override
			public void onClick(ClickEvent event) {
				
				openArrowButtonForTab2.setVisible(true);
				gridForExistingTrajectory.setVisible(false);
//				flexTable.setVisible(false);
				panelForPattern.setVisible(false);
				patternLabel.setText("");
				definedPattern.setVisible(false);
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
		
		
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																														
		
//		TabPanel tabPanel = new TabPanel();
//        tabPanel.setResizeTabs(true);        
//        tabPanel.setAnimScroll(true);
//        tabPanel.setWidth(450);  
//        tabPanel.setAutoHeight(true);  
//        tabPanel.setBodyBorder(true);
//       
//        
//        
//        TabItem tbtmCreateTrajectory = new TabItem("Create trajectory"); 
//        
//        Grid grid = new Grid(2, 2);        
//        grid.setCellPadding(10);
//        tbtmCreateTrajectory.add(grid);
//        grid.setSize("200px", "200px");
//        tbtmCreateTrajectory.setStyleAttribute("padding", "10px");
//        
//        
//        
//        Label lblUploadFile = new Label("Upload file:");       
//        lblUploadFile.setStyleName("uploadLabel", true);
//        grid.setWidget(0, 0, lblUploadFile);
//        lblUploadFile.setWidth("113px");
//        
//        FileUpload fileUpload = new FileUpload();        
//        grid.setWidget(0, 1, fileUpload);       
//        
//        Label lblSelectTypeOf = new Label("select type of symbolic trajectory");
//        grid.setWidget(1, 0, lblSelectTypeOf);
//        
//        ListBox comboBox = new ListBox();
//        comboBox.addItem("speed mode");
//        comboBox.addItem("directions");
//        comboBox.addItem("transportation mode");
//        comboBox.setVisibleItemCount(3);
//        
//        grid.setWidget(1, 1, comboBox);
//                      
//                
//        tabPanel.add(tbtmCreateTrajectory);
//        
//        TabItem personal = new TabItem();
//        personal.setStyleAttribute("padding", "10px");
//        personal.setText("Personal Details");
//        personal.setLayout(new FormLayout());
//        
//
//        TextField<String> name = new TextField<String>();
//        name.setFieldLabel("First Name");
//        name.setValue("Darrell");
//        personal.add(name, new FormData("100%"));
//
//        
//        tabPanel.add(personal);
//        
//        
//        
//        TabItem tbtmTryTrajectory = new TabItem("Try trajectory");
//        tbtmTryTrajectory.setLayout(new FillLayout(Orientation.HORIZONTAL));
//        
//        tabPanel.add(tbtmTryTrajectory);        
        
		
	}

	/**
	 * @return
	 */
	private void createGridForInputValuesForPattern() {
//		HorizontalPanel panelForInputValuesForPattern = new HorizontalPanel();
//		panelForInputValuesForPattern.setWidth("300px");
//		final PatternNode node =pattern.new PatternNode();
		
		HorizontalPanel panelForVariable = new HorizontalPanel();		
        checkBoxForVariable.setEnabled(true);        
        checkBoxForVariable.addClickHandler(new ClickHandler() {
			
			@SuppressWarnings("deprecation")
			@Override
			public void onClick(ClickEvent event) {
				if(checkBoxForVariable.isChecked()){
					textBoxForVariable.setEnabled(true);
				}
				
			}
		});
        
        textBoxForVariable.setMaxLength(1);        
        textBoxForVariable.setWidth("50px");
        textBoxForVariable.setEnabled(false);
        
        panelForVariable.add(checkBoxForVariable);
        panelForVariable.add(textBoxForVariable);
        panelForVariable.add(addPatternButton);
//        flexTable.setWidget(1, 0, panelForVariable);
        panelForPattern.add(panelForVariable);
        
//        panelForInputValuesForPattern.add(checkBoxForVariable);        
        
        
        
//        panelForInputValuesForPattern.add(textBoxForVariable);        
        
        final DecoratedPopupPanel simplePopup = new DecoratedPopupPanel(true);
        simplePopup.ensureDebugId("cwBasicPopup-simplePopup");
        simplePopup.setStyleName("triangle-right");        
        simplePopup.setWidth("100px");
        simplePopup.setWidget(new HTML("<div>Allowed only alphabetic inputs</div>"));
        
        textBoxForVariable.addKeyPressHandler(new KeyPressHandler() {
			
			@Override
			public void onKeyPress(KeyPressEvent event) {
				TextBox source = (TextBox) event.getSource();
			        if (!Character.isLetter(event.getCharCode())) {
			        	
			        	source.cancelKey();
			        	int left = source.getAbsoluteLeft() + 10;
			            int top = source.getAbsoluteTop() + 10;
			            simplePopup.setPopupPosition(left, top);
			           
			            simplePopup.show();
			            source.setEnabled(true);
			        }			       
			      
				
			}
		});
        
        
        patternBox.setEnabled(true);
        flexTable.setWidget(1,2,  patternBox);
        
       
       
       MultiWordSuggestOracle oracle = new MultiWordSuggestOracle();
       oracle.add("_");
       oracle.add("2014-02-01");
       optionsForTimeInPattern = new SuggestBox(oracle);
       optionsForTimeInPattern.setWidth("40px");
       optionsForTimeInPattern.isAutoSelectEnabled();
       optionsForTimeInPattern.addSelectionHandler(new SelectionHandler<MultiWordSuggestOracle.Suggestion>() {

		@Override
		public void onSelection(SelectionEvent<Suggestion> event) {
//			node.getTimeOrLabel().add(event.getSelectedItem().toString());
			patternLabel.setText(patternLabel.getText()+" "+event.getSelectedItem().toString());
			
		}

        

		
       });
       
//   flexTable.setWidget(1, 2, optionsForTimeInPattern);
       
       optionsForLabelInPattern = new SuggestBox(oracle);
       optionsForLabelInPattern.setWidth("40px");
       optionsForLabelInPattern.isAutoSelectEnabled();
       optionsForLabelInPattern.addSelectionHandler(new SelectionHandler<MultiWordSuggestOracle.Suggestion>() {

		@Override
		public void onSelection(SelectionEvent<Suggestion> event) {
//			node.getTimeOrLabel().add(event.getSelectedItem().toString());
			patternLabel.setText(patternLabel.getText()+" "+event.getSelectedItem().toString());
			
		}
	});
       
//       flexTable.setWidget(1, 3, optionsForLabelInPattern);
       
//       panelForInputValuesForPattern.add(optionsForTimeInPattern);
//       panelForInputValuesForPattern.add(optionsForLabelInPattern);
       
       
       
       optionsWithSequencePattern.setSelectedIndex(0);
       optionsWithSequencePattern.setWidth("40px");
       optionsWithSequencePattern.setVisibleItemCount(1);
       optionsWithSequencePattern.addItem(" ");
       optionsWithSequencePattern.addItem("*");
       optionsWithSequencePattern.addItem("+");
       optionsWithSequencePattern.addItem("[]");
       optionsWithSequencePattern.addItem("|");
       optionsWithSequencePattern.addItem("[]+");
       optionsWithSequencePattern.addItem("[]*");
       optionsWithSequencePattern.addItem("[]?");
       
       optionsWithSequencePattern.addClickHandler(new ClickHandler() {
		
		@Override
		public void onClick(ClickEvent event) {
			ListBox box=(ListBox)event.getSource();
			int index= box.getSelectedIndex();
//			node.getSequencePattern().add(box.getValue(index));
			patternLabel.setText(patternLabel.getText()+" "+box.getValue(index));
			
		}
	});
//       flexTable.setWidget(1, 4, optionsWithSequencePattern);
       
//       panelForInputValuesForPattern.add(optionsWithSequencePattern);
		
	}
	
	

	/**
	 * @return drop-dawn list with options to select
	 */
	private ListBox createBoxWithSelectOptionsForExistingTrajectories() {
		ListBox selectOptionsForExistingTrajectories = new ListBox();
        selectOptionsForExistingTrajectories.addItem("geotrips_part");
        selectOptionsForExistingTrajectories.addItem("geolife_part");
        selectOptionsForExistingTrajectories.setVisibleItemCount(1);
//        selectOptionsForExistingTrajectories.setWidth("150px");
		return selectOptionsForExistingTrajectories;
	}

	private Label createLabel(String textForLabel) {
		Label label = new Label(textForLabel);
//        label.setStyleName("centered");
//        label.setStyleName("labelTextInOneLine");
		return label;
	}

	/**
	 * @return the uploadButton
	 */
	public Button getUploadButton() {
		return uploadButton;
	}

	/**
	 * @return the fileUpload
	 */
	public FileUpload getFileUpload() {
		return fileUpload;
	}

	/**
	 * @return the formPanel
	 */
	public FormPanel getFormPanel() {
		return formPanel;
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
	 * @return the nameOfUploadedFile
	 */
	public String getNameOfUploadedFile() {
		return nameOfUploadedFile;
	}
	
	
	private VerticalPanel createAnimationItem() {

	    animationPanel.setSpacing(4);
	    
	    HorizontalPanel playpanel= new HorizontalPanel();
        playpanel.add(playIcon);
        playpanel.add(playLink);     
	
	    HorizontalPanel forwardpanel = new HorizontalPanel();
        forwardpanel.add(forwardIcon);
        forwardpanel.add(forwardLink);       
        
	    
	    HorizontalPanel rewindpanel = new HorizontalPanel();
        rewindpanel.add(rewindIcon);
        rewindpanel.add(rewindLink);       
        
	    
        HorizontalPanel pausepanel = new HorizontalPanel();
        pausepanel.add(pauseIcon);
        pausepanel.add(pauseLink); 
        panelForPause.add(pausepanel);
        panelForPause.add(forwardpanel);
        panelForPause.add(rewindpanel);
        
        panelForPlay.add(playpanel);
        panelForPlay.add(forwardpanel);
        panelForPlay.add(rewindpanel);
        
        animationPanel.add(panelForPlay);
	    
	    animationPanel.add(timeCounter);
	    
	    animationPanel.add(timeSlider.getMainPanel());

	    return animationPanel;
	  }
	
	/**Resets the animation panel to the default values*/
	public void resetAnimationPanel(){
		
		animationPanel.remove(0);
		animationPanel.insert(panelForPlay, 0);
		timeSlider.resetSlider();
		timeCounter.setText("Press Play to start animation");		
	}

	
	


	private static native void fileClick(Element el) /*-{
		
//    Element el=document.getElementById("gwt-debug-fileUploadHidden");
//	window.alert("On input"+el.value);
//    document.getElementById("gwt-debug-textBoxForUpload").value=el.value; 
//    el.addEventListener('input',handleInput,false); 
    el.click();
    $doc.getElementById("gwt-debug-textBoxForUpload").value=el.value; 
      
    

}-*/;

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
		return animateButton;
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
		return panelForPause;
	}

	/**
	 * @return the panelForPlay
	 */
	public HorizontalPanel getPanelForPlay() {
		return panelForPlay;
	}

	private void setTextInPatternLabel(String text){
		if (!text.equals("variable") && !text.equals("enter your pattern")) {
			
			patternLabel.setText(patternLabel.getText() + " " + text);
		}
	}

	

	/**
	 * @return the matchButton
	 */
	public Button getMatchButton() {
		return matchButton;
	}

	

	/**
	 * @param attributeNameOfMpointInRelation the attributeNameOfMpointInRelation to set
	 */
	public void setAttributeNameOfMpointInRelation(
			String attributeNameOfMpointInRelation) {
		this.attributeNameOfMpointInRelation = attributeNameOfMpointInRelation;
	}
	
	public String getCommandForPatternMatching(){
		int selectedInd=selectOptionsForExistingTrajectories.getSelectedIndex();
		String command="";
		if(selectedInd!=-1){
		command="query "+selectOptionsForExistingTrajectories.getItemText(selectedInd);
		command = command+" feed filtermatches["+ attributeNameOfMpointInRelation+",";
		command=command+" '"+ patternLabel.getText()+"'] consume";
		}
		return command;
	}
	
	public String getCommandForQueryRelation(){
		int selectedInd=selectOptionsForExistingTrajectories.getSelectedIndex();
		String command="";
		if(selectedInd!=-1){
			command="query "+selectOptionsForExistingTrajectories.getItemText(selectedInd);	
		}
		return command;
		
	}
	
	

	
	
}
