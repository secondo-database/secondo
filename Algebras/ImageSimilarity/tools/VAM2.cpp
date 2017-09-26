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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Image Similarity Algebra

March 2017 Michael Loris

The Image Similarity Algebra for SECONDO provides operations to store 
pictures in JPEG format into m-trees. It consists a ~Signature~ object 
representing a the signature of a single JPEG image. The object is a 
subtype of the Attribute class. The Image Similarity Algebra consists of 
multiple files, including external files for k-means clustering.


[TOC]

1.2 Defines and includes

*/



#include <algorithm>
#include <set>
#include <queue>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <random>
#include "../JPEGImage.h"
#include <iomanip>
#include <stack>

#include <fstream>

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <exception>

#include <sys/types.h>
#include <dirent.h>

#include <string>
#include <glob.h>



/*
 1.0 Euclidean distance 

*/
 
double euclidDist(FeatureSignatureTuple ist1,
                    FeatureSignatureTuple ist2)
{
    double tmpRes = 0.0;
    
    tmpRes += pow(ist1.weight - ist2.weight, 2);
    tmpRes += pow(ist1.centroid.x - ist2.centroid.x, 2);
    tmpRes += pow(ist1.centroid.y - ist2.centroid.y, 2);
    tmpRes += pow(ist1.centroid.colorValue1 
    - ist2.centroid.colorValue1, 2);
    tmpRes += pow(ist1.centroid.colorValue2
    - ist2.centroid.colorValue2, 2);
    tmpRes += pow(ist1.centroid.colorValue3 
    - ist2.centroid.colorValue3, 2);
    tmpRes += pow(ist1.centroid.coarseness 
    - ist2.centroid.coarseness, 2);
    tmpRes += pow(ist1.centroid.contrast - ist2.centroid.contrast, 2);
    
    return sqrt(tmpRes);
}


/*
 2.0 Type for a cell of the tableau
 
*/

struct Cell
{
    int x;
    int y;
    long val;
};


/*
 3.0 Type for a vertex
 
*/


class Vertex
{
public:
    Vertex() {};
    Vertex(int x, int y, bool visited) : _x(x), _y(y), 
    _visited(visited){}
    int _x;
    int _y;
    bool _visited;
};

bool operator== (const Vertex v1, const Vertex v2)
{
    return ((v1._x == v2._x) && (v1._y == v2._y));
}

bool operator!= (const Vertex v1, const Vertex v2)
{
    return ((v1._x != v2._x) || (v1._y != v2._y));
}

bool operator< (const Vertex v1, const Vertex v2)
{
    return ((v1._x < v2._x) || (v1._y < v2._y));
}

/*
 4.0 Type for an edge
 
*/


class Edge
{
public:
    Edge(Vertex* from, Vertex* to): _from(from), _to(to){} 
    Vertex* _from;
    Vertex* _to;
};


/*
 5.0 Minigraph class, needed in optimization step
 
*/


class MiniGraph
{
public:
~MiniGraph();
void addVertex(int x, int y, bool visited);
void addEdge(Vertex* vp1, Vertex* vp2); 
void getElementaryPath(Vertex* startingVertex);

bool dfs(int v, int p);
bool* visited;
std::vector<std::vector<int>> g; 

std::vector<Edge> edges;
std::vector<Vertex> vertices;
//std::vector<Vertex>path;
std::vector<int> path;

};

MiniGraph::~MiniGraph()
{
    
}

void MiniGraph::addVertex(int x, int y, bool visited)
{
    Vertex v = Vertex(x, y, visited);
    //Vertex* vp = &v;
    this->vertices.push_back(v);
}

void MiniGraph::addEdge(Vertex* vp1, Vertex* vp2)
{
    Edge e = Edge(vp1, vp2);
    //Edge* ep = &e;
    this->edges.push_back(e);
}


bool MiniGraph::dfs(int v, int p)
{
    
    this->visited[v] = true;
    //std::cout << "pp:" << p << std::endl;
    this->path.push_back(p);
    for (int i = 0; i < (int)this->g[v].size(); i++)
    {
       if (!this->visited[this->g[v][i]])
       {
           //std::cout << "p:" << p << std::endl;
           //this->path.push_back(p);
           dfs(this->g[v][i], v);
           return true;
       }
        if (this->g[v][i] != p)
       {
           return true;
       }
    }
    
    this->path.pop_back();
    return false;
}



/*
 6.0 Index improvement entry, same as a cell in a tableau
 
*/

struct IIEntry
{
    double value;
    int x;
    int y;
};


struct MyCoord
    {
        int x;
        int y;
        bool visited;
    };


/*
 7.0 definition of the transport problem
 
*/

class TransportProblem
{
public:
    TransportProblem();
    ~TransportProblem();
    
    
    double distance;
    
    bool visitedSteppingStones;
    bool visitedShadowCosts;
    bool visitedUpdateSolution;
    
    
//    void initialNWCM(std::vector<FeatureSignatureTuple> fst1, 
//        std::vector<FeatureSignatureTuple> fst2);
    
    void initialVAM(std::vector<FeatureSignatureTuple> fst1, 
        std::vector<FeatureSignatureTuple> fst2);
    
    int getRowLowestCost(int row);
    int getColLowestCost(int col);
    void calcRowPenalties();
    void calcColPenalties();
    std::vector<FeatureSignatureTuple> sup;
    std::vector<FeatureSignatureTuple> dem;
    
    int maxRowPenIdx();
    int maxColPenIdx();
   
    
    int rowPenIdx;
    int rowLcIdx;
    int colPenIdx;
    int colLcIdx;
    
    
   
    bool* rowSat;
    bool* colSat; 
    
    //bool* satRow;
    //bool* satCol; 
    int* rowPen;
    int* colPen;   
   // void assignGoods();
    
    
    long** distanceMatrix; // costs / distances
    long** amountMatrix; // how many goods/dirt got transported
    bool** basicsMatrix; // which fields are occupied or NULL
    
