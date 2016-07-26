/*
A Wrapper for the graphviz library.
 
March 2009, M. Spiekermann

*/

#ifndef SEC_GRAPHVIZ
#define SEC_GRAPHVIZ


#include <stdio.h>
#include <graphviz/gvc.h>

#include <string>
#include <iostream>
#include <map>


FILE* createTmp() {

    FILE* tmp = tmpfile();
    assert(tmp != 0);
    setvbuf(tmp, 0, _IOFBF, 4*4096);
    return tmp;
}	


FILE* string2FILE(const std::string&  s) {

    FILE* tmp = createTmp(); 
    int rc = fputs(s.c_str(), tmp);
    assert( rc != EOF);
    rewind(tmp);    
    return tmp;
}

/* 
Copies s into a tmpfile and rewinds it

*/

std::string FILE2string(FILE* f) {
  
  std::string s("");
  rewind(f);
  char* err = 0;
  char* e = new char[4*4096];
  err = fgets(e, 4*4096, f);
  while ( err != 0 ) {
    s += e; 	
    err = fgets(e, 4*4096, f);
  }
  if (ferror(f))
    std::cerr << "Error in reading tmpfile!" << std::endl;	  
  fclose(f);	  
  delete [] e;
  return s;  
}	

/* 
Rewinds f and copies its content to a string. f is assumed to
be a tmpfile and is closed afterwards.

*/

/*
The class GraphViz encapsulates the graphviz library. Based on a graph notation
in the so called DOT language it offers a render method which returns a string
containing node and arc placement information. There are several layout
algorithms and string output formats available. For details please consult the
graphviz project: www.graphviz.org.

*/

class GraphViz {

  public:
  // Render types 	
  typedef enum { plain, xplain, dot, xdot, gxl, svg } RType;

  // Layout algorithms
  typedef enum { DOT, NEATO, FDP, TWOPI, CIRCO } LType;

  GraphViz(const std::string& dotSpec) {
	  
    gvc = gvContext();	  
   
    FILE* tmp = string2FILE(dotSpec);
    g = agread(tmp,0);
    fclose(tmp);

    rtype2str[plain] = "plain";
    rtype2str[xplain] = "plain-ext";
    rtype2str[dot] = "dot";
    rtype2str[xdot] = "xdot";
    rtype2str[gxl] = "gxl";
    rtype2str[svg] = "svg";

    ltype2str[DOT] = "dot";
    ltype2str[NEATO] = "neato";
    ltype2str[FDP] = "fdp";
    ltype2str[TWOPI] = "twopi";
    ltype2str[CIRCO] = "circo";
  }	  

  ~GraphViz() { 

     gvFreeLayout(gvc, g);
     agclose(g);
     gvFreeContext(gvc);
  }
  
  std::string render(RType r, LType l) {

    gvLayout(gvc, g, ltype2str[l]);
    FILE* f = createTmp();
    gvRender(gvc, g, rtype2str[r], f);
    return FILE2string(f);
  }	  


  private:	  
  graph_t*  g; 
  GVC_t*    gvc;

  std::map<RType, const char*> rtype2str;
  std::map<LType, const char*> ltype2str;

};

 /*
 int main(int argc, char** argv) {

   const string hello = "digraph G {hello->world}";

   GraphViz g(hello);

   cout << g.render(GraphViz::svg, GraphViz::DOT) << endl;

   return 0;
 }
 */

#endif
