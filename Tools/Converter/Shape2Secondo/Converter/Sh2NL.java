import uk.ac.leeds.ccg.shapefile.*;
import java.io.*;

class Sh2NL {
    public static boolean verbose;

    private static void printlnInfo(String s) {
	if (verbose == true) System.out.println(s);
    }

    private static void error(int errno) {
	if (errno == 1) System.out.println("Syntax: java Sh2NL <shape file> <export file> [--verbose]");
	else System.out.println("Unknown error.");
	System.exit(0);
    }

    private static String shapeTypeToString(int no) {
	switch(no) {
	case Shapefile.POINT: return "POINT";
	case Shapefile.ARC: return "ARC";
	case Shapefile.POLYGON: return "POLYGON";
	case Shapefile.ARC_M: return "ARC_M";
	case Shapefile.MULTIPOINT: return "MULTIPOINT";
	case Shapefile.NULL: return "NULL";
	}
	return "UNDEFINED";
    }

    private static String spaceString(int indent) {
	StringBuffer sb = new StringBuffer();
	
	for (int i = 0; i < indent; i++) {
	    sb.append(" ");
	}

	return sb.toString();
    }

    private static String filenameToObjectType(String filename) {
	StringBuffer result = new StringBuffer();
	int len = (int)filename.length();
	for (int i = 0; i < len; i++) {
	    char c = filename.charAt(i);
	    if (Character.isLetter(c)) {
		result.append(c);
	    }
	    else {
		return result.toString();
	    }
	}
	return result.toString();
    }

    public static void Sh2NL
	(String shape_filename, 
	 String export_filename) throws IOException, ShapefileException {
	printlnInfo("Opening shape file: " + shape_filename);
	Shapefile shapefile = new Shapefile(shape_filename);
	FileWriter exportfile = new FileWriter(export_filename); 
	int numRec = shapefile.getRecordCount();
	printlnInfo("Number of records: " + numRec);
	int typeRecs = shapefile.getShapeType();
	printlnInfo("Type of records: " + shapeTypeToString(typeRecs));
       
	int i = 0;
	int j = 0;
	int k = 0;

	switch (typeRecs) {
	case Shapefile.POINT: 
	    exportfile.write
		(
		 "(OBJECT " + filenameToObjectType(shape_filename) + "\n" 
		 + spaceString(3) + "()\n" 
		 + spaceString(6) + "(rel\n" 
		 + spaceString(9) + "(tuple\n" 
		 + spaceString(12) + "((geodaten point))))\n"
		 + "(\n");
	
	    for (i = 0; i < numRec; i++) {
		ShapePoint sp = (ShapePoint)shapefile.getShape(i);
		double[] pp = sp.getPoint();
		printlnInfo("[" + pp[0] + "/" + pp[1] + "]");
		// Converting:
		exportfile.write(spaceString(3) + "(" + pp[0] + ")(" + pp[1] + ")" + ((i < numRec - 1) ? "\n" : ""));
	    }
	    exportfile.write(")\n");
	    exportfile.write("())");
	    break;
	case Shapefile.ARC:
	    printlnInfo("ARC: Not supported yet.");
	    break;
	case Shapefile.POLYGON:
	    exportfile.write
		(
		 "(OBJECT " + filenameToObjectType(shape_filename) + "\n" 
		 + spaceString(3) + "()\n" 
		 + spaceString(6) + "(rel\n" 
		 + spaceString(9) + "(tuple\n" 
		 + spaceString(12) + "((geodaten region))))\n"
		 + "(\n");
	
	    for (i = 0; i < numRec; i++) {
		printlnInfo(spaceString(3) + "Record No.: " + i);
		ShapePolygon sp = (ShapePolygon)shapefile.getShape(i);
		// get the number of cycles.
		int numOfParts = sp.getNumParts();
		printlnInfo(spaceString(6) + "Number of parts: " + numOfParts);
		for (j = 0; j < numOfParts; j++) {
		    exportfile.write(spaceString(3) + "(\n");
		    ShapePoint[] sps = sp.getPartPoints(j);
		    int spsl = sps.length;
		    printlnInfo(spaceString(9) + "Part No.: " + j 
				+ ", Number of points: " + spsl);
		    for (k = 0; k < spsl; k++) {
			double x = sps[k].getX();
			double y = sps[k].getY();
			printlnInfo(spaceString(12) + "Point: [" 
				    + x + "/" 
				    + y + "]");
			exportfile.write(spaceString(6) 
					 + "(" + x + ")(" + y + ")" 
					 + ((i < numRec - 1) ? "\n" : ""));
 
		    }
		    exportfile.write(spaceString(3) + ")\n");
		}
	    }
	    exportfile.write(")\n");
	    exportfile.write("())");
	    break;
	case Shapefile.ARC_M:
	    printlnInfo("ARC_M: Not supported yet.");
	    break;
	case Shapefile.MULTIPOINT:
	    printlnInfo("MULTIPOINT: Not supported yet.");
	    break;
	case Shapefile.NULL:
	    printlnInfo("NULL: Not supported yet.");
	    break;
	default: 
	    printlnInfo("Unknown type.");
	    break;
	}
	printlnInfo("Finished.");
	exportfile.close();
    }

    public static void main (String args[]) {
	try {
	    // too less parameters
	    if (args.length == 0) error(1);
	    // no filename given
	    if (args.length == 1) error(1);
	    // too many parameters?
	    if (args.length >= 4) error(1);
	    // verbose or not?
	    if (args[2].equals("--verbose")) verbose = true;
	    else verbose = false;
	    Sh2NL(args[0], args[1]);
	}
	catch (Exception e) {
	    e.printStackTrace();
	}
    }
}
