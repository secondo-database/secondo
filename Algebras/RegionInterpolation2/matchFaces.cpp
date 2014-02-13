/*
 1 matchFaces contains matching-strategies to pair faces from source- and
 destination-regions.
 
 All strategies must have the following signature:
 
 vector<pair<Face *, Face *> > fn(vector<Face> *src, vector<Face> *dst,
                                  int depth, string args);
 
  
*/

#include "interpolate.h"

#include <string>

#ifdef USE_LUA
// This is the interface to Lua, the mainly used strategy.
vector<pair<Face *, Face *> > _matchFacesLua(vector<Face> *src,
        vector<Face> *dst, int depth, string args);

vector<pair<Face *, Face *> > matchFacesLua(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    return _matchFacesLua(src, dst, depth, args);
}
#endif

/*
 1.1 matchFacesNull does not match any faces.
 Mainly for testing- and demonstration-purposes.
 
*/
vector<pair<Face *, Face *> > matchFacesNull(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    vector<pair<Face *, Face *> > ret;

    return ret;
}

/*
 1.2 matchFacesSimple matches faces in the order they are in the list.
 Mainly for testing- and demonstration-purposes.
 
*/
vector<pair<Face *, Face *> > matchFacesSimple(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    vector<pair<Face *, Face *> > ret;

    for (unsigned int i = 0; (i < src->size() || (i < dst->size())); i++) {
        if ((i < src->size()) && (i < dst->size())) {
            pair<Face *, Face *> p(&((*src)[i]), &((*dst)[i]));
            ret.push_back(p);
        }
    }

    return ret;
}


/*
 1.3 matchFacesDistance uses a simple distance matching algorithmus. It tries
 to pair faces with low distance.
 
*/
vector<pair<Face *, Face *> > matchFacesDistance(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    vector<pair<Face *, Face *> > ret;

    Pt srcoff = Face::GetBoundingBox(*src).first;
    Pt dstoff = Face::GetBoundingBox(*dst).first;

    if (src->size() >= dst->size()) {
        for (unsigned int i = 0; i < dst->size(); i++) {
            int dist = 2000000000;
            int candidate = -1;
            for (unsigned int j = 0; j < src->size(); j++) {
                if ((*src)[j].used == 1)
                    continue;
                Pt srcm = (*src)[j].GetMiddle() - srcoff;
                Pt dstm = (*dst)[i].GetMiddle() - dstoff;
                int d2 = dstm.distance(srcm);
                if (d2 < dist) {
                    dist = d2;
                    candidate = j;
                }
            }
            pair<Face *, Face *> p(&(*src)[candidate], &(*dst)[i]);
            ret.push_back(p);
            (*src)[candidate].used = 1;
        }
        for (unsigned int j = 0; j < src->size(); j++) {
            if ((*src)[j].used)
                continue;
            pair<Face *, Face *> p(&(*src)[j], NULL);
            ret.push_back(p);
        }
    } else {
        for (unsigned int i = 0; i < src->size(); i++) {
            int dist = 2000000000;
            int candidate = -1;
            for (unsigned int j = 0; j < dst->size(); j++) {
                if ((*dst)[j].used == 1)
                    continue;
                Pt srcm = (*src)[i].GetMiddle() - srcoff;
                Pt dstm = (*dst)[j].GetMiddle() - dstoff;
                int d2 = dstm.distance(srcm);
                if (d2 < dist) {
                    dist = d2;
                    candidate = j;
                }
            }
            pair<Face *, Face *> p(&(*src)[i], &(*dst)[candidate]);
            ret.push_back(p);
            (*dst)[candidate].used = 1;
        }
        for (unsigned int j = 0; j < dst->size(); j++) {
            if ((*dst)[j].used)
                continue;
            pair<Face *, Face *> p(NULL, &(*dst)[j]);
            ret.push_back(p);
        }

    }

    return ret;
}

// Helper function for matchFacesLowerLeft below to sort a list of faces by
// their lower-(leftmost)-point
static bool sortLowerLeft(const Face& r1, const Face& r2) {
    return r1.v[0].s < r2.v[0].s;
}

/*
 1.4 matchFacesLowerLeft matches the faces by the lowest-(leftmost)-point.
 It is used in the evaporation-phase, when we only need to match faces
 with identical hull.
 
*/
vector<pair<Face *, Face *> > matchFacesLowerLeft(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    vector<pair<Face *, Face *> > ret;

    // Sort the faces by their lower-(leftmost)-point which is always the first
    // point if the face is sorted correctly (which it should be!)
    std::sort(src->begin(), src->end(), sortLowerLeft);
    std::sort(dst->begin(), dst->end(), sortLowerLeft);

    // Traverse the two lists in parallel and try to find matches
    unsigned int i = 0, j = 0;
    while (i < src->size() && j < dst->size()) {
        Pt p1 = (*src)[i].v[0].s;
        Pt p2 = (*dst)[j].v[0].s;

        if (p1 == p2) {
            (*src)[i].used = 1;
            (*dst)[j].used = 1;
            ret.push_back(pair<Face*, Face*>(&(*src)[i], &(*dst)[j]));
            i++;
            j++;
        } else if (p1 < p2) {
            i++;
        } else {
            j++;
        }
    }

    return ret;
}
