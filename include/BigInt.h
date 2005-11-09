/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [3] abstract: [\begin{abstract}] [\end{abstract}]
//[<<] [\textless\textless]
//[title] [\title{The BigInt class}\author{Thomas Behr}\maketitle]
//[content] [\tableofcontents]
//[|] [\ensuremath{\mid}]

[title]

[content]



[3] This class provides a integer with the usual operations. 
The size of this integer can be selected by the template
argument size. The actual size will be 4[*]size +1 bytes.
One byte is reserved for the signum. All other bytes are used
for storing the absolute value of this number. 

*/

#ifndef BIGINT_H
#define BIGINT_H

/*
1 Preparations

1.1 Includes

For overloading the  operator [<<] the ostream class is required.
So, we have to include the iostream header.

*/

#include<iostream>

/*
1.2 Some Forward declarations

*/

template<unsigned int size> class BigInt;
template<unsigned int size> std::ostream& 
               operator<<(std::ostream &o,const BigInt<size>& i);


/*
2 Declaration and Definition of the class


*/
template<unsigned int size> class BigInt{
/*
2.1 Private Members

This sections conatins all member which should be not used 
by external classes.

2.1.1 The data for representation

*/

private:
/*
~signum~

This boolean value describes the signum of this number. 
When the value is false, the number is negative - otherwise
the number is positive.

*/  
  bool signum;

/*
~value~

This array contains the representation of the absolute value of this number. 

*/
  unsigned long value[size];


/*
2.1.2 Private Functions

*/

/*
~Add~

This function adds the value arguments. The summands are given as 
s1 and s2 respectively. The result is stored in sum. This function works 
also when an argument is equals to the result, e.g. Add(v,v,v,overflow)
will double the value of v. 

*/
static void Add( const unsigned long* s1, 
                 const unsigned long* s2, 
                 unsigned long* sum,
                 bool& overflow){
 unsigned char c1,c2,c3,res;
 // go thought all value
 c3 = 0; // no overflow 
 unsigned long pos;
 // first, we initialize sum with 0
 for(unsigned int i=0; i<size ; i++){
    for(int j=0;j<32;j++){
      pos = 1L << j;
      c1 = ((s1[i] & pos) !=0)?1:0;
      c2 = ((s2[i] & pos) !=0)?1:0;
      res = c1+c2+c3;
      switch(res){
        case 0: c3 = 0; sum[i] = sum[i] & ~pos;break;
        case 1: c3 = 0; sum[i] = sum[i] | pos;break;
        case 2: c3 = 1; sum[i] = sum[i] & ~pos;break;
        case 3: c3 = 1; sum[i] = sum[i] | pos; break;
        default: assert(false); // inpossible result  
      }
    }
 }
 overflow = c3!=0;
}

static int valueof(char c){
   return (int)(c-'0');
}


/*
~Complement1~

This function computes the complement of this number, e.g. all bits
(and also the signum) will be inverted.

*/
void Complement1(){
   for(unsigned int i=0;i<size;i++){
     value[i] = ~value[i];
   }
   signum = !signum;
}

/*
~Complement2~

This function computes the complement on 2 of this number. This means,
all bits are inverted and after this, a value of 1 is added to this number.
This function is necessary for realizing the ~Minus~ operator.

*/
void Complement2(bool& overflow){
   Complement1();
   Add(this->value,ONE.value,this->value,overflow);
}

/*
~IsOne~

This function checks whether the bit at position p is set to 1.

*/
bool IsOne(unsigned int p){
   unsigned int p1 = p/32;
   if(p1>=size){ // outside the valid range
       return false;
   }
   unsigned long v = value[p1];
   int pos = p%32;
   unsigned long bitmask = 1L << pos;
   return (v&bitmask)!=0;
}

/*
~SigPos~

This function checks how many positions are required for writing down the
absolute value of this number without leading zeros. If the value is zero,
also the result will be zero.

*/
unsigned int SigPos()const{
  unsigned int result = 32u*size;
  bool done = false;
  unsigned long pos;
  for(int i=size-1; i>=0 && !done ;i--){
     if(value[i]==0){
        result -= 32; 
     }else{
       for(int j=31;j>=0;j--){
          pos = 1uL << j;
          if ( (pos & value[i])==0){
             result --;
          }else{
             return result;;
          }
       }
     }
  }
  return result; 
}

/*
~mul2~

Multiplies the content of work given in decimal system with 2.
The return value specifies whether a overflow is occured.
This function is used in a decimal formatted output of this number.

*/
static bool  mul2(unsigned char* work,unsigned int maxsize){
  unsigned char last=0;
  unsigned char current;
  for(unsigned int i=0;i<maxsize;i++){
    current=work[i]*2+last;
    last = current/10;
    work[i] = current%10;
  }
  return last>0;
}

/*
~plus1~

Adds 1 to the content of work. Work is representing a decimal 
number.This function is used for output this number in
decimal format.

*/
static bool plus1(unsigned char* work,unsigned int maxsize){
 unsigned char of=1;
 unsigned char current;
 for(unsigned int i=0;i<maxsize && of>0;i++){
    current = work[i]+of;
    of = current/10;
    work[i] = current%10;    
 }
 return of>0;
}

/*
~write~

This functions writes the content of work (describing a decimal number)
to o. This function is used for output this number in decimal format.

*/
static void  write(std::ostream& o, unsigned char* work, 
                   unsigned int maxsize){
  bool run = false;
  for(int i=maxsize-1;i>=0;i--){
     if(work[i]>0){
        run = true;
     }
     if(run){
        o << (unsigned int)work[i];
     }
  }  
  if(!run)
     o << "0"; 

}

/*
2.2 Public Section

This section contains members and functions useful in
external classes.

*/
public:
/*
2.2.1 Constructors

~Constructor~

This constructor creates a new BigInt instance with value zero.

*/
  BigInt(){
    ReadFrom(0);
  }

/*
~Constructor~

This constructor  creates a new BigInt instance with the given value;

*/
  BigInt(const long value){
    ReadFrom(value);
  } 

/*
~Constructor~

This constructor reads the value of this instance from the 
character array. The array is readed up to length or when an
invalid symbol occurs. The format of the array must be:

  *  ([+-][ ][|][+-])?[0-9][*].

*/
  BigInt(char* content,size_t length){
    ReadFrom(content,length);
  }

/*
~Constructor~

When this constructor is called, the value of the new instance will be equal 
to the value of the argument.

*/
  BigInt(const BigInt<size>& arg){
     Equalize(arg);
  }


/*
2.2.2 Some frequently used constants

*/

/*
~ONE~

This member describes the constant value 1.

*/
const static BigInt<size> ONE;

/*
~ZERO~

This member describes the constant value 0.

*/

const static BigInt<size> ZERO;

/*
~TEN~

This member describes the constant value 10.

*/

const static BigInt<size> TEN;

/*
2.2.3 Public functions


~Equalize~

When this function is called, the value of this bigint is taken from the argument.

*/
void Equalize(const BigInt<size>& arg){
   this->signum = arg.signum;
   for(unsigned int i=0;i<size;i++){
      this->value[i]=arg.value[i];
   }
}

/*
~Equals~

The function checks this objects for equality with the argument. A call of this function
is more efficient than the use of ~CompareTo~

*/
bool Equals(const BigInt<size>& arg)const{
  if(signum!=arg.signum)
      return false;
  for(unsigned int i=0;i<size;i++)
     if(value[i]!=arg.value[i])
       return false;
  return true;
}

/*
~IsZero~

This function checks whether the value of this number is zero.
This result of this function is independly of the signum.

*/
bool IsZero(){
  for(unsigned int i=0;i<size;i++){
     if(value[i]!=0){
         return  false;
     }
  } 
  return true;
}

/*
~ToLong~

This function returns the value of this BigInt as a long value.
The additional argument ~correct~ specifies whether the
long value is able to represent the value of this BigInt. 
If not, the result will conatain an undefined value.

*/
 long ToLong(bool& correct){
    // check whether more than the first part is used
    for(unsigned int i=1;i<size;i++){
        if(value[i]!=0)
          correct = false;
    }
    // If the highest bit of the first part is used, a long
    // value is also too small for representing this value.
    if( ((1L << 31) & value[0]) !=0){
        correct = false;
    }
    // copy the value of the lowest part into a result
    unsigned long v = value[0];
    long r;
    unsigned long pos = 1L << 31;
    for(int i=0;i<32;i++){
       if( (v&pos)>0)
          r++;
       if(i<31)r=r<<1;
       pos = pos >> 1;
    }
    if(!signum)
       r = -1*r;
    return r; 
 }

/*
~Add~

This operator realizes the addition of the arguments.

*/
  BigInt<size> Add(const BigInt<size>& summand,bool& overflow)const{
     BigInt<size> result(0);
     result.Equalize(*this);
     result.AddInternal(summand,overflow);
     return result;
  }

/*
~AddInternal~

The ~AddInternal~ method adds the argument to this instance.
In contrast to the ~Add~ operator, this instance is changed
while this operation.

*/
 void AddInternal(const BigInt<size>& summand,bool& overflow){
     if(this->signum==summand.signum){
          Add(this->value,summand.value,this->value,overflow); 
     }else{ // different signums
        bool of;
        this->Complement2(of);
        Add(this->value,summand.value,this->value,overflow);
        if(!overflow && !of){
          this->Complement2(overflow);
          this->signum=false;
        }else{
          this->signum=true;
        }
        if(!summand.signum){
           this->signum = !this->signum;
        } 
     }
  }

/*
~MinusInternal~

This function subtract the argument fro this instance.
No result is created, rather the result is stored in 
this instance directly.

*/
void MinusInternal(const BigInt<size>& subtrahend, bool& overflow){
   this->signum=!this->signum;
   AddInternal(subtrahend,overflow);
   this->signum=!this->signum;
   if(this->IsZero()){
      this->signum=true;
   }

}

/*
~Minus~

This operator performs the subtraction of the argument from this
instance.

*/
  BigInt<size> Minus(const BigInt<size>& subtrahend,bool& overflow)const{
    BigInt<size> result(0);
    result.Equalize(*this);
    result.MinusInternal(subtrahend,overflow);
    return result;
  }


/*
~Mul~

When calling this function, the result will be the product of
this with the argument. The argument overflow will be true after calling
this function when the size of the numbers is'nt sufficient to hold the
value of the result. 

*/
BigInt<size> Mul(const BigInt<size>& factor, bool& overflow)const{
  BigInt<size> result(0);
  result.Equalize(*this);
  result.MulInternal(factor,overflow);
  return result;
}

/*
~MulInternal~

This function multiplies this with the argument.

*/
void MulInternal(const BigInt<size>& factor, bool& overflow){
  overflow = false;
  bool of = false;
  int cmp = CompareAbsTo(factor);
  BigInt<size> f1,f2;
  if(cmp<0){
     f1.Equalize(factor);
     f2.Equalize(*this);
  }else{
     f2.Equalize(factor);
     f1.Equalize(*this);
  } 

  bool sig = !(factor.signum ^ this->signum);
  this->ReadFrom(0);
  int num = f2.SigPos();
  f1.signum=true;
  f2.signum=true; 
  for(int i=0;i<num;i++){
     if(f2.IsOne(i)){
       this->AddInternal(f1,of);
       if(of)
         overflow=true;
     }
     if(f1.ShiftLeft1() ){
         overflow=true;
     }
  } 
  this->signum = sig;
}

/*
~Div~

This function performs a division between the this bigint and the argument.
The result of the division is returned. The remainder of this operation 
is stored in the coresponding argument.

*/
BigInt<size> Div(const BigInt<size> divisor, BigInt<size>& remainder)const{
   int num1 = this->SigPos();
   int num2 = divisor.SigPos();
   BigInt<size> n1;
   n1.Equalize(*this);
   BigInt<size> n2;
   n2.Equalize(divisor);
   BigInt<size> result(0);
   if(num1<num2 ){ // this is smaller than the divisor
      remainder.Equalize(*this);
      return result; // zero at this point 
   } 
   if(num2==0){
        return result;
   }
   n1.signum = true; // remove negations
   n2.signum = true;
   // shift the value of the divisor 'under' the value of this

   int dif = num1-num2;
   n2.ShiftLeft(dif);
   BigInt<size> tmp;
   
   bool of;
   for(int i=0;i<=dif;i++){
		 tmp = n1.Minus(n2,of);
		 result.ShiftLeft1(); 
		 if(tmp.signum || tmp.IsZero()){ //non-negative number
			 result.AddInternal(ONE,of);
			 n1.Equalize(tmp);
		 }
     n2.ShiftRight1();
   }
   remainder.Equalize(n1); 
   cout << "This = " << (*this) << endl;
   remainder.signum = this->signum;
   result.signum=!(this->signum ^ divisor.signum);   
   return result;
}

/*
~AbsInternal~

Sets the value of this number to its absolute value.

*/
void AbsInternal(){
   signum=true;
}

/*
~Abs~

This functions returns the absolute value of this number.

*/
BigInt<size> Abs(){
   BigInt<size> result(*this);
   result.signum=true;
   return result;
}

/*
~CorrectSignum~

This function removes the signum of this number when its value is zero. 

*/
void CorrectSignum(){
   if(IsZero){
       signum=true;
   }
}


/*
~CompareTo~

Compares this with the argument. The result will be:

  * -1 : if this number is smaller than the argument

  * 0 : if this number is greater than the argument

  * 1 : if the value of this object is greater than the argument

Note that this function differs between a negative and a positive 
zero. This means, a zero with negative signum is smaller than a
positive zero. The avois this behavior, first call the ~CorrectSignum~
function which removes any signum when the absolute value is zero.

*/
int CompareTo(const BigInt<size> arg)const{
  if(!signum && arg.signum){
    return -1;
  }
  if(signum && ! arg.signum){
    return 1;
  }
  int sig = signum?1:-1;
  unsigned long pos;
  for(int i=size-1;i>=0;i--){
    for(int j=31;j>=0;j--){
       pos = 1ul<<j;
       if( ((value[i]&pos) !=0) && ((arg.value[i]&pos)==0)){
           return sig;
       }
       if( ((value[i]&pos) ==0) && ((arg.value[i]&pos)!=0)){
           return -sig;
       }
    }
  }
  return 0;
}

/*
~Negate~

This function will invert the signum of this number.

*/
void Negate(){
   signum = !signum;
}


/*
~CompareAbsTo~

Compares the values of this integers regardless to the signum.

*/
int CompareAbsTo(const BigInt<size> arg)const{
  unsigned long v1,v2;
  for(int i=size-1;i>=0;i++){
     for(int j=31;j>=0;j--){
        v1 = (1L << j) | value[i];
        v2 = (1L << j) | arg.value[i];
        if(v1>v2) 
            return 1;
        if(v2>v1)
            return -1;
     }
  }
  return 0;
}

/*
~ShiftLeft1~

Shifts this value at 1 position to left filling with zeros.
The result is true when an overflow is occured. This function 
is a specialized version of ShiftLeft(int). Because we can 
save some computations, this function is a little bit faster 
than the genaral version of the shiftleft operator. The shift
on only one position is a frequently operation. Thereby we have decided 
to provide this version separately.

*/
bool ShiftLeft1(){
    unsigned long tmp=0;
    bool of = ((1uL << 31) & value[size-1]) !=0;
    for(int i=size-1; i>=0; i--){
       value[i] = value[i] << 1;
       if(i>0){
          tmp = value[i-1];
          tmp = tmp >> 31;
				  value[i] = value[i] | tmp;
       }     
    }
    return of;
}

/*
~ShiftLeft~

This function shifts the content of this bigint to left on pos positions.
The signum is not affected by using this function.

*/
void ShiftLeft(const unsigned int pos){
  if(pos==0)
    return;
  int jump  = pos / 32;
  int shift = pos % 32;
  unsigned long tmp;
  for(int i=size-1;i>=0;i--){
     if(i-jump<0){ // fill with zeros
        value[i] = 0uL;
     }else{
        value[i] = value[i-jump] << shift;      
        if(i-jump-1<0){
           tmp=0;
        } else{
           tmp = value[i-jump-1];
        }
        tmp = tmp >> (32-shift);
        value[i] = value[i] | tmp;
    }  
  }
}

/*
~ShiftRight1~

This operator shifts the value of this bigint 1 position
to right and fills the left side with zero. 

*/
void ShiftRight1(){
   for(unsigned int i=0;i<size;i++){
      value[i] = value[i] >> 1;
      if(i<size-1){
        value[i] = value[i] |  (( value[i+1] & 1L) << 31);
      }
   }
}

/*
~ShiftRight~

This is the general version of the shiftright operator.

*/

void ShiftRight(const int positions){
  if(positions==0)
     return;
  int jump  = position / 32;
  int shift = positions % 32;
  unsigned long tmp;
  for(unsigned int i=0;i<size;i++){
    if( (i+jump)>=size){
      value[i] = 0L; 
    }else{
       if(i+jump+1<=size){
          tmp=0L;
       }else{
          tmp = value[i+jump+1] << (32-shift);
       }
       value[i] = (value[i+jump] >> shift) | tmp;
    }
  }
}

/*
~ReadFrom~

This function reads the value of this bigint from a long value;

*/
  void ReadFrom(const long value){
    unsigned long v;
    if(value<0){
      signum=false;
      v = -value;
    }else{
      signum=true;
      v = value;
    }
    for(unsigned int i=1;i<size;i++){
      this->value[i]=0L;
    }
    unsigned long pos=0;
    this->value[0]=0;
    while(v>0){
      pos++;
      if((v&1L)!=0){
         this->value[0] = this->value[0] | 1L<<(pos-1); 
      }
      v = v /2;
    }
  }

/*
~ReadFrom~

*/
bool ReadFrom(char* content,size_t length){
    // test the first character
    ReadFrom(0); // first initialization
    if(length <=0)
       return false;
    char c;
    c = content[0];
    int state=0;
    bool ok = true;
    unsigned int pos = 0;
    bool overflow;
    BigInt<size> digit;
    bool sig = true;
    while(ok && (pos<length)){
      switch(state){
       case 0 : switch(c){
                  case '+' : state=1; break;
                  case '-' : state=1; sig=false; break;
                  case '0':
                  case '1': 
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':
                              ReadFrom(valueof(c));
                              state=2; 
                              break;
                  default : ok = false;
                }
                break;
       case 1 : switch(c){
                  case ' ' : state=2; break;
                  case '0':
                  case '1': 
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':
                             ReadFrom(valueof(c));
                             state=2; break;
                  default : ok = false;
                }
                break;
       case 2 : switch(c){
                  case '0':
                  case '1': 
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':  
                             MulInternal(TEN,overflow);
                             if(overflow){
                                 ok=false;
                             }
                             digit.ReadFrom(valueof(c));
                             AddInternal(digit,overflow);
                             if(overflow){
                                 ok = false;
                             }
                             break;
                    default : ok = false;
                }
                break;
       default : assert(false); // unknown state
      }
      pos++;
      if(pos<length)
          c = content[pos];
   }
   signum = sig;
   return ok;
  }



/*
~GetMax~

Return the maximal representable value of this instantiation of this class.

*/
static BigInt<size> GetMax(){
   // create a long value with 1 at each position
   unsigned long pos = 1;
   unsigned long v = 0;
   
   for(int i=0;i<32;i++){
      v = v | pos;
      pos = pos << 1;
   }  
   BigInt<size> result(0);
   for(unsigned int i=0;i<size;i++){
      result.value[i]=v;
   }
   return result;
}

/*
~GetMin~

Return the mininimum representable value.

*/
static BigInt<size> GetMin(){
   BigInt<size> result = GetMax();
   result.signum=false;
   return result;
}



/*
~WriteTo~

Writes this number binary to ~o~ without leading zeros.

*/
void WriteTo(std::ostream& o)const{
  if(!signum)
     o << "-";
  bool write=false;
  unsigned long pos;
  unsigned long test;
  for(int i=size-1;i>=0;i--){
    for(int j=31;j>-1;j--){
      pos = 1L << j;
      test=value[i]&pos;
      if(test!=0){
         write=true;
         o << "1";
      }  else{
        if(write) o << "0";
      }
    }
  } 
}

/*
~WriteComplete~

Writes this number to o in binary format
with leading zeros and signum.

*/
void WriteComplete(std::ostream& o)const{
  if(!signum)
     o << "-";
  else
     o << "+";
  unsigned long pos;
  unsigned long test;
  for(int i=size-1;i>=0;i--){
    for(int j=31;j>-1;j--){
      pos = 1L << j;
      test=value[i]&pos;
      if(test!=0){
         o << "1";
      }  else{
         o << "0";
      }
    }
  } 
}


/*
~WriteTo10~

This function writes the value of this bigint to o
using a base of ten.

*/
void WriteTo10(std::ostream& o)const{
  unsigned int maxsize = size*10; 
  unsigned char work[maxsize];
  for(unsigned int i=0;i<maxsize;i++){
     work[i]=0;
  }
  unsigned long pos;
  bool run = false;
  bool one = false;
  unsigned  long v;
  for(int i=(size-1);i>=0;i--){
     for(int j=31;j>-1;j--){
        pos = 1L << j;
        v = value[i]&pos;
        one = v!=0;
        if(one){
          if(!run){
            run = true;
            plus1(work,maxsize);
          }else{
               mul2(work,maxsize);
               plus1(work,maxsize);
          }
        }else{ // zero found
          if(run){
             mul2(work,maxsize);
          }
        }
     }
  }
  if(!signum )
     o << "-";
  write(o,work,maxsize);
}

/*
2.2.2 Operators

These operators can be used for making computation on
BigInt instances like on usual integer types.

*/
/*
~Arithmetik operators~

*/

BigInt<size> operator-(const BigInt<size>& i2){
   bool dummy;
   return Minus(i2,dummy);
}

BigInt<size> operator+(const BigInt<size>& i2){
   bool dummy;
   return Add(i2,dummy);
}

BigInt<size> operator*(const BigInt<size>& i2){
   bool dummy;
   return Mul(i2,dummy);
}

BigInt<size> operator/(const BigInt<size>& i2){
   BigInt<size> dummy; 
   return Div(i2,dummy);
}

BigInt<size> operator%(const BigInt<size>& i2){
   BigInt<size> remainder; 
   Div(i2,remainder);
   return remainder;
}

/*
~Assignment operator~

*/
BigInt<size>& operator=(const BigInt<size>& i2){
    Equalize(i2);
    return *this;
}

/*
~Comparision operators~

*/
bool operator<(const BigInt<size>& i1){
   return CompareTo(i1)<0;
}

bool operator>(const BigInt<size>& i1){
	 return CompareTo(i1)>0;
}

bool operator==(const BigInt<size>& i1){
 return Equals(i1); 
}

bool operator!=(const BigInt<size>& i1){
 return !Equals(i1); 
}

}; // end of class 

/*
~Output Operator~

*/


template<unsigned int size> std::ostream& 
          operator<<(std::ostream &o,const BigInt<size>& i){
   i.WriteTo10(o);
   return o;
}


/*
Definitions of some constants. Here are frequently used constants
defined.

*/
template<unsigned int size> const BigInt<size> BigInt<size>::ONE(1);
template<unsigned int size> const BigInt<size> BigInt<size>::ZERO(0);
template<unsigned int size> const BigInt<size> BigInt<size>::TEN(10);



#endif
