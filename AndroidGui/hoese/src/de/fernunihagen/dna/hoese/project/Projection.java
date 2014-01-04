package de.fernunihagen.dna.hoese.project;

import java.util.Properties;


public interface Projection{
	public boolean project(double lambda, double phi,
			javamini.awt.geom.Point2D.Double result);

	public boolean getOrig(double px, double py,
			javamini.awt.geom.Point2D.Double result);

	public boolean showSettings();

	public String getName();

	public boolean isReversible();

	public Properties getProperties();

	public boolean setProperties(Properties p);

	static final double LOG_E = Math.log(Math.E);
	static final double PI = Math.PI;
}

