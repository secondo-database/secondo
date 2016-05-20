#ifndef FILE_NGEXCEPTION
#define FILE_NGEXCEPTION

#include <string>

/**************************************************************************/
/* File:   ngexception.hpp                                                */
/* Author: Joachim Schoeberl                                              */
/* Date:   16. Jan. 2002                                                  */
/**************************************************************************/


/// Base class for all ng exceptions
class NgException 
{
  /// verbal description of exception
  std::string what;
public:
  ///
  NgException (const std::string & s);
  ///
  virtual ~NgException ();

  /// append string to description
  void Append (const std::string & s);
  //  void Append (const char * s);
  
  /// verbal description of exception
  const std::string & What() const { return what; }
};

#endif
