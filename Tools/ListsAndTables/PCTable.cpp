/*
1 Implementation File: Compact Table - Code for the Disk-Memory Implementation


Jan - May 2003 M. Spiekermann, Code was splitted into the two files CTable.cpp and PCTable.cpp.

August 2003 M. Spiekermann, Constructor changed. 

June 2004 M. Spiekermann, Operator [] changed.


Note: Since debugging of template classes does not work properly, many instructions for
printing information at the display are included in this code. Comment it out if you need
this for bug-fixing. There is also some code which may be used for constructing a CTable
object from a stored Record on disk. Currently there is no need for this, but the code
presented here may be used as a first draft when implementing such a constructor. 

*/




/*

1.1 Constructor/Destructor of a CTable 

*/

#include <typeinfo>


template<typename T>

string
CTable<T>::MemoryModel() {
	return "PERSISTENT";
}



template<typename T>

void
CTable<T>::initialize() {

  oState.elemCount = 0;
  oState.leastFree = 0;
  oState. highestValid = 0;
  setTRUE = true;
  setFALSE = false;
  doRecFilePtrDelete = false;

}


template<typename T>

CTable<T>::CTable(  Cardinal const count, SmiRecordFile* _ptr2RecFile /* = 0 */) : 
 dummyElem(new T()),
 elemCount(oState.elemCount), 
 leastFree(oState.leastFree),  
 highestValid(oState.highestValid)
{

 initialize(); 
 ptr2RecFile = _ptr2RecFile;

 if (ptr2RecFile == 0) {
     bool ok = false;
     ptr2RecFile = new SmiRecordFile(false,0,true);
     ok = ptr2RecFile->Create();
     doRecFilePtrDelete = true;
     assert( ok == true ); 
  }

  //cout << endl << "### CTable<T>::CTable(" << endl 
  //             << "ptr2RecFile: " << (void*) ptr2RecFile << endl;

  table = new PagedArray<T>(ptr2RecFile, true);
  valid = new PagedArray<bool>(ptr2RecFile);

  //cout << endl << "### PArrays created" << endl
  //	         << "### count: " << count << endl;

  // Initialization of count times slots is not necessary, since the PArray grows one by one.
  elemCount = 0;
  leastFree = 1;
  highestValid = 0;
  
}

template<typename T>

CTable<T>::~CTable() {

  //oStateRec.Put(0, oState);
  
  delete dummyElem;
  delete table;
  delete valid;

  if (doRecFilePtrDelete) {
    ptr2RecFile->Close();
    delete ptr2RecFile;
  }
}


template<typename T>

void
CTable<T>::totalMemory( Cardinal &mem, Cardinal &pageChanges, Cardinal &slotAccess ) {
 
 T* ptrT = 0;
 bool* ptrb = 0;
 
 // calculation of allocated memory	 
 long dT = ((long)++ptrT); 
 long db = ((long)++ptrb);
  
 mem = (Cardinal)((dT + db) * elemCount);
 pageChanges = table->PageChanges(); 
 slotAccess = table->SlotAccess();
}


/*

1.1  Write to disk.

*/

template<typename T>

void
CTable<T>::Put( Cardinal const n,  T& elem ) {

  assert( n > 0 && n <= elemCount );

  if ( n == leastFree ) {

    bool isvalid = false;
    do {

      ++leastFree;      
      valid->Get(leastFree-1, isvalid);
    }
    while ( leastFree <= elemCount && isvalid );
  }

  valid->Put(n-1, setTRUE);

  if (highestValid < n) {

    highestValid = n;
  }
  
  table->Put(n-1, elem);
}



/*

1.1 Read from Disk

*/

template<typename T>

void
CTable<T>::Get( Cardinal const n, T& elem ) {

  assert( n > 0 && n <= elemCount );

  table->Get(n-1, elem);
}


/*

1.1 For convenience: Access of an element as an rvalue

*/

template<typename T>

