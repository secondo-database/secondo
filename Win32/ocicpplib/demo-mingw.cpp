#include <ocicpp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <map>
#include <iostream.h>
using std::string;
using namespace OCICPP;

void printHead(Cursor &);
void printRow(Cursor &);
void help(Connection &);
void connect(Connection &);
void transStart(Connection &);
void transCommit(Connection &);
void transRollback(Connection &);
void test_hash_cap(Connection &);
void exit(Connection &);
void test_blob(Connection &);
void test_bfile(Connection &);
void test_aqm(Connection &);
void test_rowid(Connection &);
void test_refcur(Connection &);
void test_ntable(Connection &);
void test_perfomance(Connection &);
void test_prefetch(Connection &);
void createBFile(Connection &);
void serverVersion(Connection &);
void test_huge_insert(Connection &);

void getConnection(Connection &);
void testGetConnection(Connection &);

typedef void (*func_type) (Connection &);

map<string,func_type> commands_map;
map<string,string> help_map;

void init_command_map() {
	commands_map["connect"]=connect;
	commands_map["exit"]=exit;
	commands_map["help"]=help;
	commands_map["transStart"]=transStart;
	commands_map["transCommit"]=transCommit;
	commands_map["transRollback"]=transRollback;
	commands_map["test_hash_cap"]=test_hash_cap;
	commands_map["test_blob"]=test_blob;
	commands_map["test_aqm"]=test_aqm;
	commands_map["test_rowid"]=test_rowid;
	commands_map["test_refcur"]=test_refcur;
	commands_map["test_ntable"]=test_ntable;
	commands_map["test_perf"]=test_perfomance;
	commands_map["test_prefetch"]=test_prefetch;
	commands_map["test_bfile"]=test_bfile;
	commands_map["createBFile"]=createBFile;	
	commands_map["serverVersion"]=serverVersion;
	commands_map["test_huge_insert"]=test_huge_insert;
	commands_map["test_get_connection"]=testGetConnection;		
}
void init_help_map() {	
	help_map["connect"]="connecting  to database prompts for tnsname , user and password";
	help_map["exit"]="exit's demo";
	help_map["help"]="Prints this help";
	help_map["transStart"]="Starts new serializable transaction";
	help_map["transCommit"]="Commits current transaction"; 
	help_map["transRollback"]="Rollbacks current transaction";
	help_map["test_hash_cap"]="Shows use of hash capabilities";
	help_map["test_blob"]="Shows use of lob capabilities.";
	help_map["test_aqm"]="Shows use of bind calls.Uses as code example.";
	help_map["test_rowid"]="Shows use of rowid.Uses as code example";
	help_map["test_refcur"]="Shows use of refcursor.Uses as code example";
	help_map["test_ntable"]="Shows use of nested tables.Uses as code example";
	help_map["test_bfile"]="Shows use of BFile selected from table";
	help_map["createBFile"]="Shows use of BFiles specified by directory and name";
	help_map["serverVersion"]="Print server version";
}

Connection con;

int main(int argv,char **argc) {
	db::init();
	//Connection con;
	// Cursor cur;
	string sql;
	init_command_map();
	init_help_map();
	help(con);
	connect(con);
	while(1) {
		cout<<">";
		getline(cin,sql);
		try {
			 map<string,func_type>::const_iterator iter=commands_map.find(sql);
			 if(iter!=commands_map.end()) {
			 	(iter->second)(con);
			 } else if(sql=="") {
			 	help(con);
			 } else {
			 	cout<<"Constructing cursor"<<endl;
			 	Cursor cur;
			 	cout<<"Dropping it"<<endl;
			 	cur.drop();
				cout<<"Executing query"<<endl;
				con.execQuery(sql,cur);
				cout<<"executed ok"<<endl;
				printHead(cur);
				while(cur.fetch()) {
					printRow(cur);	
				}
			}
		} catch(OraError &er) {
			cout << er.message<<endl;
		}
	}
}	

void getConnection(Connection &con_) {
	con_=con;
}

void testGetConnection(Connection &con_) {
	OCICPP::Connection c;
	getConnection(c);
	OCICPP::Cursor cur;
	string sql="select * from dual";
	c.execQuery(sql,cur);
	printHead(cur);
	while(cur.fetch()) {
		printRow(cur);
	}
}



void connect(Connection &con) {
	string tns,user,passwd;
	cout << "connect string:";
	getline(cin,tns);
	cout<<"user:";
	getline(cin,user);
	cout<<"password:";
	getline(cin,passwd);
	cout<<"Connecting..."<<endl;
	con.drop();
	db::connect(tns.c_str(),user.c_str(),passwd.c_str(),con);
	cout<<"connected."<<endl;
}