    double transport(std::vector<FeatureSignatureTuple> fst1, 
    std::vector<FeatureSignatureTuple> fst2);
    
    std::vector<Cell>basics; 
    // basic variables (cells used in solution)
  //  std::vector<Cell>nonBasics;
   
    std::vector<Cell>basics2; 
    // basic variables (cells used in solution)
    
   // std::vector<std::vector<Cell>>pathes; // elementary pathes
    
    //std::vector<double> uCosts; // shadow costs
    //std::vector<double> vCosts; // shadow costs (costs = u + v)
    bool basicsError;
    
    int* ucost;
    int* vcost;
    
    int supSize; // nodes in supply (size of signature 1)
    int demSize; // nodes in demand (size of signature 2)
    
    void calcShadowCosts();
    
    Cell enteringCell;
    
    std::vector<IIEntry>improvementIndexes;
    static bool compareIIEntries(const IIEntry& i1, const IIEntry& i2); 
    //todo: IIEntry should be an inner class
    
    MiniGraph* mg; // graph required for finding stepping stones
    
  //  bool** adjMatrix;
    void findSteppingStones(); // takes coordidates of entering cell
    
     
    void bfs(MyCoord start, MyCoord destination);
    std::vector<MyCoord>path;

    int* parents;
    int* colors;
    int** adjList;
    bool* visited;
    int* path2;
    
    void getPathes(int s, int d);
    void getPathesUtil(int u, int d, int pathIdx, 
    std::vector<std::vector<int>> g);
    
    void dfs(int v);
    //void dfsRun(int v, bool visited[],
    //std::vector<std::vector<int>> g);
    void dfsRun(int v, bool visited[], std::vector<std::vector<int>> g, 
    std::stack<int>* vis);

    bool visitVertex(int v, int parent, 
    std::vector<std::vector<int>> g);    
    void printCycle(int v, int u);
    //void triggerChainReaction(Vertex start, MiniGraph* g);
    //void runPath(Cell c, Cell dest, bool horizontal, int x, int y, 
    //std::vector<Cell>* path);    
    //void scanRow(int x, int y, Cell dest);
    //void scanCol(int x, int y, Cell dest);
    
    std::stack<int> st;
    int* vis;
    void depth_first_search(int u, std::vector<std::vector<int>> g);
    void 
    dfs_visit(int u, int dest, int time, 
    std::vector<std::vector<int>> g);
    
    std::vector<Cell>loop;
    void findSimpleLoop(int x, int y);
    
    void updateSolution();
    
    double currentDistance;
    double newDistance;
    
};


TransportProblem::TransportProblem() : visitedSteppingStones(false), 
    visitedShadowCosts(false), visitedUpdateSolution(false)
{
    this->mg = new MiniGraph();
}


// this method will only detect rectangular loops
// closed loops can be way more complex and switching
// multiple times between x and y axis.

void TransportProblem::findSimpleLoop(int x, int y)
{
    
    std::vector<Cell> xBasics;
    for (int j = 0; j < (int) this->supSize; j++)
    {
        for (int i = 0; i < (int) this->demSize; i++)
        {    
            if (this->basicsMatrix[j][i])
            {    
                Cell yb = {i, j, this->distanceMatrix[j][x]};
                basics.push_back(yb);
            }
        }
    }



    
    std::vector<Cell>xEntries;
    for (int i = 0; i < (int)this->demSize; i++)
    {
        if (this->basicsMatrix[y][i])
        {
            Cell xb = {i, y, this->distanceMatrix[i][x]};
            xEntries.push_back(xb);
        }
    }
    
    
    std::vector<Cell>yEntries;
    for (int j = 0; j < (int)this->supSize; j++)
    {
        if (this->basicsMatrix[j][x])

        {            
            Cell xb = {x, j, this->distanceMatrix[j][x]};
            yEntries.push_back(xb);
        }
    }
    
    Cell candidate;
    Cell yEntry;
    Cell xEntry;
    for (auto c : xEntries)
    {
        for (auto b : basics)
        {
            if ((c.x == b.x) && (c.y != b.y))
            {
                // for each match check if there's a yEntry
                for (auto ye : yEntries)
                {
                    if (b.y == ye.y)
                    {
                        candidate = {b.x, b.y, 
                            this->distanceMatrix[b.y][b.x]};
                        yEntry = {ye.x, ye.y, 
                            this->distanceMatrix[ye.y][ye.x]};
                        xEntry = {c.x, c.y, 
                            this->distanceMatrix[c.y][c.x]};
                        break;
                    }
                }
            }
        }
    }
    
    this->loop.push_back({x, y, this->distanceMatrix[y][x]});    
    this->loop.push_back({xEntry.x, xEntry.y, xEntry.val});
    this->loop.push_back({candidate.x, candidate.y, candidate.val});
    
    
    this->loop.push_back({yEntry.x, yEntry.y, yEntry.val});
    
    
    // find all basics in same row
    //std::vector<Cell> xBasics;
    for (int i = 0; i < (int) this->demSize; i++)
    {
        if (this->basicsMatrix[y][i])
        {
            Cell xb = {i, y, this->distanceMatrix[y][i]};
            xBasics.push_back(xb);            
        }
    }
    // find all basics in same column
    std::vector<Cell> yBasics;
    for (int j = 0; j < (int) this->supSize; j++)
    {
        if (this->basicsMatrix[j][x])
        {
            Cell yb = {x, j, this->distanceMatrix[j][x]};
            yBasics.push_back(yb);
                
        }
    }
    
     
    std::vector<Cell> candidates;
    for (auto c : xBasics)
    {
        //std::cout << "checking xBasics: x:" << c.x << ", " 
        //<< c.y << std::endl;
        for (int j = 0; j < (int)this->supSize;j++)
        {            
            if (this->basicsMatrix[j][c.x]
            && (c.y != j)
            )             
            {
                Cell cc = {c.x, j, 0};
                candidates.push_back(cc);
            }
        }
    //    std::cout << std::endl;    
    }
    
    
    std::vector<Cell> solution;
    for (auto c : candidates)
    {
        for (int i = 0; i < (int)this->demSize; i++)
        {
            if (this->basicsMatrix[c.y][i] &&
            (c.x != i)
            )     
            {
                Cell s = {i,c.y, 0};
                solution.push_back(s);
            }
        }
    }
    
}

