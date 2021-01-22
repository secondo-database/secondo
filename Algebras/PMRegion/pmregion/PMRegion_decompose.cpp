/* 
 * This file is part of libpmregion
 * 
 * File:   decompose.cpp
 * Author: Florian Heinz <fh@sysv.de>

 1 decompose.cpp
   pmregcli component for decomposing moving regions
  
*/


#include <set>
#include <sstream>
#include <algorithm>


#include "PMRegion_internal.h"

#include <CGAL/Polygon_set_2.h>
typedef CGAL::Polygon_set_2<Kernel> Polygon_set_2;

using namespace std;
using namespace pmr;

#define POLYGON_START 0
#define POLYGON_END 2

/* This class represents a bounding box for a 2d
   point set */
class BoundingBox {
    public:
        Kernel::FT llx, lly, urx, ury;
        bool valid;

        BoundingBox () : valid(false) {}

        // Update the bounding box to contain the given point
        void update (Point_2 p) {
            if (!valid) {
                llx = urx = p.x();
                lly = ury = p.y();
                valid = true;
            } else {
                if (p.x() < llx)
                    llx = p.x();
                else if (p.x() > urx)
                    urx = p.x();
                if (p.y() < lly)
                    lly = p.y();
                else if (p.y() > ury)
                    ury = p.y();
            }
        }

        // check if the bounding box overlaps another bounding box
        bool overlaps (BoundingBox& bb) {
            if (urx < bb.llx || bb.urx < llx ||
                    ury < bb.lly || bb.ury < lly)
                return false;

            return true;
        }

        string ToString () {
            stringstream ss;
            ss << "( " << llx << " / " << lly << " ) => " << 
                  "( " << urx << " / " << ury << " )";

            return ss.str();
        }

};

/* Convert a moving face into a CGAL Polygon_set_2                       */
/* Uses the "off" parameter to get either the source region (off = 0)    */
/* or the destination region (off = 2) of the moving face                */
/* Also calculates the bounding box of the resulting polygon set in "bb" */
Polygon_set_2* getPolygon (RList* face, int off, BoundingBox& bb) {
    vector<Polygon> ps;
    for (unsigned int f = 0; f < face->items.size(); f++) {
        RList* cycle = &face->items[f];
        int sz = cycle->items.size();
        Polygon p;
        for (unsigned int i = 0; i < cycle->items.size(); i++) {
            Kernel::FT sx = cycle->items[i].items[off].getFt();
            Kernel::FT sy = cycle->items[i].items[off+1].getFt();
            Kernel::FT ex = cycle->items[(i+1)%sz].items[off].getFt();
            Kernel::FT ey = cycle->items[(i+1)%sz].items[off+1].getFt();
            Point_2 s(sx, sy);
            Point_2 e(ex, ey);
            bb.update(s);
            bb.update(e);
            if (sx != ex || sy != ey) {
                p.push_back(s);
            }
        }
        if (f > 0) // hole cycles must have reverse orientation
            p.reverse_orientation();
        if (!p.is_simple()) {
            // Should not happen with sane source data
            cerr << "Not simple: " << endl << p << endl << endl;
            assert(p.is_simple());
        }
        ps.push_back(p);
    }

    // Join the individual polygons to a polygon set
    Polygon_with_holes_2 ph(ps[0], ps.begin()+1, ps.end());
    Polygon_set_2 *S = new Polygon_set_2();
    S->join(ph);


    return S;
}

// Calculates the area of the given polygon
static Kernel::FT getarea (Polygon_with_holes_2 p) {
    Kernel::FT ret = 0;

    // First calculate the area of the outer boundary
    ret += abs(p.outer_boundary().area());
    for (Hole_const_iterator hi = p.holes_begin(); hi != p.holes_end(); hi++) {
        // Then subtract the areas of all holes from it
        ret -= abs(hi->area());
    }

    return ret;
}


/* Classes for the Adjacency Graph for decomposition */

class Node;

/* A directed edge of the adjacency graph */
class Edge {
    public:
        Node *src, *dst; // Start and end node of the edge
        double weight;   // weight of the edge (e.g. the overlapping area)
        bool deleted;    // marker if the edge is to delete

        // All fields constructor
        Edge(Node *src, Node *dst, double weight) : src(src), dst(dst),
        weight(weight), deleted(0) {}
};


/* A node in the adjacency graph */
class Node {
    private:
        // Initial and final region of the
        // corresponding elementary unit region
        Polygon_set_2 *ps, *pe;

