/*
   1 ~Matches.cpp~ Matches object
   Couples corresponding faces in the source and
   destination set.

*/

#include "interpolate.h"

Matches::Matches() {
}

/*
   1.1 Constructor for the common case of two faces paired

*/
Matches::Matches(Face *sface, Face *dface) {
    if (sface) src.push_back(sface);
    if (dface) dst.push_back(dface);
}

/*
   1.2 Checks, if the specified faces are valid.

*/
bool Matches::isValid() {
    // Is one set empty?
    if (!src.size() || !dst.size())
        return false;
    // Only one set may contain more than one element, we only
    // support 1:m resp. m:1 assignments yet
    if ((src.size() != 1) && (dst.size() != 1))
        return false;

    bool valid = true;
    // Check, if a face is already used or in the wrong place
    for (unsigned int i = 0; i < src.size(); i++) {
        if (src[i]->used || src[i]->isdst)
            valid = false;
        src[i]->used++; // Mark temporary as used
    }
    for (unsigned int i = 0; i < dst.size(); i++) {
        if (dst[i]->used || !dst[i]->isdst)
            valid = false;
        dst[i]->used++; // Mark temporary as used
    }
    for (unsigned int i = 0; i < src.size(); i++) {
        src[i]->used--; // Undo the temporary in-use mark
    }
    for (unsigned int i = 0; i < src.size(); i++) {
        dst[i]->used--; // Undo the temporary in-use mark
    }

    return valid;
}

/*
   1.3 Marks the involved faces as used, so they can't
   be used a second time.

*/
void Matches::markUsed() {
    for (unsigned int i = 0; i < src.size(); i++) {
        src[i]->used = 1;
    }
    for (unsigned int i = 0; i < dst.size(); i++) {
        dst[i]->used = 1;
    }
}

/*
   1.4 Convenience function: Return the first source face
   of the couple or NULL.

*/
Face* Matches::srcface() {
    if (src.size())
        return src[0];
    return NULL;
}

/*
   1.5 Convenience function: Return the first destination face
   of the couple or NULL.

*/
Face* Matches::dstface() {
    if (dst.size())
        return dst[0];
    return NULL;
}
