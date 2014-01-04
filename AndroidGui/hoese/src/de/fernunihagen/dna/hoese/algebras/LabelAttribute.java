package de.fernunihagen.dna.hoese.algebras;

/**
 * This interface should be implemented by all Display classes which should 
 *be used as a label to a graphical object.
 **/
public interface LabelAttribute {
	  String getLabel(double time);
}
