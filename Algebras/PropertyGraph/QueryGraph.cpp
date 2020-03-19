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

#include "QueryGraph.h"

#include "NestedList.h"
#include "ListUtils.h"
#include "Utils.h"
#include "PropertyGraphQueryProcessor.h"
#include <set>

using namespace std;

namespace pgraph {

//----------------------------------------------------------------------------
QueryGraph::QueryGraph(RelationRegistry *relreg)
{
   RelRegistry=relreg;
}

//----------------------------------------------------------------------------
QueryGraph::~QueryGraph()
{
}

//----------------------------------------------------------------------------
void QueryGraph::DumpGraph()
{
     cout << "GRAPH:"<<endl;
     cout << "  NODES:" <<endl;
     for (auto&& n : Nodes)
     {
        cout << "     "  << n.Alias  << " [:" << n.TypeName << "]" << endl;
        for (auto&& p : n.Filters)
           cout << "       "  << p->Name  << " = " << p->Value<< endl;;
     }
     cout << "  EDGES:" <<endl;
     for (auto&& e : Edges)
     {
        cout << "     "  << e.FromNode->Alias  << " -> " << e.ToNode->Alias 
             << "[:"<< e.TypeName <<"]" << endl;
        for (auto&& p : e.Filters)
           cout << "       "  << p->Name  << " = " << p->Value<< endl;;
     }
     cout << "." <<endl;
}

//----------------------------------------------------------------------------
void QueryGraph::DumpGraphDot(string fn)
{
   LOGOP(20,"QueryGraph::DumpGraphDot");
   ostringstream data;
   data << "digraph g {\n";

   for (auto&& n : Nodes)
   {
      data <<"n"<<n._uid<<+"[label=<"+n.Alias;
      if (n.TypeName!="")
         data<<"<BR/>:"<<n.TypeName;
       
      data<<"<BR/> CARD: "<<n.Cost<<"<BR/>";   

      list <QueryFilter*> :: iterator itf;         
      for(auto&& item : n.Filters) 
      {
         
         data << ((item->Indexed)?"(INDEXED) ":"");
         data << item->Name <<" = " 
              << item->Value << "<BR/>\n";
      }

      data<<">]\n";    
   }

    //
   for (auto&& e : Edges)
   {
      data << "n"<<e.FromNode->_uid<<"->n"<<e.ToNode->_uid<<"\n";
      data << " [label=<";
      if (e.Alias!="")
         data<<e.Alias;
      if (e.TypeName!="")
         data<<":"<<e.TypeName;
      data<<"<BR/>AVG: "<<round(e.CostFw,4)<<"/"<<round(e.CostBw,4)<<"<BR/>"; 
      data<<">]\n";    
    }
   data<<"}\n";

   std::ofstream outfile;
   outfile.open(fn, std::ios_base::trunc);
   outfile << data.str();     
    
    LOGOP(20,"/QueryGraph::DumpGraphDot");
}

//----------------------------------------------------------------------------
QueryGraphNode::QueryGraphNode()
{
   static int treenodeids=100;
   _uid=treenodeids++;
}
//----------------------------------------------------------------------------
QueryGraphNode* QueryGraph::addNode(string alias, string typename_, 
    ListExpr *props)
{
   LOGOP(30,"ADDNODE ",alias,":",typename_, (props==NULL?"":" PROPS") );
   QueryGraphNode n;
   n.ID=NextNodeID;
   n.Alias=alias;
   n.TypeName=typename_;

   AliasList.AddNode(n.Alias, n.TypeName);

   if (props!=NULL)
      readFilters(n.Filters, *props);

   Nodes.push_back(n);

   NextNodeID++;

   return &Nodes.back();
}

//----------------------------------------------------------------------------
void QueryGraph::addEdge(string edgelias, string typename_,string alias, 
    string alias2, ListExpr *props)
{
   LOGOP(30, "ADDEDGE ", alias, " -[",edgelias,":", typename_, "]-> ", 
       alias2, (props==NULL?"":" PROPS") );

   QueryGraphEdge e;
   e.ID=NextEdgeID;
   e.Alias=edgelias;
   e.TypeName=typename_;
   for(auto&& n:Nodes)
      if (n.Alias==alias)
         e.FromNode=&n;
   for(auto&& n:Nodes)
      if (n.Alias==alias2)
         e.ToNode=&n;

   AliasList.AddEdge(e.Alias, e.TypeName);

   RelationInfo *relinfo = RelRegistry->GetRelationInfo(e.TypeName);
   if (relinfo!=NULL) {
      e.CostFw=relinfo->statistics->avgcardForward;
      e.CostBw=relinfo->statistics->avgcardBackward;;
   }

   // create missing nodes
   if (e.FromNode==NULL) 
      e.FromNode=addNode(alias,"", NULL);

   if (e.ToNode==NULL)
      e.ToNode=addNode(alias2,"", NULL);


   if (props!=NULL)
      readFilters(e.Filters, *props);

   Edges.push_back(e);

   if ((e.ToNode==NULL)||(e.FromNode==NULL))
      throw PGraphException("edge #"+to_string(NextEdgeID)+
          " could not be connected");

   NextEdgeID++;
}

//----------------------------------------------------------------------------
void dfs(vector<int> adj[], bool *vis, int x) 
{ 
    vis[x] = true; 
    for (auto i : adj[x])  
    {
        if (!vis[i]) 
            dfs(adj,vis,i); 
    }
}  

//----------------------------------------------------------------------------
void QueryGraph::Validate() 
{
   CompleteTypes();

   if (!IsConnected())
        throw PGraphException("query graph is not connected and cycle free");

   for (auto&& n : Nodes) 
   {
      if (n.TypeName=="") throw PGraphException("missing node type"+
         (n.Alias!=""?" for alias "+n.Alias:""));

      RelationInfo *relinfo = RelRegistry->GetRelationInfo(n.TypeName);

      // mark filters as indexed
      for(auto&& fi:n.Filters)
      {
         if (RelRegistry->IsIndexed(relinfo->Name, fi->Name))
            fi->Indexed=true;
      }

      // get statistics
      if (relinfo!=NULL) {
         n.Cost=relinfo->statistics->cardinality;
      }

   }
   for (auto&& e : Edges) 
   {
      if (e.TypeName=="") throw PGraphException("missing edge type"+
          (e.Alias!=""?" for alias "+e.Alias:""));
   }



}

//----------------------------------------------------------------------------
bool QueryGraph::IsConnected() 
{ 
   // prepare adjacency-lists (double sided!)  
   vector<int> grFw[Nodes.size()], grBw[Nodes.size()]; 
   for (auto&& e : Edges) 
   {
      grFw[e.FromNode->ID].push_back(e.ToNode->ID);
      grFw[e.ToNode->ID].push_back(e.FromNode->ID);
      grBw[e.ToNode->ID].push_back(e.FromNode->ID);
      grBw[e.FromNode->ID].push_back(e.ToNode->ID);
   }

   bool vis1[Nodes.size()], vis2[Nodes.size()]; 
   for (uint ni = 0; ni < Nodes.size(); ni++) 
   { 
      // go forward
      memset(vis1, false, sizeof vis1); 
      dfs( grFw, vis1, ni); 
   
      // go backwards
      memset(vis2, false, sizeof vis2); 
      dfs( grBw, vis2, ni); 
   
      // unconnected?
      for (uint i = 0; i < Nodes.size(); i++) { 
         if (!vis1[i] and !vis2[i]) 
            return false; 
      } 
   }
  
    return true;
} 

//----------------------------------------------------------------------------
QueryTree *QueryGraph::CreateOptimalQueryTree(string forcealias)
{
    //TODO
    if (Nodes.size()==0)
      throw PGraphException("no QueryGraph!");

   QueryTree *bestSoFar=NULL;
   QueryTree *forced=NULL;
   double minCost=0;

   for (auto&& n : Nodes)
   {
      if (n.TypeName=="") continue;
      QueryTree *t = CreateQueryTree(&n);
      
      double cost=t->CalcCost();
      LOGOP(10, "QueryGraph::CreateOptimalQueryTree", " COST FROM ",n.Alias, 
          ":",n.TypeName,"  :", int(cost));

      if (forcealias!="" && (forcealias==n.Alias))
         forced=t;

      if (bestSoFar==NULL)
      {  
         bestSoFar=t;
         minCost=cost;
      }
      else
      {
         if (cost<minCost)  {
            minCost=cost;
            bestSoFar=t;
         }
      }
   }

   if (forced!=NULL)
   {
       LOGOP(10, "QueryGraph::CreateOptimalQueryTree", " forced ", forced->Root
       ->Alias,":",forced->Root->TypeName);
       return forced;
   }

   LOGOP(10, "QueryGraph::CreateOptimalQueryTree", " taking ", bestSoFar->Root
       ->Alias,":",bestSoFar->Root->TypeName);
   
   return bestSoFar;
}

//----------------------------------------------------------------------------
void CreateQueryTree_rec(set<QueryGraphNode*> *visited, QueryGraph *qg, 
    QueryTree *tree, QueryTreeNode *parent, QueryGraphNode *n, bool reverse, 
    QueryGraphEdge *qe)
{
   if (n==NULL) return;
    
   visited->insert(n);

   QueryTreeNode *tn = new QueryTreeNode();
   tn->Alias=n->Alias;
   tn->TypeName=n->TypeName;
   tn->Cost=n->Cost;
 
   // take filters   
   int idx=0;
   for(auto&& f: n->Filters) {
      tn->Filters.push_back(f->Clone() );
      if (f->Indexed) idx++;
   }

   // use cost = 1 for indexed properties
   if (n->Filters.size()==1 && idx==1)
      tn->Cost=1;

   if (tree->Root==NULL)
   {
      tree->Root=tn;
   }
   else
   {
         // add to parent
         QueryTreeEdge *e=new QueryTreeEdge();
         e->FromNode=parent;
         e->ToNode=tn;
         e->Alias=qe->Alias;
         e->TypeName=qe->TypeName;
         e->Reverse=reverse;
         e->Cost=(!reverse)?qe->CostFw:qe->CostBw;
         parent->Edges.push_back(e);
   }

   for(auto&& e : qg->Edges)
   {
      if (e.FromNode==n){
         if (visited->find(e.ToNode)==visited->end())
            CreateQueryTree_rec(visited, qg, tree, tn, e.ToNode, false, &e );
      }
      if (e.ToNode==n) {
         if (visited->find(e.FromNode)==visited->end())
            CreateQueryTree_rec(visited, qg, tree, tn, e.FromNode, true, &e );
      }
   }
}

QueryTree *QueryGraph::CreateQueryTree(QueryGraphNode *n)
{
   //
   set<QueryGraphNode*> visited;

   QueryTree *tree=new QueryTree();

   CreateQueryTree_rec(&visited, this, tree, NULL, n, true, NULL);

   return tree;
}


//----------------------------------------------------------------------------
void QueryGraph::readFilters(list<QueryFilter*> &filters, ListExpr list)
{
      while(!nl->IsEmpty(list))
      {
         QueryFilter *f=new QueryFilter();
         filters.push_back(f);
         f->Name = nl->ToString(nl->First(nl->First(list)));
         f->Value = nl->ToString(nl->Second(nl->First(list)));

         ReplaceStringInPlace(f->Value, "\"","");
         list=nl->Rest(list);
      }
}

//----------------------------------------------------------------------------
void QueryGraph::ReadQueryGraph(string slist)
{
    ListExpr alist=0;
    nl->ReadFromString(slist, alist);
    try
    {
       ReadQueryGraph(alist);
    }
    catch(...)
   {
    nl->Destroy(alist);  
    throw;
   }
    nl->Destroy(alist);

}

//----------------------------------------------------------------------------
void checkOrcompleteType(QueryAliasList *aliaslist, string typename_, 
   QueryGraphNode *n)
{
   if (n->TypeName=="") 
   {
      LOGOP(20,"QueryGraph::CompleteTypes","completed:"+n->Alias+":"+typename_);
      n->TypeName=typename_;
      aliaslist->Update(n->Alias, n->TypeName);
   }
   else
   {
      if (n->TypeName!=typename_)
         throw PGraphException("Type conflict: "+n->TypeName+" != "+typename_);
   }
}

void QueryGraph::CompleteTypes()
 {
    // derive typenames from edge relations, if any
   for(auto&& e:Edges)
   { 
      LOGOP(20,"checking",e.Alias+":"+e.TypeName);  
      RelationInfo *ri = NULL;
      if (e.TypeName!="") ri=RelRegistry->GetRelationInfo(e.TypeName);
      
      for(auto&& n : Nodes)
      {
         if (e.FromNode==&n)
            if (ri!=NULL) 
               checkOrcompleteType(&AliasList, ri->FromName, e.FromNode);

         if (e.ToNode==&n)
            if (ri!=NULL) 
               checkOrcompleteType(&AliasList, ri->ToName, e.ToNode);
      }

      for(auto&& e : Edges)
      {
         if (e.TypeName=="")
         {
            // search if any edge relation is matching
            for(auto&& ri: RelRegistry->RelationInfos )
            {
                 if (ri->roleType==RoleEdge)
                 {
                     if ((e.FromNode->TypeName==ri->FromName) 
                        && (e.ToNode->TypeName==ri->ToName))
                     {
                        e.TypeName=ri->Name;
                        AliasList.Update(e.Alias, e.TypeName);
                     }
                 }
            }
         }
      }
   }
 }

//----------------------------------------------------------------------------
void QueryGraph::ReadQueryGraph(ListExpr alist)
{
    try
    {
        if (nl->ListLength(alist)==0)
            throw PGraphException("At least a list with at least one "
               " node is required");
         
         for(int pass=1; pass<=2;pass++)
         {
            ListExpr list=alist;
            while(!nl->IsEmpty(list))
            {
               // split item
               ListExpr item=nl->First(list);
               list=nl->Rest(list);

               // detect propertylist
               ListExpr proplist;
               bool hasPropList=false;
               int count=nl->ListLength(item);
               proplist=nl->Nth( nl->ListLength(item), item);
               if (nl->ListLength( proplist)>0)
               {
                  hasPropList=true;
                  count--;
               }
               string s1="", s2="", s3="",  s4="";
               if (count>0) s1=nl->ToString(nl->First(item));
               if (count>1) s2=nl->ToString(nl->Second(item));
               if (count>2) s3=nl->ToString(nl->Third(item));
               if (count>3) s4=nl->ToString(nl->Fourth(item));

               if ( (count==1) || (count==2 && FirstUpper(s2)) ) 
               {
                  // node 
                  if (pass==1)
                     addNode( s1, s2, hasPropList?&proplist:NULL);
               }
               else
               {
                  // or edge
                  if (pass==2)
                  {
                     if (count==2) {
                        addEdge( "", "", s1, s2 , hasPropList?&proplist:NULL);
                     }
                     if (count==3) {
                        addEdge( "", s2, s1, s3 , hasPropList?&proplist:NULL);
                     }
                     if (count==4) {
                        addEdge( s2, s3, s1, s4 , hasPropList?&proplist:NULL);
                     }
                  }
               }
            }
         }
    }
    catch(PGraphException &e)
    {
        throw;
    }
}


} // namespace