
package tools;


import java.util.Map;
import java.util.HashMap;


/** This class privides an LRU mechanism. 
  * You can mark an object as used or delete the
  * element last recently marked as used.
  **/

class LRU<E> {

  /*
    The implementation constists of a map where the value elements
    are additionally arranged within a double linked list. The head of the
    lists is the objects recently marked to be used. The elements are
    arranged in the order of times where the object was used.
  */

  /** creates a new LRU object **/
   public LRU(){
      map = new HashMap<E,LRUElem<E>>();
      head = null;
      end = null;
   }


   /** marks elem as newly used **/
   public void use(E elem){
     if(elem==null){ // don't store null elements
        return;
     }
     if(!map.containsKey(elem)){ 
        // a new Element
        // just create new entry, put it to the head of the
        // list and into the map.

        LRUElem<E> newHead = new LRUElem<E>(elem);
        if(head==null){ // first element
           head=newHead;
           end = newHead;
        } else { // at least one element is already stored
           newHead.connect(head);
           head = newHead;
        }
        map.put(elem,newHead);
     } else { // element already inside the LRU
        // get the stored element and move it to the head of 
        // the list
        LRUElem<E> newHead = map.get(elem);
        if(head==newHead){  // already the first element
           return;
        }
        if(end==newHead){ // lastElement move end
           end = end.prev;
        }
        newHead.disconnect();
        newHead.connect(head);
        head = newHead;
     }
   }

   /** Removes the last recently used Element from this structure and 
     * returns it. If this structure is empty, null is returned. 
     **/ 

    public E deleteLastElem(){

       if(head==null){ // empty lru
          return null;
       }

       if(head==end){ // the last element;
          // assert(map.size()==1);
          map.clear();
          E elem = head.getElem();
          head = null;
          end = null;
          return elem;
       }
       // the list contains at least two elements
       LRUElem<E> victim = end;
       end = end.prev;
       victim.disconnect(); 
       E elem = victim.getElem();
       map.remove(elem);
       return elem;
    }


   /** Removes an element from LRU **/
   public boolean remove(E elem){
     if(!map.containsKey(elem)){
        return false;
     }
     LRUElem<E> victim = map.get(elem);
     if(head==victim){
          head = victim.next;
     }  
     if(end==victim){
         end = victim.prev;
     }
     victim.disconnect();
     map.remove(elem);
     return true;
   }

    public void clear(){
        map.clear();
        head=null;
        end=null;
    }



  private Map<E, LRUElem<E>> map;
  private LRUElem<E> head;
  private LRUElem<E> end;


  // a own implementation of a linked list **/
  private class LRUElem<E>{
     /** Creates a new unconnected element having elem as element. */
     public LRUElem(E elem){
        this.elem = elem;
        this.prev = null;
        this.next = null;
     }

     /** connect the predecessor and successor of this element and 
       * cuts the connection from this to these elements 
       **/
     public void disconnect(){
        if(prev!=null){
           prev.next = next;
        }
        if(next!=null){
           next.prev = prev;
        }
        next = null;
        prev = null;
     }

     /** Inserts this between nextElem and its predecessor. 
       * if this element is connected to other elements,
       * first, this connection is removed.
       **/
     public void connect(LRUElem<E> nextElem){
        if(next!=null || prev!=null){
           disconnect();
        }
        if(nextElem==null){ // nothing to connect
           return;
        }
        this.prev = nextElem.prev;
        next = nextElem;
        if(nextElem.prev!=null){
           nextElem.prev.next=this;
        } 
        next.prev = this;
     } 


     /** returns the stored element **/
     public E getElem(){
        return elem;
     }

     private E elem;
     private LRUElem<E> prev;
     private LRUElem<E> next;
  }

}