/*
 7.0 depth first search in a
 
*/

void TransportProblem::depth_first_search(int u, 
std::vector<std::vector<int>> g)
{
    this->vis = new int[this->mg->vertices.size()]{0}; // white
   // this->parents = new int[this->mg->vertices.size()]{-1};
    std::cout << "smoke 1" << std::endl;
    this->dfs_visit(u, u, 1, g);
    
}

void TransportProblem::dfs_visit(int u, int dest, int time, 
std::vector<std::vector<int>> g)
{
}



void TransportProblem::dfsRun(int v, bool visited[], 
std::vector<std::vector<int>> g, std::stack<int>* vis)
{
    
    vis->push(v);
    //std::cout << "visiting:" << v << std::endl;
    if (visited[v])
    {
        std::cout << "found a cycle" << std::endl;
        for (int i = 0; i < (int)this->mg->vertices.size(); i++)
        {
            std::cout << vis->top() << ",";
            vis->pop();
        }
        return;
    }
    else
    {
        visited[v] = true;
    
        for (int i = 0; i < (int)g[v].size(); i++)
        {
           // std::cout << "i:" << i << std::endl;
         
            if (!visited[g[v][i]])
            {
                dfsRun(g[v][i], visited, g, vis);
                visited[v] = false;
                vis->pop();
            }
            
            
        }
    }
}

void TransportProblem::dfs(int s)
{
    //this->visited = new bool[this->mg->vertices.size()];
    std::vector<std::vector<int>> g(this->mg->vertices.size());
    
    std::vector<int> p;
    
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        for (int j = 0; j < (int)this->mg->vertices.size(); j++)
        {
            if ((this->mg->vertices[i]._x == this->mg->vertices[j]._x) 
            ||(this->mg->vertices[i]._y == this->mg->vertices[j]._y)) {
                if (j != i)
                {
                    g[i].push_back(j);
                }
            
            }
        }
    }
    
    std::cout << "displaying list before:" << std::endl;
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        std::cout << i << ":";
        for (int j = 0; j < (int)g.at(i).size(); j++)
        {
            std::cout << g[i][j] << ",";
        }
        std::cout << std::endl;
    }
    
    std::cout << "displaying vertices" << std::endl;
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        std::cout 
        << "vert " << i << " :"
        << this->mg->vertices[i]._y
        << ","
        << this->mg->vertices[i]._x
        << std::endl;
        
    }
    
    
    this->visited = new bool[this->mg->vertices.size()]{false};
    //int* parent = new int[this->mg->vertices.size()]{false};
    
   // depth_first_search(s,g);
   
   
   
     
    std::cout << "end of dfs" << std::endl;
    
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        std::cout << this->parents[i] << ", ";
    }
    
}



/*
 8.0 Destructor of the Transportation problem
 
*/


TransportProblem::~TransportProblem()
{
    
   for (int i = 0; i < this->supSize; i++)
   {
       delete[] this->amountMatrix[i];
   }
   delete[] this->amountMatrix;
  
    delete [] rowSat;
    delete [] colSat; 
    
    
    delete [] rowPen;
    delete [] colPen;   
   
   
    for (int i = 0; i < this->supSize; i++)
    {
        delete[] this->distanceMatrix[i];
    }
    delete [] this->distanceMatrix;
    
    
    
    delete this->mg;
    
    
    if (this->visitedShadowCosts)
    {
        delete [] ucost;
        delete [] vcost;
    }
    
    
   
    for (int i = 0; i < this->supSize; i++)
    {
        delete[] basicsMatrix[i];
    }
    
    delete [] basicsMatrix;
            
}

bool TransportProblem::compareIIEntries(
    const IIEntry& i1, const IIEntry& i2)
{
    return i1.value < i2.value;
}

void TransportProblem::getPathes(int s, int d)
{
    std::vector<std::vector<int>> g(this->mg->vertices.size());
    
    
    this->parents = new int[this->mg->vertices.size()]{};    
    this->colors = new int[this->mg->vertices.size()]{0}; // white
    
    
    // build adjacency list    
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        for (int j = 0; j < (int)this->mg->vertices.size(); j++)
        {
            if (
            (this->mg->vertices[i]._x == this->mg->vertices[j]._x) ||
            (this->mg->vertices[i]._y == this->mg->vertices[j]._y)) {
                if (j != i)
                {
                        g[i].push_back(j);
                }
            }
        }
    }
    // now I cut all vertical edges from the entering cell, so that
    // no cycle exists
    // the destination points will be the basic cells which are
    // on the same column
    Vertex sVert = this->mg->vertices[s];
//    Vertex dVert = this->mg->vertices[d];
    
    for (int i = 0; i < (int)g[s].size(); i++)
    {
        for (int j = 0; j < (int)g[s].size(); j++)
        {
            if (this->mg->vertices[j]._y == sVert._y)
            {
                g[s].erase(g[s].begin() + j);
            }
        }
    }
    std::cout << std::endl;
    
    
//    std::cout << "dfs displaying list:" << std::endl;
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        std::cout << i << ":";
        for (int j = 0; j < (int)g.at(i).size(); j++)
        {
            std::cout << g[i][j] << ",";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    
    this->visited = new bool[this->mg->vertices.size()]{false};
    
    this->path2 = new int[this->mg->vertices.size()]{};
    
    int pathIdx = 0;
    
    this->getPathesUtil(s, d, pathIdx, g);
    
}

