/*
 1 matchFaces contains matching-strategies to pair faces from source- and
 destination-regions.
 
 All strategies must have the following signature:
 
 vector<pair<Face *, Face *> > fn(vector<Face> *src, vector<Face> *dst,
                                  int depth, string args);
 
 and must be registered with a name in the struct matchFacesStrategies below.

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


// Helper function for matchFacesLowerLeft below to sort a list of faces by
// their lower-(leftmost)-point
static bool sortLowerLeft(const Face& r1, const Face& r2) {
    if (r1.isEmpty())
        return true;
    else if (r2.isEmpty()) 
        return false;
    else
        return r1.v[0].s < r2.v[0].s;
}

/*
 1.3 matchFacesLowerLeft matches the faces by the lowest-(leftmost)-point.
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

        if ((*src)[i].isEmpty()) {
            i++;
        } else if ((*dst)[j].isEmpty()) {
            j++;
        } else {
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
    }

    return ret;
}

// Helper class for matchFacesCriterion
class matchItem {
public:
    Face *s, *d;
    double val;
    
    matchItem(Face *s, Face *d, double val) : s(s), d(d), val(val) {};
    bool operator<(const matchItem& a) const { return val < a.val; }
};

// Try to find a suitable offset and scale for this set of faces.
// After transformation, the bounding box of the parent of these faces will be
// (0/0) - (SCALESIZE/SCALESIZE). If there is no parent, the common bounding box
// of all faces of the set is used.
// SCALESIZE is defined in interpolate.h.
static pair<Pt, Pt> getOffsetAndScale (vector<Face> *fcs) {
    if (fcs->empty())
        return pair<Pt,Pt>(Pt(0,0), Pt(1,1));
    Face *parent = ((*fcs)[0]).parent;
    pair<Pt, Pt> bbox, ret;
    if (parent) {
        // If we have a parent face, use its bounding box
        bbox = parent->GetBoundingBox();
    } else {
        // otherwise get the common bounding box of all faces in this set
        bbox = Face::GetBoundingBox(*fcs);
    }
    
    
    ret.first = -bbox.first;
        
    if ((bbox.second.x > bbox.first.x) &&
        (bbox.second.y > bbox.first.y)) {
        // Calculate the factor which scales the bounding box to SCALESIZE
        // (defined in interpolate.h) in each direction.
        Pt scale(SCALESIZE / (bbox.second.x - bbox.first.x),
                 SCALESIZE / (bbox.second.y - bbox.first.y));
        ret.second = scale;
    } else {
        // The factor would be invalid (FPE), so just use 1
        ret.second = Pt(1, 1);
    }
    
    return ret;
}

/*
 1.4 matchFacesCriterion is a framework for matching-strategies.
 It takes the two lists of faces and a scoring-function ~fn~, which calculates
 a score for each pair of faces. A smaller score wins over higher scores.
 Then, the results are sorted by this value and the best pairings are taken
 into the result list. An optional threshold defines the maximum score a
 pair may have to be a candidate for the result list.
 
*/
static vector<pair<Face *, Face *> > matchFacesCriterion(vector<Face> *src,
        vector<Face> *dst, int depth,
        double (*fn)(Face *src, Face *dst), double thres) {
    vector<pair<Face *, Face *> > ret;
    vector<matchItem> mtab;
    
    // Get source- and destination-transform-values
    pair<Pt, Pt> stf = getOffsetAndScale(src);
    pair<Pt, Pt> dtf = getOffsetAndScale(dst);
    
    for (unsigned int i = 0; i < src->size(); i++) {
        for (unsigned int j = 0; j < dst->size(); j++) {
            Face s = (*src)[i], d = (*dst)[j];
            s.Transform(stf.first, stf.second);
            d.Transform(dtf.first, dtf.second);
            double val = fn(&s, &d);
            DEBUG(4, "Criterion: " << i << "/" << j << " = " << val);
            if (val < thres) {
                mtab.push_back(matchItem(&((*src)[i]), &((*dst)[j]), val));
            }
        }
    }
    std::sort(mtab.begin(), mtab.end());
    std::vector<matchItem>::iterator it = mtab.begin();
    while (it != mtab.end()) {
        if (!it->s->used && !it->d->used) {
            it->s->used = 1;
            it->d->used = 1;
            ret.push_back(pair<Face *, Face *>(it->s, it->d));
        }
        it++;
    }
    
    return ret;
}

// Helper-function for matchFacesOverlap below. Calculates a score from the
// overlapping area of two faces.
static double overlap (Face *src, Face *dst) {
    Region r1 = src->MakeRegion(false);
    Region r2 = dst->MakeRegion(false);
    Region is(0);
    r1.Intersection(r2, is, NULL);

    double a1 = r1.Area(NULL);
    double a2 = r2.Area(NULL);
    double ai = is.Area(NULL);
    
    // Calculate the average overlap percentage between a1 and a2 and vice versa
    double ret = (ai*100/a1+ai*100/a2)/2;
    
    // Subtract the value from 100, since matchFacesCriterion tries to minimize
    return 100-ret;
}

/*
 1.5 matchFacesOverlap uses matchFacesCriterion for matching faces by their
 overlapping area. The average overlap must be 30 percent minimum if no other
 value is given.
 
*/
static vector<pair<Face *, Face *> > matchFacesOverlap(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    int minpercent = atoi(args.c_str());
    if ((minpercent <= 0) || (minpercent > 100))
        minpercent = 30;
    DEBUG(2, "Strategy overlap with minimum " << minpercent << "%");
    return matchFacesCriterion(src, dst, depth, overlap, (100 - minpercent));
}

