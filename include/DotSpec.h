/*
April 2, 2009. A simple utility class for constructing a DOT language
specification used by graphviz (www.graphviz.org) for a directed graph.

*/

#ifndef SEC_DOTSPEC
#define SEC_DOTSPEC

#include<string>
#include<list>
#include<map>
#include<iostream>
#include<assert.h>



class DotSpec {

  public:
  DotSpec(const std::string& name) : gName(name) {}
  ~DotSpec() {}


  void addNodeType(const std::string& type, const std::string& spec) {
    types[type] = "node " + spec;	  
  }	  

  bool addNode(const std::string& type, const std::string& id,
               const std::string& label) {
    
   TTypeMap::const_iterator it = types.find(type);
   if (it == types.end()) {
     return false;	   
   }	   
   nodes[type].push_back( make_pair(id, label));
   return true;
  }

  void addEdge(const std::string& n1, const std::string& n2) {
    edges.push_back( make_pair(n1, n2));	  
  }	  

  void clearGraph() { 
    edges.clear(); 
    nodes.clear(); 
  }

  void buildGraph(std::ostream& os) const {

    os << "digraph \"" << gName << "\"{" << endl;
       	    
    TTypeMap::const_iterator it1 = types.begin();
    while (it1 != types.end()) {

      // define a node type	    
      os << it1->second << ";" << endl;	    
 
      // get the nodes list for the current type 
      //cerr << it1->first << endl;
      TNodesMap::const_iterator it2 = nodes.find(it1->first);
      
      if (it2 != nodes.end()) {
      std::list<TPair>::const_iterator it3 = it2->second.begin();

      // write the nodes for this type
      while ( it3 != it2->second.end() ) {
        os << it3->first << " [label=\"" << it3->second << "\"];" << endl;    
        it3++;	    
      }
      }
     it1++;	   
    }

    // write all edges
    TEdgeList::const_iterator it = edges.begin();
    while (it != edges.end()) {
     os << it->first << "->" << it->second << ";" << endl ;	    
     it++;	    
    }	    

    os << "}" << endl; 
  }	  

  private:

  typedef std::map<std::string, std::string> TTypeMap;
  typedef std::pair<std::string, std::string> TPair;
  typedef std::map<std::string, std::list<TPair> > TNodesMap;
  typedef std::list< TPair > TEdgeList;
  TTypeMap types;
  TEdgeList edges;
  TNodesMap nodes;
  std::string gName;
};

#endif