void transStart(Connection &con) {
	con.transStart();
}

void transCommit(Connection &con) {
	con.transCommit();
}

void transRollback(Connection &con) {
	con.transRollback();
}

void test_hash_cap(Connection &con) {
	Cursor cur;
	string sql,cell;
	cout << "sql:";
	getline(cin,sql);
	cout << "enter '<' when finish playing"<<endl;
	con.execQuery(sql,cur);
	printHead(cur);
	if(!cur.fetch()) return;
	cout<<"fetched ok"<<endl;
	while(1) {
		string field;
		cout<<"Field to print or '.' to next row:";
		cin>>field;
		if(field==".") {
			if(!cur.fetch()) break;
		} else if(field=="<") {
			break;	
		} else {
			try {
				cur.getStr(field,cell);
				cout<<cell<<endl;
			} catch(OraError &er) {
				if(er.errtype()==OCICPPERROR) {
					cout<<er.message<<endl;
					continue;
				} else throw;
			}
		}
	}
}

void exit(Connection &con) {
	try {
		con.drop();
	} catch(OraError er) {
		cout << er.message<<endl;
		exit(1);
	}
	exit(0);
}

void test_bfile(Connection &con) {
	Cursor cur;
	string sql,field,mode,dir,pos,len;
	cout << "sql: ";
	getline(cin,sql);
	con.execQuery(sql,cur);
	if(!cur.fetch()) {
		cout << "No more rows"<<endl;
		return;
	}
	while(1) {
		cout<< "field to process or . for next row:";
		getline(cin,field);
		if(field==".") {
			if(!cur.fetch()) {
				cout << "No more rows "<<endl;
				return;
			}
		} else if(field=="") {
			return;
		} else {
			BFile bfile;
			unsigned actread;
			if(cur.getColType(field)==SQLT_FILE) {
				cur.getFILE(field,bfile);
				bfile.open();
			} else {
				return;
			}
			
			while(1) {
				cout << "mode(r/s/l/o) Enter to exit:";
				getline(cin,mode);
				if(mode=="s") {
					cout<<"Direction(SET/CUR/END:";
					getline(cin,dir);
					cout<<"Position:";
					getline(cin,pos);
					if(dir=="SET") bfile.seek(atoi(pos.c_str()),LOB_SET);
					else if(dir=="CUR") bfile.seek(atoi(pos.c_str()),LOB_CUR);
					else bfile.seek(atoi(pos.c_str()),LOB_END);
				} else if(mode=="o") {
					cout<<"Offset:"<<bfile.tell()<<endl;
				} else if(mode=="l") {
					cout << "Len: "<<bfile.getLen()<<endl;
				} else if(mode=="r") {
					char buf[256];
					while((actread=bfile.read(buf,254))) {
						buf[actread]='\0';
						cout << buf;
					}
					cout << endl;
				} else break;
			}
			bfile.close();
		}			
	}
}
void test_blob(Connection &con) {
	Cursor cur;
	string sql,field,mode,blob,dir,pos,len;
	cout << "sql: ";
	getline(cin,sql);
	con.execQuery(sql,cur);
	if(!cur.fetch()) {
		cout << "No more rows"<<endl;
		return;
	}
	while(1) {
		cout<< "field to process or . for next row:";
		getline(cin,field);
		if(field==".") {
			if(!cur.fetch()) {
				cout << "No more rows "<<endl;
				return;
			}
		} else if(field=="") {
			return;
		} else {
			Lob lob;
			unsigned actread;
			if(cur.getColType(field)==SQLT_BLOB) cur.getBLOB(field,lob);
			else if(cur.getColType(field)==SQLT_CLOB) cur.getCLOB(field,lob);
	
			while(1) {
				cout << "mode(r/w/s/t/l/o) Enter to exit:";
				getline(cin,mode);
				if(mode=="s") {
					cout<<"Direction(SET/CUR/END:";
					getline(cin,dir);
					cout<<"Position:";
					getline(cin,pos);
					if(dir=="SET") lob.seek(atoi(pos.c_str()),LOB_SET);
					else if(dir=="CUR") lob.seek(atoi(pos.c_str()),LOB_CUR);
					else lob.seek(atoi(pos.c_str()),LOB_END);
				} else if(mode=="o") {
					cout<<"Offset:"<<lob.tell()<<endl;
				} else if(mode=="t") {
					cout << "new len:";
					getline(cin,len);
					lob.trunc(atoi(len.c_str()));
				} else if(mode=="l") {
					cout << "Len: "<<lob.getLen()<<endl;
				} else if(mode=="r") {
					char buf[256];
					while((actread=lob.read(buf,254))) {
						buf[actread]='\0';
						cout << buf;
					}
					cout << endl;
				} else if(mode=="w") {
					cout<<"data:";
					getline(cin,blob);
					lob.write(blob.c_str(),strlen(blob.c_str()));
				} else break;
			}
		}			
	}
}
	
