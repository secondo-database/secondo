/*
1 The File RepTree.h

This tree detects repetitions in an integer array and
reconstructs itself for representing this repetations.


*/


#ifndef REPTREE_H
#define REPTREE_H

/*
1.1 The Class RepTree

This class provides a data structure for representing a list of integer 
values. The integers are not stored strong linearly but while the 
construction repetitions are recognized and reperesented in a tree
structure.

*/

class RepTree {

 public:
/*
~Constructor~

This conmtructor creates a single leaf holding the 
given value.

*/
    RepTree(const int content);

/*
~Constructor~

This constructor creates a single composite tree from the 
given integer list. After that, the detectrepetitions function
is called to build a real repetition tree from the initial 
composite node.

*/
    RepTree(int *content, const int count);

/*
~Constructor~

This constructor takes a composite, or simple node and
creates a repetition with given count from it. The count is
stored in the content value;

*/
    RepTree(RepTree* node,const int repetitions);
/*
~Constructor~

This constructor takes an array of pointers to a tree, a startindex
as well as an endindex and constructs a new composite move from this
informations. 

*/
    RepTree(RepTree** nodes,const int startindex, const int endindex);

/*
~Constructor~

This constructor creates a new repetition node referering to a new
composite node which is created from the nodearray given as the first 
argument as well as the index information from the next two arguments.

*/
    RepTree(RepTree** nodes,const int startindex,const int endindex,
            const int repcount);
/*
~Destructor~

This destructor deletes only this node, subtrees are not deleted. To remove the complete
tree, use the Destroy method first and delete this node after this.

*/
    ~RepTree();

/*
~Destroy~

This function removes all substress from this node.

*/

   void Destroy();

/*
~GetSon~

This function returns the son with the given index.

*/
   RepTree* GetSon(int index)const;
    

/*
~NumberOfNodes~

This function computes the number of nodes contained in this
(sub-) tree. This can be useful in statistical computations.

*/
    int NumberOfNodes()const;
    
/*
~NumberOfLeafs~

This function computes the number of leafs contained in this
(sub-) tree. The ratio between number of leafs before detecting of
repetitions and after this detection is a measure for the success of
this replacement. 

*/
    int NumberOfLeafs()const;
/*
~GetNodeType~

This returns the type of this node. Possible values are
SIMPLE, COMPOSITION, and REPETITION. 

*/
    int GetNodeType()const;

/*
~NumberOfSons~

Returns the number of sons of this tree.

*/
    int NumberOfSons()const;


/*
~NumberofNodesWithType~

This function computes the number of nodes with the given type.

*/
    int NumberOfNodesWithType(int type)const;


/*
~NumberOfCompositeSons~

This very spicialized function computes the number of sons of
all composite moves of this tree. 

*/
    int NumberOfCompositeSons()const;
/*
~RepCount~

This function returns the number of repetitions for this node. If the
type of this node is not REPETITION, the result will be -1.

*/

   int RepCount()const;


/*
~Content~

This function returns the content of this node for a SIMPLE node.
If this node is not simple, the result will be -1.

*/

   int Content()const;




/*
~Print~

This function prints out the tree with all subtrees. 

*/


   void Print();


/*
~PrintNL~

This functions prints out this tree in nested list format
usuable in the TreeViewer of Javagui.

*/
void PrintNL();


 private:
 

    int type;     // contains the type of this node
    int content;  // contains the content of a leaf or -1 if a non-leaf node
    int id;       // contains an id
    RepTree** sons ;  // the sons or NULL 

/*
~Print~

This function prints out the tree with an indent of the given depth

*/
   void Print(int depth);

/*
~detectRepetitions~

This is the most important function  of this class. Starting with
an composite tree, this function detects repetitions in the sons and
replaces it by the appropriate nodes.

*/
    
    void DetectRepetitions();
/*
~PrintNlValue~

This function prints out the value of the nested list representation of this tree.

*/

    void PrintNlValue();
     
/*
~EqualID~

This function checks whether this node and the node given
as argument have the same id.

*/
    bool EqualID(const RepTree* T2)const;

/*
~EqualsWithDepth~

This function checks the equality of two tree up to a given length.
At level 0, the trees are only compared via there id's.

*/
    bool EqualWithDepth(const RepTree* T2, const int depth)const;
};

#endif
