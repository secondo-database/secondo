/*
This file is part of SECONDO.

Copyright (C) 2018, 
University in Hagen, 
Faculty of Mathematics and of Computer Science,
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


1 Class DBATree

This class provides an  AVL tree implementation within a DBArray

*/

#ifndef DBATREE_H
#define DBATREE_H

#include <Tools/Flob/DbArray.h>
#include <ostream>


namespace dbatree{

/*
1.1 Class StdComparator

This class privides a comparator function for the most 
standard types.

*/
template <class Elem>
class StdComparator{
  public:
     int operator()(const Elem& e1, const Elem& e2) const{
        if(e1<e2) return -1;
        if(e1==e2) return 0;
        return 1;
     }
};


template<class Elem, class Comp = StdComparator<Elem> >
class DBATree{

 private:

/*
1.2 Class TreeEntry

This class is used to represent a single node in the tree.

Besides the element to store, it contains symbolic links to
the childs and the father as well as the information
of the height of the corresponding subtree.

*/ 
 class TreeEntry{
  public:

/*
1.2.1 Constructor

The standard constructor does nothing. This is a requirement of
the DBArray.

*/  
     TreeEntry(){}

/*
1.2.2 Constructor

This constructor create a node without connections.

*/
     TreeEntry(Elem e): elem(e), left(-1),right(-1),father(-1),height(1){}

/*
1.2.3 Copy Constructor

*/
     TreeEntry(const TreeEntry& e): elem(e.elem), left(e.left), 
             right(e.right), father(e.father),height(e.height){}

/*
1.2.4 Assignment operator

*/
     TreeEntry& operator=(const TreeEntry& e){
        elem = e.elem;
        left = e.left;
        right = e.right;
        father = e.father;
        height = e.height;
        return *this;
     }

/*
1.2.5 swapConnections

This operator swaps all members of this and e except the content.

*/     
     void swapConnections(TreeEntry& e){
       std::swap(left,e.left);
       std::swap(right,e.right);
       std::swap(father,e.father);
       std::swap(height, e.height);
     }

/*
1.2.6 print

Writes a readable representation to out.

*/
     std::ostream& print(std::ostream& out)const{
      //out << elem << ", l:" << left << ",R:" << right 
      //    << ",f: " << father << ",h:" << height;
      return out;
     }

/*
1.2.7 Member variables

Because this class is only accessible from the Tree class,
all members are public.

*/
     Elem elem;
     int left;
     int right;
     int father;
     int height;
  };



  public:

/*
1.3 Class DBATree

1.3.1 Standard Constructor

This constructor creates an empty tree.

*/
     DBATree( ): array(0), lastPos(0), cmp(), root(-1){}

/*
1.3.2 ~clear~

Removes all entries from the tree.

*/
     void clear(){
        array.clean();
        lastPos = 0;
        root = -1;
     }


     void destroy(){
         clear();
         array.Destroy();
     }


/*
1.3.3 ~inserts~

Inserts an element to the tree. If a corresponding entrie already 
exists in the tree, the tree is keept unchanged.

*/
     bool insert(const Elem& elem){
        TreeEntry te(elem);
        if(lastPos==0){ // first elem in tree
           array.Put(0,te);
           lastPos++;
           root = 0;
           return true;
        } else {
           bool ok = insert(root,te,false);
           return ok;
        }
     }


/*
1.3.4 ~insertOrUpdate~

Inserts a element into the tree. If the element is already part of the 
tree, its value is overwritten by the new value. This makes sense if 
the comparison function only uses a part of elem.

*/
     bool insertOrUpdate(const Elem& elem){
        TreeEntry te(elem);
        if(lastPos==0){ // first elem in tree
           array.Put(0,te);
           lastPos++;
           root = 0;
           return true;
        } else {
           bool ok = insert(root,te,true);
           return ok;
        }
     }


/*
1.3.5 ~contains~

Checks whether an element with given value is part of the tree.

*/

     inline bool contains(const Elem& elem){
       return getIndex(elem,false) >=0;
     }

/*
1.3.6 ~update~

Updates an entry in the tree.

*/
    inline bool update(const Elem& elem){
       return getIndex(elem,true) >=0;
    }

/*
1.3.7 ~get~

Changes the value of elem to that is stored in the tree.
If the element is not found, the argument is not changed.

*/
    inline bool get(Elem& elem){
       return get(root,elem);
    }


/*
1.3.6 remove
 
Removes an element from the tree.
 
*/
     inline bool remove(const Elem& elem) {
        if(lastPos==0) {
           return false;
        }
        return remove(root,elem);
     }

