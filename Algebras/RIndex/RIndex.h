


/*
1 General class


*/
#include "RectangleAlgebra.h"



/*
1 The Index class

*/
template<int rdim, typename T, int cdim> class RIndexNode;

template <int dim, class T>
class RIndex{
   public:

/*
1.1 Constructor

This constructor creates a new empty index;

*/
      RIndex():root(0){}

/*
1.2 Destructor

The index is destroyed   

*/
     ~RIndex(){
         if(root){
             delete root;
             root = 0;
         }
      }    

/*
1.3 Insert

Inserts a pair consisting of a rectangle and data belonging to this rectangle
into this index;

*/
      void insert(const Rectangle<dim> & r, 
                  const T& t){
         if(!r.IsDefined()){
            return;
         }
         if(root){
           root->insert(r,t);
         } else {
           root = new RIndexNode<dim, T, dim>(r,t);  
         }
      }

/*
1.4 Erase

This function removes an entry from the index. If the entry was not found, the
result of this function is false, true otherwise.

*/
      bool erase(const Rectangle<dim>& r, const T& t){
         if(!r.IsDefined()){
            return;
         }
         if(!root){
            return false;
         }
         bool found = false;
         RIndexNode<dim, T, dim>* newRoot = root->erase(r,t,found);
         if(newRoot == 0){
             delete root;
             root = 0;
         }
         return found;
      }

/*
1.5 findsimple

This functions collects all rectangles insersecting the parameter 
rectangle and stores them
into res. This function is just for checking the structure. 

*/

    void findSimple(const Rectangle<dim>& r, 
                    std::vector<std::pair<Rectangle<dim> , T> >& res){
        
      res.clear();
      if(!r.IsDefined()){
        return;
      }
      if(root){
         root->findSimple(r,res);
      }
    }



/*
1.6 countEntries

  Returns the number of elements even if their are erased.

*/
   uint32_t countEntries(){
      if(root){
        return root->countEntries();
      } else {
         return 0;
      } 
   }

/*
1.8 height

Returns the height of a tree (0, iff tree is empty, 1 for a single leaf).

*/
    uint32_t height(){
       if(!root){
         return 0;
       } else {
          return root->height();
       }
    }


    uint32_t noLeafs(){
       if(!root){
         return 0;
       } else {
          return root->noLeafs();
       }
    }

    uint32_t dim0Entries(){
       if(!root){
         return 0;
       } else {
          return root->dim0Entries();
       }
    }
    

    uint32_t dim0Leafs(){
       if(!root){
         return 0;
       } else {
          return root->dim0Leafs();
       }
    }

    uint32_t maxBranches(){
       if(!root){
         return 0;
       } else {
          return root->maxBranches();
       }
    }

    
    uint32_t branches(){
       if(!root){
         return 0;
       } else {
          return root->branches();
       }
    }

    size_t usedMem(){
      if(!root){
        return sizeof(*this);
      } else {
        return sizeof(*this) + root->usedMem();
      }
    }


/*
TODO:

Implementing an iterator class and a find function

*/

   private:
      RIndexNode<dim, T, dim>* root;

};


/*
2 Implementation of a single node


*/


template<int rdim, typename T, int cdim>
class RIndexNode{

  public:


/*
2.1 Constructor

This constructor creates a single node without sons.

*/
     RIndexNode(const Rectangle<rdim> r, const T& t):
      rect(r), data(t), bbox(r), erased(false){
       quadrants = new  RIndexNode<rdim, T, cdim>*[ (1 << cdim) ];
       planes = new RIndexNode<rdim, T, cdim-1>*[cdim];
       for(int i=0; i< (1 << cdim) ; i++){
         quadrants[i] = 0;
       }
       for(int i=0;i<cdim; i++){
         planes[i] = 0;
       }
     }

/*
2.2 Destructor

Destroys this node and all subtrees.

*/

     ~RIndexNode(){
       for(int i=0;i< (1 << cdim); i++){
         if(quadrants[i]){
           delete quadrants[i];
           quadrants[i] = 0;
         }
       }
       for(int i=0;i<cdim; i++){
         if(planes[i]){
            delete planes[i];
            planes[i] = 0;
         }
       }
       delete[] quadrants;
       delete[] planes;
     }



/*
2.3 Insert

Inserts a new entry into this subtree.

*/
     void insert(const Rectangle<rdim>& r, const T& t){

        bbox = bbox.Union(r);
        int i = 0;
        bool usePlanes = getSon(r,i);


        if(usePlanes){
           if(planes[i]){
             planes[i]->insert(r,t);
            } else {
             planes[i] = new RIndexNode<rdim,T,cdim-1>(r,t);
            }
        } else {
           if(quadrants[i]){
              quadrants[i]->insert(r,t);
           } else {
              quadrants[i] = new RIndexNode<rdim,T,cdim>(r,t);
           }
        }
     }



/*
2.4 Erase

Removes an entry from this structure. 
The parameter found must be initialized with false to get a correct result.
The return value is this node or 0, if this subtree becomes empty.

*/

