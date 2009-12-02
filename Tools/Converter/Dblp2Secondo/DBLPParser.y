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
char* authorlist[200];
char splitbuffer[50];
char lowerbuffer[50];

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
  fprintf(docsfile,"%s"," <text>");
  for (j=0; j < aindex; j++) {
    if (j == (aindex-1)) fprintf(docsfile,"%s", authorlist[j]);          
    else fprintf(docsfile,"%s, ", authorlist[j]);
  }
  fprintf(docsfile,"%s","</text--->");
  fprintf(docsfile," <text>%s</text---> ", title);
  fprintf(docsfile," <text>%s</text---> ", booktitle);
  fprintf(docsfile," \"%s\" " , pages);
  fprintf(docsfile," \"%s\" " , year);
  fprintf(docsfile," <text>%s</text---> " , journal);
  fprintf(docsfile," \"%s\" " , volume);
  fprintf(docsfile," \"%s\" " , number);
  fprintf(docsfile," \"%s\" " , month);
  fprintf(docsfile," <text>%s</text---> " , url);
  fprintf(docsfile," <text>%s</text---> ", school);
  fprintf(docsfile," <text>%s</text---> ", publisher);
  fprintf(docsfile," \"%s\" )\n" , isbn);

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

document        : article { }
                | inproceedings
                | proceedings
                | book
                | incollection
                | phdthesis
                | masterthesis
                | www
                ;

article         : STARTARTICLE fieldlist ENDARTICLE { 
                    fprintf(docsfile,"%s","( \"article\" ");
                    fprintf(docsfile," %d", ++docid);
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
                        fprintf(authorsfile,"( \"%s\" \"%s\" %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data);
			            fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);			  
	                  }
			          else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);
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
                    fprintf(docsfile,"%s","( \"inproceedings\" ");
                    fprintf(docsfile," %d", ++docid);
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
                        fprintf(authorsfile,"( \"%s\" \"%s\" %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);
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
                    fprintf(docsfile,"%s","( \"proceedings\" ");
                    fprintf(docsfile," %d", ++docid);
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
                        fprintf(authorsfile,"( \"%s\" \"%s\" %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);
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
                    fprintf(docsfile,"%s","( \"book\" ");
                    fprintf(docsfile," %d", ++docid);
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
                        fprintf(authorsfile,"( \"%s\" \"%s\" %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);
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
                    fprintf(docsfile,"%s","( \"incollection\" ");
                    fprintf(docsfile," %d", ++docid);
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
                        fprintf(authorsfile,"( \"%s\" \"%s\" %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);
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
                    fprintf(docsfile,"%s","( \"phdthesis\" ");
                    fprintf(docsfile," %d", ++docid);
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
                        fprintf(authorsfile,"( \"%s\" \"%s\" %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);
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
                    fprintf(docsfile,"%s","( \"mastersthesis\" ");
                    fprintf(docsfile," %d", ++docid);
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
                        fprintf(authorsfile,"( \"%s\" \"%s\" %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);
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
                    fprintf(docsfile,"%s","( \"www\" ");
                    fprintf(docsfile," %d", ++docid);
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
                        fprintf(authorsfile,"( \"%s\" \"%s\" %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);            
                      }
                      else {
                        strsplit(splitbuffer, (char*)key.data);
                        strtolower(lowerbuffer, splitbuffer);
                        fprintf(authordocfile,"( \"%s\" \"%s\" %d %d )\n",(char*)key.data,lowerbuffer,*(int*)data.data,docid);
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

    fprintf(docsfile,"%s","(OBJECT Document\n\t()\n\t(rel\n\t\t(tuple\n\t\t\t(\n");
    fprintf(docsfile,"%s","\t\t\t(Type string)\n");
    fprintf(docsfile,"%s","\t\t\t(Docid int)\n");
    fprintf(docsfile,"%s","\t\t\t(Authors text)\n");
    fprintf(docsfile,"%s","\t\t\t(Title text)\n");
    fprintf(docsfile,"%s","\t\t\t(Booktitle text)\n");
    fprintf(docsfile,"%s","\t\t\t(Pages string)\n");
    fprintf(docsfile,"%s","\t\t\t(Year string)\n");
    fprintf(docsfile,"%s","\t\t\t(Journal text)\n");
    fprintf(docsfile,"%s","\t\t\t(Volume string)\n");
    fprintf(docsfile,"%s","\t\t\t(Number string)\n");
    fprintf(docsfile,"%s","\t\t\t(Month string)\n");
    fprintf(docsfile,"%s","\t\t\t(Url text)\n");
    fprintf(docsfile,"%s","\t\t\t(School text)\n");
    fprintf(docsfile,"%s","\t\t\t(Publisher text)\n");
    fprintf(docsfile,"%s","\t\t\t(Isbn string))))\n(\n");

    fprintf(authordocfile,"%s","(OBJECT Authordoc\n\t()\n\t(rel\n\t\t(tuple\n\t\t\t(\n");
    fprintf(authordocfile,"%s","\t\t\t(Name string)\n");
    fprintf(authordocfile,"%s","\t\t\t(Lclastname string)\n");
    fprintf(authordocfile,"%s","\t\t\t(Authorid int)\n");
    fprintf(authordocfile,"%s","\t\t\t(Docid int))))\n(\n");

    fprintf(authorsfile,"%s","(OBJECT Author\n\t()\n\t(rel\n\t\t(tuple\n\t\t\t(\n");
    fprintf(authorsfile,"%s","\t\t\t(Name string)\n");
    fprintf(authorsfile,"%s","\t\t\t(Lclastname string)\n");
    fprintf(authorsfile,"%s","\t\t\t(Authorid int))))\n(\n");

    yyparse();
    if(oldstyle){
       fprintf(docsfile,"%s",")\n\t\t())\n");
       fprintf(authordocfile,"%s",")\n\t\t())\n");
       fprintf(authorsfile,"%s",")\n\t\t())\n");
    }else{
       fprintf(docsfile,"%s",")\n\t\t)\n");
       fprintf(authordocfile,"%s",")\n\t\t)\n");
       fprintf(authorsfile,"%s",")\n\t\t)\n");

    }
    return 0;
}
