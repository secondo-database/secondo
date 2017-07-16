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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] title: [{\Large \bf ]   [}]

April 2017 Michael Loris


[1] Standalone program to calculate the Earth Mover's distance

*/


#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <random>
#include "../JPEGImage.h"

#include <fstream>

double transport2(std::vector<FeatureSignatureTuple> ist1,
                    std::vector<FeatureSignatureTuple> ist2);


struct Flow
{
  double fromWeight;
  double distance;
  double toWeight;
};


double euclidDist(FeatureSignatureTuple ist1,
                    FeatureSignatureTuple ist2)
{
	double tmpRes = 0.0;
	
	tmpRes += pow(ist1.weight - ist2.weight, 2);
	tmpRes += pow(ist1.centroid.x - ist2.centroid.x, 2);
	tmpRes += pow(ist1.centroid.y - ist2.centroid.y, 2);
	tmpRes += pow(ist1.centroid.colorValue1 - ist2.centroid.colorValue1, 2);
	tmpRes += pow(ist1.centroid.colorValue2 - ist2.centroid.colorValue2, 2);
	tmpRes += pow(ist1.centroid.colorValue3 - ist2.centroid.colorValue3, 2);
	tmpRes += pow(ist1.centroid.coarseness - ist2.centroid.coarseness, 2);
	tmpRes += pow(ist1.centroid.contrast - ist2.centroid.contrast, 2);
	
	return sqrt(tmpRes);
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
        
    std::cout << argv[1] << " " << argv[2] << std::endl;
        
	double w;
    int x, y;    
    double r,g,b;
    double coa, con;
    
    std::ifstream infile(argv[1]);
    std::string std;
    int counter = 0;
    while (infile >> w >> x >> y >> r >> g >> b >> coa >> con)
    {        	
		ist1.push_back({w, x, y, r, g, b, coa, con});	
    }
    
    counter = 0;
    std::ifstream infile2(argv[2]);
    while (infile2 >> w >> x >> y >> r >> g >> b >> coa >> con)
    {	
		ist2.push_back({w, x, y, r, g, b, coa, con});	
    }
    
    
    double res = transport2(ist1, ist2);
    
    std::cout << res << std::endl;
    return 0;
}

