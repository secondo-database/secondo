
#include <assert.h>
#include <string>
#include <Base64.h>
#include <iostream>

char 
Base64::base64Alphabet[] = {'A','B','C','D','E','F','G','H','I','J','K',
                        'L','M','N','O','P','Q','R','S','T','U','V',
	  	        'W','X','Y','Z','a','b','c','d','e','f','g',
		        'h','i','j','k','l','m','n','o','p','q','r','s',
		        't','u','v','w','x','y','z','0','1','2','3',
		        '4','5','6','7','8','9','+','/'};




void
Base64::encode(char* buffer, string& text, int length) {

    assert (length <= 54 ); // 54 * 4/3 = 72, a linebreak is only inserted after 72 bytes

    static char resultbuffer[72];
    char* result =0;
    unsigned char *data = 0;
    int pos = 0, outPos = 0;
    resultbuffer[73]=0;

    while (length > 0) {
    data = (unsigned char*) &(buffer[pos]);
    // set all undefined byte to zero
    for(int i=length;i<3;i++) {
       data[i] = 0;
    }
    // code all given bytes in a single integer
    // this code must be modified for windows, since endianess is important.
    int all = 65536*data[0]+ 256*data[1]+ data[2];
    
    result = &(resultbuffer[outPos]);
    // extract the 6 bit values
    for(int i=3;i>=0;i--){
      int index = (all & 63);
      assert( (index >= 0) && (index < 64) );
      result[i] = base64Alphabet[index];
      all = all >> 6;
    }
    for(int i=0;i<3-length;i++) {
       result[3-i]='=';
    }

    length -= 3;
    pos += 3;
    outPos += 4;
    //cout << "length: " << length << " " << endl; 
    }
    
    string resultStr(&(resultbuffer[0]));
    text = resultStr + "\n";    

}


