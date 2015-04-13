/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[NP] [\newpage]
//[ue] [\"u]
//[e] [\'e]
//[lt] [\verb+<+]
//[gt] [\verb+>+]

----
SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----
 
[1] 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 

[1] WS 2014 / 2015

Jens Breit, Joachim Dechow, Daniel Fuchs, Simon Jacobi, G[ue]nther Milosits, 
Daijun Nagamine, Hans-Joachim Klauke.

Betreuer: Dr. Thomas Behr, Fabio Vald[e]s


[1] Implementation of a Spatial3D algebra: SetOps

[TOC]

[NP]

1 Includes and Defines

*/
#include "NestedList.h"
#include "TypeMapUtils.h"
#include "Spatial3D.h"
#include "Spatial3DSTLfileops.h"
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <cctype>
#include "FTextAlgebra.h" //FText
#include "StandardTypes.h" //CcString
#include "ListUtils.h"
#include "Spatial3D.h"
#include "Operator.h"

extern NestedList* nl;

using namespace std;

namespace spatial3DSTLfileops {
/*
1 Helper functions

1.1 readNextStringToken

Consumes the next token from the provided stream and compares it
to the provided token. If they do not match an exception will be thrown.

*/ 
  void readNextStringToken(istream& is, const string& token) {
    string token_from_stream = "";
    is >> token_from_stream;
    if (token_from_stream != token) throw 1;
  }

  
/*
1.1 readNextDouble

Consumes and returns the next double from the input stream. If the next token
in the stream is no double an exception will be thrown.

*/
  
  double readNextDouble(istream& is) {
    char c;
    is >> c; //read the next char skipping whitespace
    is.unget(); // and put it back, so we can later read it using operator>>
    if (!isdigit(c) && !(c == '.')&& !(c == '-')) { // no double...
      throw 1;
    }
    double from_stream = 0;
    is >> from_stream;
    return from_stream;
  }
  
/*
1.1 readFromTextStl

Read an STL text file ("filename") into a volume3D. If reading the file fails
(technical or syntactical problems) volume3D will be undefined and 
false will be returned. The provided BulkLoadOptions will be called on the to
be constructed volume3D.

*/
  
