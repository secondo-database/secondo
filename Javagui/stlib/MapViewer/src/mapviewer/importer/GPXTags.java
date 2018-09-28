package mapviewer.importer;

/**
 * Enum for gpx tags
 * 
 * @author Markus Fuessel
 *
 */
public enum GPXTags {

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
