/*
 * put your module comment here
 * formatted with JxBeauty (c) johann.langhofer@nextra.at
 */


package  viewer.hoese;

import viewer.HoeseViewer;


/**
 * A rendering type that varies the width of the outline.
 */
public class OutLineWidth
    implements CalcCategory {
  private static int VariantNr = 1;

  /**
   * This method creates the new characteristic (different outline) category, based on a template one.
   * @param actValue A value between 0..valueNr
   * @param valueNr  The maximal Nr of different values
   * @param templCat The template category
   * @return A new category
   * @see generic.Category
   * @see <a href="OutLineWidthsrc.html#calcCategory">Source</a>
   */
  public Category calcCategory (int actValue, int valueNr, Category templCat) {
    Category NewCat = null;
    try {
      NewCat = (Category)templCat.clone();
    } catch (Exception e) {}
    NewCat.setName(NewCat.getName() + "OLW" + VariantNr++);
    int inc = actValue*16/valueNr;
    NewCat.setLineWidth(inc);
    return  NewCat;
  }

  /**
   * 
   * @return The entry for the rendering type list. 
   * @see <a href="OutLineWidthsrc.html#toString">Source</a>
   */
  public String toString () {
    return  "OutLine Width";
  }
}