     RIndexNode<rdim, T, cdim>*  erase(const Rectangle<rdim>& r, 
                                       const T& t, bool& found){
        
        if( (rect==r) && (t==data) && (!erased) ){ 
           // this node is remove 
           found = true;
           if(isLeaf() ){
              erased = true;
              return 0;   
           } else {
              erased = true;
              return this;
           }
        }

         // try to find the victim within a subtree
        int i=0;
        bool isPlane = getSon(r,i);

        if(isPlane){
            if(!planes[i]){ // not found
               return this;
            } else {
              RIndexNode<rdim,T,cdim-1>* newSon = planes[i].erase(r,t);
              if(newSon==0){
                  delete planes[i];
                  planes[i] = 0;
                  if(erased && isLeaf()){
                     return 0; 
                  } else {
                     updateBBox();
                     return this;
                  }
              } else {
                 planes[i] = newSon;
                 return this;
              }
            }
        } else { // victim is in a quadrant
            if(!quadrants[i]){
               return this; // not found
            } else {
               RIndexNode<rdim, T, cdim>* newSon = quadrants[i].earse(r,t);
               if(newSon==0){
                   delete quadrants[i];
                   quadrants[i] = 0;
                   if(erased && isLeaf()){
                     return 0;
                   } else {
                      updateBBox();
                      return this;
                   }
               }
            }

        }    
     }



  void findSimple(const Rectangle<rdim>& r, 
                  std::vector<std::pair<Rectangle<rdim> , T> >& res){
     if(!bbox.Intersects(r)){
        return;
     }
     if(rect.Intersects(r)){
        std::pair<Rectangle<rdim> , T> p(rect,data);
        res.push_back(p);
     }
     for(int i=0;i<cdim; i++){
       if(planes[i]){
          planes[i]->findSimple(r,res);
       }
     }
     // we don't check whether we have to search for a special 
     // quadrant because the bounding box
     // check will do it
     for(int i=0; i< (1 << cdim) ; i++){
       if(quadrants[i]){
          quadrants[i]->findSimple(r,res);
       }     
     }
  }
     

  uint32_t countEntries(){
     uint32_t sum = 0;
     for(int i=0;i<cdim; i++){
       if(planes[i]){
          sum += planes[i]->countEntries();
       }
     }
     // we don't check whether we have to search for a
     // special quadrant because the bounding box
     // check will do it
     for(int i=0; i< (1 << cdim) ; i++){
       if(quadrants[i]){
          sum += quadrants[i]->countEntries();
       }     
     }
     return sum +  1;
  }


  uint32_t height(){
    uint32_t h  = 0;
    for(int i=0;i<cdim; i++){
       if(planes[i]){
          uint32_t h2 = planes[i]->height();
          h = h>h2?h:h2;
       }
     }
     for(int i=0; i< (1 << cdim) ; i++){
       if(quadrants[i]){
          uint32_t h2 = quadrants[i]->height();
          h = h>h2?h:h2;
       }     
     }
     return h + 1;
  }


  const Rectangle<rdim>& getBBox() const{
     return bbox;
  }


  uint32_t noLeafs(){
    int count = 0;
     for(int i=0;i<cdim; i++){
       if(planes[i]){
          count += planes[i]->noLeafs();
       }
     }
     for(int i=0; i< (1 << cdim) ; i++){
       if(quadrants[i]){
          count += quadrants[i]->noLeafs();
       }     
     }
     return count>0?count:1;
  }
  
  uint32_t dim0Entries(){
    int count = 0;
     for(int i=0;i<cdim; i++){
       if(planes[i]){
          count += planes[i]->dim0Entries();
       }
     }
     return count;
  }

  
  uint32_t dim0Leafs(){
    int count = 0;
     for(int i=0;i<cdim; i++){
       if(planes[i]){
          count += planes[i]->dim0Leafs();
       }
     }
     return count;
  }


  uint32_t maxBranches(){
    int branches = 0;
     for(int i=0;i<cdim; i++){
       if(planes[i]){
          branches += planes[i]->maxBranches();
       }
     }
     for(int i=0; i< (1 << cdim) ; i++){
       if(quadrants[i]){
          branches += quadrants[i]->maxBranches();
       }     
     }
     return branches + cdim + (1 << cdim); 
  }