void TransportProblem::getPathesUtil(int u, int d, int pathIdx, 
std::vector<std::vector<int>> g)
{
    this->visited[u] = true;
    this->path2[pathIdx] = u;
    pathIdx++;
    
    if (u == d)
    {        
        for (int i = 0; i < pathIdx; i++)
        {
            std::cout << this->path2[i] << " ";
        }
        std::cout << std::endl;
    }
    else
    {
        for (int i = 0; i < (int)g[u].size(); i++)
        {
            if (!this->visited[i])
            {
                getPathesUtil(i, d, pathIdx, g);
            }
        }
    }
    pathIdx--;
    this->visited[u] = false;
}





void TransportProblem::printCycle(int v, int u)
{
    std::cout << "print cycle: u:" << u << " v:" << v << std::endl;
    do
    {
        std::cout << u << ",";
        u = this->parents[u];
    }
    while (u != v);
}

void TransportProblem::bfs(MyCoord start, MyCoord destination)
{

    std::vector<std::vector<int>> g(this->mg->vertices.size());
    int n; // number of vertices
    int s; // start point
    int to;
    
    
    n =  (int)this->mg->vertices.size();
    
    //std::cout << "displaying s" << std::endl;
    
    // find index of s
    s = 0;
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        if ((this->mg->vertices[i]._x == start.x) &&
        (this->mg->vertices[i]._y == start.y))
        {
            s = i;
            break;
        }
        
    }
    
    //std::cout << "displaying to" << std::endl;
    // find index of to
    to = 0;
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        if ((this->mg->vertices[i]._x == destination.x) &&
        (this->mg->vertices[i]._y == destination.y))
        {
            to = i;
            break;
        }
        
    }
    
    
    //std::cout << "building list" << std::endl;
    // create adjacency list 
    // the connection between must be broken
    
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        for (int j = 0; j < (int)this->mg->vertices.size(); j++)
        {
            if (
            (this->mg->vertices[i]._x == this->mg->vertices[j]._x) ||
            (this->mg->vertices[i]._y == this->mg->vertices[j]._y))    {
                if (j != i)
                {
                    if (!((i == s) && (j == to)))
                    {
                        g[i].push_back(j);
                    }
                }
            
            }
        }
    }
    
    
    std::cout << "displaying list" << std::endl;
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        std::cout << i << ":";
        for (int j = 0; j < (int)g.at(i).size(); j++)
        {
            std::cout << g[i][j] << ",";
        }
        std::cout << std::endl;
    }
    
    std::cout << "destination y:" << destination.y << " x:" 
    << destination.x << std::endl;
    
    std::cout << "displaying vertices" << std::endl;
    for (int i = 0; i < (int)this->mg->vertices.size(); i++)
    {
        std::cout 
        << "vert " << i << " :"
        << this->mg->vertices[i]._y
        << ","
        << this->mg->vertices[i]._x
        << std::endl;
        
    }
    
        
    
        
    
    std::queue<int> q;
    q.push(s);
    
    std::vector<bool> used(n);
    std::vector<int> d(n);
    std::vector<int> p(n);
    used[s] = true;
    p[s] = -1; // parent

    //std::cout << "starting queue" << std::endl;
    
    while (!q.empty())
    {
        int v = q.front();
        q.pop();
        
        for (int i = 0; i < (int)g[v].size(); ++i)
        {
            int to = g[v][i];
            if (!used[to])
            {
                used[to] = true;
                q.push(to);
                d[to] = d[v] + 1;
                p[to] = v;
            }
        }
    }
    
    
    //std::cout << "displaying path" << std::endl;

    if (!used[to])
    {
        // no path
        //std::cout << "no path" << std::endl; 
        ;       
    }
    else
    {
        std::vector<int> path;
        for (int v = to; v != -1; v = p[v])
        {
            path.push_back(v);
            int xv = this->mg->vertices[v]._x;
            int yv = this->mg->vertices[v]._y;
            MyCoord c = {xv, yv};
            this->path.push_back(c);
        }
        reverse(path.begin(), path.end());
        reverse(this->path.begin(), this->path.end());
        
        /*
        std::cout << "path:" << std::endl;
        for (int i = 0; i < (int)path.size(); i++)
        {
            std::cout << path[i] << ", ";
        }
        */
    }
    
    
    
}



/*
 9.0 Method to find stepping stones in the tableau
 
*/


void TransportProblem::findSteppingStones()
{

    
    
   
   this->mg->vertices.erase(this->mg->vertices.begin(), 
   this->mg->vertices.end());
   
   int cntVert = 0;
  // std::cout << "adding vertices" << std::endl;
    for (int y = 0; y < this->supSize; y++)
    {
        for (int x = 0; x < this->demSize; x++)
        {
            if (this->basicsMatrix[y][x]) 
            {
                this->mg->addVertex(x, y, false);
                cntVert++;
            }
        }
    }
    
   this->mg->addVertex(this->enteringCell.x,
   this->enteringCell.y,this->enteringCell.val);
   
    for (int y = 0; y < this->supSize; y++)
    {
        for (int x = 0; x < this->demSize; x++)
        {
            if (this->basicsMatrix[y][x]) 
            {
                this->mg->addVertex(x, y, false);
                cntVert++;
            }
        }
    }
   this->mg->addVertex(this->enteringCell.x,
   this->enteringCell.y,this->enteringCell.val);
   
   mg->g.resize(mg->vertices.size());
   
 
   
  // std::cout << "adding edges" << std::endl;

    int half = (int)this->mg->vertices.size() / 2;
    
    //std::cout << "half:" << half << std::endl;
    
    for (int i = 0; i < half ; i++)
    {
        for (int j = 0; j < half; j++)
        {
            if (j != i)
            {
                if (this->mg->vertices[i]._x 
                == this->mg->vertices[j]._x)
                {
                    this->mg->g[i].push_back(j+half);
                }
            }
        }
    }
    
   
   //std::cout << "adding second batch of edges" << std::endl;
   for (int i = 0; i < half ; i++)
    {
        for (int j = 0; j < half; j++)
        {
            if (j != i)
            {
                if (this->mg->vertices[i]._y 
                == this->mg->vertices[j]._y)
                {
                    this->mg->g[i+half].push_back(j);
                }
            }
        }
    }
    
  
    mg->visited = new bool[mg->vertices.size()]{false};
    //mg->g.resize(mg->vertices.size());
    
    int pos = this->mg->vertices.size() -1;    
    
    if (!this->mg->dfs(pos, -1))
    {
        this->basicsError = true;
    }
    
  
    return;
}


