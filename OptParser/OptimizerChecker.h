
/*
Checks whether the query given as __argument__ is a valid 
prolog query term. If not, the result will be false and the
pointer __errmsg__ is linked to an error message. If this 
function called, the argument errmsg should point to null.
If the errormessage is set to be non-null, the caller of this 
function has to free this pointer after processing its content.


*/

bool checkOptimizerQuery(const char* argument, char*& errmsg);


/*
Enables or disables an option depending on the value of the
boolean parameter. If the option is unknown, the result will be
false, true otherwise.

*/
bool setSqlParserOption(const std::string& optionName, const bool enable);