  uint32_t branches(){
    int branches = 0;
     for(int i=0;i<cdim; i++){
       if(planes[i]){
          branches++;
          branches += planes[i]->branches();
       }
     }
     for(int i=0; i< (1 << cdim) ; i++){
       if(quadrants[i]){
          branches++; 
          branches += quadrants[i]->branches();
       }     
     }
     return branches; 
  }


  size_t usedMem(){
    int mem = 0;
     for(int i=0;i<cdim; i++){
       if(planes[i]){
          mem += planes[i]->usedMem();
       }
     }
     for(int i=0; i< (1 << cdim) ; i++){
       if(quadrants[i]){
          mem += quadrants[i]->usedMem();
       }     
     }
     return mem + sizeof(*this);; 
  }

  


  private:
     Rectangle<rdim> rect;      // the assigned rectangle
     T              data;       // the assigned data
     Rectangle<rdim> bbox;      // bounding box of the complete subtree
     bool           erased;     // mark for deleted entry
     RIndexNode<rdim, T, cdim>** quadrants; // sons for the quadrants
     RIndexNode<rdim, T, cdim-1>** planes;// sons for all planes


/*
2.3 getSon

*/



     // returns true, if this rectangle must be inserted into planes
     // returns false, if the rectangle must be inserted into quadrants
     // the index is set to the appropriate index within the corresponding array
     bool getSon(const Rectangle<rdim>& r, int& index) const{
        // check planes
        for(int i=0;i<cdim; i++){
           if( (r.MinD(i)<=rect.MinD(i)) && (r.MaxD(i)>=rect.MinD(i))){
              index = i;
              return true;
           }
         }
         index = 0;
         for(int i=0; i<cdim; i++){
            if(r.MaxD(i) < rect.MinD(i)){
                index |= (1 << i);
            }
         }
         return false;
        
     }

     int noSons() const{
       int sum = 0;
       for(int i=0;i<cdim;i++){
         if(planes[i]){
            sum++;
         }
       }
       for(int i=0;i< (1<< cdim); i++){
          if(quadrants[i]){
            sum++;
          }
       }
       return sum;
     }

     bool isLeaf() const{
       for(int i=0;i<cdim;i++){
         if(planes[i]){
            return false;
         }
       }
       for(int i=0;i< (1<< cdim); i++){
          if(quadrants[i]){
            return false;
          }
       }
       return true;
     } 


    // computes the bbox from the stored rectangle and the bboxes from the sons
     void updateBBox(){
        bbox = rect;
        for(int i=0;i<cdim;i++){
          if(planes[i]){
             bbox = bbox.Union(planes[i]->bbox);
          }
        }
        for(int i=0;i < (1 << cdim); i++){
           if(quadrants[i]){
              bbox = bbox.Union(quadrants[i]->bbox);
           }
        }
     }


};

/*
2 Specialisation for cdim == 1

*/
template<int rdim, typename T>
class RIndexNode<rdim, T,0>{

   public:
      RIndexNode(const Rectangle<rdim> r, const T& t){
         bbox = r;
         insert(r,t);
      }
      ~RIndexNode(){
        
      }

     void insert(const Rectangle<rdim>& r, const T& t){
           bbox = bbox.Union(r);
           std::pair<Rectangle<rdim>,T> p(r,t);
           content.push_back(p);
     }

     RIndexNode<rdim, T , 1>*  erase(const Rectangle<rdim>& r, 
                                     const T& t, bool& found){
       assert(false); // not implemented yet 
     }

     void findSimple(const Rectangle<rdim>& r, 
                     std::vector<std::pair<Rectangle<rdim> , T> >& res){
        if(!bbox.Intersects(r)){
            return;
        }
        typename std::vector<std::pair<Rectangle<rdim>,T> >::iterator it;
        for(it = content.begin(); it!=content.end(); it++){
           if(it->first.Intersects(r)){
              res.push_back(*it);
           }
        }

     } 

     uint32_t countEntries(){
         return content.size();
     }

     const Rectangle<rdim>& getBBox(){
        return bbox;
     }

     uint32_t height(){
        return 1;
     }

     uint32_t noLeafs(){ // at dimension 0, we have a leaf in each case
        return 1;
     }

     uint32_t dim0Entries(){
        return content.size();
     }

     uint32_t dim0Leafs(){
        return 1;
     }

     uint32_t branches() {
        return 0;
     }
     
     uint32_t maxBranches(){
        return  0;
     }

     size_t usedMem(){
        return sizeof(*this) + 
               sizeof(std::pair<Rectangle<rdim>,T>) * content.capacity();
     }


   private:
      Rectangle<rdim> bbox;
      std::vector<std::pair<Rectangle<rdim>,T> > content;


};




