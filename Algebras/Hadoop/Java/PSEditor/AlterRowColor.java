package PSEditor;

import java.awt.Color;
import java.awt.Component;

import javax.swing.JTable;
import javax.swing.table.DefaultTableCellRenderer;

public class AlterRowColor extends DefaultTableCellRenderer {

	/* (non-Javadoc)
	 * @see javax.swing.table.DefaultTableCellRenderer#getTableCellRendererComponent(javax.swing.JTable, java.lang.Object, boolean, boolean, int, int)
	 */
	@Override
	public Component getTableCellRendererComponent(JTable table, Object value,
			boolean isSelected, boolean isFocus, int row, int column) {
		// TODO Auto-generated method stub
		Component cell = super.getTableCellRendererComponent(table, value, isSelected, isFocus, row, column);
		
		if (row % 2 != 0){
			cell.setForeground(Color.BLACK);
			cell.setBackground(Color.LIGHT_GRAY);
		}
		else{
			cell.setForeground(Color.BLACK);
			cell.setBackground(Color.WHITE);
		}

		return cell;
	}
}
