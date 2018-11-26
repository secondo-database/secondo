/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/
%{

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "db.h"

#define DATABASE "access.db"

int docid = 0, authorid = 0, aindex = 0, j, authorflag = 0;

FILE* authorsfile;
FILE* authordocfile;
FILE* docsfile;
FILE* tmpkwfile;

DB *dbp;
DBT key, data;
int ret, titleno;

char* title="";
char* author="", *editor="";
char* booktitle="";
char* pages="";
char* year="";
char* journal="";
char* volume="";
char* number="";
char* month="";
char* url="";
char* school="";
char* publisher="";
char* isbn="";
char* authorlist[1000];
char splitbuffer[50];
char lowerbuffer[50];

char separator='|';

extern FILE* yyin;
extern int yylineno;
extern char* yytext;

void strsplit(char s[], char t[]) {
    char* temp;
    
    temp = t;
    while ((*t)!= '\0') {
        if (*t == ' ') temp = t;
        t++;
    }
    if (*temp == ' ') temp++;
    strcpy(s, temp);
}

void strtolower(char s[], char t[]) {

    while (*t != '\0') {
      *s = tolower(*t);
      t++;
      s++;
    }
    *s='\0';
}

void
printschema() {
  for (j=0; j < aindex; j++) {
    if (j == (aindex-1)) fprintf(docsfile,"%s", authorlist[j]);          
    else fprintf(docsfile,"%s, ", authorlist[j]);
  }

  fprintf(docsfile,"%c",separator);
  fprintf(docsfile,"%s%c", title, separator);
  fprintf(docsfile,"%s%c", booktitle, separator);
  fprintf(docsfile,"%s%c" , pages, separator);
  fprintf(docsfile,"%s%c" , year, separator);
  fprintf(docsfile,"%s%c" , journal, separator);
  fprintf(docsfile,"%s%c" , volume, separator);
  fprintf(docsfile,"%s%c" , number, separator);
  fprintf(docsfile,"%s%c" , month, separator);
  fprintf(docsfile,"%s%c" , url, separator);
  fprintf(docsfile,"%s%c", school, separator);
  fprintf(docsfile,"%s%c", publisher, separator);
  fprintf(docsfile,"%s%c\n" , isbn, separator);

  fprintf(tmpkwfile,"%d %s\n",docid,title);
  
  title = ""; author = ""; editor = ""; booktitle = "";
  pages = ""; year = ""; journal = ""; volume = "";
  number = ""; month = ""; url = ""; school = "";
  publisher = ""; isbn = "";
}

%}

%union {
    char* string;
}

%token STARTARTICLE ENDARTICLE STARTINPROCEEDINGS 
       ENDINPROCEEDINGS STARTPROCEEDINGS ENDPROCEEDINGS
       STARTBOOK ENDBOOK STARTINCOLLECTION ENDINCOLLECTION
       STARTPHDTHESIS ENDPHDTHESIS STARTMASTERTHESIS
       ENDMASTERTHESIS STARTWWW ENDWWW EDITOR CITE CROSSREF

%token <string> AUTHOR TITLE BOOKTITLE PAGES YEAR
%token <string> JOURNAL VOLUME NUMBER MONTH URL
%token <string> SCHOOL PUBLISHER ISBN

%%
 
dblp            : documentlist
                ;

documentlist    : document
                | documentlist document
                ;

document        : article 
                | inproceedings
                | proceedings
                | book
                | incollection
                | phdthesis
                | masterthesis
                | www
                ;

article         : STARTARTICLE fieldlist ENDARTICLE { 
                    fprintf(docsfile,"%s%c","article", separator);
                    fprintf(docsfile,"%d%c", ++docid, separator);
		            if (authorflag) {
                      for (j=0; j < aindex; j++) {		    
                        key.data = authorlist[j];
			            key.size = strlen(authorlist[j])+1;
                        ret = dbp->get(dbp, NULL, &key, &data, 0);
	                    if (ret == DB_NOTFOUND) {
			              ++authorid;
			              data.data = &authorid;
                          data.size = sizeof(int);
		                  if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0) {}
                          else { dbp->err(dbp, ret, "DB->put"); }
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authorsfile,"%s%c%s%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator);
			            fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);			  
	                  }
			          else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);
                      }	
		            }
		            }
                    printschema();
                    aindex = 0;
		            authorflag = 0;
                    titleno=0;
                 }
                ;