void test_aqm(Connection &con) {
	string sql,param,val;
	short isNull;
	map<string,string> params;
	Cursor cur;
	cout<<"sql to run:";
	getline(cin,sql);
	con.prepare(sql,cur);
	cout<<"Statement prepared"<<endl;
	do {
		cout<<"parameter name:";
		getline(cin,param);
		if(param=="") break;
		cout<<"value:";
		getline(cin,val);
		params[param]=val;
		cur.bind(param,val,&isNull);
	} while(1);
	cur.execute();
	printHead(cur);
	while(cur.fetch()) {
		printRow(cur);	
	}
}
void test_rowid(Connection &con) {
	string sql="select rowid,id,var_col from ociex where id<5 for update";
	string id,upSql;
	Cursor cur1,cur2;
	RowID rowid;
	short isNull;
	con.execQuery(sql,cur1);
	while(cur1.fetch()) {
		cur1.getRowID("ROWID",rowid);
		cur1.getStr("ID",id);
		upSql="update ociex set var_col='testing rowid id="+id+"' where rowid=:rid";
		cout<<upSql<<endl;
		con.prepare(upSql,cur2);
		cout<<"Statement for cur2 prepared"<<endl;
		cur2.bind(":rid",rowid,&isNull);
		cout<<"binded ok"<<endl;
		cur2.execute();
		cout<<"executed ok"<<endl;
	}
} 

void test_refcur(Connection &con) {
	Cursor cur1,cur2;
	string sql="
	begin 
		OPEN :cursor1 FOR SELECT * FROM ociex; 
	end;";
	con.prepare(sql,cur1);
	cur1.bind(":cursor1",cur2);
	cur1.execute();
	cur2.execute();
	printHead(cur2);
	while(cur2.fetch()) {
		printRow(cur2);
	}
}

void test_ntable(Connection &con) {
	Cursor cur1,cur2,cur3;
/*
	string sql="select p.poster_id,CURSOR(select pp.play_id,t.text prod_name 
	                          from t_play pp, t_text t
	                          where t.object_type_id(+)=12
	                           and  t.text_type_id(+)=23
	                           and  t.object_id(+)=pp.play_id
	                           and  t.language_id(+)=1
	                           and  pp.poster_id=p.poster_id) ddd from t_poster p";
*/
/*
	string sql="select CURSOR(select pp.play_id,t.text prod_name 
	                          from t_play pp, t_text t
	                          where t.object_type_id(+)=12
	                           and  t.text_type_id(+)=23
	                           and  t.object_id(+)=pp.play_id
	                           and  t.language_id(+)=1
	                           ) ddd from dual";
*/
	string sql="begin PLAY.get_select(:cur); end;";
	con.prepare(sql,cur3);
	cur3.bind(":cur",cur1);
	cur3.execute();
	cur1.execute();
//	con.execQuery(sql,cur3);
	printHead(cur1);
	while(cur1.fetch()) {
		cur1.getCursor(1,cur2);
		cur2.execute();
		printHead(cur2);
		while(cur2.fetch()) {
			printRow(cur2);
		}
		cur2.drop();
	}		
}

void test_perfomance(Connection &con) {
#ifndef __MINGW32__
	timeval start,end;
#else
	clock_t start,end;
#endif
	Cursor cur;
	string sql;
	unsigned prefetch;
	double diff,overall=0;
	int nrows=0;
	cout << "sql:";
	getline(cin,sql);
	cout << "rows to prefetch:";
	cin >> prefetch;
#ifndef __MINGW32__
	gettimeofday(&start,0);
#else
	start = clock();
#endif
	con.prepare(sql,cur);
#ifndef __MINGW32__
	gettimeofday(&end,0);
	diff=(end.tv_sec-start.tv_sec)+(end.tv_usec-start.tv_usec)/1000000.0;
#else
	end = clock();
	diff = (end - start) * 1.0 / CLOCKS_PER_SEC;
#endif
	overall+=diff;
	cout << "Prepared in " << diff <<endl;
	start=end;
	cur.execute();
#ifndef __MINGW32__
	gettimeofday(&end,0);
	diff=(end.tv_sec-start.tv_sec)+((double)(end.tv_usec-start.tv_usec))/1000000;
#else
	end = clock();
	diff = (end - start) * 1.0 / CLOCKS_PER_SEC;
#endif
	overall+=diff;
	cout << "Executed in " << diff <<endl;	
	start=end;
	while(cur.fetch()) {
		/* dummy while */
		nrows++;
	}
#ifndef __MINGW32__
	gettimeofday(&end,0);		
	diff=(end.tv_sec-start.tv_sec)+((double)(end.tv_usec-start.tv_usec))/1000000;
#else
	end = clock();
	diff = (end - start) * 1.0 / CLOCKS_PER_SEC;
#endif
	overall+=diff;
	cout << "Fetched " << nrows << " rows in " << diff << "seconds. Overall "<<overall << endl;
}		

