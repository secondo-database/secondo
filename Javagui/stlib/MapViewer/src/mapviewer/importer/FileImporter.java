package mapviewer.importer;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import stlib.datatypes.moving.MovingPoint;
import stlib.interfaces.moving.MovingPointIF;

/**
 * Class to manage file imports
 * 
 * @author Markus Fuessel
 */
public class FileImporter {

   /**
    * Load the passed file
    * 
    * @param file
    * @return
    */
   public List<MovingPointIF> loadFile(File file) {

      System.out.println("Load file " + file.getName() + "...");

      List<MovingPointIF> movingPointList = new ArrayList<>();
      MovingPointIF mpoint = new MovingPoint(0);

      try {
         GPXParser gpxParser = new GPXParser(file, 10);
         gpxParser.initParser();
         gpxParser.parse();

         while (!gpxParser.isEof()) {
            mpoint = gpxParser.getNextTrack();

            if (mpoint.getNoUnits() > 0) {
               movingPointList.add(mpoint);
            }
         }

      } catch (Exception e) {
         e.printStackTrace();
      }

      return movingPointList;
   }

}
