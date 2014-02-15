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
    cerr << "Called matchFacesLua(" << args << ")\n";
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
    cerr << "Called matchFacesNull(" << args << ")\n";

    return ret;
}

/*
 1.2 matchFacesSimple matches faces in the order they are in the list.
 Mainly for testing- and demonstration-purposes.
 
*/
vector<pair<Face *, Face *> > matchFacesSimple(vector<Face> *src,
        vector<Face> *dst, int depth, string args) {
    vector<pair<Face *, Face *> > ret;
    cerr << "Called matchFacesSimple(" << args << ")\n";

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
    cerr << "Called matchFacesDistance(" << args << ")\n";

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
            cerr << (*src)[i].ToString() << "\n";
            cerr << (*dst)[j].ToString() << "\n";
            cerr << "MatchFacesds matched " << i << " with " << j << "\n";
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

#define nrMatchFacesStrategies (sizeof(matchFacesStrategies)/            \
                                sizeof(matchFacesStrategies[0]))

// Register all matching strategies in this list.
// The argument must be in the form "<name>[:<paramstring>]"
static struct {
    string name;
    matchFaces_t fn;
} matchFacesStrategies[] = {
    { "Distance", matchFacesDistance }, //The first is also the default strategy
    { "Null", matchFacesNull },
    { "Simple", matchFacesSimple },
    { "LowerLeft", matchFacesLowerLeft }
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
    unsigned int pos = fname.find(':');
    if (pos != string::npos) {
        // Split it into name and params, if a colon is present
        fname = args.substr(0, pos);
        fargs = args.substr(pos+1, string::npos);
    } else {
        // Otherwise the argument string is only the name
        fname = args;
    }
    
    for (unsigned int i = 0; i < nrMatchFacesStrategies; i++) {
        if (matchFacesStrategies[i].name == fname) {
            fn = matchFacesStrategies[i].fn;
            break;
        }
    }
    
    // The real work is done here
    if (fn) {
        pairs = fn(src, dst, depth, fargs);
    } else {
#ifdef USE_LUA
        pairs = matchFacesLua(src, dst, depth, args);
#else
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

