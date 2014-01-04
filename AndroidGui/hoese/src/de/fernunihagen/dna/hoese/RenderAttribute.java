package de.fernunihagen.dna.hoese;

/** This Interface is used for changing the attrbitutes of an
 * graphical object during the animation/displaying.
 **/
public interface RenderAttribute{
  /** returns the defined state at the given time **/
  boolean isDefined(double time );
  /** returns the minimum value of this attribute **/
  double getRenderValue(double time);
  /** return whether this objects is defined at any time**/
  boolean mayBeDefined();
  /** returns the maximum value of this attribute **/
  double getMinRenderValue();
  /** returns the value of this attribute for the given time **/
  double getMaxRenderValue();
}