     inline bool isEmpty() const{
         return lastPos == 0;
     }


/*
1.3.7 print

Prints out a textual representation of the tree.

*/
     std::ostream& print(std::ostream& out) const{
       out << "( tree ";
       printRec(root,out,true);
       out << ")";
       return out;
     }


/*
1.3.8 printArray

prints out the internal representation of the tree

*/
     std::ostream& printArray(std::ostream& out) const{
        out << "size of Array " << array.Size() << std::endl;;
        out << "lastPos " << lastPos << std::endl;
        out << "root " << root << std::endl;
        out << "Contents" << std::endl;
        TreeEntry e ;
        for(int i=0;i<array.Size();i++){
           array.Get(i,e);
           out << i << ":" ; e.print(out) << std::endl;
        }
        return out;
     }


/*
1.3.9 check

This function checks whether the structire of the tree is correct.

*/
    bool check(){
       bool res =  check(root,true);
       if(!res){
          std::cerr << " invalid tree structure" << std::endl;
          printArray(std::cerr) << std::endl << std::endl;
          print(std::cerr) << std::endl << std::endl;
       }
       return res;
    }

  private:

/*
1.3.10 Member variables

*/
    DbArray<TreeEntry> array;  // container
    size_t lastPos; // number of nodes in the tree
    Comp cmp;   // the used comparator
    int root;   // index of tree's root


/*
1.3.11 insert

Inserts a new element or updates an existing one.

*/
    bool insert(int root, TreeEntry& elem, bool update){
        TreeEntry current;
        bool done = false;
        while(!done){
           array.Get(root,current);
           int c = cmp(elem.elem, current.elem);
           if(c==0){
              if(update){
                  current.elem = elem.elem;
                  array.Put(root,current);
              }
              return false; // element already present
           }
           if(c<0){
              if(current.left<0){ // found insertion pos
                 current.left = lastPos;
                 array.Put(root,current);
                 elem.father = root;
                 array.Put(lastPos,elem);
                 lastPos++;
                 done = true;
              } else {
                 root = current.left;
              }
           } else {
              if(current.right<0){
                 current.right = lastPos;
                 array.Put(root,current);
                 elem.father = root;
                 array.Put(lastPos,elem);
                 lastPos++;
                 done = true;
              } else {
                 root = current.right;
              }
           }
        }
        // correct heights if necessary
        correctPath(root);
        return true; 
    }

    void correctPath(int index){
      while(index >=0 ){
         index = balance(index);
      }
    }


/*
1.3.12 ~getIndex~

Returns the index of an stored element or -1 if the element is 
not part of the tree. If update is set to true, an existing 
element is overwritten by the input argument.

*/

    int getIndex(const Elem& elem,bool update) {
       if(lastPos==0) return -1;
       int root = this->root;
       TreeEntry current;
       while(true){
         array.Get(root,current);
         int c = cmp(elem, current.elem);
         if(c==0){
            if(update){
                current.elem = elem;
                array.Put(root,current);
            }
            return root; 
         }
         if(c<0){
           if(current.left<0){ 
              return -1;
           }
           root = current.left;
         } else {
           if(current.right<0){
             return -1;
           }
           root = current.right;
         }
        }
        return -1; 
    }

/*
1.3.12 ~get~

*/