    public:
        // Bounding boxes of initial and final region
        // to speed up overlap tests
        BoundingBox bbs, bbe;
        // nested list representation of the moving face of the
        // elementary unit region
        RList* face;
        // Start and end instant of the elementary unit region
        Kernel::FT start, end;
        // node id, for debugging
        int id;
        // marker, if the node was already processed
        bool processed;

        // List of edges that are incoming to or outgoing from this node
        vector<Edge *> incoming, outgoing;

        // Constructor from an elementary unit region with moving face "f"
        // start instant "s" and end instant "e"
        Node(RList* f, Kernel::FT s, Kernel::FT e, int nodeid) {
            face = f;
            start = s;
            end = e;
            ps = getPolygon(face, POLYGON_START, bbs);
            pe = getPolygon(face, POLYGON_END, bbe);
            id = nodeid;
            processed = 0;
        };

        // Get the polygon at start (startend == POLYGON_START) or
        // end (staŕtend == POLYGON_END) instant of the elementary
        // unit region. Cache the calculated polygon for further
        // use.
        Polygon_set_2 *polygon(int startend) {
            if (startend == POLYGON_START) {
                if (!ps)
                    ps = getPolygon(face, POLYGON_START, bbs);
                return ps;
            } else {
                if (!pe)
                    pe = getPolygon(face, POLYGON_END, bbe);
                return pe;
            }
        }

        // Calculate the adjacency relation between this node and
        // the given node "n". If adjacency is detected, generate
        // an edge connecting the two nodes.
        bool adjacent (Node *n) {
            // Check if the given node directly succeeds this node
            // which is mandatory for adjacent nodes
            if (this->end != n->start)
                return false;
            // Retrieve the two polygons
            Polygon_set_2* p1 = polygon(POLYGON_END);
            Polygon_set_2* p2 = n->polygon(POLYGON_START);
            // If their bounding box does not overlap no
            // adjacency is possible
            if (!bbe.overlaps(n->bbs))
                return false;

            // Calculate the overlapping area of the two polygons.
            list<Polygon_with_holes_2> pwh;
            Polygon_set_2 tmp = *p1;
            tmp.intersection(*p2);
            tmp.polygons_with_holes(back_inserter(pwh));
            Kernel::FT a = 0;
            for (list<Polygon_with_holes_2>::iterator i = pwh.begin();
                    i != pwh.end(); i++) {
                a += getarea(*i);
            }

            // If the area is non-zero, an adjacency has been found
            if (a > 0) {
                // Create a directed edge from this node to "n" with
                // the overlapping area as edge weight
                Edge *e = new Edge(this, n, CGAL::to_double(a));
                // Put it into the list of outgoing edges from this node
                outgoing.push_back(e);
                // and into the list of incoming edges of "n"
                n->incoming.push_back(e);

                // Adjacency found
                return true;
            }

            // No adjacency
            return false;
        }
};


// Create a list of nodes containing elementary unit regions
// from a moving region "mr". These are the nodes of the
// adjacency graph.
vector<Node> createElementaryURegions (RList *mr) {
    vector<Node> ret;
    mr = &mr->items[4];
    // Iterate over the unit regions
    for (unsigned int i = 0; i < mr->items.size(); i++) {
        RList* ureg = &mr->items[i];
        RList* iv = &ureg->items[0];
        // Start and end instant of the unit region
        Kernel::FT start = parsetime(iv->items[0].getString());
        Kernel::FT end = parsetime(iv->items[1].getString());
        RList* faces = &ureg->items[1];
        for (unsigned int j = 0; j < faces->items.size(); j++) {
            // Create a node with an elementary unit region from
            // each face of each unit region
            Node n(&faces->items[j], start, end, ret.size()+1);
            ret.push_back(n);
            if (i < 1000 || (i%1000)==0)
                cerr << "Creating EUR nr " << (ret.size()+1) <<
                    " (from URegion " << i << "/" << mr->items.size() <<
                    ")" << endl;
        }
    }

    return ret;
}

