
package tools;

import java.io.File;

public interface ObjectLoader<T>{

  public T loadFromFile(File f);

}
