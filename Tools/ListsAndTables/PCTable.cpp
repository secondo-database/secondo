/*
1 Implementation File: Compact Table - Code for the Disk-Memory Implementation


Jan - May 2003 M. Spiekermann, Code was splitted into the two files CTable.cpp and PCTable.cpp.

August 2003 M. Spiekermann, Constructor changed. 


Note: Since debugging of template classes does not work properly, many instructions for
printing information at the display are included in this code. Comment it out if you need
this for bug-fixing. There is also some code which may be used for constructing a CTable
object from a stored Record on disk. Currently there is no need for this, but the code
presented here may be used as a first draft when implementing such a constructor. 

*/




/*

1.1 Constructor/Destructor of a CTable 

*/

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

  table = new PagedArray<T>(ptr2RecFile);
  valid = new PagedArray<bool>(ptr2RecFile);

  //cout << endl << "### PArrays created" << endl
  //	         << "### count: " << count << endl;


  // Initialization of count times slots is not necessary, since the PArray grows one by one.
  elemCount = 0;
  leastFree = 1;
  highestValid = 0;
  
  // store record ids of the persistent arrays
  oState.tableId = table->Id();
  oState.validId = valid->Id();     
  //oStateRecId = oStateRec->Id();

  //cerr << "### CTable(...) : (tableId, validId, masterRecordId) --> " 
  //     << oState.tableId << "," << oState.validId << "," << oStateRecId << endl; 
}


/* 

ToDo: Construction of a CTable from a stored version on disk.

template<typename T>

CTable<T>::CTable( SmiRecordId id, bool update ) : 
 oStateRec(id),  
 dummyElem(new T()),
 elemCount(oState.elemCount), 
 leastFree(oState.leastFree),  
 highestValid(oState.highestValid)
{
 initialize();
	
 oStateRecId = id;
	
 // construct CTable from persistent arrays
 oStateRec.Get(0, oState);

 SmiRecordFile
 
 table = new PArray<T>(&RecFile, oState.tableId, update);
 valid = new PArray<bool>(&RecFile, oState.validId, update);

 cerr << "### CTable( SmiRecordId id ) : (tableId, validId, masterRecordId) --> " 
      << oState.tableId << "," << oState.validId << "," << oStateRecId << endl; 
}

*/

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

/*
template<typename T>

SmiRecordId
CTable<T>::GetId() {
 
  // return ID of master record where the PArray record ids are stored.
  return oStateRecId;
  
}


template<typename T>

void 
CTable<T>::MarkDelete() {
	
  table->MarkDelete();
  valid->MarkDelete();
  oStateRec.MarkDelete();
}

*/


template<typename T>

void
CTable<T>::totalMemory( Cardinal &mem, Cardinal &pageChanges ) {
 
 T* ptrT = 0;
 bool* ptrb = 0;
 
 // calculation of allocated memory	 
 long dT = ((long)++ptrT); 
 long db = ((long)++ptrb);
  
 mem = (Cardinal)((dT + db) * elemCount);
 pageChanges = table->PageChanges() + valid->PageChanges(); 
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

  assert( n > 0 && n <= elemCount );

  static T elem;
  table->Get(n-1, elem);

  return elem;
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
	  
    valid->Put(elemCount+1, setFALSE);
    table->Put(elemCount+1, *dummyElem);
    elemCount++;
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
