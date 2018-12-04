package mapviewer.list;

public class CheckboxListItem {

   protected String label;
   private boolean isSelected = false;

   public CheckboxListItem() {
      super();
   }

   public boolean isSelected() {
      return isSelected;
   }

   public void setSelected(boolean isSelected) {
      this.isSelected = isSelected;
   }

   public String toString() {
      return label;
   }

}