// Connect the nodes of the adjacency graph with
// directed edges.
int connectNodes (vector<Node>& nodes) {
    map<Kernel::FT, vector<Node *> > start;
    set<Kernel::FT> events;
    int edges = 0;

    // First, create a lookup map to quickly retrieve
    // all elementary unit region nodes, that start at
    // a specific instant in time.
    for (unsigned int i = 0; i < nodes.size(); i++) {
        start[nodes[i].start].push_back(&nodes[i]);
        events.insert(nodes[i].start);
    }

    // Now iterate over all nodes
    for (unsigned int i = 0; i < nodes.size(); i++) {
        Node *n = &nodes[i];
        // Candidates for an adjacency to "n" are all elementary unit region
        // nodes that start in the instant when "n" ends.
        vector<Node *> candidates = start[n->end];
        for (unsigned int j = 0; j < candidates.size(); j++) {
            // Iterate over all candidates and try to establish an adjacency
            Node *n2 = candidates[j];
            if (n->adjacent(n2)) {
                // An adjacency has been found. The edges were already
                // established by the "adjacent()" method, so we only have to
                // update the counter here.
                edges++;
            }
        }
    }

    // Return the number of established edges
    return edges;
}

// Helper function to sort a list of edges by their weight (descending)
struct edgesort {
    inline bool operator() (const Edge* e1, const Edge* e2) {
        return e1->weight > e2->weight;
    }
};

// Decompose the nodes according to the "debranch" criterion.
// This means, the incoming edges of each node are deleted if
// there is more than one (same for outgoing edges).
// if "keeplargest" is 1, then the edge of the adjacency with the
// largest overlapping area is not deleted, but all others.
int decomposeNodes (vector<Node>& nodes, int keeplargest) {
    int deletions = 0;

    // Iterate over all nodes
    for (unsigned int i = 0; i < nodes.size(); i++) {
        // If the number of incoming edges is greater than one, edges have to
        // be deleted
        if (nodes[i].incoming.size() > 1) {
            // First, sort the incoming edges according to their weight
            // in descending order
            sort(nodes[i].incoming.begin(), nodes[i].incoming.end(),edgesort());

            // Iterate over the edges
            for (vector<Edge*>::iterator it = nodes[i].incoming.begin();
                    it != nodes[i].incoming.end(); it++) {
                Edge *e = *it;
                // Do not delete the edge if:
                // - it is already deleted
                // - it is the first edge and keeplargest is 1
                if (!e->deleted && (it != nodes[i].incoming.begin() ||
                            !keeplargest)) {
                    // otherwise, mark it deleted
                    deletions++;
                    e->deleted = 1;
                }
            }
        }
        // If the number of outgoing edges is greater than one, edges have to
        // be deleted
        if (nodes[i].outgoing.size() > 1) {
            // First, sort the outgoing edges according to their weight
            // in descending order
            sort(nodes[i].outgoing.begin(), nodes[i].outgoing.end(),edgesort());

            // Iterate over the edges
            for (vector<Edge*>::iterator it = nodes[i].outgoing.begin();
                    it != nodes[i].outgoing.end(); it++) {
                Edge *e = *it;
                // Do not delete the edge if:
                // - it is already deleted
                // - it is the first edge and keeplargest is 1
                if (!e->deleted && (it != nodes[i].outgoing.begin() ||
                            !keeplargest)) {
                    // otherwise, mark it deleted
                    deletions++;
                    e->deleted = 1;
                }
            }
        }
    }

    // Return the number of deleted edges
    return deletions;
}

/* This is a recursive function that traverses a weaky connected component
   in a directed graph. The traversal starts at node "n" and all traversed
   nodes are stored in the vector "nodes"                                  */
void traverseConnectedComponent (Node *n, vector<Node *>& nodes) {
    //Ignore already processed nodes. The directed acyclic graph has no directed
    //cycles, but we follow outgoing and incoming edges, so we have to implement
    //a mechanism to avoid double-processing of nodes.
    if (n->processed)
        return;

    // Store the node in the output list
    nodes.push_back(n);
    // and mark it as processed
    n->processed = 1;

    // Recursively call this function for each incoming and outgoing
    // edge that is not marked as deleted to further traverse the
    // connected component
    for (unsigned int i = 0; i < n->incoming.size(); i++) {
        if (!n->incoming[i]->deleted)
            traverseConnectedComponent(n->incoming[i]->src, nodes);
    }
    for (unsigned int i = 0; i < n->outgoing.size(); i++)
        if (!n->outgoing[i]->deleted)
            traverseConnectedComponent(n->outgoing[i]->dst, nodes);
}

// Helper function to sort elementary region unit nodes by their start instant
struct sortbystartinstant {
    inline bool operator() (const Node* n1, const Node* n2) {
        return n1->start < n2->start;
    }
}; 

