
%{
	#include <iostream>
	#include "web.h"
	#include "string.h"
	#define YY_DECL int WebLex::yylex(int switchcond)

	
	using namespace std;

	//This just a stub. It will never be called because of YY_DECL. 
	int yyFlexLexer::yylex()
	{
		return 0;
	}
	
	string element;
	int myleng;
	//if we read a script element we have to use _RSCRIPT instead of _RCONTENT because in the content of script elements "<" and ">" is allowed
	int script_element=0;
	

	
%}
%x _MURI
%x _MSCHEME
%x _MAUTHORITY
%x _MPATH
%x _PHTML
%x _HREF
%x _RELEM_WA
%x _RELEM
%x _RCONTENT
%x _COMMENT
%x _COMMENT_NEU
%x _RCOMMENT
%x _EXTRACT
%x _FINDELEMSTART
%x _ATTVALUE
%x _RSCRIPT
%option stack
%option noyywrap

hexdig		[0-9a-fA-F]
alpha			[a-zA-Z]
digit			[0-9]

name			[0-9a-zA-Z\-\.\:]*

uri     	       {scheme}":"{hier-part}("?"{query})?("#"{fragment})?
hier-part    	("//"{authority}{path-abempty}|{path-absolute}|{path-rootless}|{path-empty})
uri-reference	({uri}|{relative-ref})
absolute-uri	{scheme}":"{hier-part}("?"query)?
relative-ref		{relative-part}("?"{query})?("#"{fragment})?
relative-part	("//"{authority}{path-abempty}|{path-absolute}|{path-noscheme}|{path-empty})
scheme        	{alpha}({alpha}|{digit}|"+"|"-"|".")*
authority     	({userinfo}"@")?{host}(":"{port})?
userinfo      	({unreserved}|{pct-encoded}|{sub-delims}|":")*
host          	({ipv4address}|{reg-name})
port          		{digit}*
ipv4address   	{dec-octet}"."{dec-octet}"."{dec-octet}"."{dec-octet}
dec-octet     	({digit}|{digit}{digit}|1{digit}{digit}|2[0-4]{digit}|25[0-5])
reg-name            ({unreserved}|{pct-encoded}|{sub-delims})*
path          	({path-abempty}|{path-absolute}|{path-noscheme}|{path-rootless}|{path-empty})


path-abempty	("/"{segment})*
path-absolute	"/"({segment-nz}("/"{segment})*)?
path-noscheme	{segment-nz-nc}("/"{segment})*
path-rootless	{segment-nz}("/"{segment})*


path-empty	""
segment		{pchar}*
segment-nz	{pchar}+
segment-nz-nc	({unreserved}|{pct-encoded}|{sub-delims}|"@")+
pchar			({unreserved}|{pct-encoded}|{sub-delims}|":"|"@")
query			({pchar}|"/"|"?")*
fragment		({pchar}|"/"|"?")*
pct-encoded	"%"{hexdig}{hexdig}
unreserved		({alpha}|{digit}|"-"|"."|"_"|"~")
reserved		({gen-delims}|{sub-delims})
gen-delims		(":"|"/"|"?"|"#"|"["|"]"|"@")
sub-delims		("!"|"$"|"&"|"'"|"("|")"|"*"|"+"|","|";"|"=")
tag_open		<
tag_close		>
white	[ \t\n]
nwhite [^ \t\n]


%%
	{
		
		
		
		
		switch (switchcond){
			case MURI:
				switchcond = -1;
				BEGIN(_MURI);
				break;
				
			case MSCHEME:
				switchcond = -1;
				BEGIN (_MSCHEME);
				break;

			
			case FINDELEMSTART:
				switchcond = -1;
				BEGIN  (_FINDELEMSTART);
				break;
				
			case RELEM_WA:
				switchcond = -1;
				BEGIN  (_RELEM_WA);
				break;

			case RSCRIPT:
				switchcond = -1;
				yy_push_state(_RSCRIPT);
				break;
				
			
			case -1:
				break;
				
			default: 
				cout << "Ungültige Startbedingung" << endl;
				return ERROR;
				
			}
	}
	
				

<_MURI>{uri}	return URI;
<_MURI>{relative-ref} return RURI;
<_MURI,_MSCHEME>{scheme}":"	{BEGIN(_MAUTHORITY);return SCHEME;}
<_MAUTHORITY>"//"{authority}	{BEGIN(_MPATH);return AUTHORITY;}
<_MPATH>{path-abempty}("?"{query})?("#"{fragment})?	return PATH;


