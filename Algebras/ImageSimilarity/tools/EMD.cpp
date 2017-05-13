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

double transport2(std::vector<ImageSignatureTuple> ist1,
					std::vector<ImageSignatureTuple> ist2);


struct Flow
{
  double fromWeight;
  double distance;
  double toWeight;
};



int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "please provide two signature files" << std::endl;
		return 1;
	}
	std::vector<ImageSignatureTuple> ist1;
	std::vector<ImageSignatureTuple> ist2;
		
	std::cout << argv[1] << " " << argv[2] << std::endl;
		

	int x, y;
	double w;
	
	std::ifstream infile(argv[1]);
	std::string std;
	while (infile >> x >> y >> w)
	{		
		ist1.push_back({x, y, w});
	}
	
	
	std::ifstream infile2(argv[2]);
	while (infile2 >> x >> y >> w)
	{
		ist2.push_back({x, y, w});
	}
	
	
	
	double res = transport2(ist1, ist2);
	
	std::cout << "res:" << res << std::endl;
	return 0;
}

double transport2(std::vector<ImageSignatureTuple> ist1, 
	std::vector<ImageSignatureTuple> ist2)
{

	// set the larger vector to p to avoid artifical nodes
    std::vector<ImageSignatureTuple> p;
    std::vector<ImageSignatureTuple> q;
  

	if (ist1.size() > ist2.size())
	{
		p = ist1;
		q = ist2;
	}
	else
	{
		p = ist2;
		q = ist1;
	}
	
	  
    std::cout << " p:" << p.size() << std::endl;
    std::cout << " q:" << q.size() << std::endl;

	struct TableauTuple
	{
		double distance;
		double delivered;
	};
	
    double** distMat = new double*[p.size()];
    for (unsigned int i = 0; i < p.size(); i++)
		distMat[i] = new double[q.size()];
   
    // fill distance matrix
    for (unsigned int i = 0; i < p.size(); i++) // rows
    {
		for (unsigned int j = 0; j < q.size(); j++) // columns
		{
			distMat[i][j] 
			= std::abs(p.at(i).weight - q.at(j).weight);
		}
		std::cout << std::endl;
    }

	std::cout << "survived" << std::endl;
	std::cout << std::abs(1.0 - 0.5) << std::endl;
	
    // side preliminaries
    // 1. indices start with 1
    // 2. no negative signs
    double cost =  0.0;
    unsigned int i = 0;
    unsigned int j = 0;
	
	double sumDists = 0.0;
	
    // start in top left corner
    // supply and demand are equal -> no costs, move one down
    // supply < demand -> move one down,
    // supply > demand -> move one right,
    // adjust supply (demand - supply) 
    
	while (i < q.size() && j < p.size())
	{
		if (p.at(j).weight > q.at(i).weight) // supply higher
	  	{
			//std::cout << " case 1, i:" << i << " j:" << j
			// << " p:" << p.at(j).weight << " q:" 
			//<< q.at(i).weight << std::endl;
			p.at(j).weight -= q.at(i).weight;
	    	cost += distMat[i][j] * q.at(i).weight;
	    	sumDists += distMat[i][j];
			q.at(i).weight = 0;
        	i++;
	  	}
	  	else if (p.at(j).weight < q.at(i).weight) // demand higher
	  	{
			//std::cout << " case 2, i:" << i << " j:" 
			//<< j << " p:" << p.at(j).weight << " q:"
			// << q.at(i).weight << std::endl;
			q.at(i).weight -= p.at(j).weight;
	    	cost += distMat[i][j] * q.at(j).weight;
	    	sumDists += distMat[i][j];
			p.at(j).weight = 0;
        	j++;
	  	}
	  	else
	  	{
			//std::cout << " case 3, i:" << i << " j:" 
			//<< j << " p:" << p.at(j).weight << " q:" 
			//<< q.at(i).weight << std::endl;
	    	distMat[i][j] *= q.at(i).weight;
	    	cost += distMat[i][j] * q.at(j).weight;
			p.at(j).weight = 0;
			q.at(i).weight = 0;
        	j++;
	  	}
	}
	//std::cout << std::endl;
	std::cout << "cost:" << cost << std::endl;
  	double result = (sumDists * cost) / cost;
  	return result;
}