inproceedings   : STARTINPROCEEDINGS fieldlist ENDINPROCEEDINGS {
                    fprintf(docsfile,"%s%c","inproceedings", separator);
                    fprintf(docsfile,"%d%c", ++docid, separator);
		    if (authorflag) {
                      for (j=0; j < aindex; j++) {		    
                        key.data = authorlist[j];
            key.size = strlen(authorlist[j])+1;
                        ret = dbp->get(dbp, NULL, &key, &data, 0);
	                if (ret == DB_NOTFOUND) {
			  ++authorid;
			  data.data = &authorid;
                          data.size = sizeof(int);
		          if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0) {}
                          else { dbp->err(dbp, ret, "DB->put"); }
                                                  strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authorsfile,"%s%c%s%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);
                      } 				 	
		      }
		    }
                    printschema();
                    aindex = 0;
		    authorflag=0;
            titleno=0;
		    
                  }
                ;

proceedings     : STARTPROCEEDINGS fieldlist ENDPROCEEDINGS { 
                    fprintf(docsfile,"%s%c","proceedings", separator);
                    fprintf(docsfile,"%d%c", ++docid, separator);
		    if (authorflag) {
                      for (j=0; j < aindex; j++) {		    
                        key.data = authorlist[j];
            key.size = strlen(authorlist[j])+1;
                        ret = dbp->get(dbp, NULL, &key, &data, 0);
	                if (ret == DB_NOTFOUND) {
			  ++authorid;
			  data.data = &authorid;
                          data.size = sizeof(int);
		          if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0) {}
                          else { dbp->err(dbp, ret, "DB->put"); }
                                                  strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authorsfile,"%s%c%s%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);
                      } 				  	
		      }
		    }
                    printschema();
                    aindex = 0;
		    authorflag=0;
            titleno=0;
                  }
                ;

book            : STARTBOOK fieldlist ENDBOOK { 
                    fprintf(docsfile,"%s%c","book", separator);
                    fprintf(docsfile,"%d%c", ++docid, separator);
		    if (authorflag) {
                      for (j=0; j < aindex; j++) {		    
                        key.data = authorlist[j];
            key.size = strlen(authorlist[j])+1;
                        ret = dbp->get(dbp, NULL, &key, &data, 0);
	                if (ret == DB_NOTFOUND) {
			  ++authorid;
			  data.data = &authorid;
                          data.size = sizeof(int);
		          if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0) {}
                          else { dbp->err(dbp, ret, "DB->put"); }
                                                  strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authorsfile,"%s%c%s%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);
                      } 		
		      }
		    }
                    printschema();
                    aindex = 0;
		    authorflag=0;
            titleno=0;
                  }
                ;

incollection    : STARTINCOLLECTION fieldlist ENDINCOLLECTION { 
                    fprintf(docsfile,"%s%c","incollection", separator);
                    fprintf(docsfile,"%d%c", ++docid, separator);
		    if (authorflag) {
                      for (j=0; j < aindex; j++) {		    
                        key.data = authorlist[j];
            key.size = strlen(authorlist[j])+1;
                        ret = dbp->get(dbp, NULL, &key, &data, 0);
	                if (ret == DB_NOTFOUND) {
			  ++authorid;
			  data.data = &authorid;
                          data.size = sizeof(int);
		          if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0) {}
                          else { dbp->err(dbp, ret, "DB->put"); }
                                                  strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authorsfile,"%s%c%s%c%d%c\n",(char*)key.data,separator,lowerbuffer, separator,*(int*)data.data, separator);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);
                      } 		
		      }
		    }
                    printschema();
                    aindex = 0;
		    authorflag=0;
            titleno=0;
                  }
                ;

phdthesis       : STARTPHDTHESIS fieldlist ENDPHDTHESIS { 
                    fprintf(docsfile,"%s%c","phdthesis", separator);
                    fprintf(docsfile,"%d%c", ++docid, separator);
		    if (authorflag) {
                      for (j=0; j < aindex; j++) {		    
                        key.data = authorlist[j];
            key.size = strlen(authorlist[j])+1;
                        ret = dbp->get(dbp, NULL, &key, &data, 0);
	                if (ret == DB_NOTFOUND) {
			  ++authorid;
			  data.data = &authorid;
                          data.size = sizeof(int);
		          if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0) {}
                          else { dbp->err(dbp, ret, "DB->put"); }
                                                  strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authorsfile,"%s%c%s%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);
                      } 		
		      }
		    }
                    printschema();
                    aindex = 0;
		    authorflag=0;
            titleno=0;
                  }
                ;

