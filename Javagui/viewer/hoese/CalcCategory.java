
package  viewer.hoese;

  /**
   * This interface need to be implemented for a new entry in ViewConfig's rendering type
   * list (e.g. OutLineWidth).
   * @see generic.OutLineWidth
   * @see generic.ViewConfig
   */
public interface CalcCategory {

  /**
   * This method creates the new characteristic category, based on a template one.
   * @param actValue A value between 0..valueNr
   * @param valueNr  The maximal Nr of different values
   * @param templCat The template category
   * @return A new category
   * @see generic.Category
   */
  public Category calcCategory (int actValue, int valueNr, Category templCat);



  /**
   * 
   * @return The entry for the rendering type list. 
   */
  public String toString ();
}