  bool readFromTextStl (const string filename, 
                    const BulkLoadOptions options, Volume3d* vol) {
    
    vol->startBulkLoad();
    ifstream ifs;
    
    try { // read STL text file. Parsing func will throw upon invalid tokens.
      ifs.open(filename.c_str(), ios::in);
      
      readNextStringToken(ifs, "solid");
      
      string solid_name = "";
      ifs >> solid_name; // we'll later check if we have the same closing name
                         // with the endsolid tag
                         
      while (true) { // we'll break if we are done and throw upon errors
        string token = "";
        ifs >> token;
        if (token == "endsolid") break; // we're done
        if (token != "facet") throw 1; // only facet allowed here
        
        readNextStringToken(ifs, "normal");
        
        float norm_vec_coord; // will be discarded
        for (int i = 0; i <3; i++) {
          norm_vec_coord = readNextDouble(ifs);
        }
        readNextStringToken(ifs, "outer");
        readNextStringToken(ifs, "loop");
        
        SimplePoint3d pt[3];

        for (int p = 0; p<3; p++) {
          readNextStringToken(ifs, "vertex");
          float x = readNextDouble(ifs);
          float y = readNextDouble(ifs);
          float z = readNextDouble(ifs);
          
          pt[p].set(x,y,z);
         
        }
        Triangle tri(pt[0],pt[1],pt[2]);
        vol->add(tri); 
        
        readNextStringToken(ifs, "endloop");
        readNextStringToken(ifs, "endfacet");
      }
      
      // we've read the endsolid token. Now lets check if there is just
      // additional whitespace and possibly the name of the solid 
      string endsolid_name = "";
      ifs >> endsolid_name;
      if (endsolid_name != "" && endsolid_name != solid_name) {
        throw 1;
      }
      
      // now make sure we really reached the end of the file
      char some_char = 0;
      ifs >> some_char;    
      if (some_char !=0) {
        throw 1;
      }

      ifs.close();
      vol->endBulkLoad(options);
      return true;
    } catch(int a) {
      ifs.close();
      vol->endBulkLoad(options);
      vol->clear();
      vol->SetDefined(false);
      return false;
    }
    return false;
  }
  
/*
1.1 readFromBinaryStl

Read an STL binary file ("filename") into a volume3D. If reading the file fails
(technical or syntactical problems) volume3D will be undefined and 
false will be returned. The provided BulkLoadOptions will be called on the to
be constructed volume3D. Comparing the encoded number of triangles with the
file size allows early detection of invalid files.

*/ 
  bool readFromBinaryStl (const string filename, 
                          const BulkLoadOptions options, Volume3d* vol) {  

    
    ifstream ifs;
    ifs.open(filename.c_str(), ios::in | ios::binary);

    // get length of file:
    ifs.seekg (0, ifs.end);
    int file_size = ifs.tellg();
    
    //discard first 80 bytes
    ifs.seekg (80, ifs.beg);   
    
    //read 32 bit unsigned int -> number of facets
    uint32_t num_of_facets = 0;
    ifs.read((char*)&num_of_facets, sizeof(num_of_facets));
    
    float normal_vec[3]; 
    float tri_points[9];
    uint16_t attr_size = 0;
    
    int expected_size = 80 + sizeof(num_of_facets)+ num_of_facets * (
      sizeof(normal_vec) + sizeof(tri_points) + sizeof(attr_size));
    
    // do a quick check if filesize is ok to prevent costly reading
    if (file_size != expected_size || file_size < 82) {
      /*  // uncomment this to get details of file size check
          // removed to prevent error messages in TTY 
      cerr << "Binary file size mismatch. Expected "
        << expected_size << " bytes, but got "
        << file_size << " bytes." << endl;
      */
      ifs.close();
      vol->clear();
      vol->SetDefined(false);
      return false;
    }
    
    vol->startBulkLoad();
    for (unsigned int t = 0; t < num_of_facets; t++) {
      ifs.read((char*)&normal_vec, sizeof(normal_vec)); // normal vector
      ifs.read((char*)&tri_points, sizeof(tri_points)); // 3*3 Points3D
      ifs.read((char*)&attr_size, sizeof(attr_size)); // attribute size
      if (ifs.gcount() != sizeof(attr_size)) {
        ifs.setstate(ifstream::failbit);
        break; //we have reached the end of the stream too early
      }
      
      Point3d pt[3];
      for (int p = 0; p<3; p++) {
        float x = tri_points[p*3];
        float y = tri_points[p*3+1];
        float z = tri_points[p*3+2];
        pt[p].set(x,y,z);
        pt[p].SetDefined(true);
      }
      vol->add(Triangle(pt[0],pt[1],pt[2])); 
      
    }
    
    if (ifs.fail()) {
      cerr << "failed\n";
      ifs.close();
      vol->endBulkLoad(options);
      vol->clear();
      vol->SetDefined(false);
      return false;
    }
    
    char c;
    ifs.get(c); // just read one char to check for eof
    if (!ifs.eof()) {    
      cerr << "extra bytes at end of file\n";
      ifs.close();
      vol->endBulkLoad(options);
      vol->clear();
      vol->SetDefined(false);
      return false;
    }    
    
    ifs.close();
    vol->endBulkLoad(options);
    return true;
  }
  
/*
1.1 writeToBinaryStl

Export to binary STL files.

*/  
  bool writeToBinaryStl(const Volume3d* const vol,
                        const string objectName, const string fileName) {
    
    ofstream ofs;
    ofs.open(fileName.c_str(), ios::out | ios::binary);
    
    // not very elegant, but should avoid fiddling with multibyte chars
    // blow up object name with spaces, so we have at least 80 bytes...
    // ... then only write 80 bytes
    size_t headerLen = 80;
    string header = string (headerLen, ' '); 
    header.insert(0, objectName);
    ofs.write((char*)header.c_str(), headerLen);
    
    //TODO check for potential overflows
    uint32_t num_of_facets = vol->size(); // get num of triangles...
    ofs.write((char*)&num_of_facets, sizeof(num_of_facets));
    
    for (uint32_t facet_index = 0; facet_index < num_of_facets; facet_index++){
      float normal_vec[3]; 
      float tri_points[9];
      uint16_t attr_size = 0;
      
      Triangle tri = vol->get(facet_index);
      Vector3d norm_vec = tri.getNormalVector();
      Point3d a = tri.getA();
      Point3d b = tri.getB();
      Point3d c = tri.getC();
      
      normal_vec[0] = norm_vec.getX();
      normal_vec[1] = norm_vec.getY();
      normal_vec[2] = norm_vec.getZ();
      
      tri_points[0] = a.getX(); 
      tri_points[1] = a.getY();
      tri_points[2] = a.getZ();
      
      tri_points[3] = b.getX();
      tri_points[4] = b.getY();
      tri_points[5] = b.getZ();
      
      tri_points[6] = c.getX();
      tri_points[7] = c.getY();
      tri_points[8] = c.getZ();
      
      ofs.write((char*)&normal_vec, sizeof(normal_vec)); // normal vector
      ofs.write((char*)&tri_points, sizeof(tri_points)); // 3*3 Points3D
      ofs.write((char*)&attr_size, sizeof(attr_size)); // attribute size
      
    }
    
    ofs.close();
    return true;
  }
  
/*
1.1 writeToTextStl

Export to text STL files.

*/   
  bool writeToTextStl(const Volume3d* const vol,
                      string objectName, string fileName) {
    
    
    ofstream ofs;
    ofs.open(fileName.c_str(), ios::out);
    
    ofs << "solid " << objectName << endl;
    
    //TODO check for potential overflows
    uint32_t num_of_facets = vol->size(); // get num of triangles...
    
    for (uint32_t facet_index = 0; facet_index < num_of_facets; facet_index++){
      ofs << "  facet" << endl;
      
      Triangle tri = vol->get(facet_index);
      Vector3d norm_vec = tri.getNormalVector();
      Point3d a = tri.getA();
      Point3d b = tri.getB();
      Point3d c = tri.getC();
      
      ofs << "    normal ";
      ofs << fixed << norm_vec.getX() << " ";
      ofs << fixed << norm_vec.getY() << " ";
      ofs << fixed << norm_vec.getZ() << endl;
      
      ofs << "    outer loop" << endl;
      ofs << "      vertex ";
      ofs << fixed << a.getX() << " ";
      ofs << fixed << a.getY() << " ";
      ofs << fixed << a.getZ() << endl;

      ofs << "      vertex ";
      ofs << fixed << b.getX() << " ";
      ofs << fixed << b.getY() << " ";
      ofs << fixed << b.getZ() << endl;
      
      ofs << "      vertex ";
      ofs << fixed << c.getX() << " ";
      ofs << fixed << c.getY() << " ";
      ofs << fixed << c.getZ() << endl;
      ofs << "    endloop" << endl;
      ofs << "  endfacet" << endl;   
    }
    ofs << "endsolid " << objectName << endl;
    
    ofs.close();
    return true;
  }
   
/*
1 Operator Specifcations

1.1 importSTL

Imports a volume3d object from an STL file.
Both binary and text STL files are accepted.
The import automatically choses the correct import format based on file.
A simple correction (i.e. filling of small holes, removal of triangles
inside of the volume) will be performed.
If anything goes wrong an undefined volume3d object will be returned.

*/ 
  OperatorSpec importSTLSpec (
    "text -> volume3d",
    "importSTL ( <filename> )",
    "imports a volume3D object from a valid STL file\n"
      "Both text and binary STL files are accepted.\n"
      "Will return undefined object in case of problems",
    "query importSTL ('cube.stl');"
  );
/*
1.1 exportSTL

exports a volume3d object into an STL file. The format (binary or text)
can be chosen.

*/
  OperatorSpec exportSTLSpec (
    "volume3d x string x text x bool -> bool",
    "<volume3d> exportSTL [ <objectname>, <filename>, <export_to_binary> ]",
    "exports a volume3D object to STL file\n"
    "Will return FALSE in case of error.\n"
    "Set export_to_binary to true if you want to use the binary format.",
    "query mySTLcube exportSTL ['my Cube', 'cube.stl', FALSE];"
  );  
  
/*
1 Type Mappings

1.1 importSTL

*/  
  ListExpr importSTLTM(ListExpr args) {
    if(!nl->HasLength(args,1)){
      return listutils::typeError("one argument expected");
    }
    if(!FText::checkType(nl->First(args))){
      string err = "expected argument of type text but got "
                        + nl->ToString(nl->First(args));
      return listutils::typeError(err);
    }
    return  listutils::basicSymbol<Volume3d>();
  }

/*
1.1 exportSTL

*/  
  ListExpr exportSTLTM(ListExpr args) {
    if(!nl->HasLength(args,4)){
      return listutils::typeError("four arguments expected");
    }
    if(!Volume3d::checkType(nl->First(args))){
      string err = 
      "expected argument of type volume3d as first argument but got "
      + nl->ToString(nl->First(args));
      return listutils::typeError(err);
    }
    if(!CcString::checkType(nl->Second(args))){
      string err = 
      "expected argument of type string as second argument but got "
      + nl->ToString(nl->Second(args));
      return listutils::typeError(err);
    }
    if(!FText::checkType(nl->Third(args))){
      string err = 
      "expected argument of type text as third argument but got "
      + nl->ToString(nl->Third(args));
      return listutils::typeError(err);
    }
    if(!CcBool::checkType(nl->Fourth(args))){
      string err = 
      "expected argument of type bool as fourth argument but got "
      + nl->ToString(nl->Fourth(args));
      return listutils::typeError(err);
    }    
    return  listutils::basicSymbol<CcBool>();
  }

/*
1 Value Mappings

1.1 importSTL

The respective helper function for binary and text import are called.
The binary format is tried first, as it allows a quick check based on filesize
to determine if it is valid. Only if the file is not in binary format,
the text format will be tried. Which might make it necessary to read through
the whole file just to discover it is corrupt.

*/
  int importSTLVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ) {
    FText* fileName = (FText*) args[0].addr;
    result = qp->ResultStorage(s);
    Volume3d* res = (Volume3d*) result.addr;
    
    BulkLoadOptions opts = REPAIR;
    //BulkLoadOptions opts = NO_REPAIR;
    
    string filename = fileName->GetValue();
    
    // First try to read binary file, as we can quickly check if valid
    bool importresult = readFromBinaryStl(filename, opts, res);
    importresult = importresult || readFromTextStl(filename, opts, res);
    return 0;
  }

