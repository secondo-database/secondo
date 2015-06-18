/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2004-2007, University in Hagen, Department of Computer Science,
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

#ifndef TIN_H_
#define TIN_H_
#include "TinHelper.h"
#include "TinPart.h"
#include "AlgoFortune.h"
#include <vector>
#include <map>
#include <deque>
#include <stack>
#include "SecondoDependencies.h"
#include <TinLogging.h>
#include <string>
using std::vector;
using std::map;
using std::deque;

namespace tin {

class EventQueue;
class TinAttribute;

class ColumnKey {
private:
VERTEX_COORDINATE xFrom;
VERTEX_COORDINATE xTo;
mutable uint32_t noPart;
public:
ColumnKey(VERTEX_COORDINATE ixfrom, VERTEX_COORDINATE ixto,
uint32_t inoPart);
ColumnKey(VERTEX_COORDINATE ix, uint32_t inoPart);
ColumnKey() {
}
;
~ColumnKey();

uint32_t getNoPart() const {
return noPart;
}
void setNoPart(uint32_t inoPart) const {
noPart = inoPart;
}
bool operator<(const ColumnKey & cpart) const;
};

#ifndef UNIT_TEST
class Tin: public noncopyable
#else
class Tin: public noncopyable
#endif
{
#ifndef UNIT_TEST
friend class TinAttribute;
#endif
friend class EventQueue;

protected:
deque<TinPart*> tinParts;
TinConfiguration config;
AbstractType tinTypeCurrent;
TinFeatures features;
TIN_SIZE noParts;
#ifndef UNIT_TEST
mutable SmiRecordFile file;
SmiRecordId contentId;
R_Tree<2, uint32_t>* rtree;
#endif
map<ColumnKey, TinPart*> columnMap;
EventQueue* constructionQueue;
bool defined;
protected:
////////Construction/Destruction/////////////////////////////////
#ifndef UNIT_TEST
Tin() :
file(false) {
LOGP
noParts = 0;
constructionQueue = 0;
config = TinConfiguration::DEFAULT;
rtree = 0;
defined = false;

LOGP}
;

#else
Tin() {
constructionQueue = 0;
noParts=0;
};
#endif

Tin * clone() const;
void cloneTin(Tin * result) const;

public:
Tin(VertexContainerSet * vertices,
const TinConfiguration &conf = TinConfiguration::DEFAULT);
Tin(const TinConfiguration & conf);
~Tin();
////////Manipulation/////////////////////////////////////////////
public:
void unaryOp(VERTEX_Z (*op)(VERTEX_Z z));
void setDefined(bool def = true) {
defined = def;
}
bool resetConfiguration(const TinConfiguration& conf) {
if(noParts != 0)
return false;

config = conf;

return true;
}
protected:

void initSimpleLayout();
void initColumnLayout(VertexContainerSet & vc);
void finishLayout(bool updatePartsFeatures = true);

void triangulateSection(VertexContainerSet * vc);
void finishTriangulation();
void triangulateBulk(VertexContainerSet * vc);

void addTriangle(const Vertex & v1,const Vertex & v2,const Vertex & v3,
Triangle** newtriangle);
void addTriangle_p(const Vertex & v1,const Vertex & v2,const Vertex & v3,
Triangle** newtriangle);

void addPart(TinPart * p);
TinPart * getNewPart();
void finishPart(std::pair<const ColumnKey,TinPart*>& partpair,
bool updatePartFeat = false);

#ifndef UNIT_TEST
void unaryOp(void* function,Tin * result);
#endif

///////Query/////////////////////////////////////////////////////
public:
bool isDefined()const {
return defined;
}
class triangle_iterator {
protected:
Tin* tin;
deque<TinPart*>::iterator itParts;
TinPart::iterator itTriangles;
int currentPart;

protected:
triangle_iterator() {
tin = 0;
currentPart = 0;
}
public:

triangle_iterator(Tin* itin) {
tin = itin;
currentPart = 0;
itParts = tin->tinParts.begin();
if (itParts != tin->tinParts.end())
itTriangles = (*itParts)->begin();
}
Triangle* operator*() {
return (*itTriangles);
}
Tin::triangle_iterator& operator++(int) {
itTriangles++;
if (*itTriangles)
return *this;
else {

if (itParts != tin->tinParts.end())
(*itParts)->unloadContentData();

itParts++;
currentPart++;
if (itParts != tin->tinParts.end())
itTriangles = (*itParts)->begin();

return *this;
}
}
int getCurrentPart() {
return currentPart;
}
};

Tin::triangle_iterator begin() {
return Tin::triangle_iterator(this);
}
Triangle::triangleWalker getWalker(const Point_p & p);

VERTEX_Z atlocation(const Point_p& p);
const Rectangle& bbox() const;
VERTEX_Z minimum() const;
VERTEX_Z maximum() const;

MemoryState getMemoryState()const
{
return config.memoryState;
}
TIN_SIZE getSizeInMemory()
{
//ATTENTION this can only be an estimate,
//since STL containers do not expose their size
deque<TinPart *>::iterator it = tinParts.begin();
TIN_SIZE size = 0;

size += sizeof(*this);

//if(rtree)
//size +=rtree->???

//columnMap neglected as well as incompleteParts

while (it != tinParts.end()) {
LOGP
size +=(*it)->getSizeInMemory();
++it;
}

return size;
}
TIN_SIZE estimateMaxSizeInMemory(int inoParts = -1)const
{
TIN_SIZE size = 0;
//only a rough estimate !
size += sizeof(*this);
if(inoParts == -1)
size +=noParts * TinPart::estimateMaxSizeInMemory(config);
else
size +=inoParts * TinPart::estimateMaxSizeInMemory(config);

return size;

}

const TinConfiguration& getConfiguration()const
{
return config;
}

Triangle* findNeighbor(const Edge& commonEdge, Triangle* caller);
bool hasTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
bool checkDelaunay();
bool checkNeighborRelations();

protected:
TinPart * getNextPartIntersecting(Rectangle& bbox);
double calculateNoColumns(VertexContainerSet & vc);
double calculateNoRows(VertexContainerSet & vc);
bool canAddTriangle(const Triangle & t, std::string & msg);

///////Presentation//////////////////////////////////////////////
public:
void print(std::ostream& os = std::cout);
void saveSTLFile(std::ostream& os = std::cout);
///////Persistence///////////////////////////////////////////////
public:
TIN_SIZE getPartHeadOrigin()const;
protected:
void setMemoryState(MemoryState st);
void setMemoryStateGradual();
TIN_SIZE getSizeOnDisc() const;
void unloadAllParts();
void loadAllParts();

#ifndef UNIT_TEST
void deleteFile();
bool open(SmiRecord& valueRecord,bool bypass = false,
SmiFileId ifileid = 0, SmiRecordId irec = 0);
bool openParts(bool bulkload = false);
TinPart * getPartFromDisc(SmiRecord & valueRecord,bool bulkload = false);
TinPart * getPartFromDisc(uint32_t idx);
public:
bool save(SmiRecord& valueRecord,bool bypass =false);
SmiRecordFile* getFile();
#endif

/////////////////////////////////////////////////////////////////

#ifndef UNIT_TEST
///////Secondo atom type required functions//////////////////////
static void parseInTinConfig(const ListExpr confExp,
TinConfiguration& conf);

static std::string BasicType();
static ListExpr Property();
static Word Create(const ListExpr typeInfo);
static void Delete(const ListExpr typeInfo, Word & w);
static ListExpr Out(ListExpr typeInfo, Word value);
static Word In(const ListExpr typeInfo, const ListExpr instance,
const int errorPos, ListExpr& errorInfo, bool& correct);
static void Close(const ListExpr typeInfo, Word & w);
static bool Open(SmiRecord& valueRecord, size_t& offset,
const ListExpr typeInfo, Word& value);
static bool Save(SmiRecord& valueRecord, size_t& offset,
const ListExpr typeInfo, Word& value);
static Word Clone(const ListExpr typeInfo, const Word & w);
static int sizeOfObj();
static bool CheckType(ListExpr type, ListExpr & errorInfo) {
return listutils::isSymbol(type,Tin::BasicType());
}
/////Operators///////////////////////////////////////////////////

static ListExpr unaryOp_tm(ListExpr args);
static int unaryOp_vm(Word* args, Word& result, int message, Word& local,
Supplier s);

static ListExpr tin2stlfile_tm(ListExpr args);
static int tin2stlfile_vm(Word* args, Word& result, int message, Word& local,
Supplier s);

static ListExpr atlocation_tm(ListExpr args);
static int atlocation_sf(ListExpr args);
static int atlocation_vm(Word* args, Word& result, int message, Word& local,
Supplier s);

static ListExpr tinmin_tm(ListExpr args);
static int tinmin_sf(ListExpr args);
static int tinmin_vm(Word* args, Word& result, int message, Word& local,
Supplier s);

static ListExpr tinmax_tm(ListExpr args);
static int tinmax_sf(ListExpr args);
static int tinmax_vm(Word* args, Word& result, int message, Word& local,
Supplier s);

static ListExpr createTin_tm(ListExpr args);
static int createTin_sf(ListExpr args);
template <typename N>
static int createTin_vm(Word* args, Word& result, int message, Word& local,
Supplier s);

static ListExpr tin2tuplestream_tm(ListExpr args);
static int tin2tuplestream_vm(Word* args, Word& result, int message,
Word& local, Supplier s);

static ListExpr tin2tinattribute_tm(ListExpr args);
static int tin2tinattribute_vm(Word* args, Word& result, int message,
Word& local, Supplier s);

static ListExpr raster2tin_tm(ListExpr args);
static int raster2tin_sf(ListExpr args);
template<class T>
static int raster2tin_vm(Word* args, Word& result, int message,
Word& local, Supplier s);
/////////////////////////////////////////////////////////////////

#endif

};

#ifndef UNIT_TEST
class TinAttribute: public Attribute, public TinPart {
protected:
Flob binData;
public:
TinAttribute() :
TinPart(true) {
}
TinAttribute(bool defined) :
Attribute(defined), binData(0) {
config = TinConfiguration::ATTRIBUTE;
}
TinAttribute(TinPart & part);
TinPart* clone(Tin*tt);
Triangle* getNeighbor(const Edge& commonEdge, Triangle* caller);
bool isContentDataLoaded() const {
return true;
}
void loadContentData() {
}
void unloadContentData() {
}
void deleteContentReference() {
}
void addContentReference() {
}
TIN_SIZE getSizeOnDisc() const {
LOGP
TIN_SIZE size = getSizeOnDisc_content();
LOGP
return size;
}
static TIN_SIZE getSizeOnDisc_head() {
LOGP
TIN_SIZE size = sizeof(TIN_SIZE) + sizeof(TIN_SIZE)
+ TinFeatures::getSizeOnDisc() + sizeof(uint8_t);
return size;
}
bool save(SmiRecord& valueRecord) {
throw std::runtime_error("Not implemented for TinAttribute.");
}
void serialize(char* storage, size_t& offset) {
throw std::runtime_error("Not implemented for TinAttribute.");
}
void rebuild(char* state, size_t& offset) {
throw std::runtime_error("Not implemented for TinAttribute.");
}

protected:

void save_content() {
throw std::runtime_error("Not implemented for TinAttribute.");
}
void open_content() {
throw std::runtime_error("Not implemented for TinAttribute.");
}

bool open(SmiRecord& valueRecord) {
throw std::runtime_error("Not implemented for TinAttribute.");
}
bool open_head(SmiRecord& valueRecord, uint32_t idx) {
throw std::runtime_error("Not implemented for TinAttribute.");
}

/////Secondo attribute type required methods/////////////////////
public:

size_t Sizeof() const {
LOGP
return sizeof(*this);
}
int Compare(const Attribute *rhs) const {
TinAttribute * attr = (TinAttribute *) rhs;
bool tok = false;
bool tinisequal = true;
const Vertex *av , *tv;

if(!this->IsDefined() || !attr->IsDefined())
return 1;

//if rhs has more or less triangles -> unequal
//the tin with more triangles is bigger
if(this->noTriangles < attr->noTriangles)
return -1;
if(this->noTriangles > attr->noTriangles)
return 1;

const_cast<TinAttribute * >(this)->syncFlob(false);
const_cast<TinAttribute * >(attr)->syncFlob(false);

//bbox check -> unequal
if(this->features == attr->features)
{
//compare all triangles of this with rhs

for(uint32_t ithis = 0; ithis<this->noTriangles; ithis++)
{

tok = false;

for(uint32_t iattr = 0; iattr< attr->noTriangles; iattr++)
{
if(attr->arTriangles[iattr] == this->arTriangles[ithis])
{
tok = true;
break;
}

}

if(!tok)
{
//if one is missing in rhs -> unequal
tinisequal = false;
break;
}

}

}
else
tinisequal = false;

if(tinisequal)
return 0;

//unequal then order defined by the first number deviating
for(uint32_t index = 0; index<this->noTriangles; index++)
{

for(int vert = 1; vert<4; vert++)
{
av = attr->arTriangles[index].getVertex(vert);
tv = this->arTriangles[index].getVertex(vert);

if(av->getY()!=tv->getY())
{
if(av->getY()<tv->getY())
return 1;
else
return -1;
} else if(av->getX()!=tv->getX())
{
if(av->getX()<tv->getX())
return 1;
else
return -1;
} else if(av->getZ()!=tv->getZ())
{
if(av->getZ()<tv->getZ())
return 1;
else
return -1;
}
}

}

throw std::runtime_error("This line"
" should never be reached.(TinAttribute::Compare)");
}
Attribute * Clone() const {
LOGP
TinAttribute* newattr = new TinAttribute(false);
bool n;
const Vertex * v1, *v2, *v3;
LOGP

const_cast<TinAttribute *>(this)->syncFlob(false);

if (IsDefined()) {
newattr->config = config;
newattr->noTriangles = 0;
newattr->features = features;
newattr->ismodified = true;
newattr->pVertexContainer = pVertexContainer->clone_empty();
newattr->arTriangles = new Triangle[noTriangles];

for (int i = 0; i < noTriangles; i++) {
v1 = newattr->pVertexContainer->insertVertex(arTriangles[i].getVertex(1),
n);
v2 = newattr->pVertexContainer->insertVertex(arTriangles[i].getVertex(2),
n);
v3 = newattr->pVertexContainer->insertVertex(arTriangles[i].getVertex(3),
n);
newattr->constructTriangle(v1, v2, v3);
newattr->noTriangles++;
LOGP}
newattr->noVertices = newattr->pVertexContainer->getNoVertices();
newattr->contentRefs = 0;
newattr->noTrianglesMax = noTriangles;
newattr->SetDefined(true);
} else {
newattr->SetDefined(false);
}

newattr->syncFlob(true);
return newattr;
}
size_t HashValue() const {
return (uint)(features.m_maxValue-features.m_minValue);
}
void CopyFrom(const Attribute* right) {
LOGP

Attribute * attr1 = const_cast<Attribute *>(right);
TinAttribute * attr = static_cast<TinAttribute *>(attr1);

attr->syncFlob(false);

bool n;
const Vertex * v1, *v2, *v3;
LOGP

this->noTriangles = 0;
this->features = attr->features;
this->ismodified = true;
this->pVertexContainer = attr->pVertexContainer->clone_empty();
this->arTriangles = new Triangle[attr->noTriangles];

for (int i = 0; i < attr->noTriangles; i++) {
v1 = this->pVertexContainer->insertVertex(
attr->arTriangles[i].getVertex(1), n);
v2 = this->pVertexContainer->insertVertex(
attr->arTriangles[i].getVertex(2), n);
v3 = this->pVertexContainer->insertVertex(
attr->arTriangles[i].getVertex(3), n);
this->constructTriangle(v1, v2, v3);
this->noTriangles++;
LOGP}
this->noVertices = this->pVertexContainer->getNoVertices();
this->contentRefs = 0;
this->noTrianglesMax = attr->noTriangles;

this->syncFlob(true);

return;
}
inline int NumOfFLOBs() const {
LOGP
return 1;
}
inline virtual Flob* GetFLOB(const int i) {
LOGP
return &binData;
}
bool Adjacent(const Attribute *attrib) const {
return true;
}

static bool Open(SmiRecord& valueRecord, size_t& offset,
const ListExpr typeInfo, Word& value);
static bool Save(SmiRecord& valueRecord, size_t& offset,
const ListExpr typeInfo, Word& value);
static void* Cast(void * addr) {
LOGP
return new (addr) TinAttribute();
}
static std::string BasicType();
static bool CheckType(ListExpr type, ListExpr & errorInfo) {
return listutils::isSymbol(type, TinAttribute::BasicType());
}
static ListExpr Property();
static Word Create(const ListExpr typeInfo);
static void Delete(const ListExpr typeInfo, Word & w);
static ListExpr Out(ListExpr typeInfo, Word value);
static Word In(const ListExpr typeInfo, const ListExpr instance,
const int errorPos, ListExpr& errorInfo, bool& correct);
static int sizeOfObj();
static void Close(const ListExpr typeInfo, Word & w);
static Word Clone(const ListExpr typeInfo, const Word & w);

/////Operators///////////////////////////////////////////////////
static ListExpr tinattribute2tin_tm(ListExpr args);
static int tinattribute2tin_sf(ListExpr args);
static int tinattribute2tin_vm(Word* args, Word& result, int message,
Word& local, Supplier s);

static int atlocation_vm(Word* args, Word& result, int message,
Word& local, Supplier s);
static int tinmin_vm(Word* args, Word& result, int message, Word& local,
Supplier s);
static int tinmax_vm(Word* args, Word& result, int message, Word& local,
Supplier s);
/////////////////////////////////////////////////////////////////

protected:
void setMemoryState(MemoryState st) {
}
void Serialize2Flob();
void RebuildFromFlob();
void syncFlob(bool toFlob) {
if (toFlob) {
Serialize2Flob();
} else {
RebuildFromFlob();
}

}
};
#endif

#ifndef UNIT_TEST
const string map_atlocation[2][4] = { { Tin::BasicType(),
CcReal::BasicType(), CcReal::BasicType(), CcReal::BasicType() }, {
TinAttribute::BasicType(), CcReal::BasicType(), CcReal::BasicType(),
CcReal::BasicType() } };

const string map_min[2][2] = { { Tin::BasicType(), CcReal::BasicType() }, {
TinAttribute::BasicType(), CcReal::BasicType() } };
const string map_max[2][2] = { { Tin::BasicType(), CcReal::BasicType() }, {
TinAttribute::BasicType(), CcReal::BasicType() } };
template<typename N>
int Tin::createTin_vm(Word* args, Word& result, int message, Word& local,
Supplier s) {
LOGP
Stream<Tuple> stream(args[0]);
CcInt * partSize = (CcInt*) args[1].addr;
Vertex v, priorv;
Attribute* aPoint, *aHeightVal;

VertexContainerSet* vc = new VertexContainerSet(VERTEX_CONTAINER_BIG_SIZE);

result = qp->ResultStorage(s);
Tin* resultTin = (Tin*) result.addr;

if (partSize)
resultTin->config.maxSizePart = partSize->GetIntval();
else
resultTin->config.maxSizePart = TIN_PART_STANDARD_SIZE;

try {

if (!args[0].addr)
throw std::runtime_error("Null pointer as input.(Tin::createTin_vm)");

resultTin->setMemoryStateGradual();

stream.open();

Tuple * tup;

if ((tup = stream.request()) != 0) {
aPoint = tup->GetAttribute(0);
aHeightVal = tup->GetAttribute(1);

v.setX(((::Point*) aPoint)->GetX());
v.setY(((::Point*) aPoint)->GetY());
v.setZ(((N*) aHeightVal)->GetValue());

vc->insertVertex_p(&v);
priorv = v;

tup->DeleteIfAllowed();

}

while (((tup = stream.request()) != 0)) {
aPoint = tup->GetAttribute(0);
aHeightVal = tup->GetAttribute(1);
LOGP
v.setX(((::Point*) aPoint)->GetX());
v.setY(((::Point*) aPoint)->GetY());
LOGP
v.setZ(((N*) aHeightVal)->GetValue());
LOGP
LOG(v.getY())
LOG(priorv.getY())

if (v.getY() > priorv.getY())
throw std::runtime_error(E_CREATETIN_VM);

if (v.getY() != priorv.getY() && resultTin->calculateNoRows(*vc) >= 1) {
resultTin->triangulateSection(vc);
vc = new VertexContainerSet(VERTEX_CONTAINER_BIG_SIZE);
}

vc->insertVertex_p(&v);
priorv = v;

tup->DeleteIfAllowed();
}

LOGP
resultTin->triangulateSection(vc);
resultTin->finishTriangulation();

} catch (std::exception & e) {
OUT_EXCEPT(e);
stream.close();
LOGP
return -1;
}

resultTin->setDefined();
stream.close();
LOGP
return 0;
}

template<typename T>
int Tin::raster2tin_vm(Word* args, Word& result, int message, Word& local,
Supplier s) {

LOGP
result = qp->ResultStorage(s);
Tin* nt = (Tin*) result.addr;

T* p_stype = static_cast<T*>(args[0].addr);
CcInt* maxSizePart = static_cast<CcInt*>(args[1].addr);
nt->config.maxSizePart = maxSizePart->GetIntval();

try {

if (!p_stype)
throw std::runtime_error(
"Null pointer as input. Operation not possible. (Tin::raster2tin_vm)");

VertexContainerSet * vc = new VertexContainerSet(
VERTEX_CONTAINER_BIG_SIZE);

if (!p_stype->isDefined())
throw std::runtime_error(
"The input raster is undefined.(Tin::raster2tin_vm)");

VERTEX_COORDINATE grid_origin_x = p_stype->getGrid().getOriginX();
VERTEX_COORDINATE grid_origin_y = p_stype->getGrid().getOriginY();
VERTEX_COORDINATE grid_length = p_stype->getGrid().getLength();
VERTEX_COORDINATE grid_mid = p_stype->getGrid().getLength() / 2;
grid_origin_x += grid_mid;
grid_origin_y += grid_mid;

LOG(grid_origin_x)
LOG(grid_origin_y)
LOG(grid_length)

typename T::storage_type& storage = p_stype->getStorage();

for (typename T::iter_type it = storage.begin(), e = storage.end();
it != e; ++it) {
Vertex v((it.getIndex()[0]) * grid_length + grid_origin_x,
(it.getIndex()[1]) * grid_length + grid_origin_y, (VERTEX_Z) (*it));
LOG_EXP(v.print())
vc->insertVertex_p(&v);
}

if (vc->getSizeInMemory() / 4 > OPERATE_IN_MEMORY_THRESHOLD)
nt->setMemoryStateGradual();
else
nt->setMemoryState(INMEMORY);

nt->triangulateBulk(vc);

LOGP} catch (std::exception & e) {
OUT_EXCEPT(e);
return -1;
}

nt->setDefined();
LOGP
return 0;
}

#endif

} /* namespace tin */

#endif /* TINTYPE_H_ */
