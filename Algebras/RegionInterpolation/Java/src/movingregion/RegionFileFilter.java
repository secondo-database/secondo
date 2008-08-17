package movingregion;

import java.io.*;

class RegionFileFilter extends javax.swing.filechooser.FileFilter
{
    
    public RegionFileFilter()
    {
    }
    
    public boolean accept(File f)
    {
        // Auch Unterverzeichnisse anzeigen
        boolean res=false;
        if (f.isDirectory())
            return true;
        res=true;
//        InputStream fis = null;
//        try
//        {
//            fis = new FileInputStream( f );
//            ObjectInputStream o = new ObjectInputStream( fis );
//            Region test=((Region) o.readObject());
//            res=true;
//        }
//        catch ( IOException e2 )
//        { res=false; }
//        catch ( ClassNotFoundException e2 )
//        { res=false; }
//        finally
//        { try
//          { fis.close(); }
//          catch ( Exception e2 )
//          { } }
        return (res);
    }
    
    public String getDescription()
    {
        String res="Regionen";
        return (res);
    }
}

