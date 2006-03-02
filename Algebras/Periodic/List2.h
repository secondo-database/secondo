/*
1   The File List2.h 

*/


#ifndef LIST_H
#define LIST_H

#ifndef NULL
#define NULL 0
#endif

// forward declaration of the list class
template<class T>class  List;

// forward declaration of the Iterator class
template<class T> class Iterator;

/*
1.1 The Class Element

This class provides Elements for a double linked list.


*/
template<class T>
class Element{
 friend class List<T>;
 friend class Iterator<T>;
 public:

/*
~Constructor~

Creates a new element whith the given entry.

*/
   Element(T entry){
      this->entry=entry;
      next=NULL;
      prev=NULL;
   }
/*
~Connect~

Connects this element with the argument.

*/   
   void Connect(Element* e){
      this->next= e;
      e->prev=this;
   }

/*
~Destructor~

The destructor does nothing. To delete the entry of the element,
use the destroy function first.

*/
   ~Element(){}

/* 
~Destroy~

This function deletes the entry of this element. This should only be
used in combination with the destructor.

*/
   void Destroy(){
     entry = 0;
   }


/*
~GetEntry~

Returns the content of this element.

*/
   T GetEntry(){
      return entry;
   }

/*
~GetNext~

*/
  Element<T>* GetNext(){ return next;}

/*
~GetPrev~

*/
  Element<T>* GetPrev(){ return prev; }

private:
    T  entry;
    Element<T>* next;
    Element<T>* prev;
};


/*
1.2 The Class List

This class provides a double linked list with operations to
insert, delete, and ieratating over elements.

*/