    bool get(int root, Elem& elem) {
       if(lastPos==0) return -1;
       root = this->root;
       TreeEntry current;
       while(true){
         array.Get(root,current);
         int c = cmp(elem, current.elem);
         if(c==0){
            elem = current.elem;
            return true; 
         }
         if(c<0){
           if(current.left<0){ 
              return false;
           }
           root = current.left;
         } else {
           if(current.right<0){
             return false;
           }
           root = current.right;
         }
        }
        return false; 
    }
      
/*
1.3.13 Remove

Removes a given element from the tree. Returns true if the
element was found, false otherwise.

*/
    bool remove(int dummy, const Elem& elem){

       int index = getIndex(elem,false);
       if(index < 0){
          return false;
       }

       TreeEntry victim;
       array.Get(index, victim);
       if(victim.left<0 || victim.right < 0){
          int father = removeEnd(index);
          correctPath(father);
          return true;
       }
       int min = getMinIndex(victim.right);
       swap(index,min);
       int father = removeEnd(min);
       correctPath(father);
       if(lastPos==0){
         this->root = -1;
       }
       return true;
    }

/*
1.3.14 swap

Swaps the contents of two entries in the array.
This will destroy the ording of the tree but keep its
structure.


*/
    void swap(int index1, int index2){
        if(index1==index2) return;
        TreeEntry e1;
        TreeEntry e2;
        array.Get(index1,e1);
        array.Get(index2,e2);
        e1.swapConnections(e2);
        array.Put(index2,e1);
        array.Put(index1,e2);
    }

/*
1.3.15 ~removeEnd~

Removes an entry from the array. The element at the specified 
index can have at most one child. 

*/
    int removeEnd(int index) {

       TreeEntry victim;
       array.Get(index,victim);
       int r = victim.father;

       assert(victim.left<0 || victim.right<0);
       // disconnect this node 
       if(r>=0){
          TreeEntry father;
          array.Get(r,father);

          if(father.left==index){
             if(victim.left>=0) {
                father.left = victim.left;
                setFather(victim.left, victim.father);
             } else if(victim.right>=0){
                father.left = victim.right;
                setFather(victim.right,victim.father);
             } else {
                father.left = -1;
             }
          } else {
             assert(father.right==index);
             if(victim.left>=0) {
                father.right = victim.left;
                setFather(victim.left, victim.father);      
             } else if(victim.right>=0){
                father.right = victim.right;
                setFather(victim.right, victim.father);
             } else {
                father.right = -1;
             }
          }
          array.Put(victim.father,father);
       } else { // removing the root of the tree
          if(victim.left>=0){
             this->root = victim.left;
             setFather(victim.left, -1);
          }  else if(victim.right>=0){
             this->root = victim.right;
             setFather(victim.right,-1);
          } else {
              this->root = -1;
          }
       } 
       return freePosition(index, r);
    } 


/*
1.3.16 ~setFather~

Changes the information about the father of a node.

*/
    void setFather(int index, int father){
        if(index<0) return; // empty subtree has no father
        TreeEntry elem;
        array.Get(index,elem);
        elem.father = father;
        array.Put(index,elem);
    }

/*
1.3.17 ~getMaxIndex~

Returns the index of the maximum value stored in the 
subtree rooted by root.

*/
    int getMaxIndex(int root) const{
       TreeEntry current;
       array.Get(root,current);
       while(current.right>=0){
          root = current.right;
          array.Get(root,current);
       }
       return root;
    }

/*
1.3.17 ~getMinIndex~

Returns the index of the minimum value stored in the 
subtree rooted by root.

*/
    int getMinIndex(int root) const{
       TreeEntry current;
       array.Get(root,current);
       while(current.left>=0){
          root = current.left;
          array.Get(root,current);
       }
       return root;
    }


/*
1.3.18 ~freePosition~

Removed an entry in the array. If the entry is not the 
last one in the array, the last entry is moved to that
position and the last entry is removed. It returns the
possible new position of someNode, i.e. someNode or index 
if someNode was at the last position in the array.

*/
    int freePosition(int index,int someNode){
       assert(lastPos>0);
       lastPos--;
       if(someNode==lastPos){
          someNode = index;
       }
       if(index==lastPos){ // last used array element, no movement required
          return someNode;
       }
       // move element at last position to index position
       TreeEntry entry;
       array.Get(lastPos,entry);
       array.Put(index,entry);

       // correct connection of father and childs
       replaceSon(entry.father, lastPos, index);
       setFather(entry.left, index);
       setFather(entry.right, index);
       return someNode;
    }


/*
1.3.19 ~printRec~

recursive printing out the tree.

*/ 
    void printRec(int root, std::ostream& out, bool full) const{
        if(root<0){
           out << " \"\" ";
        } else {
           TreeEntry entry;
           array.Get(root,entry);
           out << "(";
           printLabel(root,entry,out,full);
           printRec(entry.left,out,full);
           printRec(entry.right,out,full);
           out << ")";
        }
    }


/*
1.3.20 ~printLabel~

Auxiliary function for printing out the tree.

*/
    void printLabel(int index, TreeEntry& e, std::ostream& out,
                    bool full) const{
         out << "\"";
         out << e.elem;
         if(full){
            out << " [i: " << index << ", f: " << e.father 
                << ", h: " << e.height << "]";
         }
         out << "\"";
         
    }


/*
1.3.21 ~correctHeight~

Changes the height information of a node based on the height 
information of its direct childs.

*/
    bool correctHeight(TreeEntry& elem) {
        int nh = max(getHeight(elem.left) , getHeight(elem.right))+1;
        if(elem.height!=nh){
            elem.height = nh;
            return true;
        }
        return false;
    } 

/*
1.3.22 ~getHeight~

Returns the height of a node.

*/
    int getHeight(int index){
       if(index<0) return 0; // empty tree
       TreeEntry elem;
       array.Get(index,elem);
       return elem.height;
    }

/*
1.2.23 max

Returns the maximum of two integer values.

*/
    inline static int max(int a, int b){
      return a>b?a:b;
    }

/*
1.2.24 ~abs~

Returns the absolute of an integer value.

*/
    inline static int abs(int a){
       return a<0?-a:a;
    }
    
/*
1.2.25 ~getBalanceAndCorrectHeight~

Returns the balance value of a node. If the height of this 
node is not correct, the correct height will be stored.

*/
    int getBalanceAndCorrectHeight(int index, TreeEntry& elem){
       int hl = getHeight(elem.left);
       int hr = getHeight(elem.right);
       int nh = max(hl,hr) + 1;
       if(elem.height!=nh){
         elem.height = nh;
         array.Put(index,elem);
       }
       return hl - hr;
    }

/*
1.2.26 ~balance~

Balances a single node if necessary and returns the father of this node.

*/
    int balance(int index){
       TreeEntry x;
       array.Get(index,x);
       int b = getBalanceAndCorrectHeight(index, x);
       if(abs(b) < 2){
         // no balancing required
         return x.father;
       }

       if(b < -1 ){  // right subtree is heigher
          TreeEntry y;
          array.Get(x.right,y);
          int by = getBalanceAndCorrectHeight(x.right,y);
          if(by <= 0){  // case a1
             return rotateLeft(index,x, x.right,y);
          } else { // case a2
             TreeEntry z;
             array.Get(x.right,z);
             array.Get(z.left,y);
             return rotateRightLeft(index, x, x.right, z, z.left,y);
         }
       } else { // left subtree is heigher
          TreeEntry y;
          array.Get(x.left,y);
          int by = getBalanceAndCorrectHeight(x.left,y);
          if(by>=0) {
             return rotateRight(index,x, x.left,y);
          } else {
             TreeEntry z;
             array.Get(x.left,z);
             array.Get(z.right,y);
             return rotateLeftRight(index, x, x.left, z, z.right,y);
          }
       }
    }


/*
1.3.26 replaceSon

Replaces the son oldson by newson of node at index father.

*/
    void replaceSon(int father, int oldson, int newson){

        if(father<0){
            assert(root==oldson);
            root = newson;
        } else {
            TreeEntry f;
            array.Get(father,f);
            if(f.left == oldson){
               f.left = newson;
            } else if(f.right==oldson){
               f.right = newson;
            } else {
              std::cerr << " try to replace son of node at index " 
                        << father << std::endl;
              std::cerr << " son to replace " << oldson << std::endl;
              std::cerr << " new son " << newson << std::endl;
              std::cerr << " father.left = " << f.left << std::endl;
              std::cerr << " father.right = " << f.right << std::endl;
              assert(false);
            }
            correctHeight(f);
            array.Put(father, f);
        }
    }

/*
1.3.26 Left Rotation

*/

