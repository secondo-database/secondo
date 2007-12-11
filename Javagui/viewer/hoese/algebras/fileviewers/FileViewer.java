

package viewer.hoese.algebras.fileviewers;

import javax.swing.JComponent;
import java.io.File;


public abstract class FileViewer extends JComponent{


public abstract boolean canDisplay(File f);

public abstract boolean display(File f);


}