/*
 10.0 Method to update a non-optimal solution
 
*/


void TransportProblem::updateSolution()
{
    
      std::vector<int>tmpPath;
    for (int i = 1; i < this->mg->path.size() / 2; i++)
    {
        tmpPath.push_back(this->mg->path[i]);
    }
    
    
   // for (auto l : tmpPath)
//    {
    //    std::cout << " tmp loop:" << this->mg->vertices[l]._x << "," 
    //<< this->mg->vertices[l]._y << std::endl;
    //}
    //std::cout << "tmp loop size:" << tmpPath.size() << std::endl;
    
    
    int minBasicVal = std::numeric_limits<int>::max();
    int idxMinVal = 0;
    
    for (int i = 0; i < this->mg->vertices.size(); i += 2)
    {
        Vertex min = this->mg->vertices[i];
        int minVal = this->amountMatrix[min._y][min._x];
        
        if (minVal < minBasicVal)
        {
            //std::cout << "minVal:" << minVal << std::endl;
            minBasicVal = minVal;
            idxMinVal = i;
        }
    }
    
    
    Vertex minBasicCell = this->mg->vertices[idxMinVal];    
        
    double theta = this->amountMatrix[minBasicCell._y][minBasicCell._x];
    
    //std::cout << "theta:" << theta << std::endl;
    
    

    int x;
    int y;
    int ii = 0;
    
    
    for (auto p : tmpPath)
    {
        //std::cout << "reducing theta" << std::endl;
        x = this->mg->vertices[p]._x;
        y = this->mg->vertices[p]._y;
        if ((ii % 2) == 0)
        {    
            //std::cout << "adding theta" << std::endl;    
            this->amountMatrix[y][x] += theta;        
        }
        else
        {            
           // std::cout << "removing theta" << std::endl;
            this->amountMatrix[y][x] -= theta;        
        }
        ii++;
    }
    
    
    int dist = 0;
    for ( int y = 0; y < (int)this->supSize; y++) // rows
    {
        for ( int x = 0; x < (int)this->demSize; x++) // columns
        {
            //std::cout << std::setw(8) << this->amountMatrix[x][y] 
            //<< "*" << 
            //this->distanceMatrix[x][y] << " = " <<
            //(this->amountMatrix[x][y] * this->distanceMatrix[x][y])
            //<< "|";
            dist += (this->amountMatrix[y][x] 
            * this->distanceMatrix[y][x]);
        } 
        //std::cout << "| " << std::setw(8) << dem.at(y).weight << "|";      
        //std::cout << std::endl;
    }
    
    //std::cout << "new dist:" << this->newDistance << std::endl;
    this->newDistance = dist;
    
}

    
/*
 11.0 Method to calculate the opportunity costs, first step
 of the u-v method
 
*/


void TransportProblem::calcShadowCosts()
{
    this->ucost = new int[this->supSize]{0};
    this->vcost = new int[this->demSize]{0};
    this->visitedShadowCosts = true;
    
    
    
    ucost[0] = 0;
    
    // for each cell calculate: c_ij = ucost[x] + vcost[y]
    // the first row equals the distance/costs
    
    // for every first cell in a row -> calculate ucost[y]
    // for all subsequent cells in a row -> calculate vcost[x]
    
    for ( int x = 0; x < (int)this->demSize; x++)
    { 
        if (this->basicsMatrix[0][x])
        {            
            vcost[x] = this->distanceMatrix[0][x];
        }
    }
   
    // the first cell in a row determines the u value of this row
    // all following cells determine v values
    for (int y = 1; y < (int)this->supSize; y++)
    {
        bool firstCell = true;
        for (int x = 0; x < (int)this->demSize; x++)
        { 
            if (this->basicsMatrix[y][x])
            {
                if (firstCell)
                {
                    ucost[y] = this->distanceMatrix[y][x] - vcost[x];
                    firstCell = false;
                }
                else
                {
                    vcost[x] = this->distanceMatrix[y][x] - ucost[y];
                }
            }
        } 
    }
    
    double** oppCostMatrix = new double*[this->supSize];
    for ( int y = 0; y < (int)this->supSize; y++)
        oppCostMatrix[y] = new double[this->demSize];
   
    
    // the entering cell is the non-basic cell with the lowest value
    Cell enteringCell;
    int minOppCost = std::numeric_limits<int>::max();
    //std::cout << std::endl;
    for (int y = 0; y < (int)this->supSize; y++)
    {
        for (int x = 0; x < (int)this->demSize; x++)
        { 
            oppCostMatrix[y][x] = this->distanceMatrix[y][x] 
            - (ucost[y] + vcost[x]);
            if (!this->basicsMatrix[y][x])
            {
                int cost = this->distanceMatrix[y][x] 
                - (ucost[y] + vcost[x]);
                //std::cout << cost << "|";
                if (cost < minOppCost)
                {
                    minOppCost = cost;
                    enteringCell = {x, y, cost};
                }
            }
        } 
        //std::cout << std::endl;
    }
    //std::cout << std::endl;

    // 
   // std::cout << "enteringCell.x:" << enteringCell.x 
   //<< ", " << enteringCell.y << " " 
   //<< enteringCell.val << std::endl;
    this->enteringCell = enteringCell;
    
    
    
    
    for ( int y = 0; y < (int)this->supSize; y++)
    {
        delete [] oppCostMatrix[y];
    }
    
    delete [] oppCostMatrix;
    
    
}


/*
 12.0 Method to calculate colum penalties, part of Vogel's approximation
 method
 
*/


