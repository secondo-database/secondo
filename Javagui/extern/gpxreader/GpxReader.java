//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package extern.gpxreader;

import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.InputStream;
import java.time.Instant;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.TimeZone;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import extern.SecondoImporter;
import sj.lang.ListExpr;

/**
 * A reader for gpx files
 * 
 * @author Markus Fuessel
 *
 */
public class GpxReader implements SecondoImporter {

   private String errorString = "NO_ERROR";

   private GPXParser gpxParser = new GPXParser();

   private Counter tubleCounter;

   /**
    * Get the file content as nested list
    * 
    * @param fileName
    * 
    * @return ListExpr
    */
   public ListExpr getList(String fileName) {
      tubleCounter = new Counter();
      File file = new File(fileName);
      List<File> gpxFiles = new ArrayList<File>();

      FileFilter gpxFilter = new FileFilter() {

         @Override
         public boolean accept(File pathname) {
            return pathname.getName().toLowerCase().endsWith(".gpx");
         }
      };

      if (!file.exists()) {
         errorString = "FILE_NOT_EXISTS";

         return null;
      }

      if (file.isDirectory()) {
         gpxFiles = Arrays.asList(file.listFiles(gpxFilter));

         if (gpxFiles.isEmpty()) {
            errorString = "NO_FILES_IN_DIRECTORY";

            return null;
         }

      } else {
         gpxFiles.add(file);
      }

      ListExpr result = null;
      ListExpr lastResult = null;

      for (File gpxFile : gpxFiles) {

         try {
            ListExpr fileResult = gpxParser.parseFile(gpxFile);

            if (result == null) {
               result = fileResult;
            } else {
               result = ListExpr.concat(result, fileResult.first());
            }

         } catch (Exception e) {

            errorString = "Problem in reading file " + gpxFile.getAbsolutePath() + ". " + e.getMessage();

            return null;
         }

      }

      return ListExpr.twoElemList(getRelationHeader(), result);
   }

   /**
    * Get the relation header
    * 
    * @return
    */
   public ListExpr getRelationHeader() {
      String[] attrList = { "Id", "File", "TrackNo", "Track" };
      String[] attrType = { "int", "string", "int", "mpoint" };

      ListExpr TupleList = ListExpr
            .oneElemList(ListExpr.twoElemList(ListExpr.symbolAtom(attrList[0]), ListExpr.symbolAtom(attrType[0])));
      ListExpr last = TupleList;

      for (int i = 1; i < attrList.length; i++) {
         ListExpr attr = ListExpr.twoElemList(ListExpr.symbolAtom(attrList[i]), ListExpr.symbolAtom(attrType[i]));
         last = ListExpr.append(last, attr);
      }

      ListExpr RelList = ListExpr.twoElemList(ListExpr.symbolAtom("tuple"), TupleList);
      return ListExpr.twoElemList(ListExpr.symbolAtom("rel"), RelList);
   }

   /**
    * Getter for the error message
    */
   public String getErrorString() {
      return errorString;
   }

   /**
    * Get file description
    */
   public String getFileDescription() {
      return "a single gpx-file or a directory containing gpx-files";
   }

   /**
    * Enum for gpx tags
    * 
    * @author Markus Fuessel
    *
    */
   private enum GPXTags {

      NODE_GPX("gpx"), NODE_METADATA("metadata"), NODE_TRK("trk"), NODE_RTE("rte"), NODE_WPT("wpt"), NODE_EXTENSIONS(
            "extensions"), NODE_TRKPT(
                  "trkpt"), NODE_TRKSEG("trkseg"), ATTR_CREATOR("creator"), ATTR_VERSION("version"), NODE_TIME("time"),

      ATTR_LAT("lat"), ATTR_LON("lon");

      private final String value;

      private GPXTags(String value) {
         this.value = value;
      }

      public String getValue() {
         return this.value;
      }

   }

