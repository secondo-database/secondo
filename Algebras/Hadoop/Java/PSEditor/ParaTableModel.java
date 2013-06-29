package PSEditor;

import javax.swing.table.DefaultTableModel;

public class ParaTableModel extends DefaultTableModel {
	
	
	public ParaTableModel(Object[][] data, Object[] columnNames){
		super(data, columnNames);
	}

	/* (non-Javadoc)
	 * @see javax.swing.table.DefaultTableModel#isCellEditable(int, int)
	 */
	@Override
	public boolean isCellEditable(int row, int column) {
		// TODO Auto-generated method stub
		
		String v_c0 = (String)getValueAt(row, 0);
		String v_c1 = (String)getValueAt(row, 1);
		String v_c2 = (String)getValueAt(row, 2);
		String v_c3 = (String)getValueAt(row, 3);
		
		if (!v_c0.isEmpty() && v_c1.isEmpty() 
				&& v_c2.isEmpty() && v_c3.isEmpty() ){
			return false;
		}
		else {
			if (column == 1)
				return true;
			else
				return false;
		}
	}
	
	

}
