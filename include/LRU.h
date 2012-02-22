
/*
---- 
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science, 
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


*/

#include <map>


template<typename Key, typename Value> class LRU;


template<typename Key,typename Value>
class LRUEntry{
  friend class LRU<Key,Value>;

  public:
    LRUEntry( const Key& _key, const Value& _value):
    key(_key), value(_value), prev(0), next(0)
    { }
   
     void connect(LRUEntry<Key,Value>* second){
        if(second==0){
          prev  = 0;
          next = 0;
          return;
        }
        next = second;
        prev = second->prev;
        second->prev = this;
        if(prev){
          prev->next = this;
        } 
     }

     void disconnect(){
        if(prev){
           prev->next = next;
        }
        if(next){
          next->prev = prev;
        }
        prev = 0;
        next = 0;
     }


    // public members    
    Key key;
    Value value;


   private:
    LRUEntry<Key,Value>* prev;
    LRUEntry<Key,Value>* next;
    
};



template<typename Key, typename Value>
class LRU{
  public:
/*
~Constructor~

Creates a new LRU chache with given capacity.

*/
    LRU(const size_t _maxEntries):
    first(0), last(0), maxEntries(_maxEntries), entries(0), m(),
    hits(0), failures(0), insertions(0), removements(0)
    { 
       if(maxEntries < 1){
          maxEntries = 1;
       }
    }

/*
~Destructor~

*/
    ~LRU() {
        clear();
    }

/*
~use~

If key is already stored, this operation marks the 
element as recently used. There is no check for
equality of stored and given value. If the key is not
stored, a new entry is inserted into the lru. In case 
of an overflow, the removed element is returned. 
It's the task of the caller to destroy this element.

*/
   LRUEntry<Key, Value>* use(Key key, Value value){

       LRUEntry<Key,Value>* elem;
       if(first==0){
          elem = new LRUEntry<Key,Value>(key,value);
          first = elem;
          last = elem;
          entries++;
          insertions++;
          m[key] = elem;
          return 0;
       }


       typename map<Key, LRUEntry<Key, Value>*>::iterator it = m.find(key);
       if(it == m.end()){
          elem = new LRUEntry<Key, Value>(key,value);
          m[key] = elem;
          entries++;
       } else {
          elem = it->second;
          if(elem==first){
             return 0;
          }

          if(elem==last){
            last = elem->prev;
          } 
          elem->disconnect();
       }
       elem->connect(first);
       first = elem;
       if(entries>maxEntries){
          removements++;
          return deleteLast();
       } else {
          return 0;
       }
    }
/*
~get~

Returns a pointer to a stored value or null if key is not present.

*/
    Value* get(Key key){
      typename map<Key, LRUEntry<Key, Value>*>::iterator it = m.find(key);
      if(it==m.end()){
        failures++;
        return 0; // not stored
      } else {
        hits++;
        LRUEntry<Key,Value>* hit = it->second;
        if(hit==first){
           return &(hit->value);
        } 
        if(hit==last){
           last = last->prev;
        }
        hit->disconnect();
        hit->connect(first);
        first = hit;
        return &(hit->value);
      }
    }

/*
~remove~

Removes a specific key from the cache. The LRU element storing this
element is returned. The caller has to destroy the element.

*/

    LRUEntry<Key,Value>* remove(Key key) {
      typename map<Key, LRUEntry<Key, Value>*>::iterator it = m.find(key);
      if(it==m.end()){
        return 0; // not stored
      } else {
        LRUEntry<Key,Value>* res = it->second;
        if(res==first){
          first = first->next;
        }
        if(res==last){
          last = last.prev;
        }
        res->disconnect;
        m.erase(it);
        entries--; 
      }

    }

    bool empty() const{
      return first==0;
    }

    void clear(){
       while(first){
          LRUEntry<Key,Value>* victim = first;
          first = first->next;
          victim->disconnect();
          delete victim;
       } 
       m.clear();
    }

    LRUEntry<Key,Value>* deleteLast(){
       if(!last){
         return 0;
       }
       entries--; 
       m.erase(last->key);
       if(last->prev==0){
          LRUEntry<Key,Value>* res = last;
          res->disconnect();
          last = 0;
          first = 0;
          return res;
       } else {
         LRUEntry<Key,Value>* victim = last;
         last = last->prev;
         victim->disconnect();
         return victim;
       }
    }
    

    void printStats(ostream& o){
       o << "maxEntries = " << maxEntries << endl
         << "entries    = " << entries << endl
         << "hits       = " << hits << endl
         << "failures   = " << failures << endl
         << "removed    = " << removements << endl; 
    }

  private:

    LRUEntry<Key, Value>* first;
    LRUEntry<Key, Value>* last;
    size_t maxEntries;
    size_t entries;
    map<Key, LRUEntry<Key, Value>*> m;


    // statistic values
    size_t hits;
    size_t failures;
    size_t insertions;
    size_t removements;


};


