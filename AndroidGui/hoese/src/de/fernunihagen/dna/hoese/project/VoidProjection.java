package de.fernunihagen.dna.hoese.project;

import java.util.Properties;
import tools.Reporter;
import javamini.awt.geom.Point2D;

public class VoidProjection implements Projection {
	public boolean project(double lambda, double phi, Point2D.Double result){
	     result.x = lambda;
	     result.y = phi;
	     return true;
	   }

	   public boolean showSettings(){
	     Reporter.writeWarning("VoidProjection.showSettings not implemented");
	     return true;
	   }

	   public String getName(){
	     return "VoidProjection";
	   }

	   public boolean isReversible(){
	      return true;
	   }

	   public boolean getOrig(double x, double y, Point2D.Double result){
	       result.x = x;
	       result.y = y;
	       return true;
	   }

		public Properties getProperties() {
			return new Properties();
		}

		public boolean setProperties(Properties p) {
			return true;
		}
}
