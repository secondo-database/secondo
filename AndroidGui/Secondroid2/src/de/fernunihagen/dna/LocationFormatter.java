package de.fernunihagen.dna;

import java.util.Locale;
import android.location.Location;

/// Format a given GeopPoint in SECONDO constant
public class LocationFormatter {
	final static String FORMAT = "[const point value (%.6f %.6f)]";
	
	static String format(Location location) {
		return String.format(Locale.US, FORMAT, location.getLongitude(), location.getLatitude());
	}
}