// Build a moving region from a weakly connected component
// starting at node "n"
RList buildMovingRegion (Node& n) {
    RList mr;
    vector<Node *> nodes;

    // traverse the connected component and store all nodes
    // belonging to it in the vector "nodes"
    traverseConnectedComponent(&n, nodes);

    // Sort the resulting nodes according to their start instant
    sort(nodes.begin(), nodes.end(), sortbystartinstant());

    Kernel::FT prev;
    RList rl;
    // Traverse the list and build a unit region from all elementary
    // unit regions with the same start and end instant.
    for (unsigned int i = 0; i < nodes.size(); i++) {
        Kernel::FT now = nodes[i]->start;
        if (i > 0 && prev != now) {
            // A new time interval has started, append the unit region to
            // the moving region structure.
            mr.append(rl);
        }
        // Create a new unit region if it is the first one (i == 0) or
        // if a new time interval has started
        if (i == 0 || prev != now) {
            rl = RList();
            RList *iv = rl.nest();
            iv->append(timestr(now));
            iv->append(timestr(nodes[i]->end));
            iv->append(true);
            iv->append(false);
            rl.nest();
            prev = now;
        }
        // Append the face to the current unit region
        rl.items[1].append(*nodes[i]->face);
    }
    // Append the last unit region to the moving region
    mr.append(rl);

    return mr;
}

// Build a SECONDO relation of moving regions from the node
// list of an adjacency graph. Each moving region corresponds
// to a weakly connected component in the directed acyclic graph
// Additionally, store the individual moving regions in the
// vector "mregs"
static int nr = 0;
RList buildMovingRegionsRelation (vector<Node>& nodes, vector<RList>& mregs) {
    RList ret, empty;

    // Type declarations of the resulting relation object
    ret.appendsym("OBJECT");
    ret.appendsym("mregions");
    ret.append(empty);
    RList *rel = ret.nest();
    rel->appendsym("rel");
    RList *tuple = rel->nest();
    tuple->appendsym("tuple");
    RList *type = tuple->nest();
    type->appendsym("MRegion");
    type->appendsym("mregion");

    RList *objects = &ret;
    // Try to create a moving region from the connected component
    // starting with node "nodes[i]"
    for (unsigned int i = 0; i < nodes.size(); i++) {
        if (nodes[i].processed)
            continue; // The node was already processed in another component
        // Build moving region from the connected component that "nodes[i]"
        // belongs to.
        RList mreg = buildMovingRegion(nodes[i]);
        // Append the moving region to the relation
        objects->append(mreg);
        nr++;
        // Uncomment the following to additionally create one object file
        // for each moving region, mainly for debugging purposes.
        RList obj;
        obj.appendsym("OBJECT");
        obj.appendsym("mregion");
        obj.nest();
        obj.appendsym("mregion");
        obj.append(mreg);
        mregs.push_back(obj);
    }
    cerr  << "Decomposed to " << nr << " moving regions" << endl;

    return ret;
}

// Decompose the moving region in "mreg" to individual moving regions.
// steps == 0 : only perform isolation step
// steps == 1 : perform debranching step
// steps == 2 : perform debranching step, keep the largest connection.
// Stores the decomposed moving regions in "mregs"
// Returns a SECONDO relation object with the moving regions
RList pmr::decompose_mreg (RList *mreg, int steps, vector<RList>& mregs) {
    cerr << "Creating elementary uregions" << endl;
    vector<Node> nodes = createElementaryURegions(mreg);
    cerr << "Number of EUR: " << nodes.size() << endl;

    cerr << "Connecting nodes" << endl;
    int nredges = connectNodes(nodes);
    cerr << "Number of edges: " << nredges << endl;

    if (steps > 0) {
        cerr << "Decomposing nodes" << endl;
        int deletions = decomposeNodes(nodes, steps == 2);
        cerr << "Number of deleted edges: " << deletions << endl;
    }

    cerr << "Rebuilding moving regions" << endl;
    RList ret = buildMovingRegionsRelation(nodes, mregs);
    cerr << "Done." << endl;

    return ret;
}

// Cluster of polyhedron vertices, according to the faces they appear in.
// Used for identifying individual volumes of the polyhedron.
class PMCluster {
    public:
        // All indices of vertices belonging to this cluster
        set<int> cluster;

        // All faces belonging to this cluster
        set<vector<int> > faces;

        // Initialize a cluster with a first face
        PMCluster (vector<int> face) {
            for (unsigned int i = 0; i < face.size(); i++) {
                cluster.insert(face[i]);
            }
            faces.insert(face);
        }

