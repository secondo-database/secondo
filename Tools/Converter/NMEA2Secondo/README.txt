
NMEA2Secondo
===========

This tool can be used for converting data produced by an GPS receiver 
supporting the NMEA protocol into a moving point object. The only
data field evaluated is the GPGGA field. So the file to convert must
contain this field. Other data sets are ignored. You can create a 
NMEA file using the free "kompass" software on your pda.
To convert the file, perform the following steps:

1. Compile the tool by calling make
2. Run the tool with the required arguments
java NMEA2Secondo filename [dayoffset [houroffset [epsilon]]]

The filename is the name of the file to convert or '-' for the standard input.

The dayoffset is an integer value for the distance to the NULLDAY. In the current
implementation of the DateTime Algebra this date is fixed as "2000-01-03". This is required
because no information about the date is available in the GPGGA dataset.
To get the correct value for a desired day, use Secondo with enabled DateTimeAlgebra. 
E.g. the value for 2005-06-27 can be get by the query:

query [const instant value "2005-06-27"] - [const instant value 0.0]

The result will be a duration with value (2002 0). Put the value 2002 as dayoffset into
this converter.
You can also use the keyword 'file' as parameter. In this case, the value is taken from
the last modiification time of the file to convert. 



The houroffset is also an integer value describing the difference between local time and
utc (coordinated universal time). For gemany, use 1 (or 2 for summer time). 
Instead of using a explicit value, you can also use the keyword 'local'. Than the 
difference is taken from the system using the date given from days (to recognize daylight 
time).

Using the epsilon parameter, it is possible to control the number of created units. 
If a new unit is an extension of the last one, we summarize the units when the 
difference betweent the expected point and the given point is less or equal to
epsilon. Use a negative value for epsilon to avoid summarization of units.