void createBFile(Connection &con) {
	string dir,file;
	string direction,pos;
	unsigned actread;
	BFile bfile;
	cout << "dir:";
	cin >> dir;
	cout << "file name:";
	cin >> file;
	con.createBFile(dir,file,bfile);
	bfile.open();
	while(1) { 
		string c; // command
		cout << "(r/s/t/l/q) >";
		cin >> c;
		if(c=="r") {
			char buf[256];
			while((actread=bfile.read(buf,254))) {
				buf[actread]='\0';
				cout << buf;
			}
		} else if(c=="s") {
			cout<<"Direction(SET/CUR/END:";
			getline(cin,direction);
			cout<<"Position:";
			getline(cin,pos);
			if(direction=="SET") bfile.seek(atoi(pos.c_str()),LOB_SET);
			else if(direction=="CUR") bfile.seek(atoi(pos.c_str()),LOB_CUR);
			else bfile.seek(atoi(pos.c_str()),LOB_END);
		} else if(c=="t") {
			cout<<"Offset:"<<bfile.tell()<<endl; 	
		} else if(c=="l") {
			cout << "Len: "<<bfile.getLen()<<endl;
		} else if(c=="q") { 
			break;
		}
	}
	bfile.close();
}

			




void test_prefetch(Connection &con) {
#ifndef __MINGW32__
	timeval start,end;
#else
	clock_t start,end;
#endif
	Cursor cur;
	string sql;
	int prefetch,nrows=0;
	double diff,overall=0;
	cout << "sql:";
	getline(cin,sql);	
	cout << "Prefetch:";
	cin >> prefetch;
#ifndef __MINGW32__
	gettimeofday(&start,0);
#else
	start = clock();
#endif
	con.execQuery(sql,cur,prefetch);
#ifndef __MINGW32__
	gettimeofday(&end,0);
	diff=(end.tv_sec-start.tv_sec)+(end.tv_usec-start.tv_usec)/1000000.0;
#else
	end = clock();
	diff = (end - start) * 1.0 / CLOCKS_PER_SEC;
#endif
	overall+=diff;
	cout << "executed in " << diff <<endl;
	start=end;
	while(cur.fetch()) {
		/* dummy while */
		nrows++;
		cerr << cur.getStr(0) << endl;
	}
#ifndef __MINGW32__
	gettimeofday(&end,0);
	diff=(end.tv_sec-start.tv_sec)+((double)(end.tv_usec-start.tv_usec))/1000000;
#else
	end = clock();
	diff = (end - start) * 1.0 / CLOCKS_PER_SEC;
#endif
	overall+=diff;
	cout << "Fetched " << nrows << " rows in " << diff << "seconds. Overall "<<overall << endl;
}

void serverVersion(Connection &con) {
	cout << con.serverVersion() << endl;
}

void test_huge_insert(Connection &con) {
	int i;
	std::string sql;
	try {
		for(i=0;i<1000000000;i++) {
			char buf[256];
			sprintf(buf,"%d",i);
			sql="insert into test1(id,f1,f2) values(";
			sql+=buf;
			sql+=",'f1=";
			sql+=buf;
			sql+="','f2=-";
			sql+=buf;
			sql+="')";
			cout << sql << endl;
			con.execUpdate(sql);
			
		}
	} catch(OraError er) {
		cerr << "catch error:" << er.message << " i=" << i << endl;
	}
}

void printHead(Cursor &cur) {
	string attrName;
	for(int i=0;i<cur.getNCols();i++) {
		cur.getColName(i,attrName);
		if(!i) cout<<"|";
		cout<<attrName<<"("<<cur.getColSize(i)<<")|";
	}
	cout << endl;
}

void printRow(Cursor &cur) {
	string cell;
	int nCols=cur.getNCols();
	for(int i=0;i<nCols;i++) {
		cur.getStr(i,cell);
		if(!i) cout<<"|";
		cout<<cell<<"|";
	}
	cout << endl;
}

void help(Connection &con) {
	map<string,string>::const_iterator ci=help_map.begin();
	cout << "Here's a list of available commands:"<<endl;
	while(ci!=help_map.end()) {
		cout << ci->first << " " << ci->second << endl;
		*ci++;
	}
	cout << "For details please visit docs's page at http://ocicpplib.sourceforge.net/docs.html" <<endl;
}

