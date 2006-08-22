/*
1. The Repetition Tree (Implementation)

This class realizes a representation of a 
repetition-tree. This is a very specialized
tree representing repetitions in a list of integer
values. A node in the tree can be of type simple, composition
or repetition. The content of the node depends on this type. For a
simple type, the content corresponds to the integer number. For a 
composite node, the content represents the number of sons in this 
node. For a repetition node, the content describes the count of 
repetitions.

*/

#include <iostream>
#include <string>
#include <cassert>
#include "RepTree.h"
#include "List2.h"

/*
~Defining Types of nodes~

They are three types of nodes. A simple node represents a 
leaf containing a integer number as content. A composite node 
is a node with at least one son. A repetition node is a node
with exacly one son and the content of this node 
defines the count of repetitions of this son.

*/


#include "NodeTypes.h"

/*
~ID variable~

Each node in this implementation contains a id. Nodes representing
the same tree will also have the same id. Because the ids are 
assigned to the nodes at runtime, it is not possible to make 
persistent this kind of trees.

*/
static int lastid = 0;


/*
~Structure for replacement~ 

For replacing of sons by repetions
we need a structure storing the start and end
index of the sons to be replaced. Furthermore the
repetition node is stored.

*/
struct replacement{
    int startindex;
    int endindex;
    RepTree* replacetree;
}; 



/*
~Constructor~

This conmtructor creates a single leaf holding the 
given value.

*/
RepTree::RepTree(const int content){
    this->type = SIMPLE;
    this->content = content;
    this->id = lastid++;
    sons = 0;
 } 

/*
~Constructor~

This constructor creates a single composite tree from the 
given integer list. After that, the detectrepetitions function
is called to build a real repetition tree from the initial 
composite node.

*/
RepTree::RepTree(int *content,const int count){
    this->type = COMPOSITION;
    this->content = count;
    this->id = lastid++;
    sons = new RepTree*[count];
    for(int i=0;i<count;i++)
       sons[i] = new RepTree(content[i]);
    DetectRepetitions();
} 

/*
~Constructor~

This constructor takes a composite, or simple node and
creates a repetition with given count from it. The count is
stored in the content value;

*/
RepTree::RepTree(RepTree* node,const int repetitions){
    this->type = REPETITION;
    this->content = repetitions;
    this->id = lastid++;
    sons = new RepTree*[1];
    sons[0] = node;
}

/*
~Constructor~

This constructor takes an array of pointers to a tree, a startindex
as well as an endindex and constructs a new composite move from this
informations. 

*/
RepTree::RepTree(RepTree** nodes,const int startindex,const int endindex){
     this->type = COMPOSITION;
     this->content = endindex-startindex+1;
     this->id = lastid++;
     sons = new RepTree*[content];
     for(int i=startindex;i<=endindex;i++)
         sons[i-startindex]=nodes[i]; 
}

/*
~Constructor~

This constructor creates a new repetition node referering to a new
composite node which is created from the nodearray given as the first 
argument as well as the index information from the next two arguments.

*/
RepTree::RepTree(RepTree** nodes,const int startindex,
                 const int endindex,const int repcount){
     this->type = REPETITION;
     this->content=repcount;
     this->id = lastid++;
     sons = new RepTree*[1];
     sons[0] = new RepTree(nodes,startindex,endindex);
}

/*
~Destroy~

This Function removes all subtrees from this node. This is useful in
deleting a tree.

*/
void RepTree::Destroy(){
    if(type==SIMPLE) return;
    if(type==REPETITION) delete sons[0];
    if(type==COMPOSITION)
       for(int i=0;i<content;i++){
          sons[i]->Destroy();      
          delete sons[i];
       }
}

/*
~Destructor~

This destructor removes the sons array without touching the
sons themself.

*/
RepTree::~RepTree(){
    //delete[] sons;
}


/*
~GetSon~

This function returns the son with the given index.

*/
RepTree* RepTree::GetSon(int index)const{
      // determine the maximum possible index
      int maxindex=-1; // no son 
      if(type==REPETITION)
           maxindex=0; // a repetition contains exacly one son
      if(type==COMPOSITION)
          maxindex=content-1;
      if((index<0) || (index>maxindex)){
           std::cerr << "try to access sons[" << index <<
                        "] with maximum index of " << maxindex << "\n";
           assert(false);
      }      
      return sons[index]; 
   }
    
