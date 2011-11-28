#include <vector>
#include <string>
#include "SymbolicTrajectoryPattern.h"
#include "SymbolicTrajectoryTools.h"
#include "SymbolicTrajectoryDateTime.h"
#include "CharTransform.h"

using namespace std;

namespace stj {


void PrePat::clear() 
{
   left = "";
   right = "";
   variable = "";
   cp_variable = "";
   wildcard = 0;
}


int PatEquation::setValue(int key, int op, const string& value, string& errMsg)
{
   PatEquation::key = key;    
   PatEquation::op = op;   
 
   switch (key) 
   {
      case 1: PatEquation::value = value; break;
   
      case 3:
	  case 4: if ( !isDate(value) )
	          {
			     errMsg = "In condition: start/end value is not a date!";
	             return 1; 
			  } 
              else
              {
			     PatEquation::value = getFullDate(value); 
              } 			  
	          break; 
	  
	  
	  case 6: if ( !isNumeric(value) )
	          {
			     errMsg = "In condition: card value is not a number!";
	             return 1; 
			  }  
              else
              {
			     PatEquation::value = str2Int(value);
              } 			  
	          break; 
			  
			  
	  default: errMsg = "In condition: unknown key!";
	           return 1; 			    
	           break; 			     
   } // end switch

   return 0; 
}



int PatParser::setPattern(string const &text, string &errMsg)  
{
  vector<string> token; 
  
  token = split(text, "//");
  
  errMsg = "";
  
  if ( (token.size() > 2) )
  {
     errMsg = "Syntax Error! Use // only as seperator.";
     return 1;
  }
  else if ( getPrePats(token[0], errMsg) ) 
  {
     return 1;    
  }	 
  
  if ( (token.size() == 2) && getConds(token[1], errMsg) ) 
  {
     return 1;   	 
  } 	 
  
  if ( setPats( errMsg ) )
  {
     return 1;  
  }
  
  return 0;
}


PatParser::PatParser(string const &text) : valid(true) 
{
   if ( setPattern(text, errMsg) )
   {
     valid = false;     
   }  
    
}


bool PatParser::checkForUniqueVariables()
{


return true;
}



int PatParser::setPats( string &errMsg )
{
  Condition cond;
  SinglePattern pat; 
  
  for (size_t i = 0; i < prePats_.size(); ++i)  
  {
     if ( pat.set( prePats_[i], errMsg) ) return 1;     
     pats_.push_back( pat );	 
  }
  
  for (size_t i = 0; i < conds_.size(); ++i)  
  {
    cond = conds_[i];

	for ( size_t j = 0; j < pats_.size(); ++j )
	{
	    if ( pats_[j].cp_variable == cond.variable ) 
		{
            ; // conditions for cp variables will be ignored.			   
	    }			
	    if ( pats_[j].variable == cond.variable ) 
		{
		   if ( pats_[j].addCondition(cond, errMsg) ) return 1;		
		}
					
    } // end for j
	
  } // end for i  

  return 0; 
}



void PatParser::pushAndClearPrePat(PrePat &prePat) 
{
    prePats_.push_back(prePat);   
    string cp_variable = prePat.cp_variable;	
	prePat.clear();
	prePat.cp_variable = cp_variable;
}


string PatParser::toString() {
  string result = "";
  string errMsg = "";
  
  result = "prePats:\n"; 
  for (size_t i = 0; i < pats_.size(); ++i) 
  {
      result += pats_[i].toString() + "\n";
  }

  result += "\nconds:\n";  
  for (size_t i = 0; i < conds_.size(); ++i) 
  {
      result += conds_[i].toString() + "\n";		  
  }
    
  return result; 
}



int PatParser::getConds(string const &text, string &errMsg) 
{
  vector<string> conds = msplit(text, ',');
  vector<string> lhs;
  int op = 0;
  Condition cond;
  
  for (size_t i = 0; i < conds.size(); ++i) 
  {
      size_t pos;
	  vector<string> lhs;
	  string value;
      if ( (pos = conds[i].find_first_of("<>=")) != string::npos )
	  {
	      switch ( conds[i][pos] )
		  {
		     case '<' :  if ( (conds[i].length() > pos) && (conds[i][pos+1] == '=') )
			             { // operator: <=
							 op = 2 + 1;							 
							 if (cond.set(conds[i], pos, op, errMsg)) return 1;
							 conds_.push_back(cond);				 							 							 
						 }
						 else
						 { // operator: <
							 op = 2;							 
							 if (cond.set(conds[i], pos, op, errMsg)) return 1;
							 conds_.push_back(cond);
						 }
						 
						 break;
						 
			 
			 case '>' :  if ( (conds[i].length() > pos) && (conds[i][pos+1] == '=') )
			             { // operator: >=
							 op = 4 + 1;							 
							 if (cond.set(conds[i], pos, op, errMsg)) return 1;
							 conds_.push_back(cond);						 
						 }
						 else
						 { // operator: >
							 op = 4;							 
							 if (cond.set(conds[i], pos, op, errMsg)) return 1;
							 conds_.push_back(cond);						 
						 }
						 
						 break;
						 
			 
			 case '=' :  op = 1;							 
						 if (cond.set(conds[i], pos, op, errMsg)) return 1;
						 conds_.push_back(cond);				 
			 
			             break; 

						 
			 default :   errMsg = "Cannot resolve condition: " + conds[i].substr(0,15) + "...";
		                 return 1;	
			             break; 
						 
		  } // end switch
	  
	  } // end if
	  else
	  {
         errMsg = "No Operator found in condition (e.g. < or >)!";
	     return 1;	  
	  }
  
  } // end for

  return 0;
}


int PatParser::getPrePats(string text, string &errMsg) 
{
  bool open_paranthesis = false;
  bool open_square_bracket = false;  
  bool open_set_bracket = false;  
  string token = "";
  int element_count = 0;
 	
  PrePat prePat;	
  text += " ";  
  token = "";
  char char_cur; 
  for (size_t i = 0; i < text.length(); i++) 
  {
     char_cur = text[i];

     switch (char_cur)
	 {
	    case '(' : if (open_paranthesis) 
		           {
				      errMsg = "Two open paranthesis!";
					  return 1;
				   } 
				   else if (open_set_bracket)				   
				   {
				      errMsg = "Open set bracket before open paranthesis!";
					  return 1;				   
                   } 				   
				   else
				   {
				      open_paranthesis = true;
					  
					  if (!token.empty())
					  {
					      prePat.variable = token;
                          token = "";						  
					  }				  					  
		           }  
				   
				   break;				   

				   
	    case ')' : if (!open_paranthesis) 
		           {
				      errMsg = "Close paranthesis without an open!";
					  return 1;
				   } 
				   else
				   {
				      open_paranthesis = false;
					  					  
					  if (token.empty()) 
					  {
					     switch (element_count)
						 {
						    case 1:
  						      errMsg = "You entered only one element instead of two!";
					          return 1;					  
						  	  break;
							
                            case 0:	
							
						    case 2: 
                              pushAndClearPrePat(prePat);	
                              element_count = 0;
							  break;

						    default:
  						      errMsg = "You entered more than two elements!"; 
					          return 1;					  
						  	  break;							  
						 }
						 
					  } 
					  else
					  {
					     switch (++element_count)
						 {
						    case 1: 
                              prePat.left = token;
							  break;

						    case 2: 
                              prePat.right = token;
                              pushAndClearPrePat(prePat);	
                              element_count = 0;						  							  
							  break;							  
							  
						    default:
  						      errMsg = "You entered more than two elements!"; 
					          return 1;					  
						  	  break;						 						 
                         } 		
						 
                         token = "";							  
					  }	   
		           }				   
				   
			       break;				   

				   
	    case '[' : if (open_square_bracket) 
		           {
				      errMsg = "Two open square brackets!";
					  return 1;
				   } 
				   else if (open_paranthesis)				   
				   {
				      errMsg = "Open paranthesis before open square bracket!";
					  return 1;				   
                   } 				   
				   else
				   {
				      open_square_bracket = true;
					  prePat.cp_variable = (token.empty()) ? prePat.variable : token;
					  token = "";
				      prePat.variable = "";	  
		           }  
				   
				   break;				   


	    case ']' : if (!open_square_bracket) 
		           {
				      errMsg = "Close square bracket without an open!";
					  return 1;
				   } 
				   else
				   {
				      open_square_bracket = false;
					  
					  if (!token.empty() && !prePat.variable.empty())
					  {
					      prePat.variable = (!token.empty()) ? token : prePat.variable;
                          pushAndClearPrePat(prePat);						  
					  }
					  prePat.cp_variable =	"";		  
		           }				   
				   
			       break;				   

				   
	    case '{' : if (open_set_bracket) 
		           {
				      errMsg = "Two open brackets!";
					  return 1;
				   } 
				   else if (!open_paranthesis)				   
				   {
				      errMsg = "Open set bracket without open paranthesis!";
					  return 1;				   
                   }				   
				   else if (!token.empty()) 
				   {
				      errMsg = "There are unallowed characters before a bracket!";
					  return 1;                      	  
		           }  
  
     		       open_set_bracket = true;				   
				   break;
				   

	    case '}' : if (!open_set_bracket) 
		           {
				      errMsg = "Two close brackets!";
					  return 1;
				   } 
				   else if (!token.empty()) 
				   {
					     switch (++element_count)
						 {
						    case 1: 
                              prePat.left = token;
							  break;

						    case 2: 
                              prePat.right = token;
							  break;							  
							  
						    default:
  						      errMsg = "You entered more than two elements!"; 
					          return 1;					  
						  	  break;						 						 
                         }	                     	  
						 
                         token = "";						 
		           }  
				   else
				   {
				      errMsg = "The set has no value! Please use _ as a wildcard.";
					  return 1;
		           } 				   
				   
				   open_set_bracket = false;				   
				   break;				   

				   
	    case ' ' : if (!open_set_bracket)
		           { 
		           if (!token.empty())
			       {
				      if (open_paranthesis) 
					  {
					     switch (++element_count)
						 {
						    case 1: 
                              prePat.left = token;
							  break;

						    case 2: 
                              prePat.right = token;
							  break;							  
							  
						    default:
  						      errMsg = "You entered more than two elements!";
					          return 1;					  
						  	  break;						 						 
                         }					  
					  }
					  else 
					  {				   
					    if (!prePat.variable.empty()) pushAndClearPrePat(prePat);

    				    prePat.variable = token;			  
					  } 	
                      token = "";					  
				   }
				   }
				   else
				   {
				      token += char_cur;
				   }
				   
				   break;				   


	    case '+' : if (!open_set_bracket)
		           { 
                   if (!token.empty()) 
		           {
				      if (open_paranthesis) 
					  {
  						 errMsg = "The Character + is not allowed as an element!";
					     return 1;					  
					  }	
					  else 			   
					  {
					    if (!prePat.variable.empty()) pushAndClearPrePat(prePat);					      
    				    prePat.variable = token;						
                        token = "";						
					  }
				   }
				   
				   prePat.wildcard = '+';
				   pushAndClearPrePat(prePat);						
                   }
				   
		           break;				   

				   
	    case '*' : if (!open_set_bracket)
		           { 
                   if (!token.empty()) 
		           {
				      if (open_paranthesis) 
					  {
  						 errMsg = "The Character * is not allowed as an element!";
					     return 1;					  
					  }	
					  else 			   
					  {
					    if (!prePat.variable.empty()) pushAndClearPrePat(prePat);					      
    				    prePat.variable = token;						
                        token = "";						
					  }
				   }
				   
				   prePat.wildcard = '*';
				   pushAndClearPrePat(prePat);						
                   }
		           break;		
		
		
	    default  : token += char_cur;
  
                   if (!open_paranthesis && (char_cur < 65 or char_cur > 90)) 
		           {
				      errMsg = "Only capital letters are allowed for variables!";
					  return 1;
				   } 
                   if (!open_paranthesis && (token.length() > 2)) 
		           {
				      errMsg = "Maximal two letters are allowed for variables!";
					  return 1;
				   } 				   
				   
    			   break;		
		
	 } // switch	

  } // for

  if (open_paranthesis) 
  {
      errMsg = "Missing close paranthesis!";
	  return 1;
  }   
  if (open_square_bracket) 
  {
      errMsg = "Missing close square bracket!";
	  return 1;
  }

  if (!((prePat).variable).empty()) pushAndClearPrePat(prePat);  
  
  return 0;
}



//**********************************************************************************************************
//~Condition~


string Condition::toString()
{
   ostringstream result("");
   
   result << "variable=" << variable << "; key=" << key << "; ";
   result << "op=" << op << "; value=" << value;

   return result.str();
} 


int Condition::setKey(string key)
{
   if ( key == "lbs" || key == "lb" ) 
   {
      Condition::key = 1;
   }
   else if ( key == "start" )
   {
      Condition::key = 3;
   }   
   else if ( key == "end" )
   {
      Condition::key = 4;
   } 
   else if ( key == "card" )
   {
      Condition::key = 6;
   }   
   else
   {
      return 1; // cannot resolve key
   }
   
   return 0;
}


int Condition::set(string cond, int op_pos, int op, string &errMsg)
{  
   vector<string> lhs;
   Condition::op = op;
   Condition::value = trim( (op == 1 || op == 2 || op == 4) ? cond.substr(op_pos + 1) : cond.substr(op_pos + 2) );  
							 
   lhs = split( cond.substr(0, op_pos), '.' );
   if ( lhs.size() != 2 ) 
   {
       errMsg = "No <variable>.<key> found in condition!";
       return 1;							     
   }
   if (Condition::setKey( trim(lhs[1]) )) 
   {
       errMsg = "Unknown key found in condition (e.g. .start, ...)!";
       return 1;   
   }
   string variable = trim(lhs[0]);   
   if ( !( variable.size() == 2 && ( variable[0] >= 65 or variable[0] <= 90 ) 
                               && ( variable[1] >= 65 or variable[1] <= 90 ) )
     && !( variable.size() == 1 && ( variable[0] >= 65 or variable[0] <= 90 ) ) )
   { 
       errMsg = "Unknown variable type (only capital letters)!";
       return 1;    
   }
   else
   {
       Condition::variable = variable;
   }
   
   return 0;   
}




//**********************************************************************************************************
//~SinglePattern~


string SinglePattern::toString() 
{
   string result;
   result = cp_variable + ": " + variable + " " + wildcard + "\n";

   result += "lbs: ";   
   for (size_t i = 0; i < lbs.size(); ++i) result += lbs[i] + "; ";
   result += "\n";   
   
   result += "dtrs: ";
   for (size_t i = 0; i < dtrs.size(); ++i) result += dtrs[i] + "; ";
   result += "\n";
   
   result += "trs: ";
   for (size_t i = 0; i < trs.size(); ++i) result += trs[i] + "; ";
   result += "\n";

   result += "sts: ";   
   for (size_t i = 0; i < sts.size(); ++i)
   {
      result += "type=" + int2Str( sts[i].type ) + " ,value=" 
	          + int2Str( sts[i].value ) + "; ";
   }
   result += "\n";   

   result += "condis: ";   
   for (size_t i = 0; i < conditions.size(); ++i)
   {
      result += "key=" + int2Str( conditions[i].key ) + " ,op=" 
	          + int2Str( conditions[i].op ) + " ,value=" 
	          + conditions[i].value + "; ";
   }
   result += "\n";

   
   return result;   
}



bool SinglePattern::isSequence()
{
   return ( variable.length() == 2 || wildcard != 0);
}


int SinglePattern::set(PrePat const &prePat, string &errMsg)
{
   variable = trim( prePat.variable );
   cp_variable = trim( prePat.cp_variable );	 
   wildcard = prePat.wildcard;	 
 
   isSeq = ( variable.length() == 2 || wildcard != 0 ); 
 
   lbs.clear();
   trs.clear();
   sts.clear();
   dtrs.clear();
   
   if (!prePat.right.empty() && prePat.right != "_" ) lbs = getElementsFromSet(prePat.right);
      
   if (!prePat.left.empty() && prePat.left != "_") 
   {
     vector<string> elements = getElementsFromSet(prePat.left);
     string datetime;
	 int type;
	 SemTime semt;
	 
	 for (size_t i = 0; i < elements.size(); ++i)
	 {
	    type = getType(elements[i], datetime);
			
		switch (type) 
		{
		   case 1:  dtrs.push_back( datetime );
		            break;    
					
					
		   case 2:  trs.push_back( datetime );
		            break;
					
		   
		   case 3:  
		   case 4:
		   case 5:  semt.type = type;
		            semt.value = str2Int( datetime );
					sts.push_back( semt );
		            break;					
					
					
		   default: errMsg = "Cannot resolve time element!";
		            return 1; 		   
		
		} // end switch
 	 } // end for
	 
   } // end if	     

   return 0;   
}


int SinglePattern::addCondition(Condition const &cond, string &errMsg)
{  
   if ( (SinglePattern::variable == cond.variable) ) 
   {
      PatEquation patEquation;
      if ( patEquation.setValue(cond.key, cond.op, cond.value, errMsg) ) return 1;
      SinglePattern::conditions.push_back(patEquation);	  
   }

   return 0; 
}

}