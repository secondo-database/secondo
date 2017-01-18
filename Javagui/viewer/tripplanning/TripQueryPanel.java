package viewer.tripplanning;

import java.text.NumberFormat;
import java.util.List;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;
import javax.swing.JTextField;

import viewer.update2.gui.RelationPanel;

public class TripQueryPanel extends JPanel{
    /**
     * 
     */
    private static final long serialVersionUID = -7507534934364730928L;
    private String viewerName;
    private JPanel actionPanel;
    private JTabbedPane tabbedPane;
    private List<RelationPanel> relationPanels;
    private JTextField tfStreet;
    private JTextField tfNo;
    private JTextField tfPlz;
    private JTextField tfCity;
    private JTextField tfStreetDest;
    private JTextField tfNoDest;
    private JTextField tfPlzDest;
    private JTextField tfCityDest;
    private JTextField tfGradient;
    private JButton searchButton;

    public TripQueryPanel(String viewerName) {
        super();
        this.viewerName = viewerName;
        this.setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

        JLabel label = new JLabel("START");
        this.add(label);

        JLabel streetLabel = new JLabel("Street");
        this.add(streetLabel);
        tfStreet = new JTextField(50);
        this.add(tfStreet);
        JLabel noLabel = new JLabel("No.");
        this.add(noLabel);
        tfNo = new JTextField(10);
        this.add(tfNo);
        JLabel plzLabel = new JLabel("postcode");
        this.add(plzLabel);
        tfPlz = new JTextField(10);
        this.add(tfPlz);
        JLabel cityLabel = new JLabel("town/city");
        this.add(cityLabel);
        tfCity = new JTextField(50);
        this.add(tfCity);

        JLabel labelDest = new JLabel("DESTINATION");
        this.add(labelDest);

        JLabel streetLabelDest = new JLabel("Street");
        this.add(streetLabelDest);
        tfStreetDest = new JTextField(50);
        this.add(tfStreetDest);
        JLabel noLabelDest = new JLabel("No. ");
        this.add(noLabelDest);
        tfNoDest = new JTextField(10);
        this.add(tfNoDest);
        JLabel plzLabelDest = new JLabel("postcode");
        this.add(plzLabelDest);
        tfPlzDest = new JTextField(10);
        this.add(tfPlzDest);
        JLabel cityLabelDest = new JLabel("town/city");
        this.add(cityLabelDest);
        tfCityDest = new JTextField(50);
        this.add(tfCityDest);
        
        JLabel gradientLabel = new JLabel("gradient weight (>=0)");
        this.add(gradientLabel);
        tfGradient = new JFormattedTextField(NumberFormat.getNumberInstance());
        this.add(tfGradient);

        searchButton = new JButton(TripplanningViewerController.SEARCH);
        //searchButton.addActionListener(tvController);
        this.add(searchButton);
    }

    public String getViewerName() {
        return viewerName;
    }

    public JPanel getActionPanel() {
        return actionPanel;
    }

    public JTabbedPane getTabbedPane() {
        return tabbedPane;
    }

    public List<RelationPanel> getRelationPanels() {
        return relationPanels;
    }

    public JTextField getTfStreet() {
        return tfStreet;
    }

    public JTextField getTfPlz() {
        return tfPlz;
    }

    public JTextField getTfCity() {
        return tfCity;
    }

    public JTextField getTfStreetDest() {
        return tfStreetDest;
    }

    public JTextField getTfPlzDest() {
        return tfPlzDest;
    }

    public JTextField getTfCityDest() {
        return tfCityDest;
    }

    public JTextField getTfGradient() {
        return tfGradient;
    }
    public JTextField getTfNo() {
        return tfNo;
    }
    public JTextField getTfNoDest() {
        return tfNoDest;
    }

    public JButton getSearchButton() {
        return searchButton;
    }
    
}