/*
~EqualID~

This function checks whether this node and the node given
as argument have the same id.

*/
bool RepTree::EqualID(const RepTree* T2)const{
   if(this->type != T2->type)
      return false;
   if(this->type==SIMPLE){ // use content instead of id
      return this->content==T2->content;
   }
   return this->id == T2->id;
} 

/*
~EqualsWithDepth~

This function checks the equality of two tree up to a given length.
At level 0, the trees are only compared via there id's.

*/
bool RepTree::EqualWithDepth(const RepTree* T2, const int depth)const{
   if(depth<=0)
      return EqualID(T2);
   if(this->type != T2->type)
      return false;
   if(this->type==SIMPLE)
      return this->content==T2->content;
   if(this->type==REPETITION)
      return (this->content==T2->content)  &&
              this->sons[0]->EqualWithDepth(T2->sons[0],depth-1); 
    // type is composite
    if(this->content!=T2->content)
       return false;
    bool ok = true;
    for(int i=0;i<this->content&&ok;i++)
         ok = this->sons[i]->EqualWithDepth(T2->sons[i],depth-1);
         return ok;
    }


/*
~NumberOfNodes~

This function computes the number of nodes contained in this
(sub-) tree. This can be useful in statistical computations.

*/
int RepTree::NumberOfNodes()const{
   if(this->type==SIMPLE)
        return 1;
   int res = 0;
   if(this->type==COMPOSITION)
     for(int i=0;i<content;i++)
        res += (sons[i])->NumberOfNodes();
   if(this->type==REPETITION)
        res = sons[0]->NumberOfNodes();
   return res+1;
}
    
/*
~NumberOfLeafs~

This function computes the number of leafs contained in this
(sub-) tree. The ratio between number of leafs before detecting of
repetitions and after this detection is a measure for the success of
this replacement. 

*/
int RepTree::NumberOfLeafs()const{
   if(this->type==SIMPLE)
      return 1;
   else if(this->type==REPETITION)
      return sons[0]->NumberOfLeafs();

   int res = 0;
   for(int i=0;i<content;i++)
       res += sons[i]->NumberOfLeafs();
   return res;
}

/*
~GetNodeType~

This returns the type of this node. Possible values are
SIMPLE, COMPOSITION, and REPETITION. 

*/
int RepTree::GetNodeType()const{
   return this->type;
}

/*
~NumberOfSons~

This functions returns the number of sons of this node.

*/
int RepTree::NumberOfSons()const{
   if(type==SIMPLE) return 0;
   if(type==REPETITION) return 1;
   return content;
}

/*
~NumberofNodesWithType~

This function computes the number of nodes with the given type.

*/
int RepTree::NumberOfNodesWithType(int type)const{
   if(this->type==SIMPLE){
      if(this->type==type)    
        return 1;
      else
        return 0;
   }
   int res = 0;
   if(this->type==COMPOSITION)
     for(int i=0;i<content;i++)
        res += (sons[i])->NumberOfNodesWithType(type);
   if(this->type==REPETITION)
        res = sons[0]->NumberOfNodesWithType(type);

   if(this->type==type) 
     res++;
   return res;
}


/*
~NumberOfCompositeSons~

This very spicialized function computes the number of sons of
all composite moves of this tree. 

*/
    int RepTree::NumberOfCompositeSons()const{
       if(type==SIMPLE)
          return 0;
       if(type==REPETITION)
          return sons[0]->NumberOfCompositeSons();
       int res=content; 
       for(int i=0;i<content;i++)
          res+= sons[i]->NumberOfCompositeSons();
       return res; 
    }


/*
~RepCount~

This function returns the number of repetitions for a 
node of type REPETITION. If this node id of another type,
the result will be -1.

*/
int RepTree::RepCount()const{
  if(type!=REPETITION) return -1;
  return content;
}

/*
~Content~

This function can be used to receive the constent of a simple
node. If the node is not of thios type, the result will be 
-1. 

*/
int RepTree::Content()const{
  if(type!=SIMPLE) return -1;
  return content;
}


