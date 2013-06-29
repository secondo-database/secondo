package PSEditor;

import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;

import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JTable;
import javax.swing.plaf.basic.BasicTableUI;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellEditor;

public class ParaTable extends JTable {

	private DefaultTableModel model;
	
	public ParaTable(){
		super();
		String[] header = {"Name","Value","Default", "Type", "Editable"};
		Object[][] data = {};
		model = new DefaultTableModel(data, header);

		setModel(model);
		setDefaultRenderer(Object.class, new ParaTableRenderer());
		
		//Try to make the name column show the complete parameter name
		getColumnModel().getColumn(0).setMaxWidth(350);
		getColumnModel().getColumn(0).setMinWidth(250);
		getColumnModel().getColumn(0).setPreferredWidth(250);
		
		//Make the type column invisible
		getColumnModel().getColumn(3).setMaxWidth(0);
		getColumnModel().getColumn(3).setMinWidth(0);
		getColumnModel().getColumn(3).setPreferredWidth(0);

		//Make the editable column invisible
		getColumnModel().getColumn(4).setMaxWidth(0);
		getColumnModel().getColumn(4).setMinWidth(0);
		getColumnModel().getColumn(4).setPreferredWidth(0);

	}

	/* (non-Javadoc)
	 * @see javax.swing.JTable#getCellEditor(int, int)
	 */
	@Override
	public TableCellEditor getCellEditor(int row, int column) {
		// TODO Auto-generated method stub
		
		if (column == 1)
		{
			String type = (String)getValueAt(row, 3);
			if (SCReader.MODE.valueOf(type) == SCReader.MODE.MULTI)
			{
				JComboBox box = new JComboBox();
				box.addItem("true");
				box.addItem("false");
  
  			boolean result = Boolean.parseBoolean((String)getValueAt(row,1));
				if (result){
					box.setSelectedIndex(0);
				}
				else{
					box.setSelectedIndex(1);
				}

				return new DefaultCellEditor(box);
				
			}
		}
		return super.getCellEditor(row, column);
	}

	
	
	/* (non-Javadoc)
	 * @see javax.swing.JTable#isCellEditable(int, int)
	 */
	@Override
	public boolean isCellEditable(int row, int column) {
		// TODO Auto-generated method stub
		
		//Only the Value column is allowed to be editable
		if (column == 1)
		{
			if (!((String)getValueAt(row,4)).isEmpty())
			{
				boolean editable = Boolean.parseBoolean((String)getValueAt(row,4));
				return editable;
			}
		}

		return false;
	}

	/*
	 * Remove all rows of the table
	 * 
	 */
	public void clearTable()
	{
		model.getDataVector().removeAllElements();
	}
	
	/*
	 * Add a section row
	 * 
	 */
	public void addSection(String sectionName)
	{
		model.addRow(new Object[]{sectionName, "", "", "", ""});
	}
	
	/*
	 * Add a parameter row 
	 */
	public void addParameter(String name, String value, 
			String defaultValue, String type, String editable)
	{
		model.addRow(new Object[]{name, value, defaultValue, type, editable});
	}
	
}

class ParaTableRenderer extends DefaultTableCellRenderer {

	/* (non-Javadoc)
	 * @see PSEditor.AlterRowColor#getTableCellRendererComponent(javax.swing.JTable, java.lang.Object, boolean, boolean, int, int)
	 */
	@Override
	public Component getTableCellRendererComponent(JTable table, Object value,
			boolean isSelected, boolean isFocus, int row, int column) {
		// TODO Auto-generated method stub
		Component cell = super.getTableCellRendererComponent(table, value, isSelected, isFocus,
				row, column);
	
		String value_c0 = (String)table.getModel().getValueAt(row, 0);
		String value_c1 = (String)table.getModel().getValueAt(row, 1);
		String value_c2 = (String)table.getModel().getValueAt(row, 2);
		String value_c3 = (String)table.getModel().getValueAt(row, 3);
		String value_c4 = (String)table.getModel().getValueAt(row, 4);
		if (!value_c0.isEmpty() && value_c1.isEmpty() 
				&& value_c2.isEmpty() && value_c3.isEmpty() && value_c4.isEmpty()){
			cell.setBackground(Color.BLUE);
			cell.setForeground(Color.WHITE);
		}
		else{
			cell.setForeground(Color.BLACK);
			if (row%2 == 1){
				cell.setBackground(new Color(245,245,245)); //WhiteSmoke
			}
			else
				cell.setBackground(Color.WHITE);
			
			if (value_c1.compareTo(value_c2) != 0)
			{
				cell.setBackground(Color.YELLOW);
			}
			
			if (!(Boolean.parseBoolean(value_c4)))
			{
				cell.setBackground(Color.GRAY);
			}
			
		}

		return cell; 
	}
	
}

class ParaTableUI extends BasicTableUI{

	/* (non-Javadoc)
	 * @see javax.swing.plaf.basic.BasicTableUI#paint(java.awt.Graphics, javax.swing.JComponent)
	 */
	@Override
	public void paint(Graphics arg0, JComponent arg1) {
		// TODO Auto-generated method stub
		super.paint(arg0, arg1);
	}
	
	
	
}