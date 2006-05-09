/*
1.5 Implementation


1.5.1 Implementaion of DiceNode

*/

#include "DiceCoeff.h"


int min(int a, int b){
   return a<b?a:b;
}


DiceNode::DiceNode(unsigned int depth){
    left = 0;
    right = 0;
    this->depth = depth;
    for(int i=0;i<numberOfSons;i++){
        sons[i] = NULL;
    }
 }
   
DiceNode::~DiceNode(){
    for(int i=0;i<numberOfSons;i++){
       if(sons[i]){
          delete sons[i];
       }
    }
}
     
void DiceNode::insert(string text,bool  left){
       if(text.length()==0){
           if(left){
              this->left++;
           } else{
              this->right++;
           }
           return;
       }
       unsigned char c = text[0];
       if(!sons[c]){
          sons[c] = new DiceNode(depth+1);
       }
       sons[c]->insert(text.substr(1),left);
    }

void DiceNode::print(char root){
       if(root=='"'){
          root ='\'';
       }
       cout << "( \"" << root << " (" << left << ", " << right << ")\"" << endl;
       cout << "(" << endl;
       for(int i=0;i<numberOfSons;i++){
          if(sons[i]){
             sons[i]->print((char)i); 
          }
       }
       cout << "))" << endl;
    }




void DiceNode::computeSums(int& left, int& right, int& common){
      if(depth!=0){
         left += this->left;
         right += this->right;
         common += min(this->left,this->right);
      }
      for(int i=0;i<numberOfSons;i++){
          if(sons[i]){
            sons[i]->computeSums(left,right,common);
          }
      } 
   }


double DiceNode::getCoeff(){
   int left = 0;
   int right = 0;
   int common = 0;
   computeSums(left,right,common);
   double lr = (double) left + (double)right;
   if(lr==0){
     return 1.0;
   }
   return (2.0* ((double)common ))/ lr ;
}


/*
1.6 Implementation of DiceTree

*/

DiceTree::DiceTree(unsigned int depth){
    this->depth = depth>0?depth:1;
    tree = new DiceNode(0);
}

DiceTree::~DiceTree(){
   delete tree;
}

void DiceTree::appendText(string text, bool left){
  if(text.length()==0){
      return;
  }
  int max = text.length()>depth?depth:text.length();
  max = max;
  for(int i=0;i<max;i++){
    string subtext = text.substr(0,i);
    tree->insert(subtext,left);    
  }
  text = text.substr(1); 
  while(text.length()>0){
     string subtext = text.substr(0,depth);
     tree->insert(subtext,left);
     text = text.substr(1);  
  }
}

double DiceTree::getCoeff(){
   return tree->getCoeff();  
}

void DiceTree::printTree() {
   cout << endl << "( tree ";
   tree->print(' ');
   cout << ")";
}


