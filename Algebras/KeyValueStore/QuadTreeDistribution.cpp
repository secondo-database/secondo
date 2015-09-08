/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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

#include "QuadTreeDistribution.h"

#include <iostream>
#include <sstream>
#include <algorithm>

namespace KVS {

QuadNode::QuadNode(double x, double y, int width, int height)
    : QuadNode(0, x, y, width, height) {}

QuadNode::QuadNode(QuadNode* parent, double x, double y, int width, int height)
    : parent(parent),
      x(x),
      y(y),
      width(width),
      height(height),
      serverId(-1),
      weight(0) {
  children[0] = children[1] = children[2] = children[3] = 0;
}

bool QuadNode::isLeaf() {
  return !(children[0] || children[1] || children[2] || children[3]);
}

// does origin matter? upper left VS lower right?

/*bool QuadNode::isOverlapping(Rectangle<2>* mbb) {
  return (mbb->MinD(0) < x + width && mbb->MaxD(0) > x)
      && (mbb->MinD(1) < y + height && mbb->MaxD(1) > y);
}*/

bool QuadNode::isOverlapping(double* mbb) {
  return (mbb[0] < x + width && mbb[2] > x) &&
         (mbb[1] < y + height && mbb[3] > y);
}

QuadNode* QuadNode::get(const int& i) {
  if (children[i] == 0) {
    children[i] = new QuadNode(this, x + (i % 2 * width / 2),
                               y + (i / 2 * height / 2), width / 2, height / 2);
  }
  return children[i];
}

void QuadNode::init(QuadNode* prototype) {
  x = prototype->x;
  y = prototype->y;
  width = prototype->width;
  height = prototype->height;
  serverId = prototype->serverId;
  weight = prototype->weight;

  for (int i = 0; i < 4; ++i) {
    if (prototype->children[i]) {
      children[i] = new QuadNode(0, 0, 0, 0);
      children[i]->init(prototype->children[i]);
    }
  }
}

QuadNode* QuadNode::root() {
  QuadNode* temp = this;
  while (temp->parent != 0) {
    temp = temp->parent;
  }
  return temp;
}

int QuadNode::level() {
  int result = 1;
  QuadNode* temp = this;
  while (temp->parent != 0) {
    result++;
    temp = temp->parent;
  }
  return result;
}

int QuadNode::levels() {
  QuadNode* temp = root();
  int result = 0;
  levels(temp, 0, &result);
  return result;
}

void QuadNode::levels(QuadNode* node, const int& level, int* maxLevel) {
  if (level + 1 > *maxLevel) {
    *maxLevel = level + 1;
  }

  for (int i = 0; i < 4; ++i) {
    if (node->children[i] != 0) {
      levels(node->children[i], level + 1, maxLevel);
    }
  }
}

void QuadNode::split() {
  children[0] = new QuadNode(this, x, y, width / 2, height / 2);
  children[1] = new QuadNode(this, x + width / 2, y, width / 2, height / 2);
  children[2] = new QuadNode(this, x, y + height / 2, width / 2, height / 2);
  children[3] =
      new QuadNode(this, x + width / 2, y + height / 2, width / 2, height / 2);

  for (int i = 0; i < 4; ++i) {
    children[i]->weight = weight / 4;
    children[i]->serverId = serverId;
  }
}

void propagateUp(QuadNode* node, function<void(QuadNode*)> f) {
  f(node);
  if (node->parent != 0) {
    propagateUp(node->parent, f);
  }
}

void propagateDown(QuadNode* node, function<void(QuadNode*)> f) {
  f(node);
  for (int i = 0; i < 4; ++i) {
    if (node->children[i] != 0) {
      propagateDown(node->children[i], f);
    }
  }
}

void propagateDownB(QuadNode* node, function<bool(QuadNode*)> f) {
  if (!f(node)) {
    for (int i = 0; i < 4; ++i) {
      if (node->children[i] != 0) {
        propagateDownB(node->children[i], f);
      }
    }
  }
}

void propagateDownPost(QuadNode* node, function<void(QuadNode*)> f) {
  for (int i = 0; i < 4; ++i) {
    if (node->children[i] != 0) {
      propagateDownPost(node->children[i], f);
    }
  }

  f(node);
}

bool findDescendant(QuadNode* node, function<bool(QuadNode*)> f) {
  if (f(node)) {
    return true;
  } else {
    for (int i = 0; i < 4; ++i) {
      if (node->children[i] != 0) {
        if (findDescendant(node->children[i], f)) {
          return true;
        }
      }
    }
    return false;
  }
}

bool touching2(QuadNode* a, QuadNode* b) {
  bool temp = ((a->x <= b->x + b->width && a->x + a->width >= b->x) &&
               (a->y <= b->y + b->height && a->y + a->height >= b->y)) &&
              !((a->x + a->width == b->x || a->x == b->x + b->width) &&
                (a->y + a->height == b->y || a->y == b->y + b->height));
  return temp;
}

static const double eps = 0.001;

bool touching(QuadNode* a, QuadNode* b) {
  bool temp =
      ((a->x <= b->x + b->width && a->x + a->width >= b->x) &&
       (a->y <= b->y + b->height && a->y + a->height >= b->y)) &&
      !((a->x + a->width - b->x < eps || a->x - b->x - b->width < eps) &&
        (a->y + a->height - b->y < eps || a->y - b->y - b->height < eps));
  return temp;
}

QuadTreeDistribution::QuadTreeDistribution() : QuadTreeDistribution(10, 10, 1) {
  needsSync = true;
}

QuadTreeDistribution::QuadTreeDistribution(int initialWidth, int initialHeight,
                                           int nrServers)
    : Distribution(TYPE_QUADTREE),
      root(0),
      initialWidth(initialWidth),
      initialHeight(initialHeight) {
  needsSync = true;
  serverIdMapping.assign(nrServers, 0);
  serverWeight.assign(nrServers, 0);
}

QuadTreeDistribution::QuadTreeDistribution(const QuadTreeDistribution& qtd)
    : Distribution(TYPE_QUADTREE, qtd.serverIdMapping, qtd.serverWeight),
      root(qtd.root),
      initialWidth(qtd.initialWidth),
      initialHeight(qtd.initialHeight) {
  needsSync = true;
}

QuadTreeDistribution::~QuadTreeDistribution() {
  // delete root and children...
}

string QuadTreeDistribution::toBin() {
  stringstream data;
  data.write((char*)&type, sizeof(type));
  data.write((char*)&initialWidth, sizeof(initialWidth));
  data.write((char*)&initialHeight, sizeof(initialHeight));

  unsigned int listLen = serverIdMapping.size();
  data.write((char*)&listLen, sizeof(listLen));
  for (unsigned int i = 0; i < listLen; ++i) {
    int tempData = serverIdMapping[i];
    data.write((char*)&tempData, sizeof(tempData));
  }

  data << quadnodeToBin(root);

  return data.str();
}

string QuadTreeDistribution::quadnodeToBin(QuadNode* node) {
  stringstream data;
  bool available = false;

  if (node == 0) {
    data.write((char*)&available, sizeof(available));
  } else {
    available = true;
    data.write((char*)&available, sizeof(available));

    data.write((char*)&node->x, sizeof(node->x));
    data.write((char*)&node->y, sizeof(node->y));
    data.write((char*)&node->width, sizeof(node->width));
    data.write((char*)&node->height, sizeof(node->height));
    data.write((char*)&node->weight, sizeof(node->weight));
    data.write((char*)&node->serverId, sizeof(node->serverId));
    data << quadnodeToBin(node->children[0]);
    data << quadnodeToBin(node->children[1]);
    data << quadnodeToBin(node->children[2]);
    data << quadnodeToBin(node->children[3]);
  }

  return data.str();
}

bool QuadTreeDistribution::fromBin(const string& data) {
  stringstream dataStream(data);

  int tempType = -1;
  dataStream.read((char*)&tempType, sizeof(tempType));

  if (tempType == type) {
    dataStream.read((char*)&initialWidth, sizeof(initialWidth));
    dataStream.read((char*)&initialHeight, sizeof(initialHeight));

    serverIdMapping.clear();

    unsigned int listLen = 0;
    dataStream.read((char*)&listLen, sizeof(listLen));
    for (unsigned int i = 0; i < listLen; ++i) {
      int tempData = 0;
      dataStream.read((char*)&tempData, sizeof(tempData));
      serverIdMapping.push_back(tempData);
    }

    delete root;
    root = quadnodeFromBin(dataStream, 0);

    serverWeight.resize(serverIdMapping.size());
    updateWeightVector();

    return true;
  } else {
    return false;
  }
}

QuadNode* QuadTreeDistribution::quadnodeFromBin(stringstream& data,
                                                QuadNode* parent) {
  bool available = false;

  data.read((char*)&available, sizeof(available));

  if (available) {
    double x = 0;
    double y = 0;
    int width = 0;
    int height = 0;
    int weight = 0;
    int serverId = -1;

    data.read((char*)&x, sizeof(x));
    data.read((char*)&y, sizeof(y));
    data.read((char*)&width, sizeof(width));
    data.read((char*)&height, sizeof(height));
    data.read((char*)&weight, sizeof(weight));
    data.read((char*)&serverId, sizeof(serverId));

    QuadNode* temp = new QuadNode(parent, x, y, width, height);
    temp->weight = weight;
    temp->serverId = serverId;
    for (int i = 0; i < 4; ++i) {
      temp->children[i] = quadnodeFromBin(data, temp);
    }
    return temp;
  } else {
    return 0;
  }
}

void QuadTreeDistribution::init(QuadTreeDistribution* prototype) {
  initialWidth = prototype->initialWidth;
  initialHeight = prototype->initialHeight;

  serverWeight.clear();
  serverWeight.assign(prototype->serverWeight.begin(),
                      prototype->serverWeight.end());

  serverIdMapping.clear();
  serverIdMapping.assign(prototype->serverIdMapping.begin(),
                         prototype->serverIdMapping.end());

  if (prototype->root) {
    // root = new QuadNode(prototype->root->x, prototype->root->y,
    // prototype->root->width, prototype->root->height);
    root = new QuadNode(0, 0, 0, 0);
    root->init(prototype->root);
  }
}

void QuadTreeDistribution::resetWeight() {
  if (root) {
    propagateDown(root, [](QuadNode* node) { node->weight = 0; });

    serverWeight.assign(serverWeight.size(), 0);
  }
}

void QuadTreeDistribution::updateWeightVector() {
  if (root) {
    serverWeight.assign(serverWeight.size(), 0);

    propagateDown(root, [this](QuadNode* node) {
      if (node->isLeaf()) {
        if (node->serverId > -1) {
          this->serverWeight[node->serverId] += node->weight;
        }
      }
    });
  }
}

void QuadTreeDistribution::addWeight(Distribution* dist, const int& id) {
  QuadTreeDistribution* qtd = static_cast<QuadTreeDistribution*>(dist);

  addWeightNode(root, qtd->root, id);
}

void QuadTreeDistribution::addWeightNode(QuadNode* base, QuadNode* add,
                                         const int& id) {
  if (base->serverId == id && add->serverId == id) {
    base->weight += add->weight;
  }

  for (int childIdx = 0; childIdx < 4; ++childIdx) {
    if (base->children[childIdx] && add->children[childIdx]) {
      addWeightNode(base->children[childIdx], add->children[childIdx], id);
    }
  }
}

string QuadTreeDistribution::serverIdAssignment(string attributeName,
                                                string distributionName) {
  string result("extendstream[" + attributeName + ": kvsServerId('" +
                distributionName + "', bbox(.GeoData), TRUE)] ");

  return result;
}

int QuadTreeDistribution::nextServerId() {
  unsigned int id = serverIdMapping.size();
  while (find(serverIdMapping.begin(), serverIdMapping.end(), id) !=
         serverIdMapping.end()) {
    id++;
  }
  return id;
}

// left first
int QuadTreeDistribution::neighbourId(int id) {
  vector<int>::iterator idPosition =
      find(serverIdMapping.begin(), serverIdMapping.end(), id);

  if (idPosition != serverIdMapping.end()) {
    if (idPosition == serverIdMapping.begin()) {
      if ((idPosition + 1) != serverIdMapping.end()) {
        return *(idPosition + 1);
      } else {
        return -1;
      }
    } else {
      return *(idPosition - 1);
    }
  } else {
    return -1;
  }
}

void QuadTreeDistribution::changeServerId(int oldid, int newid) {
  vector<int>::iterator idPos =
      find(serverIdMapping.begin(), serverIdMapping.end(), oldid);

  if (idPos != serverIdMapping.end()) {
    serverIdMapping.erase(idPos);
  }

  propagateDown(root, [oldid, newid](QuadNode* node) {
    if (node->serverId == oldid) {
      node->serverId = newid;
    }
  });
}

int QuadTreeDistribution::pointId(double x, double y) {
  int result = -1;

  propagateDownB(root, [x, y, &result](QuadNode* node) -> bool {
    if ((node->x <= x && x <= node->x + node->width) &&
        (node->y <= y && y <= node->y + node->height)) {
      if (node->isLeaf()) {
        result = node->serverId;

        return true;  // stop node found
      }
      return false;  // continue search
    } else {
      return true;  // stop
    }
  });

  return result;
}

void QuadTreeDistribution::fixNodeServerIds() {
  // assign server numbers for non leafs
  propagateDownPost(root, [](QuadNode* node) {
    int serverId = -2;
    int assignments = 0;

    for (int i = 0; i < 4; ++i) {
      if (node->children[i] != 0 && node->children[i]->serverId != serverId) {
        serverId = node->children[i]->serverId;
        assignments++;
      }
    }

    if (assignments == 1) {
      node->serverId = serverId;
    } else if (assignments > 1) {
      node->serverId = -1;
    }
  });
}

void QuadTreeDistribution::expand(double* mbb) {
  int leftright = -1;
  int updown = -1;
  int moveleft = 0;
  int moveup = 0;

  // left right?
  if (root->x > mbb[0]) {
    leftright = 1;
    moveleft = -root->width;
  } else if ((root->x + root->width) < mbb[0]) {
    leftright = 0;
  }
  // up down?
  if (root->y > mbb[1]) {
    updown = 2;
    moveup = -root->height;
  } else if ((root->y + root->height) < mbb[1]) {
    updown = 0;
  }

  if (leftright > -1 || updown > -1) {
    // decide by center
    if (leftright == -1) {
      if ((root->x + root->width / 2) < ((mbb[2] - mbb[0]) / 2)) {
        leftright = 0;
      } else {
        leftright = 1;
        moveleft = -root->width;
      }
    } else if (updown == -1) {
      if ((root->y + root->height / 2) < ((mbb[3] - mbb[1]) / 2)) {
        updown = 0;
      } else {
        updown = 2;
        moveup = -root->height;
      }
    }

    // expand
    QuadNode* temp = new QuadNode(root->x + moveleft, root->y + moveup,
                                  root->width * 2, root->height * 2);
    temp->children[leftright + updown] = root;
    root->parent = temp;
    root = temp;

    distributionUpdated();
    // recursive
    expand(mbb);
  }
}

void QuadTreeDistribution::insert(QuadNode* node, double* mbb,
                                  set<int>* results) {
  if (node->isLeaf() && node->width <= initialWidth) {
    propagateUp(node, [](QuadNode* node) { node->weight++; });

    if (node->serverId < 0) {
      // this might be a bit expensive : go through all leafes in cluster order,
      // when we reach the insert node assign last used serverId
      int lastId = -1;
      QuadNode* insertNode = node;

      leafesInClusterOrder(root, [insertNode, &lastId](QuadNode* node) -> bool {
        if (node == insertNode) {
          insertNode->serverId = lastId;
        } else if (node->serverId > -1) {
          lastId = node->serverId;
        }
        return true;  // we don't want the tree to split so we always return
                      // true
      });

      if (node->serverId == -1) {
        node->serverId = 0;
      }
    }

    serverWeight[node->serverId]++;

    results->insert(node->serverId);
  } else {
    for (int i = 0; i < 4; ++i) {
      QuadNode* child = node->get(i);
      if (child->isOverlapping(mbb)) {
        insert(child, mbb, results);
      }
    }
  }
}

void QuadTreeDistribution::retrieveIds(QuadNode* node, double* mbb,
                                       set<int>* results) {
  if (node->isLeaf() && node->width <= initialWidth) {
    if (node->serverId >= 0) {
      results->insert(node->serverId);
    }
  } else {
    for (int i = 0; i < 4; ++i) {
      if (node->children[i]) {
        if (node->children[i]->isOverlapping(mbb)) {
          retrieveIds(node->children[i], mbb, results);
        }
      }
    }
  }
}

void QuadTreeDistribution::add(int value, set<int>* resultIds) {}

void QuadTreeDistribution::add(int nrcoords, double* coords,
                               set<int>* resultIds) {
  if (root == 0) {
    root = new QuadNode(coords[0], coords[1], initialWidth, initialHeight);
  }

  expand(coords);
  insert(root, coords, resultIds);
}

void QuadTreeDistribution::request(int value, set<int>* resultIds) {}

void QuadTreeDistribution::request(int nrcoords, double* coords,
                                   set<int>* resultIds) {
  if (root != 0) {
    retrieveIds(root, coords, resultIds);
  }
}

/*void QuadTreeDistribution::addServer() {
  if (root) {

    updateWeightVector();

    int newId = nextServerId();

    //always add in front of max element
    int maxId = distance(serverWeight.begin(),
        max_element(serverWeight.begin(), serverWeight.end()));

    vector<int>::iterator insertPos = serverIdMapping.insert(
        find(serverIdMapping.begin(), serverIdMapping.end(), maxId),
        newId);

    if(newId > serverWeight.size()) {
      serverWeight.resize(newId+1, 0);
    }

    int targetWeight = serverWeight[maxId] / 2;
    int currentWeight = targetWeight;

    //assign
    leafesInClusterOrder(root,
        [&targetWeight, &currentWeight, &insertPos, maxId] (QuadNode* node) ->
bool {
      if(node->serverId == maxId) {
        //arbitrary 5% margin (fewer splits)
        int margin = targetWeight*0.05;

        if(node->weight < currentWeight+margin ) {
          node->serverId = *insertPos;
          currentWeight-=node->weight;

          if(currentWeight <= 0) {
            currentWeight = targetWeight;
            insertPos++;
          }
          return true;
        }
        return false;
      }
      return true;
    });

    fixNodeServerIds();
  }
}*/

int QuadTreeDistribution::split(int serverId) {
  if (root && serverId >= 0 &&
      serverId < static_cast<int>(serverIdMapping.size())) {
    updateWeightVector();

    int idx = nextServerId();

    vector<int>::iterator insertPos = serverIdMapping.insert(
        find(serverIdMapping.begin(), serverIdMapping.end(), serverId), idx);
    serverWeight.push_back(0);

    int targetWeight = serverWeight[serverId] / 2;
    int currentWeight = targetWeight;

    // assign
    leafesInClusterOrder(root, [&targetWeight, &currentWeight, &insertPos,
                                serverId](QuadNode* node) -> bool {
      if (node->serverId == serverId) {
        // arbitrary 5% margin (fewer splits)
        int margin = targetWeight * 0.05;

        if (node->weight < currentWeight + margin) {
          node->serverId = *insertPos;
          currentWeight -= node->weight;

          if (currentWeight <= 0) {
            currentWeight = targetWeight;
            insertPos++;
          }
          return true;
        }
        return false;
      }
      return true;
    });

    fixNodeServerIds();
    return idx;
  }

  return -1;
}

// n: number of servers after serverId (referencing serverIdMapping order)
void QuadTreeDistribution::redistribute(int serverId, int n) {
  if (root && serverId >= 0 &&
      serverId < static_cast<int>(serverIdMapping.size())) {
    updateWeightVector();

    vector<int>::iterator currentPos =
        find(serverIdMapping.begin(), serverIdMapping.end(), serverId);
    set<int> idSet;

    int targetWeight = 0;
    for (int i = 0; i < n; ++i) {
      idSet.insert(*(currentPos + i));
      targetWeight += serverWeight[*(currentPos + i)];
    }
    targetWeight = targetWeight / n;

    int currentWeight = targetWeight;

    // assign
    leafesInClusterOrder(root, [&targetWeight, &currentWeight, &currentPos,
                                &idSet](QuadNode* node) -> bool {
      if (idSet.find(node->serverId) != idSet.end()) {
        // arbitrary 5% margin (fewer splits)
        int margin = targetWeight * 0.05;

        if (node->weight < currentWeight + margin) {
          node->serverId = *currentPos;
          currentWeight -= node->weight;

          if (currentWeight <= 0) {
            currentWeight = targetWeight;
            currentPos++;
          }
          return true;
        }
        return false;
      }
      return true;
    });

    fixNodeServerIds();
  }
}

/*void QuadTreeDistribution::removeServer(int serverId) {
  if (root) {

    if (serverIdMapping.size() > 1) {
      updateWeightVector();

      vector<int>::iterator item = find(serverIdMapping.begin(),
          serverIdMapping.end(), serverId);
      vector<int>::iterator startIdx;
      //assign everything to one server
      int targetWeight = serverWeight[*item] * 1.05;
      int currentWeight = targetWeight;

      if (item == serverIdMapping.begin()) {
        //item +1 100%
        startIdx = item;
      } else if (item + 1 == serverIdMapping.end()) {
        //item -1 100%
        startIdx = item - 1;
      } else {
        startIdx = item - 1;

        //for first server
        currentWeight = (serverWeight[*(item - 1)] + serverWeight[*item]
                                      + serverWeight[*(item + 1)]) / 2
                                          - serverWeight[*(item - 1)];

        if (currentWeight < 0) {
          startIdx = item + 1;
        }
      }

      serverWeight[*item] = 0;
      serverIdMapping.erase(item);

      //assign
      leafesInClusterOrder(root,
          [&targetWeight, &currentWeight, &startIdx, serverId] (QuadNode* node)
-> bool {
        if(node->serverId == serverId) {
          //arbitrary 5% margin (fewer splits)
          int margin = targetWeight*0.05;

          if(node->weight < currentWeight+margin ) {
            node->serverId = *startIdx;
            currentWeight-=node->weight;

            if(currentWeight <= 0) {
              currentWeight = targetWeight;
              startIdx++;
            }
            return true;
          }

          return false;
        }
        return true;
      });

      //lower ids
      leafesInClusterOrder(root, [serverId] (QuadNode* node) -> bool {
        if(node->serverId > serverId) {
          node->serverId--;
        }
        return true;
      });

      for (unsigned int i = 0; i < serverIdMapping.size(); ++i) {
        if (serverIdMapping[i] > serverId) {
          serverIdMapping[i]--;
        }
      }

      fixNodeServerIds();
    }
  }
}*/

void QuadTreeDistribution::redistributeCluster() {
  if (root) {
    // reset server assignment
    propagateDown(root, [](QuadNode* node) { node->serverId = -1; });

    int weightcheck = 0;

    propagateDown(root, [&weightcheck](QuadNode* node) {
      if (node->isLeaf()) {
        weightcheck += node->weight;
      }
    });

    cout << "root->weight: " << root->weight
         << "vs weightcheck: " << weightcheck << "\n";

    // assign server numbers for all leafs
    int targetWeight = (weightcheck / serverIdMapping.size());
    int currentWeight = targetWeight;
    vector<int>::iterator currentServerId(serverIdMapping.begin());

    cout << "targetWeight:" << targetWeight << "\n";

    leafesInClusterOrder(root, [&targetWeight, &currentWeight,
                                &currentServerId](QuadNode* node) -> bool {
      // arbitrary 5% margin (fewer splits)
      int margin = targetWeight * 0.05;

      if (node->weight < currentWeight + margin) {
        node->serverId = *currentServerId;
        currentWeight -= node->weight;

        if (currentWeight <= 0) {
          currentWeight = targetWeight;
          currentServerId++;
          cout << "Switching to next server:" << *currentServerId << "\n";
        }
        return true;
      }
      return false;  // split
    });

    fixNodeServerIds();

    resetWeight();
  }
}

void QuadTreeDistribution::leafesInClusterOrder(QuadNode* node,
                                                function<bool(QuadNode*)> f) {
  // default (clockwise): upper left -> upper right -> lower right -> lower left
  QuadNode* lastVisited;

  if (node->isLeaf()) {
    f(node);
  } else {
    if (node->children[0] != 0) {
      leafesInClusterOrderR(node->children[0], 0, 1, &lastVisited, f);
    }

    if (node->children[1] != 0) {
      leafesInClusterOrderR(node->children[1], 1, 3, &lastVisited, f);
    }

    if (node->children[3] != 0) {
      leafesInClusterOrderR(node->children[3], 3, 2, &lastVisited, f);
    }

    if (node->children[2] != 0) {
      leafesInClusterOrderR(node->children[2], 2, 0, &lastVisited, f);
    }
  }
}

// Turns out this might be doing something similar to the "Hilbert-Curve"
// so there probably exists a more efficient algorithm but i don't have time to
// check or implement it

// f returning false leads to split
void QuadTreeDistribution::leafesInClusterOrderR(QuadNode* node,
                                                 const int& currentIdx,
                                                 const int& nextIdx,
                                                 QuadNode** lastVisited,
                                                 function<bool(QuadNode*)> f) {
  if (node->isLeaf()) {
    if (f(node)) {
      *lastVisited = node;
      return;
    } else {
      node->split();
    }
  }

  // determine where we have to start the "loop" if we want to stay in "touch"
  int defaultPath[4] = {0, 1, 3, 2};
  int endPoint[2] = {0, 0};

  int startPoint = 0;

  if (*lastVisited != 0) {
    for (int i = 0; i < 4; ++i) {
      if (node->children[i] && touching(node->children[i], *lastVisited)) {
        startPoint = i;
        break;
      }
    }
  }

  if (abs(currentIdx - nextIdx) == 1) {
    // horizontal movement
    endPoint[0] = nextIdx;
    endPoint[1] = (nextIdx + 2) % 4;
  } else {
    // vertical movement
    endPoint[0] = (nextIdx / 2) * 2;
    endPoint[1] = endPoint[0] + 1;
  }

  // determine start position and direction
  int pathIdx = 0;
  while (defaultPath[pathIdx] != startPoint) {
    pathIdx++;
  }

  int step = 1;
  if (defaultPath[(pathIdx + 1) % 4] == endPoint[0] ||
      defaultPath[(pathIdx + 1) % 4] == endPoint[1]) {
    step = -1;
  }

  pathIdx += 4;

  // process children
  for (int i = 0; i < 4; ++i) {
    int childNr = defaultPath[(pathIdx + (i * step)) % 4];
    if (node->children[childNr] != 0) {
      if (i == 3) {
        // change direction at the end (following level above)
        leafesInClusterOrderR(node->children[childNr], currentIdx, nextIdx,
                              lastVisited, f);
      } else {
        leafesInClusterOrderR(
            node->children[childNr], defaultPath[(pathIdx + (i * step)) % 4],
            defaultPath[(pathIdx + ((i + 1) * step)) % 4], lastVisited, f);
      }
    }
  }
}
}