   /**
    * This class implements a gpx file parser
    * 
    * @author Markus Fuessel
    */
   private class GPXParser {

      /**
       * Attribute to store date time format which is used for instant toString
       * methods values by default // 2016-08-17T16:35:21.000Z -->
       * year-month-day[-hour:minute[:second[.millisecond]]]
       */
      private DateTimeFormatter dateTimeFormat;

      /**
       * Attribute to store the time zone id which is used for instant toString
       * methods
       */
      private ZoneId timeZoneId;

      private File gpxFile;
      private Document doc;

      // contains the root node
      private Node gpxNode;

      /**
       * Constructor for a new gpx parser
       */
      public GPXParser() {
         dateTimeFormat = DateTimeFormatter.ofPattern("yyyy-MM-dd-HH:mm:ss");
         timeZoneId = TimeZone.getDefault().toZoneId();

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
       * Initialize the parser
       * 
       * @throws Exception
       */
      public void initParser() throws Exception {
         InputStream inStream = null;
         try {
            inStream = new FileInputStream(gpxFile);

            DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            doc = builder.parse(inStream);

         } finally {
            inStream.close();
         }

         Node firstChild = doc.getFirstChild();

         if (firstChild != null && GPXTags.NODE_GPX.getValue().equals(firstChild.getNodeName())) {

            gpxNode = firstChild;

         } else {
            throw new IllegalAccessException("Not a valid GPX file.");
         }
      }

      /**
       * Parse the entire gpx file
       * 
       * @param gpxFile
       * @return
       */
      public ListExpr parseFile(final File gpxFile) throws Exception {
         openFile(gpxFile);
         int trackNo = 0;

         NodeList nodes = gpxNode.getChildNodes();
         ListExpr tupel = null;
         ListExpr tupelList = null;
         ListExpr mpointsNL = null;

         for (int idx = 0; idx < nodes.getLength(); idx++) {
            Node currentNode = nodes.item(idx);

            if (GPXTags.NODE_TRK.getValue().equals(currentNode.getNodeName())) {
               trackNo++;
               mpointsNL = parseTrack(currentNode);

               tupel = ListExpr.fourElemList(ListExpr.intAtom(tubleCounter.getNext()),
                     ListExpr.stringAtom(gpxFile.getName()), ListExpr.intAtom(trackNo), mpointsNL);

               if (tubleCounter.current() <= 1) {
                  tupelList = ListExpr.oneElemList(tupel);
               } else {
                  tupelList = ListExpr.concat(tupelList, tupel);
               }
            }

         }

         return tupelList;
      }

      /**
       * Parse a track node of the current gpx file
       * 
       * @param trackNode
       * 
       * @return ListExpr, a mpoint value as nested list
       * 
       * @throws Exception
       */
      private ListExpr parseTrack(Node trackNode) throws Exception {
         ListExpr result = null;

         NodeList trkSegNodes = trackNode.getChildNodes();

         for (int idx = 0; idx < trkSegNodes.getLength(); idx++) {
            Node currentTrkSegNode = trkSegNodes.item(idx);

            if (GPXTags.NODE_TRKSEG.getValue().equals(currentTrkSegNode.getNodeName())) {
               NodeList trkPoints = currentTrkSegNode.getChildNodes();

               TrackPoint currentTrkPoint = null;
               TrackPoint lastTrkPoint = null;

               for (int idxTP = 0; idxTP < trkPoints.getLength(); idxTP++) {

                  Node currentTrkPointNode = trkPoints.item(idxTP);

                  if (GPXTags.NODE_TRKPT.getValue().equals(currentTrkPointNode.getNodeName())) {

                     currentTrkPoint = getTrackPoint(currentTrkPointNode);

                     if (lastTrkPoint != null && currentTrkPoint != null) {
                        if (result == null) {
                           result = ListExpr.oneElemList(getUPointNL(lastTrkPoint, currentTrkPoint));
                        } else {
                           result = ListExpr.concat(result, getUPointNL(lastTrkPoint, currentTrkPoint));
                        }

                     }

                     lastTrkPoint = currentTrkPoint;

                  }

               }
            }

         }

         return result;
      }

      /**
       * Get the track point information from the current track point node
       * 
       * @param currentTrkPointNode
       * 
       * @return a TrackPoint object
       */
      private TrackPoint getTrackPoint(final Node currentTrkPointNode) throws Exception {

         double latVal;
         double lonVal;
         Instant instant;

         NamedNodeMap attrs = currentTrkPointNode.getAttributes();

         Node latNode = attrs.getNamedItem(GPXTags.ATTR_LAT.getValue());
         if (latNode != null) {

            latVal = Double.parseDouble(latNode.getNodeValue());

         } else {
            throw new Exception("no lat value in trackpoint data.");
         }

         // check for lon attribute
         Node lonNode = attrs.getNamedItem(GPXTags.ATTR_LON.getValue());
         if (lonNode != null) {
            lonVal = Double.parseDouble(lonNode.getNodeValue());
         } else {
            throw new Exception("no lon value in trackpoint data.");
         }

         NodeList childTP = currentTrkPointNode.getChildNodes();

         for (int idxCTP = 0; idxCTP < childTP.getLength(); idxCTP++) {
            Node currentChildTP = childTP.item(idxCTP);

            if (GPXTags.NODE_TIME.getValue().equals(currentChildTP.getNodeName())) {
               instant = Instant.parse(currentChildTP.getTextContent());

               return new TrackPoint(latVal, lonVal, instant);
            }
         }

         throw new Exception("no time value in trackpoint data.");
      }

      /**
       * Get a unit point value as nested list
       * 
       * @param tp0
       * @param tp1
       * 
       * @return the unit point as nested list
       */
      private ListExpr getUPointNL(final TrackPoint tp0, final TrackPoint tp1) {
         ListExpr x0 = ListExpr.realAtom(tp0.x);
         ListExpr y0 = ListExpr.realAtom(tp0.y);
         ListExpr x1 = ListExpr.realAtom(tp1.x);
         ListExpr y1 = ListExpr.realAtom(tp1.y);

         // Format 2016-08-17T16:35:21.000Z to
         // year-month-day[-hour:minute[:second[.millisecond]]]

         ListExpr instant0 = ListExpr.stringAtom(tp0.instant.atZone(timeZoneId).format(dateTimeFormat));
         ListExpr instant1 = ListExpr.stringAtom(tp1.instant.atZone(timeZoneId).format(dateTimeFormat));

         ListExpr period = ListExpr.fourElemList(instant0, instant1, ListExpr.boolAtom(true), ListExpr.boolAtom(false));
         ListExpr points = ListExpr.fourElemList(x0, y0, x1, y1);

         return ListExpr.twoElemList(period, points);

      }
   }

   /**
    * A class to represent single track points of a gpx file<br>
    * This is just a simple data structre with public access to attributes
    * 
    * @author Markus Fuessel
    */
   private class TrackPoint {
      public double x;
      public double y;
      public Instant instant;

      /**
       * Constructor
       * 
       * @param x
       * @param y
       * @param instant
       */
      public TrackPoint(final double x, final double y, final Instant instant) {
         this.x = x;
         this.y = y;
         this.instant = instant;
      }

      public TrackPoint() {

      }
   }

   /**
    * A simple tubleCounter
    * 
    * @author Markus Fuessel
    *
    */
   private class Counter {

      int c;

      /**
       * Constructor
       */
      Counter() {
         c = 0;
      }

      /**
       * Get next value
       * 
       * @return
       */
      public int getNext() {
         c++;
         return c;
      }

      /**
       * Get the current value
       * 
       * @return
       */
      public int current() {
         return c;
      }
   }

}