        // merge to clusters
        bool merge (map<int,PMCluster *>& cmap, map<PMCluster *,
                set<int> >& cmapr) {
            bool merged = false;
            for (set<int>::iterator it = cluster.begin();
                    it != cluster.end(); it++) {
                int idx = *it;
                if (cmap.count(idx)) {
                    PMCluster *victim = cmap[idx];
                    if (victim == this)
                        continue;
                    cluster.insert(victim->cluster.begin(),
                            victim->cluster.end());
                    faces.insert(victim->faces.begin(), victim->faces.end());
                    for (set<int>::iterator x = cmapr[victim].begin();
                            x != cmapr[victim].end(); x++) {
                        cmap[*x] = this;
                        cmapr[this].insert(*x);
                    }
                    cmapr.erase(victim);
                    delete victim;
                } else {
                    cmap[idx] = this;
                    cmapr[this].insert(idx);
                }
            }


            return merged;
        }

};

// Perform the "isolate" decomposition step on a pmregion.
// This is performed by taking the list of faces and cluster
// these according to the vertex indices. This yields the
// individual volumes of the polyhedron.
static vector<PMRegion> isolate_pmreg (PMRegion *pmreg) {
    vector<PMRegion> ret;

    RList rl = pmreg->toRList().items[4];
    RList& pts = rl.items[0];
    RList& fcs = rl.items[1];

    vector<PMCluster *> pmc;
    // First, create an individual cluster from each face.
    for (unsigned int i = 0; i < fcs.items.size(); i++) {
        RList facerl = fcs.items[i];
        vector<int> face;
        for (unsigned int j = 0; j < facerl.items.size(); j++) {
            face.push_back((int)round(facerl.items[j].getNr()));
        }
        pmc.push_back(new PMCluster(face));
    }

    // Small clusters are merged to larger ones
    map<int,PMCluster *> cmap;
    map<PMCluster *, set<int> > cmapr;
    for (unsigned int i = 0; i < pmc.size(); i++) {
        PMCluster *c = pmc[i];
        c->merge(cmap, cmapr);
        if (i%1000==0)
            cerr << i << " / " << pmc.size() << endl;
    }

    // The remaining clusters are maximal and cannot be merged further. Store
    // them into "clusters"
    set<PMCluster*> clusters;
    for (map<int,PMCluster*>::iterator it = cmap.begin();
            it != cmap.end(); it++) {
        clusters.insert(it->second);
    }

    // Create an RList representation for each cluster
    for (set<PMCluster*>::iterator it = clusters.begin();
            it != clusters.end(); it++) {
        int curidx = 0;
        RList pmr;
        pmr.appendsym("OBJECT");
        pmr.appendsym("pmregion");
        pmr.nest();
        pmr.appendsym("pmregion");
        RList _pts;
        RList _fcs;
        // The face indices have to be remapped, because points that are not
        // part of this cluster are not stored in this pmregion
        map<int, int> remap;
        for (set<vector<int> >::iterator f = (*it)->faces.begin();
                f != (*it)->faces.end(); f++) {
            RList nf;
            for (vector<int>::const_iterator f2 = f->begin();
                    f2 != f->end(); f2++) {
                if (!remap.count(*f2)) {
                    remap[*f2] = curidx++;
                    _pts.append(pts.items[*f2]);
                }
                nf.append((double)remap[*f2]);
            }
            _fcs.append(nf);
        }
        RList body;
        body.append(_pts);
        body.append(_fcs);
        pmr.append(body);
        // Convert the RList representation to a pmregion and add it to the
        // result list
        ret.push_back(PMRegion::fromRList(pmr));
    }

    return ret;
}

vector<PMRegion> pmr::decompose_pmreg (PMRegion *pmreg, int steps,
        vector<RList>& mregs) {
    vector<PMRegion> pmregs = isolate_pmreg(pmreg);
    cerr << "Steps: " << steps << endl;

    if (steps > 0) {
        vector<PMRegion> ret;
        for (unsigned int i = 0; i < pmregs.size(); i++) {
            PMRegion& pmr = pmregs[i];
            RList mreg = pmr.toMRegion2();
            vector<RList> mregs;
            pmr::decompose_mreg(&mreg, steps, mregs);

            for (unsigned int j = 0; j < mregs.size(); j++) {
                try {
                    PMRegion pmr2 = PMRegion::fromMRegion(mregs[j]);
                    ret.push_back(pmr2);
                } catch (const std::exception& e) { }
            }
        }
        return ret;
    } else {
        return pmregs;
    }
}

