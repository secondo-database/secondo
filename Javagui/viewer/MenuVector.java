package viewer;

import javax.swing.JMenu;
import java.util.Vector;

public class MenuVector  {

private Vector content=new Vector(5,5);

public void addMenu(JMenu Menu){
    content.add(Menu);
}

public boolean removeMenu(JMenu Menu){
   return content.remove(Menu);
}

public int getSize(){
   return content.size();
}

public void clear(){
  content.clear();
}

public JMenu get(int index){
   Object o;
   JMenu Menu;
   try{
      o = content.get(index);
      Menu = (JMenu) o;
   }
   catch(Exception e){
      Menu = null;
   }
      
   return Menu;
}



}