<_FINDELEMSTART>[^\<]	{return SEARCH_ELEMENT_START;}
<_FINDELEMSTART>\<	{BEGIN (_RELEM);
							//cout << "_FINDELEMSTART  yytext: " << yytext << "  yyin->tellg(): " << (unsigned long)yyin->tellg() << " yyleng: " << yyleng << " pos: " << endl;
							
						}
<_RELEM>[a-zA-Z\-\_\:\.]{name}	{pos= ((unsigned long)yyin->tellg())- yyleng ;
							//cout << "_RELEM1 yytext: " << yytext << "  yyin->tellg(): " << (unsigned long)yyin->tellg() << " yyleng: " << yyleng << " pos: " << endl;
							//string tmp = yytext;
							if (isEqual(yytext,"script"))
								script_element=1;
							return EIDENTIFIER;
						}



<_RELEM>{white}*={white}*	{BEGIN (_ATTVALUE);}
<_RELEM>{white}*
<_RELEM>\/	//eat up
<_ATTVALUE>\"[^\"]*\"	{pos= ((unsigned long)yyin->tellg())- yyleng +1;BEGIN(_RELEM);return ATTVALUE;}
<_ATTVALUE>[^\>\t\n \"]+	{pos= ((unsigned long)yyin->tellg())- yyleng +1;BEGIN(_RELEM);return ATTVALUE;}
<_RELEM>">"	{if (script_element){
				yy_push_state(_RSCRIPT);
				script_element=0;
			}else
				BEGIN(_RCONTENT);
			return CLOSE_TAG;
			}
<_RELEM>. //eat up


<_RELEM_WA,_RELEM>"<"("!"[^\>][^-\>]|"!"[^-\>][^\>]|[\/\?]|[A-Za-z\-\.\:\_]){name}*	{
																	
																	pos= ((unsigned long)yyin->tellg())- yyleng ;
																	yy_push_state(YY_START);
																	//cout << " _RELEM2 yytext: " << yytext << "  yyin->tellg(): " << yyin->tellg() << " yyleng: " << yyleng << " pos: " << endl;
																	
																	if (YY_START == _RELEM_WA){
																		//cout << "_RELEMWA";
																		myleng= yyleng;
																		yymore();
																	} else {
																	 
																	 yytext=yytext+1;
																	 pos= (((unsigned long)yyin->tellg()))- yyleng;
																	 //cout << "<_RELEM_WA,_RELEM>" << YYText() << endl;
																	 return EIDENTIFIER;

																	 
																	}
																}

<_RELEM_WA,_RELEM>"<!--"		{yy_push_state(_COMMENT_NEU);}

<_COMMENT_NEU>.|\n		//eat up
<_COMMENT_NEU>"-->"	{BEGIN(_RCONTENT);return COMMENT;}

<_RELEM_WA>[^\>\<]*\>				{
											
											//cout << "Lexer: " << YYText() << endl;
											pos= ((unsigned long)yyin->tellg())- yyleng;
											
											
											yytext[myleng]='\0';
											yytext= yytext+1;
											//cout << yytext[0] << endl;
											
											myleng= yyleng;
											//yyleng= strlen(yytext);
											
											//cout << "Prüfe treffer ********** " << yytext << endl;



											if (isEqual(yytext, "script")){
												//cout << "*******TREFFER**********" << endl;
												yy_push_state(_RSCRIPT);
												return ELEMENT;
											}


											yy_push_state(_RCONTENT);

											if (yytext[0] == '/'){
												return ELEMENT_CLOSE;
											}
											

											
											
											if (yytext[myleng -1] == '/' || yytext[0] == '!' or yytext[0] == '?')
												return ELEMENT_SA;
											else{
												return ELEMENT;
											}
							}
															


<_RSCRIPT>.	{return CONTENT;}
<_RSCRIPT>{white}	{return CONTENT;}
<_RSCRIPT>"</"("script"|"SCRIPT"|"Script")	{yyless(0);yy_pop_state();return CONTENT_END;}

<_RCONTENT>"<"	{yyless(0);yy_pop_state();return CONTENT_END;}
<_RCONTENT>.		{return CONTENT;}
<_RCONTENT>{white}	{return CONTENT;}

							

<_PHTML>\n	//eat up word wrap
<_PHTML>.		//eat up arbitrary character
<*>{white}*	//eat up
<*>. 	{return ERROR;}


%%

/*bool isEqual (string s1, string s2){
	transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
	transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
	
	return s1 == s2;
}*/
