/*
April 2, 2009. A simple utility class for constructing a DOT language
specification used by graphviz (www.graphviz.org) for a directed graph.

*/

#ifndef SEC_DOTSPEC
#define SEC_DOTSPEC

#include<string>
#include<list>
#include<map>

using namespace std;


class DotSpec {

  public:
  DotSpec(const string& name) : gName(name) {}
  ~DotSpec() {}


  void addNodeTypeSpec(const string& type, const string& spec) {
    types[type] = spec;	  
  }	  

  bool addNode(const string& type, const string& id, const string& label) {
    
   TTypeMap::const_iterator it = types.find(type);
   if (it != types.end()) {
     return false;	   
   }	   
   nodes[type].push_back( make_pair(id, label));
   return true;
  }

  void addEdge(const string& n1, const string& n2) {
    edges.push_back( make_pair(n1, n2));	  
  }	  

  void composeDotSpec(ostream& os) const {

    os << "digraph \"" << gName << "\"{" << endl;
       	    
    TTypeMap::const_iterator it1 = types.begin();
    while (it1 != types.end()) {

      // define a node type	    
      os << it1->second << ";" << endl;	    
 
      // get the nodes list for the current type 
      TNodesMap::const_iterator it2 = nodes.find(it1->first);
      assert(it2 != nodes.end());
      list<TPair>::const_iterator it3 = it2->second.begin();

      // write the nodes for this type
      while ( it3 != it2->second.end() ) {
        os << it3->first << "[label=\"" << it3->second << "\"];" << endl;    
        it3++;	    
      }
     it1++;	   
    }

    // write all edges
    TEdgeList::const_iterator it = edges.begin();
    while (it != edges.end()) {
     os << it->first << "->" << it->second << ";" << endl ;	    
     it1++;	    
    }	    

    os << "}" << endl; 
  }	  

  private:

  typedef map<string, string> TTypeMap;
  typedef pair<string, string> TPair;
  typedef map<string, list<TPair> > TNodesMap;
  typedef list< TPair > TEdgeList;
  TTypeMap types;
  TEdgeList edges;
  TNodesMap nodes;
  string gName;
};

#endif
