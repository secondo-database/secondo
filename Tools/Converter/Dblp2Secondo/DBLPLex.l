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
#include "DBLPParser.y.h"

int i;

%}



/* regular definitions */

startarticle           <article.*>
endarticle             <\/article>
startinproceedings     <inproceedings.*>
endinproceedings       <\/inproceedings>
startproceedings       <proceedings.*>
endproceedings         <\/proceedings>
startbook              <book[ ].*>
endbook                <\/book>
startincollection      <incollection.*>
endincollection        <\/incollection>
startphdthesis         <phdthesis.*>
endphdthesis           <\/phdthesis>
startmasterthesis      <mastersthesis.*>
endmasterthesis        <\/mastersthesis>
startwww               <www.*>
endwww                 <\/www>
author                 <author>[^(<\/)]*<\/author>
editor                 <editor>.*<\/editor>
address                <address>.*<\/address>
title                  <title>.*<\/title>
booktitle              <booktitle>.*<\/booktitle>
pages                  <pages>.*<\/pages>
year                   <year>.*<\/year>
journal                <journal>.*<\/journal>
volume                 <volume>.*<\/volume>
number                 <number>.*<\/number>
month                  <month>.*<\/month>
url                    <url>.*<\/url>
cite                   <cite>.*<\/cite>
school                 <school>.*<\/school>
publisher              <publisher>.*<\/publisher>
note                   <note>.*<\/note>
crossref               <crossref>.*<\/crossref>
isbn                   <isbn>.*<\/isbn>
chapter                <chapter>.*<\/chapter>
series                 <series>.*<\/series>

%%

{startarticle}         { return(STARTARTICLE); }
{endarticle}           { return(ENDARTICLE); }
{startinproceedings}   { return(STARTINPROCEEDINGS); }
{endinproceedings}     { return(ENDINPROCEEDINGS); }
{startproceedings}     { return(STARTPROCEEDINGS); }
{endproceedings}       { return(ENDPROCEEDINGS); }
{startbook}            { return(STARTBOOK); }
{endbook}              { return(ENDBOOK); }
{startincollection}    { return(STARTINCOLLECTION); }
{endincollection}      { return(ENDINCOLLECTION); }
{startphdthesis}       { return(STARTPHDTHESIS); }
{endphdthesis}         { return(ENDPHDTHESIS); }
{startmasterthesis}    { return(STARTMASTERTHESIS); }
{endmasterthesis}      { return(ENDMASTERTHESIS); }
{startwww}             { return(STARTWWW); }
{endwww}               { return(ENDWWW); }

{author}               { yytext[yyleng-9]='\0';
                         yylval.string = strdup(yytext+8);
                         // replace " by '
                         i=0;
                         while (yylval.string[i] != '\0') {
                           if (yylval.string[i] == '\"') yylval.string[i]='\'';
                           i++;
                         }
                         return(AUTHOR);
                       }
{editor}               { yytext[yyleng-9]='\0';
                         yylval.string = strdup(yytext+8);
                         return(EDITOR);
                       }
{title}                { yytext[yyleng-8]='\0';
                         yylval.string = strdup(yytext+7);
                         return(TITLE);
                       }
{booktitle}            { yytext[yyleng-12]='\0';
                         yylval.string = strdup(yytext+11);
                         return(BOOKTITLE);
                       }
{pages}                { yytext[yyleng-8]='\0';
                         yylval.string = strdup(yytext+7);
                         return(PAGES);
                       }
{year}                 { yytext[yyleng-7]='\0';
                         yylval.string = strdup(yytext+6);
                         return(YEAR);
                       }
{journal}              { yytext[yyleng-10]='\0';
                         yylval.string = strdup(yytext+9);
                         return(JOURNAL);
                       }
{volume}               { yytext[yyleng-9]='\0';
                         yylval.string = strdup(yytext+8);
                         return(VOLUME);
                       }
{number}               { yytext[yyleng-9]='\0';
                         yylval.string = strdup(yytext+8);
                         return(NUMBER);
                       }
{month}                { yytext[yyleng-8]='\0';
                         yylval.string = strdup(yytext+7);
                         return(MONTH);
                       }
{url}                  { yytext[yyleng-6]='\0';
                         yylval.string = strdup(yytext+5);
                         return(URL);
                       }
{school}               { yytext[yyleng-9]='\0';
                         yylval.string = strdup(yytext+8);
                         return(SCHOOL);
                       }
{publisher}            { yytext[yyleng-12]='\0';
                         yylval.string = strdup(yytext+11);
                         return(PUBLISHER);
                       }
{isbn}                 { yytext[yyleng-7]='\0';
                         yylval.string = strdup(yytext+6);
                         return(ISBN);
                       }

.|\n                   { ; }


%%

 