    int rotateLeft(int posx, TreeEntry& x, 
                    int posy, TreeEntry& y){

       int father = x.father;
       x.right = y.left;
       setFather(y.left,posx);
       x.father = posy;
       correctHeight(x);
       array.Put(posx,x);

       y.left = posx;
       y.father = father;
       correctHeight(y);
       array.Put(posy,y);

       replaceSon(father,posx, posy);
 
       return father;
    }

/*
1.3.27 Right rotation

*/


    int rotateRight(int posx, TreeEntry& x,
                    int posy, TreeEntry& y){

       int father = x.father;
       x.left = y.right;
       setFather(y.right,posx);
       x.father = posy;
       correctHeight(x);
       array.Put(posx,x);

       y.right = posx;
       y.father = father;
       correctHeight(y);
       array.Put(posy,y);

       replaceSon(father,posx,posy);

       return father;
    }


/*
1.3.28 Double rotation Right Left

*/

    int rotateRightLeft(int posx, TreeEntry& x,
                         int posz, TreeEntry& z,
                         int posy, TreeEntry& y) {


        int father = x.father;

        x.right = y.left;
        setFather(y.left,posx);
        x.father = posy;
        correctHeight(x);
        array.Put(posx,x);
       
        z.left = y.right;
        setFather(y.right,posz);
        z.father = posy;
        correctHeight(z);
        array.Put(posz,z);

        y.left = posx;
        y.right = posz;
        y.father = father;
        correctHeight(y);
        array.Put(posy,y);

        replaceSon(father,posx,posy);

        return father;
    }

/*
1.3.29 Double Rotation Left Right

*/
    