template<class T>
 class List{
  friend class Iterator<T>;
  public:

/**
~Constructor~

This constructor creates a new empty double linked list.

*/
     List(){
        first=NULL;
        last=NULL;
        current=NULL;
        length=0;
     }


/*
~IsEmpty~

This function returns true iff the list does not contain 
any element.

*/

  bool IsEmpty(){
     return first==0;
  }



/**
~Append function~

This function appends a new element at the end of the list. 
The position of the current element is not influenced by this
operation except the list original list was empty.

*/
    void Append(T  entry){
       Element<T> *ne = new Element<T>(entry);
       if(!last){ // the first element
         first = ne;
         current = ne;
         last = ne;
        }else{
          last->Connect(ne);
          last=ne;
        }
        length++;
    }


/**
~The Insert function~

This function inserts a new element before the current element. 
The new element will be also the new current element. For inserting
an element at the end of the list, use the append function. 

*/
   void Insert(T entry){
      Element<T>* ne = new Element<T>(entry);
      if(!current){  // special case : list is empty
         first = ne;
         last = ne;
         current=ne;
       }else if(current==first){ // special case: the first element
         ne->connect(first);
         current=ne;
         first=ne;
        }else{ // normal case: insert in the middle
           Element<T>* prev = current->prev;
           prev->connect(ne);
           ne->connect(current);
           current=ne;
        }
      length++;
   }


/**
~Contains function~

This function checks whether the given element is a member of
this list. 

*/

   bool Contains(T entry){
      Element<T>* scan = first;
      while(scan){
        if((scan->entry)==entry)
             return true;
        scan = scan->next;
      }
      return false;
   }

/**
~The GoStart function~

This function sets the current element of the list to 
the begin of the list.

*/
    void GoStart(){
       current = first;
    }
    
/**
~The GoEnd function~

When calling this function, the current element of this list is 
set to the last element of this list.

*/
   void GoEnd(){
      current = last;
   }

/**
~GoNext~

This function moves the current element one position to behind.
If the list is empty or the current element is already at the end of this
list, the list remains unchanged. The return value is an indicator for the
success of this function.

*/
   bool GoNext(){
     if(current==last){ //is also the case when the list is empty
        return false;
     } 
     current = current->GetNext();
     return true;
   }

/*
~GoBack~

This function moves the current element back one position if possible.

*/
   bool GoBack(){
      if(current==first)  // also if the list is empty
        return false;
      current = current->prev;
      return true;
   }


/* 
~OnEnd~

This function returns false if after the current element at leat one
further element exists.

*/
   bool OnEnd(){
     return current==last;
   }

/*
~OnStart~

This function returns true if the current element is at the begin of the list.

*/
   bool OnStart(){
     return current==first;
   }



/*
~Get~

This function returns the element under the cursor. If the list is empty,
the return value will be NULL.

*/
   T Get(){
     if(!current)
        return NULL;
     return current->entry;
   }

/*
~GetFirst~

Returns the fisrt element from this list. If this list is empty,
NULL is returned.

*/
    T GetFirst(){
      if(!first) return NULL;
      return first->entry;
    }

/*
~ GetLast~

Returns the last element from this list. If the list is
empty, null will be returned.

*/
    T GetLast(){
      if(!last) return NULL;
      else return last->entry;
    }
/*
~Delete~

This function deletes the element under the cursor. The new current element 
will be the element after the original one if available otherwise the 
element before it.

*/
  bool Delete(){
		 if(!current)
            return false;
		 Element<T>* victim = current;
		 if(current==first){ // remove the first element
			 current = current->GetNext();
			 first= current;
			 if(current){
		      current->prev=NULL;
			 }else{
         if(victim==last) // the only element in this list
		         last = NULL;
			 }
			 delete victim;
		 } else if(current==last){ //remove the last element;
				 last = last->prev;
				 current=last;
				 if(current)
		       current->next = NULL;
				 delete victim;
		 }else{ // normal case delete in the middle
				 current = current->next;
				 (victim->prev)->connect(current);
				 delete victim;
		 }
     length--;
     return true;
  }


/*
~Destructor~

This destructor removes all elements from the list. Because the 
destroy function of the elements is not called within this function, 
the content in the elements are not deallocated. 

*/

  ~List(){
     while(first){
       current=first;
       first=first->next;
       delete(current);
     }
   }



/*
~Destroy~


This function removes all elements of this list. Also the
content of the single entries is removed. After calling this 
function, the list will be empty.

*/
  void Destroy(){
     while(first){
        current=first;
        first=first->next;
        current->Destroy();
        delete current;
      }
      first = current = last = 0;
      length=0;
  }


/*
~GetLength~

This function computes the length of this list.
In a later version isd would be better to store the
length in an extra member.

*/
   int GetLength()const{
      return length;
   }

/*
~ConvertToArray~

This function creates a new array containing the 
elements in the list. The caller has to deallocate 
the memory occupied by this array.

*/
   T* ConvertToArray(){
     T* res = new T[length];
     Element<T>* scan = first;
     int pos=0;
     while(scan){
        res[pos++]=scan->entry;
        scan = scan->next;
     }
     return res;
   }




  private:
     Element<T>* first;
     Element<T>* last;
     Element<T>* current;
     int length;
};


/*
1.3 An Iterator class 

Note that you can't change (insert,delete,append) the list when 
you use an iterator on it. 

*/


template<class T> class Iterator{
 public:

/*
~Constructor~

*/
   Iterator(List<T>* L){
      this->elem = L->first;
      this->L = L;
   }

/*
~Reset~

This function sets the position of this iterator to the
begin of the list.

*/   
   void Reset(){
      elem = L->first;
   }
/*
~HasNext~

This function checks whether a further element is available.

*/
   bool HasNext(){
       return (elem!=0);
   }

/*
~Next~

This function returns the next value of this iterator if available.
If no next element exists (check it by the HasNext function), the
result will be undefined.

*/
   T Next(){
      T res;
      if(!elem) return res;
      res = elem->entry;
      elem = elem->next;
      return res;
   }
/*
~SetList~

Changes this iterator to be an iterator on the argument.

*/
   void SetList(List<T> l){
       this->L = l;
       this->elem=L->first;
   }

 private:
   List<T>* L;
   Element<T>* elem; 

};



#endif
