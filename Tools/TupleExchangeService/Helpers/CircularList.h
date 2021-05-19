/*

*/

#ifndef SECONDO_CIRCULARLIST_H
#define SECONDO_CIRCULARLIST_H

#include <list>
#include <iostream>


namespace distributed3 {

template<typename T>
  class CircularList {
  public:
    CircularList() : lst{}, run{lst.begin()} {}
    
    T next() {
      if (run == lst.end()) {
        //cout << "\nCircularList:next(): am Ende angekommen";
        if (lst.empty()) throw std::out_of_range{""};
        run = lst.begin();
      }
      T& result = *run;
      ++run;
      return result;
    }
    bool isEmpty() {
      return lst.empty();
    }
    int size() {
      return lst.size();
    }
    void remove() {
      lst.erase(prev(run));
    }
   
    void clear() {
      lst.clear();
    }
    void push_back(T x) {
      lst.push_back(x);
    }
    void push_front(T x) {
      lst.push_front(x);
    }
  private:
    std::list<T> lst;
    typename std::list<T>::iterator run;
  };
}
#endif // SECONDO_CIRCULARLIST_H
