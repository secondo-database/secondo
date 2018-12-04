package mapviewer.importer;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import stlib.datatypes.intime.Intime;
import stlib.datatypes.moving.MovingPoint;
import stlib.datatypes.spatial.Point;
import stlib.datatypes.time.TimeInstant;
import stlib.interfaces.intime.IntimeIF;
import stlib.interfaces.moving.MovingPointIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.spatial.UnitPointIF;

/**
 * This class implements a gpx file parser
 * 
 * @author Markus Fuessel
 */
public class GPXParser {

   private File gpxFile;
   private Document doc;

   private Node gpxNode;

   private int lastTrackIdx = -1;

   private boolean eof = false;

   private long minSecondsBetween;

   public GPXParser() {

   }

   public GPXParser(final File gpxFile) {
      super();

      this.gpxFile = gpxFile;
      this.minSecondsBetween = 0;
   }

   public GPXParser(final File gpxFile, final long minSecondsBetween) {
      super();

      this.gpxFile = gpxFile;
      this.minSecondsBetween = minSecondsBetween;
   }

   /**
    * Initialize the parser
    * 
    * @throws Exception
    */
   public final void initParser() throws Exception {
      InputStream inStream = null;
      try {
         inStream = new FileInputStream(gpxFile);
         lastTrackIdx = -1;
         gpxNode = null;
         eof = false;

         DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
         doc = builder.parse(inStream);

      } catch (FileNotFoundException e) {
         System.err.println("File not found! " + e.getMessage());

      } finally {
         inStream.close();
      }
   }

   /**
    * Load new gpx file
    * 
    * @param gpxFile
    * @throws Exception
    */
   public void openFile(final File gpxFile) throws Exception {
      this.gpxFile = gpxFile;
      initParser();

   }

   /**
    * @return the eof
    */
   public boolean isEof() {
      return eof;
   }

   /**
    * Parse the next Track of the file
    * 
    * @return
    * @throws Exception
    */
   public MovingPointIF getNextTrack() throws Exception {
      MovingPointIF mpoint = new MovingPoint(0);

      NodeList nodes = gpxNode.getChildNodes();

      for (int idx = lastTrackIdx + 1; idx < nodes.getLength(); idx++) {
         Node currentNode = nodes.item(idx);

         if (GPXTags.NODE_TRK.getValue().equals(currentNode.getNodeName())) {
            lastTrackIdx = idx;

            mpoint = parseTrack(currentNode);

            break;
         }

         if (idx == nodes.getLength() - 1) {
            eof = true;
         }

      }

      return mpoint;
   }

   public void parse() throws Exception {

      Node firstChild = doc.getFirstChild();

      if (firstChild != null && GPXTags.NODE_GPX.getValue().equals(firstChild.getNodeName())) {

         gpxNode = firstChild;

      } else {
         throw new IllegalAccessException("Not a valid GPX file.");
      }

   }

   /**
    * Parse a track node
    * 
    * @param node
    * @throws Exception
    */
   private MovingPointIF parseTrack(Node node) throws Exception {
      MovingPointIF mpointResult = new MovingPoint(0);

      NodeList childNodes = node.getChildNodes();

      TimeInstantIF lastInstant = new TimeInstant(Instant.EPOCH);

      for (int idx = 0; idx < childNodes.getLength(); idx++) {
         Node currentNode = childNodes.item(idx);

         if (GPXTags.NODE_TRKSEG.getValue().equals(currentNode.getNodeName())) {
            List<IntimeIF<PointIF>> ipointList = new ArrayList<>();

            NodeList trkPoints = currentNode.getChildNodes();

            for (int idxTP = 0; idxTP < trkPoints.getLength(); idxTP++) {
               Node currentTrkPoint = trkPoints.item(idxTP);

               if (GPXTags.NODE_TRKPT.getValue().equals(currentTrkPoint.getNodeName())) {
                  IntimeIF<PointIF> currentTrkPT = getTrkPT(currentTrkPoint);

                  if (currentTrkPT.getInstant().after(lastInstant.plusMillis(minSecondsBetween * 1000))) {
                     ipointList.add(currentTrkPT);

                     lastInstant = currentTrkPT.getInstant();
                  }
               }
            }

            MovingPointIF mpoint = new MovingPoint(ipointList);

            for (UnitPointIF upoint : mpoint.getUnits()) {
               mpointResult.add(upoint);

            }

         }
      }

      return mpointResult;
   }

   /**
    * @param currentTrkPoint
    * @return
    * @throws Exception
    */
   public Intime<PointIF> getTrkPT(final Node currentTrkPoint) throws Exception {
      Intime<PointIF> ipoint = new Intime<>();

      Double latVal = null;
      Double lonVal = null;

      NamedNodeMap attrs = currentTrkPoint.getAttributes();

      Node latNode = attrs.getNamedItem(GPXTags.ATTR_LAT.getValue());
      if (latNode != null) {

         latVal = Double.parseDouble(latNode.getNodeValue());

      } else {
         throw new Exception("no lat value in waypoint data.");
      }
      // check for lon attribute
      Node lonNode = attrs.getNamedItem(GPXTags.ATTR_LON.getValue());
      if (lonNode != null) {
         lonVal = Double.parseDouble(lonNode.getNodeValue());
      } else {
         throw new Exception("no lon value in waypoint data.");
      }

      PointIF p = new Point(latVal.doubleValue(), lonVal.doubleValue());

      NodeList childTP = currentTrkPoint.getChildNodes();

      for (int idxCTP = 0; idxCTP < childTP.getLength(); idxCTP++) {
         Node currentChildTP = childTP.item(idxCTP);

         if (GPXTags.NODE_TIME.getValue().equals(currentChildTP.getNodeName())) {
            TimeInstantIF currentInstant = new TimeInstant(Instant.parse(currentChildTP.getTextContent()));

            ipoint = new Intime<>(currentInstant, p);

            break;
         }
      }

      return ipoint;
   }

}
