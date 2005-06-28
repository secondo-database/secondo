
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
java NMEA2Secondo filename [timeoffset [epsilon]]

The filename is the name of the file to convert or '-' for the standard input.

The timeoffset is an integer value for the distance to the NULLDAY. In the current
implementation of the DateTime Algebra this date is fixed as "2000-01-03". This is required
because no information about the date is available in the GPGGA dataset.

Using the epsilon parameter, it is possible to control the number of created units. 
If a new unit is an extension of the last one, we summarize the units when the 
difference betweent the expected point and the given point is less or equal to
epsilon. Use a negative value for epsilon to avoid summarization of units.