/*
~DetectRepetitions~

This is the most importatant function  of this class. Starting with
an composite tree, this function detects repetitions in the sons and
replaces it by the appropriate nodes.

The basic idea of this algorithms is to detect repetitions of growing
length. First, all repetitions of the current lengths are detected. 
If no repetition is found, we continue with the incremented length.
We check wether equal repetitions are found. Each of this gets the 
same id. This makes it possible to detect nested repetitions. We 
replace the sequels recognized as repetition by the appropriate 
repetition-trees and start a new search with length 1.  

*/
    
void RepTree::DetectRepetitions(){
   // only for composite nodes, this makes sense
   if(this->type!=COMPOSITION) 
      return;
     
   // we search for repetitions of currentlength
   int currentlength = 1;
   // this is the current position in the sons array
   int currentpos;


   /*
   To detect a repetition of the currentlength, we 
   need at least 2*currentlength elements
   */
   while(currentlength <= content /2){ // check different lengths
      currentpos = 0;
      // list for storing the repetition found
      List<replacement>* RepList= new List<replacement>();
      Iterator<replacement>* It = new Iterator<replacement>(RepList); 
      int noReplacements = 0; // number of replacements
      int noElements = 0; // number of involved sons
       
      while(currentpos <= content-2*currentlength){
            
        /*
          compute the number of potential repetitions
          This meanas, that we only check for cycles of this length without 
          checking the inner parts of this cycles.
        */
        int posrepcount=-1;
        bool stop =false; 
        for(int i=currentpos+currentlength; 
                i<=content&&!stop;
                i=i+currentlength){
            if(i==content)
               posrepcount++;
            else{
               if(GetSon(currentpos)->EqualID(GetSon(i)))
                  posrepcount++;
               else{
                  if(i<content)
                     posrepcount++;  
                  stop=true;  
               }
            }
        }

        /*
        At this point wen know that starting from currentpos, posrepcount 
        cycles of length currentlength exist. Now we check the inner parts 
        of this cycles and count the actual repetitions.  
        */ 
        int repcount=0;
        bool repfound = true;
        for(int rep=0;rep<posrepcount&&repfound;rep++){ 
           for(int i=1;i<currentlength&&repfound;i++){
              int firstpos = currentpos+i;
              int secondpos = currentpos+(1+repcount)*currentlength+i; 
              if(secondpos==content){
                 repfound = true; 
               } else{
                 RepTree* son1 = GetSon(firstpos);
                 RepTree* son2 = GetSon(secondpos);
                 repfound = son1->EqualID(son2);
               }
           }
           if(repfound)
              repcount++;
        } 
        if(repcount==0){
           // if no repetition is found starting from the current position,
           // we start a new search from the next position 
           currentpos++;  
        }else{
           /*
            From the current position , we have found repcount repetitions.
            We create a replacement for it and store it in a list.
           */         
           replacement* r = new replacement();
           r->startindex=currentpos;
           r->endindex= currentpos+(repcount+1)*currentlength-1;
           if(currentlength>1){
               r->replacetree=new RepTree(sons,r->startindex, 
                                          r->startindex+currentlength-1,
                                          repcount+1);
           } else { // only one son is to repeat, no composition is needed
               r->replacetree=new RepTree(sons[currentpos],repcount+1); 
           } 
               
           /*
           Now we check wether this repetition exists already. 
           If it is so, the id of the new created repetition tree 
           will be taken from the existing one. 
           */
           It->Reset();
           replacement r2;
           bool found = false;
           while(It->HasNext()&&!found){
              r2 = It->Next();
              if(r2.replacetree->EqualWithDepth(r->replacetree,2)){
                 r->replacetree->id = r2.replacetree->id;
                 if(r->replacetree->type==REPETITION)
                    r->replacetree->sons[0]->id = r2.replacetree->sons[0]->id;
                  found=true;
              } 
           }
               
           // we store the repetition in the list
           RepList->Append(*r);
           noReplacements++; // one replacement more
           noElements=noElements + r->endindex - r->startindex+1;
           // set the current position behind the repetition
           currentpos = currentpos+(repcount+1)*currentlength; 
         }
     }
     /*
      At this point, we have stored all repetitions found in the sons
      stored in the list. If repetitions exists, we have to replace it
      by a real repetition. If not, we look for repetitions with a greater
      length. 
     */
     if(RepList->IsEmpty()){ // no repetition found
         // start a new search with incremented length
         delete RepList;
         delete It;
         currentlength++; 
         currentpos=0;
      } else{
         // now we replace the sons in fact
         int newsize = content-noElements+noReplacements;
         RepTree** newsons; 
         newsons = new RepTree*[newsize]; 
         It->Reset();
         // we know that It has a element because the list is not empty
         replacement r=It->Next();
         int index = 0; // the current index in the sons
         int newindex=0;// the current index in the new sons
         bool done=false;
         while(index< content){
            if(!done){
               if(index!=r.startindex){ // copy the son
                  newsons[newindex]=sons[index];
                  index++;
                  newindex++;
                }else{
                  newsons[newindex]=r.replacetree;
                  newindex++;
                  index = index+r.endindex-r.startindex+1;
                  if(It->HasNext())
                     r=It->Next();
                  else
                     done=true;
                }
             }else{ // no more repetitions
                newsons[newindex]=sons[index];
                index++;
                newindex++;
             } 
         }
         if(newindex!=newsize){
            std::cerr << "newindex = " << newindex << 
                      " != " << newsize << " = newsize \n";
            exit(-1);
         } 
         delete[] sons; // deallocate the old sons
         sons = newsons; // assign the sons the the new computed ones
         content=newsize; 
         delete RepList;
         delete It;
         currentlength=1;
         currentpos=0;
    }
  }
  /* PostProcessing:
     If this composite move has only a single son, we replace this node
      by its son.This my be the case if initial only a single son has 
      exist or the sonlist is converted to a single repetition
   */
   if(content==1){ // a single son is not allowed in a composition
      RepTree* son = sons[0];
      type = son->type;
      content = son->content;
      id = son->id;
      if(type==COMPOSITION){
        sons= new RepTree*[content];
        for(int i=0;i<content;i++)
           sons[i] = son->sons[i];
      }else if(type==SIMPLE){
          sons=0;
      }else {
          sons = new RepTree*[1];
          sons[0] = son->sons[0];
      }
      delete son;
   } 

}

