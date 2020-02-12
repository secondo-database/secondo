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
#include "PGraphQueryProcessor.h"
#include <set>

namespace pgraph {

//----------------------------------------------------------------------------
QueryGraph::QueryGraph(PGraphQueryProcessor *qp)
{
   queryproc=qp;
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
   LOGOP(10,"QueryGraph::DumpGraphDot");
   ostringstream data;
   data << "digraph g {\n";

   for (auto&& n : Nodes)
   {
      data <<"n"<<n._uid<<+"[label=<"+n.Alias;
      if (n.TypeName!="")
         data<<"<BR/>:"<<n.TypeName;
       
      data<<"<BR/> COST: "<<n.Cost<<"<BR/>";   

      list <QueryFilter*> :: iterator itf;         
      for(itf = n.Filters.begin(); itf != n.Filters.end(); ++itf) 
      {
         data << (((*itf)->Index!="")?"(INDEX)":"") << (*itf)->Name <<" = " 
              << (*itf)->Value << "<BR/>\n";
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
      data<<"<BR/>COST: "<<e.CostFw<<"/"<<e.CostBw<<"<BR/>";      
      data<<">]\n";    
    }
   data<<"}\n";

   std::ofstream outfile;
   outfile.open(fn, std::ios_base::trunc);
   outfile << data.str();     
    
    LOGOP(10,"/QueryGraph::DumpGraphDot");
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
   LOGOP(10,"ADDNODE ",alias, (props==NULL?"":" PROPS") );
   QueryGraphNode n;
   n.ID=NextNodeID;
   n.Alias=alias;
   n.TypeName=typename_;

   // will not be available in operator typemapping
   RelationInfo *relinfo = (queryproc==NULL)?NULL:queryproc->pgraphMem
        ->RelRegistry.GetRelationInfo(n.TypeName);
   if (relinfo!=NULL)
      n.Cost=relinfo->statistics->cardinality;

   if (props!=NULL)
      readFilters(n.Filters, *props, relinfo);

   Nodes.push_back(n);

   NextNodeID++;

   return &Nodes.back();
}

//----------------------------------------------------------------------------
void QueryGraph::addEdge(string edgelias, string typename_,string alias, 
    string alias2, ListExpr *props)
{
   LOGOP(10, "ADDEDGE ", alias, " -[",edgelias,":", typename_, "]-> ", 
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

   RelationInfo *relinfo = (queryproc==NULL)?NULL:queryproc->pgraphMem->
      RelRegistry.GetRelationInfo(e.TypeName);
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
      readFilters(e.Filters, *props, relinfo);

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

bool QueryGraph::IsConnectedAndCycleFree() 
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
QueryTree *QueryGraph::CreateOptimalQueryTree()
{
    //TODO
    if (Nodes.size()==0)
      throw PGraphException("no QueryGraph!");

   QueryTree *bestSoFar=NULL;
   double minCost=0;

   for (auto&& n : Nodes)
   {
      if (n.TypeName=="") continue;
      QueryTree *t = CreateQueryTree(&n);

      double cost=t->Root->CalcCost();
      LOGOP(10, "QueryGraph::CreateOptimalQueryTree", " COST FROM ",n.Alias, 
          ":",n.TypeName,"  :", cost);

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
   for(auto&& f: n->Filters)
      tn->Filters.push_back(f->Clone() );

   LOGOP(20,"CreateQueryTree_rec", n->Alias);

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
void QueryGraph::readFilters(list<QueryFilter*> &filters, ListExpr list, 
    RelationInfo *relinfo)
{
      while(!nl->IsEmpty(list))
      {
         QueryFilter *f=new QueryFilter();
         filters.push_back(f);
         f->Name = nl->ToString(nl->First(nl->First(list)));
         f->Value = nl->ToString(nl->Second(nl->First(list)));

         // check if property is indexed
         if (relinfo!=NULL) {
            for(auto&& idx:relinfo->Indexes)
            {
                  if (idx.second->FieldName==f->Name)
                     f->Index=idx.second->IndexName;
            }
         }

         ReplaceStringInPlace(f->Value, "\"","");
         list=nl->Rest(list);
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

               if ( (count==1) || (count==2 && firstUpper(s2)) ) 
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