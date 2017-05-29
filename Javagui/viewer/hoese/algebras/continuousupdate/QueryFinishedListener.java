

package viewer.hoese.algebras.continuousupdate;


/**
 * Interface for informing a component that
 * some query was finished.
**/

interface QueryFinishedListener{
  void queryFinished(int errorCode);
}