    int rotateLeftRight(int posx, TreeEntry& x,
                         int posz, TreeEntry& z,
                         int posy, TreeEntry& y){

       int father = x.father;
       z.right = y.left;
       setFather(y.left,posz);
       z.father = posy;
       correctHeight(z);
       array.Put(posz,z);
  
       x.left = y.right;
       setFather(y.right,posx);
       x.father = posy;
       correctHeight(x);
       array.Put(posx,x);
 
       y.left = posz;
       y.right = posx;
       y.father = father;
       correctHeight(y);
       array.Put(posy,y);

       replaceSon(father, posx, posy);

       return father;
    }


/*
1.3.30 ~checkFather~

Checks whether son is a son of father

*/
    bool checkFather(int father, int son){
       if(father>=0){
          TreeEntry f;
          array.Get(father,f);
          if( f.left!=son && f.right!=son){
             return false;
          }
       }
       if(son>=0){
           TreeEntry s;
           array.Get(son,s);
           if(s.father !=father){
              return false;
           }
       }
       return true;
    }


/*
1.3.31 ~check~

Checks the structure of the tree rooted by index.
If rec is true, all substrees are checkes.
If checkBalance is set to false, checking height 
information is omitted.

*/
    bool check(int index, bool rec, bool checkBalance = true){
       if(index < 0) return true;

       if(index >=lastPos){
         std::cerr << "try to check correctness of index " 
                   << index << std::endl;
         std::cerr << "that is outside the allowed range " 
                   << lastPos << std::endl;
         return false;
       }

       TreeEntry elem;
       array.Get(index, elem);
       // check for height and balance
       if(checkBalance){
         int lh = getHeight(elem.left);
         int rh = getHeight(elem.right);
         int h = max(lh,rh)+1;
         if(h!=elem.height){
           std::cerr << "invalid height entry at index " << index << std::endl;
           std::cerr << "should be " << h << " butr stored is " 
                     << elem.height << std::endl;
           return false;
         }
        if(abs(lh-rh)>1){
            std::cerr << "node at index " << index 
                      << " is unbalanced" << std::endl;
            std::cerr << "height left : " << lh << std::endl;
            std::cerr << "height right:" << rh << std::endl;
            return false;
         }
       } 

       // check whether index in some son of the father
       int father = elem.father;
       if(father < 0){
           if(root!=index){
               std::cerr << "father at node at index " << index 
                         << " is not there " << std::endl;
               std::cerr << "hence the node should be the root of the tree"
                         << std::endl;
               std::cerr << "but the root of the tree is " << root << std::endl;
               return false;
           }
       } else {
          TreeEntry f;
          array.Get(father,f);
          if( f.left!=index && f.right!=index){
             std::cerr << "father of node at index " << index << " is " 
                       << father << std::endl;
             std::cerr << "but the cilds of that node are " << f.left 
                       << " and " << f.right << std::endl;
             return false;
          }
       }
       // check whether index is the father of its sons
       if(elem.left >=0){
          TreeEntry son;
          array.Get(elem.left,son);
          if(son.father != index){
             std::cerr << "left son of node at index " << index << " is " 
                       << elem.left << std::endl;
             std::cerr << "but the father of this node is " 
                       << son.father << std::endl;
             return false;
          }
       }
       if(elem.right >=0){
          TreeEntry son;
          array.Get(elem.right,son);
          if(son.father != index){
             std::cerr << "right son of node at index " << index 
                       << " is " << elem.right << std::endl;
             std::cerr << "but the father of this node is " 
                       << son.father << std::endl;
             return false;
          }
       }
       if(!rec){
         return true;
       }
       return    check(elem.left,true,checkBalance) 
              && check(elem.right,true,checkBalance);
    }
};

} // end of namespace


#endif