/*
~Print~

This function prints out the tree in a textual form. The depth is used
for printing the correct indent.

*/
void RepTree::Print(int depth){
    std::string indent = "     ";

    if(this->type==SIMPLE){
       //print ident
       for(int i=0;i<2*depth;i++)
          std::cout << indent;
       std::cout << content << std::endl;   
     } else if (this->type==COMPOSITION){
         int mid = content /2;
         for(int i=0;i<mid;i++)
              sons[i]->Print(depth+1);
         for(int i=0;i<2*depth;i++)
            std::cout << indent;
         std::cout << "C[" << content << "]\n";
         for(int i=mid;i<content;i++)
            sons[i]->Print(depth+1);   
      } else if(this->type==REPETITION) {
         for(int i=0;i<2*depth;i++)
            std::cout << indent;
         std::cout << "R[" << content << "]" << std::endl;
         sons[0]->Print(depth+1); 
          
      } else{
            std::cout << "unknown nodetype " << std::endl;
      }
}

/*
~Print~

 This function prints out the tree in a textual form.

*/
void RepTree::Print(){
   Print(0);
}


/*
~PrintNlValue~

This function prints out the value of the nested list representation 
of this tree.

*/
void RepTree::PrintNlValue(){
     if(type==SIMPLE){
        std::cout << "  " << content << " " ;
     }
    if(type==REPETITION){
        std::cout << "(";
        std::cout << "\"R[" << content << "]\" (";
        sons[0]->PrintNlValue();
        std::cout << "))\n";
     } 
    if(type==COMPOSITION){
       std::cout << "(";
       std::cout << "\"C\"( ";
       for(int i=0;i<content;i++){
         sons[i]->PrintNlValue();
         std::cout << " ";
       }  
       std::cout <<"))\n";
     } 
     
}

/*
~PrintNL~

This function prints out this tree in nested list format 
inclusive type information.

*/
void RepTree::PrintNL(){
      std::cout << " ( tree ";
      PrintNlValue();
      std::cout << ")";
}

