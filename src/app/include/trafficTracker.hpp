#pragma once

#include "tracklet.hpp"
#include "Hungarian.h"

/*

	vector< vector<double> > costMatrix = { {1,2},{2,1},{0,3} };

	HungarianAlgorithm HungAlgo;
	vector<int> assignment;

	double cost = HungAlgo.Solve(costMatrix, assignment);

	for (unsigned int x = 0; x < costMatrix.size(); x++)
		std::cout << x << ", assigned job:" << assignment[x] << "\t";

	std::cout << "\ncost: " << cost << std::endl;

*/

class TrafficTracker{
private:

public:

};