void TransportProblem::calcColPenalties()
{
    
        //std::cout << "calc col penalties:" << std::endl; ;   
     
       for ( int x =  0; x < (int)this->demSize; x++) 
       {    
          int min = std::numeric_limits<int>::max();
           int min2 = std::numeric_limits<int>::max();
           for ( int y = 0; y < (int)this->supSize; y++) // vertical
           {
             bool satisfied = rowSat[y];
               
               if (!satisfied)
               {        
                   if (this->distanceMatrix[y][x] < min2)
                    {
                        if (this->distanceMatrix[y][x] < min)
                        {
                            min2 = min;
                            min = this->distanceMatrix[y][x];
                        }
                        else
                        {
                            min2 = this->distanceMatrix[y][x];
                        }
                    }
                }            
           }
           if (min2 != std::numeric_limits<int>::max())
           {
               int tmpPen = min2 - min;
                this->colPen[x] = tmpPen;
            }
            else
            {            
                this->colPen[x] = min;
            }
        
        //   delete [] tmpv;          
       }
       // std::cout << std::endl;
              
       int max = std::numeric_limits<int>::min();
     //  int idx = 0;
      // std::cout << "col penalties:";
       for ( int i = 0; i < (int)this->demSize; i++)
       {
          // std::cout << this->colPen[i] << ",";
           if (max < this->colPen[i])
           {
               max = this->colPen[i];
        //       idx = i;
           }
            
       }
       
//std::cout << std::endl;
          
     
    //   std:: cout << "found max col:" << max << " at:" 
    //<< idx << std::endl;
     //  return idx;
       
}

/*
 13.0 Method to calculate row penalties, part of Vogel's approximation
 method
 
*/


void TransportProblem::calcRowPenalties()
{
    
    // std::cout << "calc row penalties:" << std::endl;;   
     
       for ( int y = 0; y < (int)this->supSize; y++) // horizontal
        {
            int min = std::numeric_limits<int>::max();
            int min2 = std::numeric_limits<int>::max();
           
           for ( int x =  0; x < (int)this->demSize; x++) 
           {
               bool satisfied = colSat[x];

               if (!satisfied)
               {                
                    if (this->distanceMatrix[y][x] < min2)
                    {
                        if (this->distanceMatrix[y][x] < min)
                        {
                            min2 = min;
                            min = this->distanceMatrix[y][x];
                        }
                        else
                        {
                            min2 = this->distanceMatrix[y][x];
                        }
                    }
                }          
           }
           if (min2 != std::numeric_limits<int>::max())
           { 
                int tmpPen = min2 - min;
                this->rowPen[y] = tmpPen;
            }
            else
            {
                this->rowPen[y] = min;
            }
            
       }
       
       int max = std::numeric_limits<int>::min();
      // int idx = 0;
     //  std::cout << "row penalties:";
       for ( int i = 0; i < (int)this->supSize; i++)
       {
           //std::cout << this->rowPen[i] << ",";
           if (max < this->rowPen[i])
           {
               max = this->rowPen[i];
        //       idx = i;
           }
       }
       //std::cout << std::endl;
       
     //  std:: cout << "found max row:" << max << " at:" << idx 
     //<< std::endl;
     //  return idx;
           
}


/*
 14.0 Method to calculate the lowest cost per row
 
*/


int TransportProblem::getColLowestCost(int row)
{
    int min = std::numeric_limits<int>::max();         
    int idx;
    //std::cout << "getColLowestCost in row:" << row << std::endl;
    for (int x = 0; x < (int)this->demSize; x++)
    {
        //std::cout << "x:" << x << std::endl;
        if (this->colSat[x])
        {
            continue;
        }
       // std::cout << "x2:" << x << std::endl;
       // std::cout << this->distanceMatrix[row][x] << ",";
            
       // std::cout << " min:" << min << " ";
        if (this->distanceMatrix[row][x] < min)
        {
                min = distanceMatrix[row][x];
                idx = x;
        }
    }
//    std::cout << "min cost col idx:" << idx << " val:" 
//<< distanceMatrix[row][idx] << std::endl;
    return idx;
}


/*
 14.0 Method to calculate the lowest cost per column
 
*/

int TransportProblem::getRowLowestCost(int col)
{
    //std::cout << std::endl;
    int min = std::numeric_limits<int>::max();         
    int idx;
    //std::cout << "getRowLowestCost in col:" << col << std::endl;
    for ( int y = 0; y < (int)this->supSize; y++)
    {
        if (this->rowSat[y])
        {
            continue;
        }
        //std::cout << this->distanceMatrix[y][col] << ",";
        
            
        if (this->distanceMatrix[y][col] < min)
        {
                min = distanceMatrix[y][col];
                idx = y;
            
        }
    }
//    std::cout << "min cost row idx:" << idx << " val:"
// << distanceMatrix[idx][col] << std::endl;
    return idx;
}

int TransportProblem::maxRowPenIdx()
{
    int idx = 0;
    int maxVal = 0;
    for ( int i = 0; i < (int)this->supSize; i++)
    {
        if (!this->rowSat[i])
        {
            if (maxVal < this->rowPen[i])
            {
                maxVal = this->rowPen[i];
                idx = i;
            }
        }
    }
    return idx;
}

int TransportProblem::maxColPenIdx()
{
    int idx = 0;
    int maxVal = 0;
    for ( int i = 0; i < (int)this->demSize; i++)
    {
     //   std::cout << "colPenalty:" << this->colPen[i] << ",";
        //std::cout << std::endl;
        if (!this->colSat[i])
        {
            if (maxVal < this->colPen[i])
            {
                maxVal = this->colPen[i];
                idx = i;
            }
        }
    }
    //std::cout << std::endl;
    //std::cout << "maxColPenIdx:" << idx << std::endl;
    return idx;
}


/*
 16.0 Method to create an inital feasible solution using Vogel's
 approximation method
 
*/