double transport2(std::vector<FeatureSignatureTuple> fst1, 
    std::vector<FeatureSignatureTuple> fst2)
{

    // set the larger vector to p to avoid artifical nodes
    std::vector<FeatureSignatureTuple> p;
    std::vector<FeatureSignatureTuple> q;
  

    if (fst1.size() > fst2.size())
    {
        p = fst1;
        q = fst2;
    }
    else
    {
        p = fst2;
        q = fst1;
    }
    
      
    long** distMat = new long*[p.size()];
    for (unsigned int i = 0; i < p.size(); i++)
        distMat[i] = new long[q.size()];
    
    // fill distance matrix
    for (unsigned int i = 0; i < p.size(); i++) // rows
    {
        for (unsigned int j = 0; j < q.size(); j++) // columns
        {
           // std::cout << "i:" << i << std::endl;
            distMat[i][j] = euclidDist(p.at(i), q.at(j));
         //   std::cout << distMat[i][j] << "|";
        }       
       // std::cout << std::endl;
    }
        
    // start in top left corner
    // north-west corner method
    // supply and demand are equal -> no costs, move one down
    // supply < demand -> move one down,
    // supply > demand -> move one right,
    // adjust supply (demand - supply) 
    
    // we need to fullfill the demand
    // the demand consists of the sum of all 
    // "dirt piles/sinks" of a signature
    // also, every "line of dirt piles" 
    //(x,y,color1,color2,color3, coarseness, contrast)
    // is multiplied by it's weight
    // let's get the complete demand first:
    long sumDemand = 0;
    long sumSupply = 0;
    
    
    // to deal with situations when demand != supply
    // I'll avoid rounding errors by multiplying
    // the weights with an arbitrarily chosen big number
    // this is not ideal, a proper scaling 
    // should be put in place for later versions
    for (auto sigTuple : p)
    {
        //todo remove magic number
        sumSupply = static_cast<long>(round(sigTuple.weight * 1000000)); 
    }
    
    for (auto sigTuple : q)
    {
        sumDemand = static_cast<long>(round(sigTuple.weight * 1000000));
    }
    
    // let's convert all weights to long
    // to avoid trouble in double country
    // for this, the FeatureSignatureTuple type 
    // can't be used anymore
    // so, I'll go with simple vectors
    
    std::vector<long>pVec;
    for (auto fst : p)
    {
        long tmpWeight = static_cast<long>(round(fst.weight * 1000000));
       // std::cout << "pVec push:" << tmpWeight << std::endl;
        pVec.push_back(tmpWeight);
    }
        
    std::vector<long>qVec;
    for (auto fst : q)
    {
        long tmpWeight = static_cast<long>(round(fst.weight * 1000000));
       // std::cout << "qVec push:" << tmpWeight << std::endl;
        qVec.push_back(tmpWeight);
    }
        

    // if demand is higher, add a zero cost dummy to supply
    // if it's the other way around, add a zero cost dummy
    // to demand
    if (sumDemand > sumSupply)
    {
        std::cout << "demand is higher" << std::endl;
        long tmpDemand = sumDemand - sumSupply;
        pVec.push_back(tmpDemand);
    }
    else if (sumSupply > sumDemand) 
    {
        long tmpSupply = sumSupply - sumDemand;
        qVec.push_back(tmpSupply);
    }
    
    
    std::cout << "len supply:" << p.size() << std::endl;
    std::cout << "len demand:" << q.size() << std::endl;
    
    
    //double cost =  0.0;
    unsigned int i = 0;
    unsigned int j = 0;
    double sumDists = 0.0;
    double sumFlows = 0.0;
    
    
    while (true) 
    {
       //std::cout << "i:" << i << std::endl;
       //std::cout << "j:" << j << std::endl;
       std::cout << "pvecsize:" << pVec.size() << std::endl;
       std::cout << "qecsize:" << qVec.size() << std::endl;
       
        if (i >= qVec.size())
        {
            std::cout << "breaking i" << std::endl;
            break;
        }
        if (j >= pVec.size())
        {
            std::cout << "breaking j" << std::endl;
            break;
        }
            
            //std::cout << "pVec:" << pVec.at(j) << " qVec:" 
            //<< qVec.at(i) << std::endl;
            if (pVec.at(j) > qVec.at(i)) // supply higher
            {
                std::cout << "supply higher" << std::endl;
                pVec.at(j) -= qVec.at(i); // decrease supply
                std::cout << "pVec:" << pVec.at(j) << std::endl;
                std::cout << "i:" << i << " j:" << j << std::endl;
                
                sumFlows += qVec.at(i); // increase float sum 
                sumDemand -= qVec.at(i);  // global counter
                sumDists += qVec.at(i) * distMat[i][j];
                i++;  // move to next requester
            }
            else if (pVec.at(j) < qVec.at(i)) // demand higher
            {
                std::cout << "demand higher" << std::endl;
                qVec.at(i) -= pVec.at(j);  // reduce supply
                std::cout << "qVec:" << qVec.at(i) << std::endl;
                std::cout << "i:" << i << " j:" << j << std::endl;
                sumFlows += pVec.at(j);
                sumDemand -= pVec.at(j);
                sumDists += pVec.at(j) * distMat[i][j];
                std::cout << "pVec:" << pVec.at(j)
                 << " distMat:" << distMat[i][j] << std::endl;
               // std::cout << "prod:" 
               //<< pVec.at(j) * distMat[i][j] << std::endl;
                
                j++; // move to next supplier
            }
            else  // supply and demand are equal
            {
                std::cout << "else"<< std::endl;
                pVec.at(j) -= qVec.at(i);
                qVec.at(i) -= pVec.at(j);
                sumFlows += qVec.at(i);
                sumDemand -= qVec.at(i);
                sumDists += pVec.at(j) * distMat[i][j];
                std::cout << "pVec:" << pVec.at(j) 
                << " distMat:" << distMat[i][j] << std::endl;
                std::cout << "i:" << i << " j:" << j << std::endl;
              //  std::cout << "prod:" 
              //<< pVec.at(j) * distMat[i][j] << std::endl;
                j++;
                i++;
            }
            std::cout << "end i:" << i << "end  j:" << j << std::endl;
              
    }
    
    std::cout << "end sumDemand:" << sumDemand << std::endl;
    std::cout << "end sumSuppply:" << sumSupply << std::endl;
    std::cout << "end sumFlow:" << sumFlows << std::endl;
    std::cout << "end sumDists:" << sumDists << std::endl;
    
    return (sumFlows * sumDists) / sumFlows;
    
    /*
      while (i < q.size() && j < p.size())
    {
        if (j >= q.size())
            break;
        if (j >= p.size())
            break;
            
        if (p.at(j).weight > q.at(i).weight) // supply higher
          {          
           //   std::cout << " fog 1a 1" << std::endl; 
            p.at(j).weight -= q.at(i).weight;
           // std::cout << " fog 1a 2" << std::endl;
            cost += (distMat[i][j] * q.at(i).weight);
            //std::cout << " fog 1a 3" << std::endl;
            sumDists += distMat[i][j];
            //std::cout << " fog 1a 4" << std::endl;
            q.at(i).weight = 0;
            i++;
          }
          else if (p.at(j).weight < q.at(i).weight) // demand higher
          {
              //std::cout << "fog 1b 1" << std::endl;
            q.at(i).weight -= p.at(j).weight;
            //std::cout << " fog 1b 2, j:" << j << std::endl;
            cost += (distMat[i][j] * q.at(j).weight); // i?
            //std::cout << " fog 1b 3" << std::endl;
            sumDists += distMat[i][j];
            //std::cout << " fog 1b 4" << std::endl;
            p.at(j).weight = 0;
            //std::cout << " fog 1b 5" << std::endl;
            j++;
          }
          else
          {
              //std::cout << "fog 1c" << std::endl;
            distMat[i][j] *= q.at(i).weight;
            cost += distMat[i][j] * q.at(j).weight;
            p.at(j).weight = 0;
            q.at(i).weight = 0;
            j++;
          }
    }
    */
    
    //if (cost == 0.0)
    //    return 0.0;
    //else
    //  return (sumDists * cost) / cost;
     
}