masterthesis    : STARTMASTERTHESIS fieldlist ENDMASTERTHESIS {
                    fprintf(docsfile,"%s%c","mastersthesis", separator);
                    fprintf(docsfile,"%d%c", ++docid, separator);
		    if (authorflag) {
                      for (j=0; j < aindex; j++) {		    
                        key.data = authorlist[j];
            key.size = strlen(authorlist[j])+1;
                        ret = dbp->get(dbp, NULL, &key, &data, 0);
	                if (ret == DB_NOTFOUND) {
			  ++authorid;
			  data.data = &authorid;
                          data.size = sizeof(int);
		          if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0) {}
                          else { dbp->err(dbp, ret, "DB->put"); }
                                                  strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authorsfile,"%s%c%s%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);
                      } 		
		      }
		    }
                    printschema();
                    aindex = 0;
		    authorflag=0;
            titleno=0;
                  }
                ;

www             : STARTWWW fieldlist ENDWWW {
                    fprintf(docsfile,"%s%c","www", separator);
                    fprintf(docsfile," %d%c", ++docid, separator);
		    if (authorflag) {
                      for (j=0; j < aindex; j++) {		    
                        key.data = authorlist[j];
            key.size = strlen(authorlist[j])+1;
                        ret = dbp->get(dbp, NULL, &key, &data, 0);
	                if (ret == DB_NOTFOUND) {
			  ++authorid;
			  data.data = &authorid;
                          data.size = sizeof(int);
		          if ((ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) == 0) {}
                          else { dbp->err(dbp, ret, "DB->put"); }
                                                  strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authorsfile,"%s%c%s%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"%s%c%s%c%d%c%d%c\n",(char*)key.data, separator,lowerbuffer, separator,*(int*)data.data, separator,docid, separator);
                      } 	
		      }
		    }
                    printschema();
                    aindex = 0;
		    authorflag=0;
            titleno=0;
                  }
                ;

fieldlist       : attributelist
                ;

attributelist   : attribute
                | attributelist attribute
                ;

attribute       : AUTHOR                       { 
                                                 authorlist[aindex] = $1;
                                                 aindex++;
						                         authorflag=1; 
                                               }
		                           
                | EDITOR                       { //editor = yylval.string; 
                               }

	        | BOOKTITLE                    { booktitle = $1; }

                | TITLE                        { ++titleno; if (titleno > 1) { printf(title); printf("\n"); printf($1); printf("\n"); } title = $1;  }

	        | PAGES                        { pages = $1; } 
                                      
		| YEAR                         { year = $1; }
                                                   
	        | JOURNAL                      { journal = $1; }
                                                       
                | VOLUME                       { volume = $1; }
                                                             
                | NUMBER                       { number = $1; }
                                          
                | MONTH                        { month = $1; }
                                       
                | URL                          { url = $1; }
                                 
                | SCHOOL                       { school = $1; }
                                  
                | PUBLISHER                    { publisher = $1; }
                                  
                | ISBN                         { isbn = $1; }

                | CITE                         {}
                
                | CROSSREF                     {}
                                  
                ;

%%



int
yyerror (const char* msg)
{
  fprintf(stderr,
            "%s at line %d reading symbol '%s'.\n", msg, yylineno, yytext);
  exit(3);
}

int
main (argc, argv)
int argc;
char** argv;
{
    FILE* ifile;
    int start=0;
    int oldstyle=0;

    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr, "db_create: %s\n", db_strerror(ret));
        exit (1);
    }
    if ((ret = dbp->open(dbp,
        NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
        dbp->err(dbp, ret, "%s", DATABASE);
    }
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    
    authorsfile = fopen("Author", "w");
    authordocfile = fopen("Authordoc", "w");
    docsfile = fopen("Document", "w");
    tmpkwfile = fopen("keyword_tmp","w");
    
    if(argc > 1 && (argv[1]=="--oldstyle")){
       oldstyle=1;
       start++;
    }

    if(argc > start+1){
       ifile = fopen(argv[start+1], "r");
       if (ifile == NULL)
       {
         fprintf(stderr,"DBLPParser::ERROR: cannot open file %s\n ",argv[start+1]);
         fprintf(stderr,argv[start+1]);
         fprintf(stderr,"\n");
         return -1;
       }
       yyin = ifile;
    }   

    yyparse();
    return 0;
}