void TransportProblem::initialVAM(
std::vector<FeatureSignatureTuple> fst1, 
    std::vector<FeatureSignatureTuple> fst2)
{
    sup = fst1;
    dem = fst2;   
    
   
    
    this->sup = sup;
    this->dem = dem;
    
    
   
   
    
    long sumSup = 0.0;
    long sumDem = 0.0;
    
    for (auto fst : sup)
        sumSup += fst.weight;
        
    for (auto fst : dem)
        sumDem += fst.weight;
        
    
    
    //std::cout << "supWeights:" ;
    std::vector<long>supWeights;
    for (int i = 0; i < (int)sup.size(); i++)
    {
    //    std::cout << (long)round(sup[i].weight * 10000.0)   
    //<< std::endl;
        supWeights.push_back((long)round(sup[i].weight * 10000.0) );
    }
        
    std::vector<long>demWeights;
     for (int i = 0; i < (int)dem.size(); i++)
    {
        demWeights.push_back((long)round(dem[i].weight * 10000.0) );
    }
    
       for (auto fst : supWeights)
        sumSup += fst;
        
    for (auto fst : demWeights)
        sumDem += fst;
   
    //std::cout << "sumSup: before:" << sumSup << std::endl;
    //std::cout << "sumDem before :" << sumDem << std::endl;
  
    
    
    if (sumSup > sumDem)
    {
        // add an artifical node to demand holding the difference
        // the cost will be zero
        FeatureSignatureTuple fst;
   //      std::cout << "sumSup:" << sumSup << std::endl;
   // std::cout << "sumDem:" << sumDem << std::endl;
   
        long dummyWeight = sumSup - sumDem;
       // std::cout << "dummy weight, sup:" << dummyWeight << std::endl;
        fst = {0.0, 0,0,0.0,0.0,0.0,0.0,0.0};
        dem.push_back(fst);
        demWeights.push_back(dummyWeight);
    }
    else if (sumSup < sumDem)
    {
        // add an artifical node to demand holding the difference
        // the cost will be zero
        FeatureSignatureTuple fst;
        long dummyWeight = sumDem - sumSup;
       // std::cout << "dummy weight, dem:" << dummyWeight << std::endl;        
        fst = {0.0, 0,0,0.0,0.0,0.0,0.0,0.0};
        sup.push_back(fst);
        supWeights.push_back(dummyWeight);
    }
    
    this->supSize = sup.size();
    this->demSize = dem.size();
    
  // std::cout << "demSize:" << this->demSize << std::endl;
  //  std::cout << "supSize 2:" << this->supSize << std::endl;
    
    sumSup = 0.0;
    sumDem = 0.0;
  
  
     for (auto fst : supWeights)
        sumSup += fst;
        
    for (auto fst : demWeights)
        sumDem += fst;
   
  //  std::cout << "sumSup:" << sumSup << std::endl;
  //  std::cout << "sumDem:" << sumDem << std::endl;
   
    
    
    // 2 a. init distance matrix    
    this->distanceMatrix = new long*[this->supSize];
    for ( int y = 0; y < (int)this->supSize; y++)
        this->distanceMatrix[y] = new long[this->demSize];
    
   // std::cout << "fog 2" << std::endl;
        
    
    // 2. fill distance matrix, 
    //the artificial node has the distance of zero
    for ( int y = 0; y < (int)this->supSize; y++) // rows
    {
        for ( int x = 0; x < (int)this->demSize; x++) // columns
        {
           // std::cout << "i:" << i << std::endl;
            this->distanceMatrix[y][x] 
            = (long)round(euclidDist(
            sup.at(y), dem.at(x)) * 1000.0) / 1000.0;          
        }               
    }
    
    
    
    // 2a. init basics matrix
    // this table will keep track of occupied cells (basic variables)
    this->basicsMatrix = new bool*[this->supSize];
    for ( int y = 0; y < (int)this->supSize; y++)
        this->basicsMatrix[y] = new bool[this->demSize]{false};
    
    //std::cout << "fog 6" << std::endl;
    
    
    // 3. init transport matrix, stores how much is transported
    this->amountMatrix = new long*[this->supSize];
    for ( int y = 0; y < (int)this->supSize; y++)
        this->amountMatrix[y] = new long[this->demSize]{};
        
        
   // 4. setup arrays to store penalties
   this->rowPen = new int[this->supSize]{0};
   this->colPen = new int[this->demSize]{0};
   
   
   // 5. setup array for column / row satisfied
   this->rowSat = new bool[this->supSize]{false};
   this->colSat = new bool[this->demSize]{false};
   
 //  std::cout << "fog 7" << std::endl;
    
    
    calcRowPenalties(); // vertical penalties
    int rowPenIdx;      
    rowPenIdx = maxRowPenIdx(); // idx of the biggest penalty
    int rowMinIdx = getColLowestCost(rowPenIdx); 
    
     
    calcColPenalties();    
    int colPenIdx;
     
    colPenIdx = maxColPenIdx();
    
     
    int colMinIdx = getRowLowestCost(colPenIdx);
   
        
   
    const int MAX_ITERATIONS = 200;

    for (int iter = 0; iter < MAX_ITERATIONS; iter++)
    {
    //std::cout << "--------------------- start ------------------- " 
    //<< iter  << std::endl;
     
        
        bool allCols = true;
        for (int i = 0; i < (int)this->demSize; i++)
        {
            if (!this->colSat[i])
            {
                allCols = false;
            }
        }
        
       
        bool allRows = true;
        for (int i = 0; i < (int)this->supSize; i++)
        {
            if (!this->rowSat[i])
            {
                allRows = false;
            }
        }
        
        if (allCols && allRows)
        {        
            //std::cout << "breaking good" << std::endl;
            break;
        }
        
        colPenIdx = maxColPenIdx();
        
        rowPenIdx = maxRowPenIdx();
        
        colMinIdx = getRowLowestCost(colPenIdx);
        
        rowMinIdx = getColLowestCost(rowPenIdx);
    
       
          if (this->rowPen[rowPenIdx] >= this->colPen[colPenIdx])
        {
            //std::cout << "max row is larger, ";            
            // set either col or row to satisfied, whichever is 0
            //std::cout << "sup[rowPenIdx]:" << supWeights[rowPenIdx]
            //<< " dem[rowMinIdx]:" << demWeights[rowMinIdx]
           // << std::endl;
            //std::cout << "rowPenIdx:" << rowPenIdx << std::endl;
            //std::cout << "rowMinIdx:" << rowMinIdx << std::endl;
            
            
            
            if (supWeights[rowPenIdx] > demWeights[rowMinIdx])
            {
                this->colSat[rowMinIdx] = true;
                //std::cout << "f1:" << std::endl;
                this->amountMatrix[rowPenIdx][rowMinIdx] 
                = demWeights[rowMinIdx];
                //std::cout << "f2:" << std::endl;
                supWeights[rowPenIdx] -= demWeights[rowMinIdx];
                //std::cout << "f3:" << std::endl;
                demWeights[rowMinIdx] = 0;
                // calcColPenalties(); // change
                calcRowPenalties();
                //rowPenIdx = maxRowPenIdx();
            }
            else if (supWeights[rowPenIdx] < demWeights[rowMinIdx])
            { // -- done
                this->rowSat[rowPenIdx] = true;
                this->amountMatrix[rowPenIdx][rowMinIdx] 
                = supWeights[rowPenIdx];
                demWeights[rowMinIdx] -= supWeights[rowPenIdx];
                supWeights[rowPenIdx] = 0;
                calcColPenalties();
            //    rowPenIdx = maxRowPenIdx();
            }
            else 
            {
                this->rowSat[rowPenIdx] = true;
                this->colSat[rowMinIdx] = true;
                this->amountMatrix[rowPenIdx][rowMinIdx] 
                = supWeights[rowPenIdx];
                demWeights[rowMinIdx] = 0;
                supWeights[rowPenIdx] = 0;
                calcColPenalties();
                calcRowPenalties();
            }    
                        
          
        }
        else
        {            
          //  std::cout << "max col is larger  ";
          //  std::cout << "col sup[rowMinIdx]:" 
          //<< supWeights[colMinIdx]
            //    << " dem[colPenIdx]:" << demWeights[colPenIdx]
            //    << std::endl;
            
            if (supWeights[colMinIdx] > demWeights[colPenIdx])
            {
                this->colSat[colPenIdx] = true;
                this->amountMatrix[colMinIdx][colPenIdx] 
                = demWeights[colPenIdx];                
                supWeights[colMinIdx] -= demWeights[colPenIdx];
                demWeights[colPenIdx] = 0;
                calcRowPenalties();
            //    colPenIdx = maxColPenIdx();
                
            }
            else if (supWeights[colMinIdx] < demWeights[colPenIdx])
            {
                this->rowSat[colMinIdx] = true;
                this->amountMatrix[colMinIdx][colPenIdx] 
                = supWeights[colMinIdx];
                demWeights[colPenIdx] -= supWeights[colMinIdx];
                supWeights[colMinIdx] = 0;
                calcColPenalties(); // changed
            //    colPenIdx = maxColPenIdx();
            }
            else
            {
                this->rowSat[colMinIdx] = true;
                this->colSat[colPenIdx] = true;
                this->amountMatrix[colMinIdx][colPenIdx] 
                = supWeights[colMinIdx];
                demWeights[colPenIdx] = 0;
                supWeights[colMinIdx] = 0;
                calcRowPenalties();
                calcColPenalties();
            }
           
            
        }
        
        
    }
       
    long dist = 0;
    
    for ( int y = 0; y < (int)this->supSize; y++) // rows
    {
        for ( int x = 0; x < (int)this->demSize; x++) // columns
        {
            dist 
            += (this->amountMatrix[y][x] * this->distanceMatrix[y][x]);
        } 
    }
    
    this->newDistance = (double)(dist / 1);    
    this->currentDistance = (double)(dist / 1);
    
}


