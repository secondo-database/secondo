/*

1 DiceCoeff

This file offers the possibility to compute the dice coefficient 
for two text elements.
The dice  coefficient can be used for finding similar texts.

To compute the dice coefficient for two string __t1__ and __t2__, 
the following lines are required:

----
   
   DiceTree* dt = new DiceTree(height);
   dt->appendText(t1,true);
   double res = dt->getCoeff();
   delete dt;
   return res;


----


*/

/*
1.1 Includes

*/

#ifndef DICECOEFF_H
#define DICECOEFF_H

#include <string>
#include <iostream>
using namespace std;

/*
1.2 numberOfSons

For computing the dice coefficient both texts to compare are inserted
into a tree where. For each character one son must exist.

*/

const int numberOfSons = sizeof(char)*256;

/*
1.3 DiceNode

This class represents a singlke node of the  tree where the texts 
are inserted.

*/

class DiceNode{

public:
    DiceNode(unsigned int depth);
    ~DiceNode();
    void insert(string text,bool  left);
    double getCoeff();
    void print(char root);
private:
   int left;
   int right;
   unsigned depth;
   DiceNode* sons[numberOfSons+1];
   
   void computeSums(int& left, int& right, int& common);
};


/*
1.4 DiceTree

The tree itself.

*/

class DiceTree{

public:
   DiceTree(unsigned depth);
   ~DiceTree();
   void appendText(string text,bool left);
   double getCoeff();
   void printTree();
private:
  unsigned int depth;
   DiceNode* tree;
};

#endif
