/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen
Faculty of Mathematic and Computer Science,
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

#include "QueryTree.h"

#include "NestedList.h"
#include "ListUtils.h"
#include "Utils.h"
#include "PropertyGraphQueryProcessor.h"

using namespace std;

namespace pgraph {

//----------------------------------------------------------------------------
QueryFilter* QueryFilter::Clone()
{
   QueryFilter* f=new QueryFilter();
   f->Name=Name;
   f->Value=Value;
   f->Indexed=Indexed;
   return f;
}

//----------------------------------------------------------------------------
QueryTreeNode::QueryTreeNode()
{  
   static int treenodeids=100;
   _uid=treenodeids++;
}

//----------------------------------------------------------------------------
double QueryTreeNode::CalcCost()
{
   double c=1;
   for(auto&& edge : Edges) 
   {
       c = c * edge->CalcCost();
   }
   return c;
}

//----------------------------------------------------------------------------
QueryTreeNode::~QueryTreeNode()
{
    for(auto&& edge : Edges) 
        delete edge;
    Edges.clear();      
}

//----------------------------------------------------------------------------
QueryTreeNode* QueryTree::ReadNode(ListExpr list)
{
   QueryTreeNode *n = ReadNodeInfo(nl->First(list));
   list=nl->Rest(list);
   ReadEdges(n, list);
   return n;
}

//----------------------------------------------------------------------------
void QueryTreeNode::Reset()
{
}

//----------------------------------------------------------------------------
void QueryTreeNode::GetPosVector(vector<QueryTreeEdge*> *posvector)
{
    
    for (auto&& qe:Edges)
    {
       posvector->push_back(qe);
       qe->ToNode->GetPosVector(posvector);
    }
}

//----------------------------------------------------------------------------
double QueryTreeEdge::CalcCost()
{
   return  Cost * ToNode->CalcCost();
}

//----------------------------------------------------------------------------
QueryTreeNode* QueryTree::ReadNodeInfo(ListExpr list)
{
    QueryTreeNode *n = new QueryTreeNode();
 
    auto getAliasOrTypeName = [](QueryTreeNode *n, ListExpr atom) 
    { 
       string s=nl->ToString(atom);
       if (FirstUpper(s))
          n->TypeName=s;
       else
          n->Alias=s;
     };

    if (nl->IsAtom(list))
    {
       getAliasOrTypeName(n,list);
    }
    else
    {
         while (!nl->IsEmpty(list))
         {
            ListExpr item=nl->First(list);
            list=nl->Rest(list);

           // alias or typename
            if (nl->IsAtom(item)) 
               getAliasOrTypeName(n,item);
            else
               QueryTree::ReadFilters(n->Filters, item);
         }
    }

    AliasList.AddNode(n->Alias, n->TypeName);

    return n;
}

//----------------------------------------------------------------------------
QueryTree::~QueryTree()
{
   if (Root!=NULL)
      delete Root;
}

//----------------------------------------------------------------------------
string QueryTree::DumpTreeAsList(QueryTreeNode *n, int level, 
   ostringstream *data)
{
   if (n==NULL)  {
      if (Root!=NULL)   {
         ostringstream os;
         DumpTreeAsList(Root, level, &os);
         return os.str();
      }
      return "";
   }

   //
   string indent="";
   for(int i=0; i<level; i++) indent+="  ";
            
   *data << "(" << n->Alias << n->TypeName << ")" << "\n";
   for(auto&& e : n->Edges) 
   {
       *data << e->Alias << e-> TypeName << "\n";
       DumpTreeAsList(e->ToNode,level+2, data);
   }
   
   return "";
}

//----------------------------------------------------------------------------
void QueryTree::ReadFilters(list<QueryFilter*> &filters, ListExpr list)
{
   while(!nl->IsEmpty(list))
   {
      if (nl->ListLength(nl->First(list))!=2)
         throw PGraphException("Node filter expects a list of (name value) "
          "lists!");

      QueryFilter *f=new QueryFilter();
      filters.push_back(f);
      f->Name = nl->ToString(nl->First(nl->First(list)));
      f->Value = nl->ToString(nl->Second(nl->First(list)));
      ReplaceStringInPlace(f->Value, "\"","");
      list=nl->Rest(list);
   }
}

//----------------------------------------------------------------------------
void QueryTree::DumpTreeDot(QueryTreeNode *n, string fn, ostringstream *data)
{
    if (n==NULL)  {
        if (Root!=NULL)  
            DumpTreeDot(Root, fn);
        return;
    }

    bool created=false;    
    if (data==NULL) { 
       data=new ostringstream(); 
       *data << "digraph g {\n";
       created=true;
    }

    *data <<"n"<<n->_uid<<+"[label=<"+n->Alias;
    if (n->TypeName!="")
        *data<<"<BR/>:"<<n->TypeName  ;
    if (n->Cost>-1) *data<<"<BR/>(COST:"<<round(n->Cost,4) << ")<BR/>";
    
    
    list <QueryFilter*> :: iterator itf;         
    for(auto&& item : n->Filters) 
        *data << (item->Indexed?"(INDEXED)":"") << item->Name <<" = " 
              << item->Value << "<BR/>\n";

    *data<<">]\n";    

    //
    for(auto&& e : n->Edges) 
    {
        if (e->ToNode!=NULL)
        {
           //if (!(*it)->Reverse)
              *data << "n"<<e->FromNode->_uid<<"->n"<<
                  e->ToNode->_uid<<"\n";
           if (e->Reverse) *data<<"[dir=back]";
           *data << " [label=<";
           if (e->Alias!="")
              *data<<e->Alias;
           if (e->TypeName!="")
              *data<<":"<<e->TypeName;
           if (e->Cost>-1) *data<<"<BR/>(COST:"<<round(e->Cost,4) << ")<BR/>";
           *data<<">]\n";    

           DumpTreeDot(e->ToNode, fn, data);
        }
    }

    if (created)
    {
      *data<<"}\n";
      std::ofstream outfile;
      outfile.open(fn, std::ios_base::trunc);
      outfile << data->str();     
      delete data;
    }
}

//----------------------------------------------------------------------------
void QueryTree::ReadEdges(QueryTreeNode *n, ListExpr list)
{
    LOGOP(20, "QueryTree::ReadEdges", nl->ToString(list));

    while(true)
    {    
       // detect entries      
       int idx=GetNextListIndex(list);
       if (idx==0) throw PGraphException("invalid format -no "
            "edge information !");
       if (idx>3) throw PGraphException("invalid format - edge "
             "information max 3 items!");
       if (idx<1) break; // no following node list found

       // create edge
       QueryTreeEdge *edge=new QueryTreeEdge();
       edge->FromNode=n;

       while (idx>0)
       {
           string s=nl->ToString(nl->First(list));
           list=nl->Rest(list);
            if (s=="->") 
               edge->Reverse=false;
            else
            if (s=="<-") 
               edge->Reverse=true;
            else
               if (FirstUpper(s)) 
                  edge->TypeName=s;
               else
                  edge->Alias=s;

           idx--;
       }
      n->Edges.push_back(edge);
      AliasList.AddNode(edge->Alias, edge->TypeName);

      //  read node
      ListExpr nodeinfo= nl->First(list);
      list=nl->Rest(list);

      edge->ToNode=ReadNode(nodeinfo);

    }
    if (!nl->IsEmpty(list)) {
        throw PGraphException("invalid format - unexpected list entries " 
             "after edge node");
    }
}

//----------------------------------------------------------------------------
void  QueryTree::Clear()
{
    if (Root!=NULL)
    {
        delete Root;
        Root=NULL;
    }
}

//----------------------------------------------------------------------------
void QueryTree::ReadFilterList(ListExpr list)
{
   LOGOP(20, "QueryTree::ReadFilterList", nl->ToString(list));
   filterList.ReadFromList(list);
}

//----------------------------------------------------------------------------
void QueryTree::Validate()
{/*
   list<string> aliases;
   aliasList(&aliases);
   for(auto&& f : outputFields.Fields)
   {
     std::list<std::string>::iterator  it = std::find(aliases.begin(),
         aliases.end(), f->NodeAlias);
     if (it==aliases.end())
        throw new PGraphException("some outputfiled aliases do not exist!");

   }*/
}

//----------------------------------------------------------------------------
void QueryTree::Reset()
{
   if (Root!=NULL)
      Root->Reset();
}

//----------------------------------------------------------------------------
void QueryTree::ReadOutputFieldList(ListExpr list)
{
   LOGOP(20, "QueryTree::ReadOutputFieldList", nl->ToString(list));
   outputFields.ReadFromList(list);
}

//----------------------------------------------------------------------------
void QueryTree::ReadQueryTree(string slist)
{
   ListExpr alist=0;
   nl->ReadFromString(slist, alist);
   try
      {
         ReadQueryTree(alist);
   }
      catch(...)
   {
      nl->Destroy(alist);  
      throw;
   }
   nl->Destroy(alist);
}

//----------------------------------------------------------------------------
double QueryTree::CalcCost()
{
   return Root->Cost * Root->CalcCost();
}

//----------------------------------------------------------------------------
void QueryTree::ReadQueryTree(ListExpr alist)
{
    Clear();

    try
    {
        if (nl->ListLength(alist)==0)
        throw PGraphException("At least a list with at least on node "
        "is required");

        Root=QueryTree::ReadNode(alist);
    }
    catch(PGraphException &e)
    {
        Clear();
        throw;
    }
}


} // namespace