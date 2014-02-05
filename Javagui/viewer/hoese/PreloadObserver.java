

package viewer.hoese;


public interface PreloadObserver{

   /** Is called whenever a file is downloaded or a download
     * is broken. The success means the success of the download
     */
   public void step(boolean success);

   /** Is called if preloading is finished. 
     * the boolean parameter indicated the completeness of all downloads,
     * i.e. finish is not triggered by canceling. **/

   public void finish(boolean complete);

}