/*
1.1 exportSTL

exports a volume3d object to an STL file (format to be chosen by the user).
Undefined volume3d objects will not be exported at all.

*/
  int exportSTLVM( Word* args, Word& result, int message,
                   Word& local, Supplier s ) {
    
    Volume3d* vol = (Volume3d*) args[0].addr;
    CcString* objectName = (CcString*) args[1].addr;   
    FText* fileName = (FText*) args[2].addr;
    CcBool* toBinary = (CcBool*) args[3].addr;
    
    result = qp->ResultStorage(s);
    CcBool* res = (CcBool*) result.addr;
    
    if ( !vol->IsDefined()
      || !objectName->IsDefined()
      || !fileName->IsDefined()
      || !toBinary->IsDefined()) {
      cerr << "Got some undefined argument";
      res->Set(true, false);
      return 0;
    }
    
    // TODO:
    // Here we assume to get valid file and object names
    // which may cause some trouble...
    
    bool exportSuccesful = true;
    if (toBinary->GetValue()) {
      exportSuccesful = writeToBinaryStl
                        (vol, objectName->GetValue(), fileName->GetValue());
    }else{
      exportSuccesful = writeToTextStl
                        (vol, objectName->GetValue(), fileName->GetValue());
    }
     
    res->Set(true, exportSuccesful);
    return 0;
  }
  
/*
1 Operator Pointers

Provide Operator Pointers for embedding in Algebra 

*/       
  Operator* getImportSTLptr(){
    return new Operator(
      "importSTL",
      importSTLSpec.getStr(),
      importSTLVM,
      Operator::SimpleSelect,
      importSTLTM
    );
  }
  
  Operator* getExportSTLptr(){
    return new Operator(
     "exportSTL",
     exportSTLSpec.getStr(),
     exportSTLVM,
     Operator::SimpleSelect,
     exportSTLTM
   );
  }
  
} //namespace spatial3DSTLfileops 