const T&
CTable<T>::operator[]( Cardinal n ) {

  static Cardinal lastIndex = 0;
  static T elem;
  T* elemPtr = new T;

  if ( !(n > 0 && n <= elemCount) ) {
     cout << "Type " << typeid(elem).name() << " called operator[" << n << "]" << endl;
     assert( n > 0 && n <= elemCount );
  }

  //if ( lastIndex != n ) { // check if call of Get() is necessary
    table->Get(n-1, *elemPtr);
    lastIndex = n;
  //}

  return *elemPtr;
}



/*

1.1 Check whether an element is valid

*/

template<typename T>

bool
CTable<T>::IsValid( Cardinal const index ) {

  assert( index > 0 && index <= elemCount );

  bool test;
  valid->Get(index-1, test); 
  //cerr << "### IsValid : " << " test = " << test << endl;
  
  return test;
}


template<typename T>

const Cardinal
CTable<T>::EmptySlot() {

  //cerr << "### EmptySlot: " << "leastFree, elemCount --> " << leastFree << "," << elemCount << endl;
  if ( leastFree > elemCount ) {
	  
    elemCount++;
    valid->Put(elemCount, setFALSE);
    table->Put(elemCount, *dummyElem);
  }
  //cerr << "### EmptySlot: " << "leastFree, elemCount --> " << leastFree << "," << elemCount << endl; 
  return leastFree;
}



template<typename T>

const Cardinal
CTable<T>::Add( const T& element ) {

  Cardinal index = EmptySlot();
  valid->Put(index-1, setTRUE);
  
  T elem = element;
  table->Put(index-1, elem);

  if ( index == leastFree ) {

    bool isvalid=false;
    do {
	    
      ++leastFree;
      valid->Get(leastFree-1, isvalid);
    }
    while ( leastFree <= elemCount && isvalid );
  }

  if (highestValid < index) {
    highestValid = index;
  }

  return index;

}


template<typename T>

void
CTable<T>::Remove( Cardinal const index ) {

  assert( index > 0 && index <= elemCount );
 
  valid->Put(index-1, setFALSE);

  if ( index < leastFree ) {

    leastFree = index;
  }
  
  if ( index == highestValid ) {
    
    bool isvalid = false;
    do {

      --highestValid;
      valid->Get(highestValid-1, isvalid);
    }
    while ( highestValid > 0 && !isvalid );
  }

}



/*

1.1 Constructor for iterator

*/

template<typename T>

CTable<T>::Iterator::Iterator( CTable<T>* ctPtr ) {

  ct = ctPtr;
  current = 0;

  bool isvalid = false;
  ct->valid->Get(current, isvalid);
  while ( current < ct->highestValid && !isvalid ) {
    ++current;
    ct->valid->Get(current, isvalid);
  }
}



/*

1.1 Copy constructors for iterator

*/




/*

1.1 Dereference of an iterator

*/

template<typename T>

const T&
CTable<T>::Iterator::operator*() const {

  assert( ct != 0 );

  bool isvalid = false;
  ct->valid->Get(current, isvalid);
  assert( current < ct->elemCount && isvalid );

  static T elem;
  ct->table->Get(current, elem);
  
  return elem;
}


/*

1.1 Iterator increment (prefix and postfix notation)

*/

template<typename T>

typename CTable<T>::Iterator&
CTable<T>::Iterator::operator++() {

  assert( ct != 0 );

  // increment as long until next slot is valid
  bool isvalid = false;
  do {
     if (current >= ct->highestValid) break;
     current++;
     ct->valid->Get(current, isvalid);
     //cerr << "### it_++prefix: current = " << current << endl;
  }
  while (!isvalid);
  
  return *this;
}



template<typename T>

const typename CTable<T>::Iterator
CTable<T>::Iterator::operator++( int ) {

  assert( ct != 0 );
  CTable<T>::Iterator temp( *this );

  // increment as long until next slot is valid
  bool isvalid = false;
  do {
     if (current >= ct->highestValid) break;
     current++;
     ct->valid->Get(current, isvalid);
     //cerr << "### it_++prefix: current = " << current << endl;
  }
  while (!isvalid);

  return temp;
}