#define nrMatchFacesStrategies (sizeof(matchFacesStrategies)/            \
                                sizeof(matchFacesStrategies[0]))


// Helper-Function for matchFacesDistance
static double distance (Face *src, Face *dst) {
    return src->GetMiddle().distance(dst->GetMiddle());
}

/*
 1.6 matchFacesDistance uses a distance-based scoring function. It tries
 to pair faces with low distance.
 
*/
vector<pair<Face *, Face *> > matchFacesDistance(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    return matchFacesCriterion(src, dst, depth, distance, 1000000);
}

/*
 1.7 matchFacesMW tries to match Faces at the highest level by distance but then
 doesn't try to match concavities or holes. This is about what McKenney and Webb
 suggested.
 
*/
vector<pair<Face *, Face *> > matchFacesMW(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    if (depth == 0) {
        return matchFacesDistance(src, dst, depth, "");
    } else {
        return matchFacesNull(src, dst, depth, "");
    }
}


// Register all matching strategies in this list.
// The argument for interpolate2 must be in the form "<name>[:<paramstring>]"
// to select the function
static struct {
    string name;
    matchFaces_t fn;
} matchFacesStrategies[] = {
    //The first is also the default strategy if Lua is not compiled
    { "overlap", matchFacesOverlap },
    { "distance", matchFacesDistance },
    { "null", matchFacesNull },
    { "simple", matchFacesSimple },
    { "mw", matchFacesMW },
    { "lowerleft", matchFacesLowerLeft }
};

/*
 2 matchFaces is the interface to all matching strategies.
 It prepares the lists of faces, calls the real matching function and cleans
 up afterwards.
 
 This function is called with the two lists ~src~ and ~dst~ of source- and
 destinationfaces, the current recursionlevel ~depth~
 and the string-argument which configures the matching-strategy. It returns
 the pairs of faces, which should be interpolated with each other.
 
 The args-string should be in this form:  "<name>[:<paramstring>]"
 
 paramstring could be also composed from several parameters, for example
 "<arg1>,<arg2>,...,<argn>", but it is the responsibility of the matching
 function to parse that.
 
 where "name" is searched in the strategies above and the corresponding
 function is called with the name stripped from the args-string.
 
 If "name" is not found and Lua is compiled, then call Lua with the full
 args-string. If Lua is not compiled, use the first strategy in the
 list above as the default-strategy.
 
*/
vector<pair<Face *, Face *> > matchFaces(
        vector<Face> *src, vector<Face> *dst, int depth,
        string args) {
    vector<pair<Face *, Face *> > pairs, ret;
    
    // Mark all faces as unused and if they are from the source- or
    // destination-set.
    for (unsigned int i = 0; i < src->size(); i++) {
        (*src)[i].used = 0;
        (*src)[i].isdst = 0;
    }
    for (unsigned int i = 0; i < dst->size(); i++) {
        (*dst)[i].used = 0;
        (*dst)[i].isdst = 1;
    }

    // Try to find the matching-strategy defined in the argument-string.
    matchFaces_t fn = NULL;
    string fname = args, fargs = "";
    size_t pos = fname.find(':');   // Find the index of the colon
    if (pos != string::npos) {
        // Split it into name and params, if a colon is present
        fname = args.substr(0, pos);
        fargs = args.substr(pos+1, string::npos);
    } else {
        // Otherwise the argument string consists only of the name (we assume)
        fname = args;
    }
    
    for (unsigned int i = 0; i < nrMatchFacesStrategies; i++) {
        if (matchFacesStrategies[i].name == fname) {
            DEBUG(2, "Using C++-MatchFaces-strategy " << fname);
            fn = matchFacesStrategies[i].fn;
            break;
        }
    }
    
    if (fn) {
        // If we have found a matching function, call it here
        pairs = fn(src, dst, depth, fargs);
    } else {
        // Otherwise use a default strategy
#ifdef USE_LUA
        DEBUG(2, "Using Lua-MatchFaces-strategy " << args);
        // which is Lua, if it is compiled
        pairs = matchFacesLua(src, dst, depth, args);
#else
        DEBUG(2, "Using Default-C++-MatchFaces-strategy " <<
                matchFacesStrategies[0].name);
        // or the first strategy in the list otherwise
        pairs = matchFacesStrategies[0].fn(src, dst, depth, fargs);
#endif
    }

    // The function above may have used the ~used~-attribute, so reset it
    for (unsigned int i = 0; i < src->size(); i++) {
        (*src)[i].used = 0;
    }
    for (unsigned int i = 0; i < dst->size(); i++) {
        (*dst)[i].used = 0;
    }

    // Put all sane pairs of faces into the return-list
    for (unsigned int i = 0; i < pairs.size(); i++) {
        // Ignore a pair if: one partner is NULL, already used or from the
        // wrong set
        if (!pairs[i].first || !pairs[i].second ||
             pairs[i].first->used || pairs[i].second->used ||
                pairs[i].first->isdst || !pairs[i].second->isdst)
            continue;
        // This pairing seems ok, mark the members as used 
        pairs[i].first->used = 1;
        pairs[i].second->used = 1;
        ret.push_back(pairs[i]); // and put it into the list
    }

    // All faces left are not in a real pairing, put them into the list
    // without a partner
    for (unsigned int i = 0; i < src->size(); i++) {
        if (!(*src)[i].used) {
            ret.push_back(pair<Face*, Face*>(&(*src)[i], NULL));
        }
    }
    for (unsigned int i = 0; i < dst->size(); i++) {
        if (!(*dst)[i].used) {
            ret.push_back(pair<Face*, Face*>(NULL, &(*dst)[i]));
        }
    }

    return ret;
}