int main(int argc, char** argv)
{
    
    
    if (argc < 2)
    {
        std::cout << "please provide two signature files" << std::endl;
        return 1;
    }
    std::vector<FeatureSignatureTuple> ist1;
    std::vector<FeatureSignatureTuple> ist2;
        
        
        
        
    double w;
    int x, y;    
    double r,g,b;
    double coa, con;
    
    std::ifstream infile(argv[1]);
    std::string std;
    //int counter = 0;
    while (infile >> w >> x >> y >> r >> g >> b >> coa >> con)
    {            
        ist1.push_back({w, x, y, r, g, b, coa, con});    
    }
    
    std::ifstream infile2(argv[2]);
    while (infile2 >> w >> x >> y >> r >> g >> b >> coa >> con)
    {    
        ist2.push_back({w, x, y, r, g, b, coa, con});    
    }
    
    
    
    TransportProblem tp;
    
    
    std::cout << argv[1] << "," << argv[2] << ",";
    
    tp.initialVAM(ist1, ist2);
    
    
   
    const int maxIterations = 50; // how often does the algorithm cycle
    int actualIterations = 0;
    tp.basicsError = false;
    
   for (int i = 0; i < maxIterations; i++)
    {
        actualIterations++;
        
        if (tp.currentDistance == 0)
        {
            break;
        }
        
                
        try
        {
            tp.calcShadowCosts();
            tp.visitedShadowCosts = true;
        
      
            if (tp.enteringCell.val > -1)
            {
                break; // solution is already optimal
            }
        
        
            tp.visitedSteppingStones = true;
            tp.findSteppingStones();
            
            
            
            if (!tp.basicsError)
            {
                tp.visitedUpdateSolution = true;
                tp.updateSolution();
            }
            else
            {
                break;
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << '\n';
            std::cout << argv[1] << " " << argv[2] << std::endl;
        }
        
         if (tp.currentDistance > tp.newDistance)
        {
            tp.currentDistance = tp.newDistance;
        }
        else
        {
            break;
        }        
    }
    
    
     std::cout << "current distance:" 
     <<  tp.currentDistance << std::endl;
    return 0;
    
}







