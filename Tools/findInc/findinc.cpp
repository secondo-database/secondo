/*

1 Simple tool for fining all directories containing a .h file

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <dirent.h>
#include <vector>

using namespace std;


bool endsWith(const std::string& a1, const std::string& a2){
    size_t len1 = a1.length();
    size_t len2 = a2.length();
    if(len2 > len1){
        return false;
    }
    return a1.substr(len1-len2)==a2;
}

const string separator="/";


void processDir(const string& dn){
   // TODO: check for readability

   //cout << "processDir " << dn << endl;

   vector<string> subdirs;
   bool written = false;
   DIR* hdir;
   struct dirent* entry;
   hdir = opendir(dn.c_str());
   struct stat state;
   do{
      entry = readdir(hdir);
      if(entry){
        string fns = entry->d_name;
        string fn = dn + separator + entry->d_name;
        
        int rc = stat(fn.c_str(),&state);
        if(rc==0){
          int type = state.st_mode & S_IFMT;
          if(type == S_IFDIR){
             if((fns!=".") && (fns!="..")){
               subdirs.push_back(fn);
             }
             //cout << "insert " << fn << " into subdirs" << endl;
          } else if( type == S_IFREG )
            //cout << "pürocess File " << fn << endl;
            if(!written && endsWith(fn,".h")){
              cout << "-I" << dn << " ";
              written = true;           
            }  
         } else {
            cerr << "cannot process file " << fn 
                 << " in directory " << dn <<  endl;
         }
       }
   } while(entry);
   closedir(hdir);
   vector<string>::const_iterator it;
   for(it=subdirs.begin();it!=subdirs.end();it++){
     processDir(*it);
   }

} 


int main(int argc, char** argv){

   string root = argc>0?string(argv[1]):".";
   struct stat state;
   int rc = stat(root.c_str(),&state);
   if(rc!=0){
     cerr << "File " << root << " does not exists" << endl;
     return 1;
   } 
   int type = state.st_mode & S_IFMT;
   if(type != S_IFDIR){
      cerr << root << " is not a directory" << endl;
      return 1;
   }
   
   processDir(root);   

}


