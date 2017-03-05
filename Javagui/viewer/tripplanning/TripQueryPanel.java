package viewer.tripplanning;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.text.NumberFormat;
import java.util.List;

import javax.swing.BorderFactory;
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
        this.setLayout(new FlowLayout());
        
        JPanel source = new JPanel();
        source.setBorder(BorderFactory.createLineBorder(Color.black));
        source.setLayout(new BoxLayout(source, BoxLayout.Y_AXIS));
        this.add(source);
        JPanel target = new JPanel();
        target.setBorder(BorderFactory.createLineBorder(Color.black));
        target.setLayout(new BoxLayout(target, BoxLayout.Y_AXIS));
        this.add(target);
        JPanel searchPanel = new JPanel();
        searchPanel.setLayout(new FlowLayout());
        this.add(searchPanel);
        
        JLabel label = new JLabel("START");
        source.add(label);

        JLabel streetLabel = new JLabel("street");
        source.add(streetLabel);
        tfStreet = new JTextField(30);
        source.add(tfStreet);
        JLabel noLabel = new JLabel("no.");
        source.add(noLabel);
        tfNo = new JTextField(10);
        source.add(tfNo);
        JLabel plzLabel = new JLabel("postcode");
        source.add(plzLabel);
        tfPlz = new JTextField(10);
        source.add(tfPlz);
        JLabel cityLabel = new JLabel("town/city");
        source.add(cityLabel);
        tfCity = new JTextField(30);
        source.add(tfCity);

        JLabel labelDest = new JLabel("DESTINATION");
        target.add(labelDest);

        JLabel streetLabelDest = new JLabel("street");
        target.add(streetLabelDest);
        tfStreetDest = new JTextField(30);
        target.add(tfStreetDest);
        JLabel noLabelDest = new JLabel("no. ");
        target.add(noLabelDest);
        tfNoDest = new JTextField(10);
        target.add(tfNoDest);
        JLabel plzLabelDest = new JLabel("postcode");
        target.add(plzLabelDest);
        tfPlzDest = new JTextField(10);
        target.add(tfPlzDest);
        JLabel cityLabelDest = new JLabel("town/city");
        target.add(cityLabelDest);
        tfCityDest = new JTextField(30);
        target.add(tfCityDest);
        
        JLabel gradientLabel = new JLabel("slope penalty (>=0)");
        searchPanel.add(gradientLabel);
        tfGradient = new JFormattedTextField(NumberFormat.getNumberInstance());
        //tfGradient.setSize(500, 1000);
        tfGradient.setPreferredSize(new Dimension(40, 20));
        tfGradient.setToolTipText("example: 0 means that the slope will not be considered. 100 means 'I would rather go 99m straight then having a difference in height of 1m.'");
        searchPanel.add(tfGradient);

        searchButton = new JButton(TripplanningViewerController.SEARCH);
        //searchButton.addActionListener(tvController);
        searchPanel.add(searchButton);
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
