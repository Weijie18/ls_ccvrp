// LLM-generated MoE accelerated version.
/*- Fixed a bug caused by the missing closing brace at the end of swap_three_three. - Integrated the ALNS framework into the VNS main loop so that it works with RVNS: RVNS performs local intensification, while ALNS provides diversified large-neighborhood search. - Implemented adaptive weight adjustment based on historical operator performance. - Implemented multiple removal and insertion operators, including Random Removal, Worst Removal, Shaw Removal, Greedy Insertion, and Regret Insertion. - Fixed data synchronization for arrival_time after node removal and forced route-information recalculation between Destroy and Repair for accuracy.*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <time.h>
#include <ctime>
#include <string>
#include <string.h>
#include <algorithm>
#include <random>
#include <iomanip>
#include <unordered_set>
#include <vector>
#include <cmath>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <cstdlib>
#include <numeric>

using namespace std;

clock_t startTime, endTime;
int timeMax = 3000;
int t = 0;
//string inFile = "c103C15_standard.txt";
//string inFile = "c101C10_standard.txt";
//string inFile = "c102_21_standard.txt";
//string inFile = "rc204_21_standard.txt";
//string inFile = "CMT1.txt";
string inFile = "Golden_17.txt";
int node_num;                                     // Number of nodes
int customer_num;                                 // Number of customer nodes

vector<double> xCor;                                                   // X-coordinate of each node
vector<double> yCor;                                                   // Y-coordinate of each node
vector<double> demand;                                                 // Demand of each node
//vector<double> serviceTime; // Service time of each node
vector<vector<double>> travelTime;                                     // Travel time between nodes; distance is treated as time.
vector<double> theta;                                                  // Theta value of each customer node in the polar coordinate system.

int load_capacity;

int vehicle_num = 22;                                                // Maximum number of available vehicles


double M = 10000000;                                                // Define a sufficiently large number
int** Tabu;                                                         // Tabu table
int tabu_step = 1;                                                  // Tabu tenure
int s_r;                                                            // Index of the evolved route

int op_num = 10;
vector<int> op_used_num(op_num, 0);                               // Number of times each operator is used
vector<int> op_effec_num(op_num, 0);                              // Number of effective uses of each operator

struct routeInformation {
	vector<vector<int>> route;                                      // Route traveled by each vehicle
	vector<vector<double>> arrival_time;                            // Arrival time at each node
	
	vector<int> used_capacity;                                      // Used capacity of each vehicle
	vector<double> single_cumulative_time;                          // Cumulative waiting time of each route
	
	
	
};
routeInformation S;

struct evaluationFunction {
	double cumulative_time;                                         // Cumulative waiting time
	
	double extra_capacity;                                          // Cargo-load overload amount
	double extra_vehicle;                                           // Number of extra vehicles used
	double evaluation_obj;                                          // Evaluation objective value of the algorithm
};
evaluationFunction F;

struct globalBest {
	vector<vector<int>> route;                                      // Global best route
	double obj;                                                     // Global best objective value
};
globalBest optimal;

// Penalty parameters from Andre L. S. Souza (COR)
struct penaltyParameter {
	
	double alpha_extra_capacity = 20;                               // Penalty coefficient for extra load
	double alpha_extra_vehicle = 800;                              // Penalty coefficient for extra vehicles
	
	double extra_capacity_delta;                                    // Change in the extra-load penalty coefficient
	double extra_vehicle_delta;                                     // Change in the extra-vehicle penalty coefficient
	double max_extra_capacity;
	double min_extra_capacity;
	double max_extra_vehicle;
	double min_extra_vehicle;
	
};
penaltyParameter penalty;

struct tempInformation {
	vector<int> single_route;                                       // Temporarily records the sequence of a route
	vector<double> single_arrival_time;                             // Temporarily records arrival times for all nodes in a route
	
	double cumulative_time;
	
};

struct operatorPara {
	
	int last_node;              // Previous node index
	int next_node;              // Next node index
	bool isInsert;              // Whether a node is pending insertion
	bool isChange;              // Whether any node has changed
	
	double delta;               // Delta value
	double tem_delta;           // Temporary delta value
	int select_last_node_index; // Selected previous-node position
	int select_next_node_index; // Selected next-node position
	int select_route_index;     // Selected route index
	int select_node_index;      // Selected node position
	int node1_index;            // Selected node position in the first route
	int node2_index;            // Selected node position in the second route
	int node1;                  // Selected node in the first route
	int node2;                  // Selected node in the second route
	int route1_index;           // Index of the selected first route
	int route2_index;           // Index of the selected second route
};

struct parameters {
	double shake_s = 4;                                                 // Number of perturbations; increasing this improves solution diversity and is suitable for CMT instances.
	int max_shake_no_improve = 100;                                     // Maximum number of non-improving iterations before applying shake, balancing search depth and efficiency.
	double max_length_RCL_percent = 0.20;                                   // Maximum RCL length as a percentage of all customer nodes; reduced to improve local-search quality.
	int max_initial_LS = 1000;                                         // Number of initial local-search iterations; increased to improve initial solution quality.
	int max_no_improve_new_S = 2000;                                    // Maximum number of non-improving iterations before regenerating an individual; increased to give the algorithm more search time.
	int omega = 3;                                                    // Adjust the weight parameter to emphasize cumulative time.
	int min_omega = 2;
	int max_omega = 6;                                                // Adjust the omega range to improve algorithm adaptability.
	int GPX_no_improve_cycle = 5;                                     // Number of GPX non-improving cycles; increased to improve crossover quality.
} p;


void readInitial_CMT(istream&);
void cumulativeTimeForRoute(const routeInformation&, evaluationFunction&);
void cumulativeTimeForSingleRoute(routeInformation&, const int);
void cumulativeTimeForSingleRoute(const vector<double>&, const vector<int>&, double&);
void extraPowerForRoute(const routeInformation&, evaluationFunction&);


void arrivalTimeForSingleRoute(vector<double>&, const vector<int>&);
void verify(const routeInformation&, const evaluationFunction&);
void verify(const routeInformation&);
void deltaEvaluation(tempInformation&, evaluationFunction&, const int);
void deltaEvaluation(tempInformation&, evaluationFunction&, const int, tempInformation&);
void deltaEvaluationForTwoSingle(tempInformation&, tempInformation&, evaluationFunction&, const int, const int);
void printVerifyInformation(const routeInformation&, const operatorPara&, const evaluationFunction&);
void printVerifyInformationForTwoRoute(const routeInformation&, const operatorPara&, const evaluationFunction&);
void printVerifySingleRoute(const tempInformation&);
void calInformation(routeInformation&, evaluationFunction&);
void GPX(const routeInformation&, const routeInformation&, routeInformation&, evaluationFunction&);


double randomDouble() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<double> dis(0.0, 1.0);

	return dis(gen);
}

int randomInt(int n) {
	// Initialize the random-number generator.
	// Create the random-number generator.
	random_device rd;  // Provides a random seed.
	mt19937 gen(rd());  // Use the Mersenne Twister algorithm.
	uniform_int_distribution<> dis(0, n - 1);  // Define the range.

	// Generate a random integer.
	int randomNumber = dis(gen);

	return randomNumber;
}

int randomInt(int lb, int ub) {
	if (lb > ub) {
		// For example, return lb or throw an exception/log a message.
		std::cerr << "Warning: lb > ub in randomInt(" << lb << ", " << ub << ")" << std::endl;
		return lb;  // Alternatively, return -1 to indicate invalid input.
	}

	// Initialize the random-number generator.
	// Create the random-number generator.
	random_device rd;  // Provides a random seed.
	mt19937 gen(rd());  // Use the Mersenne Twister algorithm.
	uniform_int_distribution<> dis(lb, ub);  // Define the range.

	// Generate a random integer.
	int randomNumber = dis(gen);

	return randomNumber;
}

void readInitial_CMT(istream& ios) {
	double wait_time;
	ios >> node_num;
	customer_num = node_num - 1;
	ios >> load_capacity;
	ios >> wait_time;
	double a;

	for (int i = 0; i < node_num; i++) {

		ios >> a;
		ios >> a;
		xCor.push_back(a);
		ios >> a;
		yCor.push_back(a);
	}

	for (auto i = 0; i < node_num; i++) {
		ios >> a;
		ios >> a;
		demand.push_back(a);
	}

	// Release the old Tabu table first.
	if (Tabu != nullptr) {
		for (int i = 0; i < node_num; i++) {
			delete[] Tabu[i];
		}
		delete[] Tabu;
		Tabu = nullptr;
	}

	Tabu = new int* [node_num];
	for (auto i = 0; i < node_num; i++) {
		Tabu[i] = new int[node_num];
		for (auto j = 0; j < node_num; j++) {
			Tabu[i][j] = -1;
		}
	}

	if (node_num <= 50) {
		p.max_no_improve_new_S = 1000;
	}
	else if (node_num > 50 && node_num < 100) {
		p.max_no_improve_new_S = 5000;
	}
	else {
		p.max_no_improve_new_S = 10000;
	}

	/*Debug output for instance data.*/
}

void readInitial_CMT(string) {
	ifstream FIC;
	FIC.open(inFile);
	if (FIC.fail()) {
		std::cout << "### Erreur open, File_Name: " << inFile << endl;
		getchar();
		exit(0);
	}
	if (FIC.eof()) {
		std::cout << "### Error open, File_Name: " << inFile << endl;
		getchar();
		exit(0);
	}
	readInitial_CMT(FIC);

}

void calDistance() {
	travelTime.resize(node_num);
	double rawTime;
	for (int i = 0; i < node_num; i++) {
		travelTime[i].reserve(node_num);
		for (int j = 0; j < node_num; j++) {
			rawTime = hypot(xCor[i] - xCor[j], yCor[i] - yCor[j]);
			travelTime[i].push_back(round(rawTime * 10000) / 10000.0);
		}
	}
}

void GRASP_initialization(routeInformation& S) {
	vector<int> U;
	for (int i = 0; i < customer_num; i++) {
		U.push_back(i + 1);
	}

	for (auto i = 0; i < vehicle_num; i++) {
		S.route.emplace_back();
		S.route.back().push_back(0);
		S.route.back().push_back(0);
		

		S.used_capacity.push_back(0);
		S.single_cumulative_time.push_back(0);
		
	}

	vector<int> RCL;
	int max_length_RCL = p.max_length_RCL_percent * customer_num;

	vector<double> delta;                  // Record the distance delta for each corresponding node in the RCL.
	vector<int> location_route;            // Record the route index into which each corresponding RCL node is inserted.
	//vector<int> tem_used_capacity; // Record the new vehicle-capacity usage after each corresponding RCL node is inserted.
	int count;
	int node;
	int last_node;
	int last_index;
	int next_index;
	int next_node;
	int RCL_length;
	double tem_delta;
	int select_node;
	int select_node_index;
	int select_route;

	while (U.size()) {
		RCL.clear();
		delta.clear();
		location_route.clear();
		count = 0;

		for (auto i = 0; i < U.size(); i++) {
			node = U[i];
			for (auto j = 0; j < vehicle_num; j++) {
				last_node = S.route[j].back();                       // Get the value at the last position.
				last_index = S.route[j].size() - 1;                  // Get the index of the last position.
				next_index = S.route[j].size() - 2;                  // Get the index of the second-to-last position.
				next_node = S.route[j][next_index];                  // Get the value at the second-to-last position.
				tem_delta = travelTime[last_node][node] + travelTime[node][next_node] - travelTime[last_node][next_node];
				if (RCL.size() <= count) {
					RCL.push_back(node);
					delta.push_back(tem_delta);
					location_route.push_back(j);
					//tem_used_capacity.push_back(S.used_capacity[j] + demand[node]);
					count++;
				}
				else {
					if (tem_delta < delta[j]) {
						delta[j] = tem_delta;
						RCL[j] = node;
						location_route[j] = j;
						//tem_used_capacity[j] = S.used_capacity[j] + demand[node];
						count++;
					}
				}

				if (count >= vehicle_num) {
					count = 0;
				}
			}
		}

		// After the RCL is generated, randomly select one node from it and insert it into the corresponding route.
		select_node_index = randomInt(RCL.size());
		select_node = RCL[select_node_index];
		select_route = location_route[select_node_index];
		next_index = S.route[select_route].size() - 1;                         // Get the index of the last position.
		S.route[select_route].insert(S.route[select_route].begin() + next_index, select_node);
		S.used_capacity[select_route] += demand[select_node];

		for (auto k = 0; k < U.size(); k++) {
			if (U[k] == select_node) {
				U.erase(U.begin() + k);
				break;
			}
		}
	}

	for (auto i = 0; i < S.route.size(); i++) {
		S.arrival_time.emplace_back();                                 // Add an empty row.
		S.arrival_time.back().reserve(node_num);                       // Preallocate space for this row.
	}

	int node1;
	int node2;
	double temp_arrival_time;
	for (auto i = 0; i < S.route.size(); i++) {
		temp_arrival_time = 0;
		S.arrival_time[i].push_back(0);
		for (auto j = 0; j < S.route[i].size() - 1; j++) {
			node1 = S.route[i][j];
			node2 = S.route[i][j + 1];
			temp_arrival_time += travelTime[node1][node2];
			S.arrival_time[i].push_back(temp_arrival_time);
		}
	}



	// Calculate the current route cumulative waiting time and the cumulative waiting time of each route.
	cumulativeTimeForRoute(S, F);

	for (auto i = 0; i < S.route.size(); i++) {
		cumulativeTimeForSingleRoute(S, i);
	}

	F.extra_capacity = 0;
	for (auto i = 0; i < S.route.size(); i++) {
		F.extra_capacity += max(0, S.used_capacity[i] - load_capacity);
	}

	F.extra_vehicle = max(0, static_cast<int>(S.route.size()) - vehicle_num);

	optimal.route = S.route;
	optimal.obj = F.cumulative_time;

	F.evaluation_obj = F.cumulative_time + penalty.alpha_extra_capacity * F.extra_capacity + penalty.alpha_extra_vehicle * F.extra_vehicle;


	/* Debug output for route and evaluation information. */
}

// Calculate information for the newly generated route.
void calInformation(routeInformation& S, evaluationFunction& F) {
	S.used_capacity.clear();
	int tem_capacity;
	for (auto i = 0; i < S.route.size(); i++) {
		tem_capacity = 0;
		for (auto j = 1; j < S.route[i].size() - 1; j++) {
			tem_capacity += demand[S.route[i][j]];
		}
		S.used_capacity.push_back(tem_capacity);
	}

	S.arrival_time.clear();
	for (auto i = 0; i < S.route.size(); i++) {
		S.arrival_time.emplace_back();                                 // Add an empty row.
		S.arrival_time.back().reserve(node_num);                       // Preallocate space for this row.
	}

	int node1;
	int node2;
	double temp_arrival_time;
	for (auto i = 0; i < S.route.size(); i++) {
		temp_arrival_time = 0;
		S.arrival_time[i].push_back(0);
		for (auto j = 0; j < S.route[i].size() - 1; j++) {
			node1 = S.route[i][j];
			node2 = S.route[i][j + 1];
			temp_arrival_time += travelTime[node1][node2];
			S.arrival_time[i].push_back(temp_arrival_time);
		}
	}

	// Calculate the current route cumulative waiting time and the cumulative waiting time of each route.
	cumulativeTimeForRoute(S, F);

	S.single_cumulative_time.clear();
	for (auto i = 0; i < S.route.size(); i++) {
		S.single_cumulative_time.push_back(0);
	}

	for (auto i = 0; i < S.route.size(); i++) {
		cumulativeTimeForSingleRoute(S, i);
	}

	F.extra_capacity = 0;
	for (auto i = 0; i < S.route.size(); i++) {
		F.extra_capacity += max(0, S.used_capacity[i] - load_capacity);
	}

	F.extra_vehicle = max(0, static_cast<int>(S.route.size()) - vehicle_num);

	F.evaluation_obj = F.cumulative_time + penalty.alpha_extra_capacity * F.extra_capacity + penalty.alpha_extra_vehicle * F.extra_vehicle;

	/* Debug output for route and evaluation information. */
}

// Calculate cumulative waiting time for all routes. Input: route information structure and evaluation-function information structure.
void cumulativeTimeForRoute(const routeInformation& S, evaluationFunction& F) {
	double temp_cumulative_time = 0;
	auto len_S_route = S.route.size();
	for (auto i = 0; i < len_S_route; i++) {
		auto len_S_route_i = S.route[i].size() - 1;
		for (auto j = 1; j < len_S_route_i; j++) {
			temp_cumulative_time += S.arrival_time[i][j];
		}
	}
	F.cumulative_time = temp_cumulative_time;
}

// Calculate cumulative waiting time for route i in the solution. Input: route information structure and route index (integer).
void cumulativeTimeForSingleRoute(routeInformation& S, const int i) {
	double temp_single_cumulative_time = 0;
	auto len_S_route_i = S.route[i].size() - 1;
	for (auto j = 1; j < len_S_route_i; j++) {
		temp_single_cumulative_time += S.arrival_time[i][j];
	}
	S.single_cumulative_time[i] = temp_single_cumulative_time;
	//cout << S.single_cumulative_time[i] << endl;
}

// Calculate cumulative waiting time for route i using temporary information for incremental evaluation. Input: route arrival-time vector, route vector, and route cumulative waiting time (double).
void cumulativeTimeForSingleRoute(const vector<double>& arrival_time, const vector<int>& route, double& single_cumulative_time) {
	double temp_single_cumulative_time = 0;
	auto len_route = route.size() - 1;
	for (auto j = 1; j < len_route; j++) {
		temp_single_cumulative_time += arrival_time[j];
	}
	single_cumulative_time = temp_single_cumulative_time;
	//cout << single_cumulative_time << endl;
}


// Calculate arrival times for a route. Input: route arrival-time vector and route vector.
void arrivalTimeForSingleRoute(vector<double>& arrival_time, const vector<int>& single_route) {
	int node1;
	int node2;
	double temp_arrival_time = 0;
	arrival_time.clear();
	arrival_time.push_back(0);
	for (auto j = 0; j < single_route.size() - 1; j++) {
		node1 = single_route[j];
		node2 = single_route[j + 1];
		temp_arrival_time += travelTime[node1][node2];
		arrival_time.push_back(temp_arrival_time);
	}

}

// Verify whether the obtained routes satisfy the capacity constraint. Input: route information structure.
void verify(const routeInformation& S) {
	// Verify the capacity constraint.
	int tem_capacity = 0;
	bool isCapacity = true;                           // Check whether the load of each route exceeds the rated load.
	for (auto i = 0; i < S.route.size(); i++) {
		tem_capacity = 0;
		for (auto j = 0; j < S.route[i].size(); j++) {
			tem_capacity += demand[S.route[i][j]];
		}
		if (tem_capacity > load_capacity) {
			isCapacity = false;
			std::cout << "The CAPACITY constraint for route " << i << " has been violated with " << (tem_capacity - load_capacity) << endl;
		}
	}


	if (!isCapacity) {
		if (!isCapacity) {
			std::cout << "△ Warning! The CAPACITY constraint is violated and the details are above ↑" << endl;
		}
	}

}

// Check whether all calculations are correct, including the load and cumulative waiting time of each route. Input: route information structure and evaluation-function information structure.
void verify(const routeInformation& S, const evaluationFunction& F) {
	// Verify whether each route load is calculated correctly.
	bool isCapacity = true;
	double tem_capacity = 0;
	for (auto i = 0; i < S.route.size(); i++) {
		tem_capacity = 0;
		for (auto j = 0; j < S.route[i].size(); j++) {
			tem_capacity += demand[S.route[i][j]];
		}
		if (abs(tem_capacity - S.used_capacity[i]) > 0.01) {
			isCapacity = false;
			std::cout << "△ Warning! The CAPACITY for route " << i << " is wrongly calculated! Original: " << S.used_capacity[i] << " Validation: " << tem_capacity << endl;
		}
	}

	// Verify whether the total extra load over all routes is correct.
	bool isExtraCapacity = true;
	double tem_extra_capacity = 0;
	for (auto i = 0; i < S.route.size(); i++) {
		tem_capacity = 0;
		for (auto j = 0; j < S.route[i].size(); j++) {
			tem_capacity += demand[S.route[i][j]];
		}
		tem_extra_capacity += max(0.0, tem_capacity - load_capacity);
	}
	if (abs(tem_extra_capacity - F.extra_capacity) > 0.01) {
		isExtraCapacity = false;
		std::cout << "△ Warning! The EXTRA CAPACITY is wrongly calculated! Original: " << F.extra_capacity << " Validation: " << tem_extra_capacity << endl;
	}

	// Verify whether the total cumulative waiting time over all routes is correct.
	bool isCumulativeTime = true;
	double temp_cumulative_time = 0;
	for (auto i = 0; i < S.route.size(); i++) {
		for (auto j = 1; j < S.route[i].size() - 1; j++) {
			temp_cumulative_time += S.arrival_time[i][j];
		}
	}
	if (abs(temp_cumulative_time - F.cumulative_time) > 0.01) {
		isCumulativeTime = false;
		std::cout << "△ Warning! The CUMULATIVE TIME is wrongly calculated! Original: " << F.cumulative_time << " Validation: " << temp_cumulative_time << endl;
	}


	// Overall verification status.
	if (isCapacity && isExtraCapacity && isCumulativeTime) {
		std::cout << endl;
		std::cout << "Congratulations!! All validations are passed!!" << endl;
	}

}

// Incremental evaluation: calculate all metrics of the changed solution except load, which must be computed manually. Input: temporary route information structure, evaluation-function information structure, and route index (integer).
void deltaEvaluation(tempInformation& single, evaluationFunction& new_F, const int i) {
	arrivalTimeForSingleRoute(single.single_arrival_time, single.single_route);
	cumulativeTimeForSingleRoute(single.single_arrival_time, single.single_route, single.cumulative_time);
	double delta_cumulative_time = single.cumulative_time - S.single_cumulative_time[i];

	new_F.cumulative_time = F.cumulative_time + delta_cumulative_time;
	//new_F.extra_capacity = F.extra_capacity;
	new_F.evaluation_obj = new_F.cumulative_time + penalty.alpha_extra_capacity * new_F.extra_capacity + penalty.alpha_extra_vehicle * new_F.extra_vehicle;
}

// Incremental evaluation: calculate all metrics of the changed solution except load, which must be computed manually. Input: temporary route information structure 1, evaluation-function information structure, route index (integer), and newly generated temporary route information structure 2.
void deltaEvaluation(tempInformation& single1, evaluationFunction& new_F, const int i, tempInformation& single2) {
	arrivalTimeForSingleRoute(single1.single_arrival_time, single1.single_route);
	cumulativeTimeForSingleRoute(single1.single_arrival_time, single1.single_route, single1.cumulative_time);
	double delta_cumulative_time1 = single1.cumulative_time - S.single_cumulative_time[i];

	arrivalTimeForSingleRoute(single2.single_arrival_time, single2.single_route);
	cumulativeTimeForSingleRoute(single2.single_arrival_time, single2.single_route, single2.cumulative_time);
	double delta_cumulative_time2 = single2.cumulative_time;

	new_F.cumulative_time = F.cumulative_time + delta_cumulative_time1 + delta_cumulative_time2;
	new_F.evaluation_obj = new_F.cumulative_time + penalty.alpha_extra_capacity * new_F.extra_capacity + penalty.alpha_extra_vehicle * new_F.extra_vehicle;
}

// Incremental evaluation: calculate all metrics of the changed solution for two single routes. Input: temporary route information structures 1 and 2, evaluation-function information structure, and route indices 1 and 2 (integers).
void deltaEvaluationForTwoSingle(tempInformation& single1, tempInformation& single2, evaluationFunction& new_F, const int i, const int j) {
	arrivalTimeForSingleRoute(single1.single_arrival_time, single1.single_route);
	cumulativeTimeForSingleRoute(single1.single_arrival_time, single1.single_route, single1.cumulative_time);
	double delta_cumulative_time1 = single1.cumulative_time - S.single_cumulative_time[i];


	arrivalTimeForSingleRoute(single2.single_arrival_time, single2.single_route);
	cumulativeTimeForSingleRoute(single2.single_arrival_time, single2.single_route, single2.cumulative_time);
	double delta_cumulative_time2 = single2.cumulative_time - S.single_cumulative_time[j];

	new_F.cumulative_time = F.cumulative_time + delta_cumulative_time1 + delta_cumulative_time2;
	new_F.evaluation_obj = new_F.cumulative_time + penalty.alpha_extra_capacity * new_F.extra_capacity + penalty.alpha_extra_vehicle * new_F.extra_vehicle;
}


// /* ================ Customer-node operators start here ================ */

// 1-Relocate: move one customer node to another position within the same route.
void relocate(routeInformation& S) {
	operatorPara para;
	para.select_route_index = randomInt(S.route.size());

	if (S.route[para.select_route_index].size() < 4) {
		cout << "relocate random number is infeasible!" << endl;
		return;
	}

	para.select_node_index = randomInt(1, S.route[para.select_route_index].size() - 2);

	/*cout << "Before operator: " << endl; printVerifyInformation(S, para, F);*/

	int node = S.route[para.select_route_index][para.select_node_index];
	if (Tabu[node][para.select_route_index] < t) {
		tempInformation single;
		single.single_route = S.route[para.select_route_index];

		int new_index = randomInt(1, S.route[para.select_route_index].size() - 2);                  // Randomly generate a new position.
		while (new_index == para.select_node_index) {
			new_index = randomInt(1, S.route[para.select_route_index].size() - 2);
		}

		if (para.select_node_index < new_index) {
			// Move forward.
			rotate(single.single_route.begin() + para.select_node_index, single.single_route.begin() + para.select_node_index + 1, single.single_route.begin() + new_index + 1);
		}
		else if (para.select_node_index > new_index) {
			// Move backward.
			rotate(single.single_route.begin() + new_index, single.single_route.begin() + para.select_node_index, single.single_route.begin() + para.select_node_index + 1);
		}

		/*cout << "The movement is from " << para.select_node_index << " to " << new_index << endl; cout << endl; printVerifySingleRoute(single);*/

		evaluationFunction new_F;
		new_F.extra_capacity = F.extra_capacity;
		new_F.extra_vehicle = F.extra_vehicle;

		deltaEvaluation(single, new_F, para.select_route_index);

		if (new_F.evaluation_obj < F.evaluation_obj) {
			S.route[para.select_route_index] = single.single_route;
			S.arrival_time[para.select_route_index] = single.single_arrival_time;
			S.single_cumulative_time[para.select_route_index] = single.cumulative_time;
			F = new_F;

			// Update the Tabu table.
			Tabu[node][para.select_route_index] = tabu_step + t;
		}
	}

	/*cout << endl; cout << "After operator: " << endl; printVerifyInformation(S, para, F);*/

}

// Opt: reverse a segment within the same route.
void opt(routeInformation& S) {
	operatorPara para;
	para.select_route_index = randomInt(S.route.size());
	
	if (S.route[para.select_route_index].size() < 4) {
		cout << "opt random number is infeasible!" << endl;
		return;
	}
	para.node1_index = randomInt(1, S.route[para.select_route_index].size() - 2);
	para.node2_index = randomInt(1, S.route[para.select_route_index].size() - 2);
	while (para.node1_index == para.node2_index) {
		para.node2_index = randomInt(1, S.route[para.select_route_index].size() - 2);
	}

	// Ensure para.node2_index is the larger index and para.node1_index is the smaller index.
	if (para.node1_index > para.node2_index) {
		int tem = para.node1_index;
		para.node1_index = para.node2_index;
		para.node2_index = tem;
	}

	tempInformation single;
	single.single_route = S.route[para.select_route_index];

	reverse(single.single_route.begin() + para.node1_index, single.single_route.begin() + para.node2_index + 1);

	evaluationFunction new_F;
	new_F.extra_capacity = F.extra_capacity;
	new_F.extra_vehicle = F.extra_vehicle;

	deltaEvaluation(single, new_F, para.select_route_index);

	if (new_F.evaluation_obj < F.evaluation_obj) {
		S.route[para.select_route_index] = single.single_route;
		S.arrival_time[para.select_route_index] = single.single_arrival_time;
		S.single_cumulative_time[para.select_route_index] = single.cumulative_time;
		F = new_F;
	}
}

// Exchange: swap the positions of two customers within the same route.
void exchange(routeInformation& S) {
	int node1_index;
	int node2_index;
	operatorPara para;
	para.select_route_index = randomInt(S.route.size());

	if (S.route[para.select_route_index].size() < 4) {
		cout << "exchange random number is infeasible!" << endl;
		return;
	}

	node1_index = randomInt(1, S.route[para.select_route_index].size() - 2);
	node2_index = randomInt(1, S.route[para.select_route_index].size() - 2);
	while (node1_index == node2_index) {
		node2_index = randomInt(1, S.route[para.select_route_index].size() - 2);
	}

	int node1 = S.route[para.select_route_index][node1_index];
	int node2 = S.route[para.select_route_index][node2_index];

	/*cout << "Before operator: " << endl; printVerifyInformation(S, para, F);*/

	if (Tabu[node1][para.select_route_index] < t && Tabu[node2][para.select_route_index] < t) {
		tempInformation single;
		single.single_route = S.route[para.select_route_index];


		swap(single.single_route[node1_index], single.single_route[node2_index]);


		/*cout << "The movement is to exchange " << node1 << " and " << node2 << endl; cout << endl; printVerifySingleRoute(single);*/

		evaluationFunction new_F;
		new_F.extra_capacity = F.extra_capacity;
		new_F.extra_vehicle = F.extra_vehicle;

		deltaEvaluation(single, new_F, para.select_route_index);

		if (new_F.evaluation_obj < F.evaluation_obj) {
			S.route[para.select_route_index] = single.single_route;
			S.arrival_time[para.select_route_index] = single.single_arrival_time;
			S.single_cumulative_time[para.select_route_index] = single.cumulative_time;
			F = new_F;
			// Update the Tabu table.
			Tabu[node1][para.select_route_index] = tabu_step + t;
			Tabu[node2][para.select_route_index] = tabu_step + t;
		}
	}

	/*cout << endl; cout << "After operator: " << endl; printVerifyInformation(S, para, F);*/

}

// 2-Relocate: select two consecutive nodes within the same route and randomly insert them at another position.
void relocate_sec(routeInformation& S) {
	operatorPara para;
	para.isChange = false;
	vector<int> available_route;
	for (auto i = 0; i < S.route.size(); i++) {
		if (S.route[i].size() - 2 >= 3) {
			available_route.push_back(i);
		}
	}

	if (available_route.size() > 0) {
		para.isChange = true;
		para.select_route_index = available_route[rand() % available_route.size()];
	}

	/*cout << "Before operator: " << endl; printVerifyInformation(S, para, F);*/

	if (para.isChange) {
		para.select_node_index = randomInt(1, S.route[para.select_route_index].size() - 3);

		if (S.route[para.select_route_index].size() - 3 < 1) {
			cout << "relocate_sec random number is infeasible!" << endl;
			return;
		}

		para.last_node = S.route[para.select_route_index][para.select_node_index];
		para.next_node = S.route[para.select_route_index][para.select_node_index + 1];
		if (Tabu[para.last_node][para.select_route_index] < t && Tabu[para.next_node][para.select_route_index] < t) {
			vector<int> temp = { para.last_node, para.next_node };
			tempInformation single;
			single.single_route = S.route[para.select_route_index];
			auto first = single.single_route.begin() + para.select_node_index;
			auto last = single.single_route.begin() + para.select_node_index + 2;
			single.single_route.erase(first, last);

			auto new_index = randomInt(1, single.single_route.size() - 1);

			if (single.single_route.size() - 1 < 1) {
				cout << "relocate_sec random number is infeasible! on movement of the solution." << endl;
				return;
			}

			/*cout << "The movement is from " << para.select_node_index << " to " << new_index << endl; cout << endl; printVerifySingleRoute(single);*/

			single.single_route.insert(single.single_route.begin() + new_index, temp.begin(), temp.end());

			evaluationFunction new_F = F;

			deltaEvaluation(single, new_F, para.select_route_index);


			if (new_F.evaluation_obj < F.evaluation_obj) {
				S.route[para.select_route_index] = single.single_route;
				S.arrival_time[para.select_route_index] = single.single_arrival_time;
				S.single_cumulative_time[para.select_route_index] = single.cumulative_time;
				F = new_F;

				// Update the Tabu table.
				Tabu[para.last_node][para.select_route_index] = tabu_step + t;
				Tabu[para.next_node][para.select_route_index] = tabu_step + t;
			}

		}

	}

	/*cout << endl; cout << "After operator: " << endl; printVerifyInformation(S, para, F);*/

}

// 1-1-Swap: select two routes and randomly exchange one customer node from each.
void swap_one_one(routeInformation& S) {
	operatorPara para;
	para.route1_index = randomInt(S.route.size());
	para.route2_index = randomInt(S.route.size());
	while (para.route1_index == para.route2_index) {
		para.route2_index = randomInt(S.route.size());
	}

	if (S.route[para.route1_index].size() - 2 < 1 || S.route[para.route2_index].size() - 2 < 1) {
		cout << "swap_one_one random number is infeasible!" << endl;
		return;
	}

	para.node1_index = randomInt(1, S.route[para.route1_index].size() - 2);
	para.node2_index = randomInt(1, S.route[para.route2_index].size() - 2);

	para.node1 = S.route[para.route1_index][para.node1_index];
	para.node2 = S.route[para.route2_index][para.node2_index];

	/*cout << "Before operator: " << endl; printVerifyInformationForTwoRoute(S, para, F);*/

	if (Tabu[para.node1][para.route2_index] < t && Tabu[para.node2][para.route1_index] < t) {
		tempInformation single1, single2;
		single1.single_route = S.route[para.route1_index];
		single2.single_route = S.route[para.route2_index];

		swap(single1.single_route[para.node1_index], single2.single_route[para.node2_index]);

		/*cout << "The movement is to exchange " << para.node1 << " and " << para.node2 << endl; cout << endl; printVerifySingleRoute(single1); cout << endl; printVerifySingleRoute(single2);*/

		evaluationFunction new_F;

		int old_route1_capacity = S.used_capacity[para.route1_index];
		int old_route2_capacity = S.used_capacity[para.route2_index];
		int new_route1_capacity = old_route1_capacity - demand[para.node1] + demand[para.node2];
		int new_route2_capacity = old_route2_capacity - demand[para.node2] + demand[para.node1];

		// Load delta: new overload minus old overload.
		int old_extra1 = max(0, old_route1_capacity - load_capacity);
		int old_extra2 = max(0, old_route2_capacity - load_capacity);
		int new_extra1 = max(0, new_route1_capacity - load_capacity);
		int new_extra2 = max(0, new_route2_capacity - load_capacity);
		int delta_extra_capacity = (new_extra1 - old_extra1) + (new_extra2 - old_extra2);

		new_F.extra_capacity = F.extra_capacity + delta_extra_capacity;  // Global total overload delta.
		new_F.extra_vehicle = F.extra_vehicle;

		deltaEvaluationForTwoSingle(single1, single2, new_F, para.route1_index, para.route2_index);

		if (new_F.evaluation_obj < F.evaluation_obj) {
			S.route[para.route1_index] = single1.single_route;
			S.arrival_time[para.route1_index] = single1.single_arrival_time;
			S.single_cumulative_time[para.route1_index] = single1.cumulative_time;
			S.used_capacity[para.route1_index] = new_route1_capacity;

			S.route[para.route2_index] = single2.single_route;
			S.arrival_time[para.route2_index] = single2.single_arrival_time;
			S.single_cumulative_time[para.route2_index] = single2.cumulative_time;
			S.used_capacity[para.route2_index] = new_route2_capacity;

			F = new_F;

			// Update the Tabu table.
			Tabu[para.node1][para.route1_index] = tabu_step + t;
			Tabu[para.node2][para.route2_index] = tabu_step + t;
		}

	}

	/*cout << endl; cout << "After operator: " << endl; printVerifyInformationForTwoRoute(S, para, F);*/

}

// 1-2-Swap: select two routes, choose one node from one route and two consecutive nodes from the other, then randomly exchange the two groups.
void swap_one_two(routeInformation& S) {
	operatorPara para;
	para.isChange = false;
	vector<int> available_route;
	for (auto i = 0; i < S.route.size(); i++) {
		if (S.route[i].size() > 3) {
			available_route.push_back(i);
		}
	}

	if (available_route.size() > 1) {
		para.isChange = true;
		para.route1_index = available_route[rand() % available_route.size()];
		para.route2_index = available_route[rand() % available_route.size()];
		while (para.route2_index == para.route1_index) {
			para.route2_index = available_route[rand() % available_route.size()];
		}
	}

	/*cout << "Before operator: " << endl; printVerifyInformationForTwoRoute(S, para, F);*/

	if (para.isChange) {

		if (S.route[para.route1_index].size() - 2 < 1 || S.route[para.route2_index].size() - 3 < 1) {
			cout << "swap_one_two random number is infeasible!" << endl;
			return;
		}

		para.node1_index = randomInt(1, S.route[para.route1_index].size() - 2);        // Select one node in the first route.
		para.node2_index = randomInt(1, S.route[para.route2_index].size() - 3);        // Select two nodes in the second route.
		auto node1 = S.route[para.route1_index][para.node1_index];
		auto last_node2 = S.route[para.route2_index][para.node2_index];
		auto next_node2 = S.route[para.route2_index][para.node2_index + 1];
		if (Tabu[node1][para.node2_index] < t && Tabu[last_node2][para.route1_index] < t && Tabu[next_node2][para.route1_index] < t) {
			vector<int> temp_node2 = { last_node2, next_node2 };
			tempInformation single1, single2;
			single1.single_route = S.route[para.route1_index];
			single2.single_route = S.route[para.route2_index];

			// Delete the original elements.
			single1.single_route.erase(single1.single_route.begin() + para.node1_index);
			single2.single_route.erase(single2.single_route.begin() + para.node2_index, single2.single_route.begin() + para.node2_index + 2);

			// Insert the exchanged elements.
			single1.single_route.insert(single1.single_route.begin() + para.node1_index, temp_node2.begin(), temp_node2.end());
			single2.single_route.insert(single2.single_route.begin() + para.node2_index, node1);

			/*cout << "The movement is to exchange " << node1 << " and " << last_node2 << " " << next_node2 << endl; cout << endl; printVerifySingleRoute(single1); cout << endl; printVerifySingleRoute(single2);*/

			evaluationFunction new_F = F;

			int old_route1_capacity = S.used_capacity[para.route1_index];
			int old_route2_capacity = S.used_capacity[para.route2_index];
			int new_route1_capacity = old_route1_capacity - demand[node1] + demand[last_node2] + demand[next_node2];
			int new_route2_capacity = old_route2_capacity - demand[last_node2] - demand[next_node2] + demand[node1];

			// Load delta: new overload minus old overload.
			int old_extra1 = max(0, old_route1_capacity - load_capacity);
			int old_extra2 = max(0, old_route2_capacity - load_capacity);
			int new_extra1 = max(0, new_route1_capacity - load_capacity);
			int new_extra2 = max(0, new_route2_capacity - load_capacity);
			int delta_extra_capacity = (new_extra1 - old_extra1) + (new_extra2 - old_extra2);

			new_F.extra_capacity = F.extra_capacity + delta_extra_capacity;  // Global total overload delta.
			new_F.extra_vehicle = F.extra_vehicle;

			deltaEvaluationForTwoSingle(single1, single2, new_F, para.route1_index, para.route2_index);

			if (new_F.evaluation_obj < F.evaluation_obj) {
				
				S.route[para.route1_index] = single1.single_route;
				S.arrival_time[para.route1_index] = single1.single_arrival_time;
				S.single_cumulative_time[para.route1_index] = single1.cumulative_time;
				S.used_capacity[para.route1_index] = new_route1_capacity;


				S.route[para.route2_index] = single2.single_route;
				S.arrival_time[para.route2_index] = single2.single_arrival_time;
				S.single_cumulative_time[para.route2_index] = single2.cumulative_time;
				S.used_capacity[para.route2_index] = new_route2_capacity;

				F = new_F;

				// Update the Tabu table.
				Tabu[node1][para.route2_index] = tabu_step + t;
				Tabu[last_node2][para.route1_index] = tabu_step + t;
				Tabu[next_node2][para.route1_index] = tabu_step + t;
			}
		}
	}

	/*cout << endl; cout << "After operator: " << endl; printVerifyInformationForTwoRoute(S, para, F);*/
}

// 2-2-Swap: select two routes, choose two consecutive nodes from each, and randomly exchange the two groups.
void swap_two_two(routeInformation& S) {
	operatorPara para;
	para.isChange = false;
	vector<int> available_route;
	for (auto i = 0; i < S.route.size(); i++) {
		if (S.route[i].size() > 3) {
			available_route.push_back(i);
		}
	}

	if (available_route.size() > 1) {
		para.isChange = true;
		para.route1_index = available_route[rand() % available_route.size()];
		para.route2_index = available_route[rand() % available_route.size()];
		while (para.route2_index == para.route1_index) {
			para.route2_index = available_route[rand() % available_route.size()];
		}
	}

	// If fewer than two routes satisfy the condition, exit the function and skip subsequent operations.
	if (!para.isChange) { return; }

	if (S.route[para.route1_index].size() - 3 < 1 || S.route[para.route2_index].size() - 3 < 1) {
		cout << "swap_two_two random number is infeasible!" << endl;
		return;
	}

	para.node1_index = randomInt(1, S.route[para.route1_index].size() - 3);        // Select one node in the first route.
	para.node2_index = randomInt(1, S.route[para.route2_index].size() - 3);        // Select two nodes in the second route.
	auto last_node1 = S.route[para.route1_index][para.node1_index];
	auto next_node1 = S.route[para.route1_index][para.node1_index + 1];
	auto last_node2 = S.route[para.route2_index][para.node2_index];
	auto next_node2 = S.route[para.route2_index][para.node2_index + 1];

	/*cout << "Before operator: " << endl; printVerifyInformationForTwoRoute(S, para, F);*/

	// If the selected node is tabu, skip subsequent operations.
	if (Tabu[last_node1][para.node2_index] >= t && Tabu[next_node1][para.node2_index] >= t && Tabu[last_node2][para.route1_index] >= t && Tabu[next_node2][para.route1_index] >= t) { return; }

	vector<int> temp_node1 = { last_node1, next_node1 };
	vector<int> temp_node2 = { last_node2, next_node2 };
	tempInformation single1, single2;

	single1.single_route = S.route[para.route1_index];
	single2.single_route = S.route[para.route2_index];

	// Delete the original elements.
	single1.single_route.erase(single1.single_route.begin() + para.node1_index, single1.single_route.begin() + para.node1_index + 2);
	single2.single_route.erase(single2.single_route.begin() + para.node2_index, single2.single_route.begin() + para.node2_index + 2);

	// Insert the exchanged elements.
	single1.single_route.insert(single1.single_route.begin() + para.node1_index, temp_node2.begin(), temp_node2.end());
	single2.single_route.insert(single2.single_route.begin() + para.node2_index, temp_node1.begin(), temp_node1.end());

	/*cout << "The movement is to exchange " << last_node1 << " " << next_node1 << " and " << last_node2 << " " << next_node2 << endl; cout << endl; printVerifySingleRoute(single1); cout << endl; printVerifySingleRoute(single2);*/

	evaluationFunction new_F = F;

	int old_route1_capacity = S.used_capacity[para.route1_index];
	int old_route2_capacity = S.used_capacity[para.route2_index];
	int new_route1_capacity = old_route1_capacity - demand[last_node1] - demand[next_node1] + demand[last_node2] + demand[next_node2];
	int new_route2_capacity = old_route2_capacity - demand[last_node2] - demand[next_node2] + demand[last_node1] + demand[next_node1];

	// Load delta: new overload minus old overload.
	int old_extra1 = max(0, old_route1_capacity - load_capacity);
	int old_extra2 = max(0, old_route2_capacity - load_capacity);
	int new_extra1 = max(0, new_route1_capacity - load_capacity);
	int new_extra2 = max(0, new_route2_capacity - load_capacity);
	int delta_extra_capacity = (new_extra1 - old_extra1) + (new_extra2 - old_extra2);

	new_F.extra_capacity = F.extra_capacity + delta_extra_capacity;  // Global total overload delta.
	new_F.extra_vehicle = F.extra_vehicle;

	deltaEvaluationForTwoSingle(single1, single2, new_F, para.route1_index, para.route2_index);

	/* Debug output for route and evaluation information. */

	if (new_F.evaluation_obj < F.evaluation_obj) {
		
		S.route[para.route1_index] = single1.single_route;
		S.arrival_time[para.route1_index] = single1.single_arrival_time;
		S.single_cumulative_time[para.route1_index] = single1.cumulative_time;
		S.used_capacity[para.route1_index] = new_route1_capacity;


		S.route[para.route2_index] = single2.single_route;
		S.arrival_time[para.route2_index] = single2.single_arrival_time;
		S.single_cumulative_time[para.route2_index] = single2.cumulative_time;
		S.used_capacity[para.route2_index] = new_route2_capacity;

		F = new_F;

		// Update the Tabu table.
		Tabu[last_node1][para.route2_index] = tabu_step + t;
		Tabu[next_node1][para.route2_index] = tabu_step + t;
		Tabu[last_node2][para.route1_index] = tabu_step + t;
		Tabu[next_node2][para.route1_index] = tabu_step + t;
	}

	/*cout << endl; cout << "After operator: " << endl; printVerifyInformationForTwoRoute(S, para, F);*/
}

// 1-Shift: select one route and insert one random customer node from it into another route; opening a new route is not allowed.
void shift(routeInformation& S) {
	operatorPara para;
	para.select_route_index = randomInt(S.route.size());                  // Randomly select a route.

	if (S.route[para.select_route_index].size() - 2 < 1) {
		cout << "shift random number is infeasible!" << endl;
		return;
	}

	para.select_node_index = randomInt(1, S.route[para.select_route_index].size() - 2);    // Randomly select one customer node from this route.

	int insert_route_index = randomInt(S.route.size());              // Randomly select a route as the insertion route for the customer node; opening a new route is not allowed.
	// If para.select_route_index has only one node, opening a new route is not allowed; the selected insert_route_index must differ from the original route.
	while ((S.route[para.select_route_index].size() == 3 && insert_route_index == S.route.size()) || (insert_route_index == para.select_route_index)) {
		insert_route_index = randomInt(S.route.size());
	}

	int node = S.route[para.select_route_index][para.select_node_index];

	if (Tabu[node][insert_route_index] >= t) { return; }                 // If this node is tabu, exit the function directly.

	/*cout << "Before operator: " << endl; printVerifyInformation(S, para, F);*/

	tempInformation single1, single2;                                    // single1 is the route from which nodes are removed; single2 is the route into which nodes are inserted.
	single1.single_route = S.route[para.select_route_index];

	// When the insertion route already exists.
	if (insert_route_index < S.route.size()) {
		single2.single_route = S.route[insert_route_index];
		int insert_index = randomInt(1, S.route[insert_route_index].size() - 1);   // Randomly generate an insertion position.

		single1.single_route.erase(single1.single_route.begin() + para.select_node_index);

		single2.single_route.insert(single2.single_route.begin() + insert_index, node);

		evaluationFunction new_F = F;

		int old_route1_capacity = S.used_capacity[para.select_route_index];
		int old_route2_capacity = S.used_capacity[insert_route_index];
		int new_route1_capacity = old_route1_capacity - demand[node];
		int new_route2_capacity = old_route2_capacity + demand[node];

		// Load delta: new overload minus old overload.
		int old_extra1 = max(0, old_route1_capacity - load_capacity);
		int old_extra2 = max(0, old_route2_capacity - load_capacity);
		int new_extra1 = max(0, new_route1_capacity - load_capacity);
		int new_extra2 = max(0, new_route2_capacity - load_capacity);
		int delta_extra_capacity = (new_extra1 - old_extra1) + (new_extra2 - old_extra2);

		new_F.extra_capacity = F.extra_capacity + delta_extra_capacity;  // Global total overload delta.


		// If a route has become empty, ignore it for now; it will be removed after all calculations are complete.
		if (single1.single_route.size() <= 2) {
			new_F.extra_vehicle = max(0, static_cast<int>(S.route.size()) - 1 - vehicle_num);
		}


		deltaEvaluationForTwoSingle(single1, single2, new_F, para.select_route_index, insert_route_index);


		if (new_F.evaluation_obj < F.evaluation_obj) {
			S.route[para.select_route_index] = single1.single_route;
			S.arrival_time[para.select_route_index] = single1.single_arrival_time;
			S.single_cumulative_time[para.select_route_index] = single1.cumulative_time;
			S.used_capacity[para.select_route_index] = new_route1_capacity;


			S.route[insert_route_index] = single2.single_route;
			S.arrival_time[insert_route_index] = single2.single_arrival_time;
			S.single_cumulative_time[insert_route_index] = single2.cumulative_time;
			S.used_capacity[insert_route_index] = new_route2_capacity;

			F = new_F;

			Tabu[node][para.select_route_index] = tabu_step + t;
			Tabu[node][insert_route_index] = tabu_step + t;
		}
	}
	else { // When the insertion route is newly generated.
		single2.single_route = { 0,node,0 };

		single1.single_route.erase(single1.single_route.begin() + para.select_node_index);

		evaluationFunction new_F = F;

		int old_route1_capacity = S.used_capacity[para.select_route_index];
		int old_route2_capacity = 0;
		int new_route1_capacity = old_route1_capacity - demand[node];
		int new_route2_capacity = old_route2_capacity + demand[node];

		// Load delta: new overload minus old overload.
		int old_extra1 = max(0, old_route1_capacity - load_capacity);
		int old_extra2 = max(0, old_route2_capacity - load_capacity);
		int new_extra1 = max(0, new_route1_capacity - load_capacity);
		int new_extra2 = max(0, new_route2_capacity - load_capacity);
		int delta_extra_capacity = (new_extra1 - old_extra1) + (new_extra2 - old_extra2);

		new_F.extra_capacity = F.extra_capacity + delta_extra_capacity;  // Global total overload delta.

		new_F.extra_vehicle = max(0, static_cast<int>(S.route.size()) + 1 - vehicle_num);

		deltaEvaluation(single1, new_F, para.select_route_index, single2);

		if (new_F.evaluation_obj < F.evaluation_obj) {
			S.route[para.select_route_index] = single1.single_route;
			S.arrival_time[para.select_route_index] = single1.single_arrival_time;
			S.single_cumulative_time[para.select_route_index] = single1.cumulative_time;
			S.used_capacity[para.select_route_index] = new_route1_capacity;

			S.route.push_back(single2.single_route);
			S.arrival_time.push_back(single2.single_arrival_time);
			S.single_cumulative_time.push_back(single2.cumulative_time);
			S.used_capacity.push_back(new_route2_capacity);

			F = new_F;

			Tabu[node][para.select_route_index] = tabu_step + t;
			Tabu[node][S.route.size() - 1] = tabu_step + t;
		}
	}

}

// 2-Shift: select one route and insert two consecutive random nodes from it into another route; opening a new route is not allowed.
void shift_sec(routeInformation& S) {
	operatorPara para;
	para.isChange = false;
	vector<int> available_route;
	for (size_t i = 0; i < S.route.size(); i++) {
		if (S.route[i].size() > 3) {
			available_route.push_back(static_cast<int>(i));
		}
	}

	if (available_route.size() >= 1) {
		para.isChange = true;
		para.select_route_index = available_route[rand() % available_route.size()];
	}

	if (!para.isChange) { return; }

	if (S.route[para.select_route_index].size() - 3 < 1) {
		cout << "shift_sec random number is infeasible!" << endl;
		return;
	}

	para.select_node_index = randomInt(1, S.route[para.select_route_index].size() - 3);

	//int insert_route_index = randomInt(S.route.size() + 1); // Randomly select a route as the insertion route for the customer node; opening a new route is allowed.
	int insert_route_index = randomInt(S.route.size());              // Randomly select a route as the insertion route for the customer node; opening a new route is not allowed.
	// If para.select_route_index has only two nodes, opening a new route is not allowed; the selected insert_route_index must differ from the original route.
	while ((S.route[para.select_route_index].size() == 4 && insert_route_index == S.route.size()) || (insert_route_index == para.select_route_index)) {
		//insert_route_index = randomInt(S.route.size() + 1);
		insert_route_index = randomInt(S.route.size());
	}

	auto last_node = S.route[para.select_route_index][para.select_node_index];
	auto next_node = S.route[para.select_route_index][para.select_node_index + 1];



	if (Tabu[last_node][insert_route_index] >= t || Tabu[next_node][insert_route_index] >= t) { return; }                 // If this node is tabu, exit the function directly.

	/*cout << "Before operator: " << endl; printVerifyInformation(S, para, F);*/

	tempInformation single1, single2;                                    // single1 is the route from which nodes are removed; single2 is the route into which nodes are inserted.
	single1.single_route = S.route[para.select_route_index];

	// Record the nodes that need to be removed.
	vector<int> temp_node = { last_node,next_node };

	// When the insertion route already exists.
	if (insert_route_index < S.route.size()) {
		single2.single_route = S.route[insert_route_index];
		int insert_index = randomInt(1, S.route[insert_route_index].size() - 1);   // Randomly generate an insertion position.

		single1.single_route.erase(single1.single_route.begin() + para.select_node_index, single1.single_route.begin() + para.select_node_index + 2);

		single2.single_route.insert(single2.single_route.begin() + insert_index, temp_node.begin(), temp_node.end());

		evaluationFunction new_F = F;

		int old_route1_capacity = S.used_capacity[para.select_route_index];
		int old_route2_capacity = S.used_capacity[insert_route_index];
		int new_route1_capacity = old_route1_capacity - demand[last_node] - demand[next_node];
		int new_route2_capacity = old_route2_capacity + demand[last_node] + demand[next_node];

		// Load delta: new overload minus old overload.
		int old_extra1 = max(0, old_route1_capacity - load_capacity);
		int old_extra2 = max(0, old_route2_capacity - load_capacity);
		int new_extra1 = max(0, new_route1_capacity - load_capacity);
		int new_extra2 = max(0, new_route2_capacity - load_capacity);
		int delta_extra_capacity = (new_extra1 - old_extra1) + (new_extra2 - old_extra2);

		new_F.extra_capacity = F.extra_capacity + delta_extra_capacity;  // Global total overload delta.

		// If a route has become empty, ignore it for now; it will be removed after all calculations are complete.
		
		if (single1.single_route.size() <= 2) {
			new_F.extra_vehicle = max(0, static_cast<int>(S.route.size()) - 1 - vehicle_num);
		}
		

		deltaEvaluationForTwoSingle(single1, single2, new_F, para.select_route_index, insert_route_index);

		if (new_F.evaluation_obj < F.evaluation_obj) {
			
			S.route[para.select_route_index] = single1.single_route;
			S.arrival_time[para.select_route_index] = single1.single_arrival_time;
			S.single_cumulative_time[para.select_route_index] = single1.cumulative_time;
			S.used_capacity[para.select_route_index] = new_route1_capacity;


			S.route[insert_route_index] = single2.single_route;
			S.arrival_time[insert_route_index] = single2.single_arrival_time;
			S.single_cumulative_time[insert_route_index] = single2.cumulative_time;
			S.used_capacity[insert_route_index] = new_route2_capacity;

			F = new_F;

			Tabu[last_node][para.select_route_index] = tabu_step + t;
			Tabu[last_node][insert_route_index] = tabu_step + t;
			Tabu[next_node][para.select_route_index] = tabu_step + t;
			Tabu[next_node][insert_route_index] = tabu_step + t;
		}
	}
	else {// When the insertion route is newly generated.
		single2.single_route = { 0,last_node,next_node,0 };

		single1.single_route.erase(single1.single_route.begin() + para.select_node_index, single1.single_route.begin() + para.select_node_index + 2);

		evaluationFunction new_F = F;

		int old_route1_capacity = S.used_capacity[para.select_route_index];
		int old_route2_capacity = 0;
		int new_route1_capacity = old_route1_capacity - demand[last_node] - demand[next_node];
		int new_route2_capacity = old_route2_capacity + demand[last_node] + demand[next_node];

		// Load delta: new overload minus old overload.
		int old_extra1 = max(0, old_route1_capacity - load_capacity);
		int old_extra2 = max(0, old_route2_capacity - load_capacity);
		int new_extra1 = max(0, new_route1_capacity - load_capacity);
		int new_extra2 = max(0, new_route2_capacity - load_capacity);
		int delta_extra_capacity = (new_extra1 - old_extra1) + (new_extra2 - old_extra2);

		new_F.extra_capacity = F.extra_capacity + delta_extra_capacity;  // Global total overload delta.

		new_F.extra_vehicle = max(0, static_cast<int>(S.route.size()) + 1 - vehicle_num);

		deltaEvaluation(single1, new_F, para.select_route_index, single2);

		if (new_F.evaluation_obj < F.evaluation_obj) {

			S.route[para.select_route_index] = single1.single_route;
			S.arrival_time[para.select_route_index] = single1.single_arrival_time;
			S.single_cumulative_time[para.select_route_index] = single1.cumulative_time;
			S.used_capacity[para.select_route_index] = new_route1_capacity;

			S.route.push_back(single2.single_route);
			S.arrival_time.push_back(single2.single_arrival_time);
			S.single_cumulative_time.push_back(single2.cumulative_time);
			S.used_capacity.push_back(new_route2_capacity);

			if (S.route.size() != S.used_capacity.size()) {
				std::cerr << "Error: Vector size mismatch after push_back in shift_sec!" << std::endl;
			}

			F = new_F;

			Tabu[last_node][para.select_route_index] = tabu_step + t;
			Tabu[last_node][S.route.size() - 1] = tabu_step + t;
			Tabu[next_node][para.select_route_index] = tabu_step + t;
			Tabu[next_node][S.route.size() - 1] = tabu_step + t;
		}
	}

	/*cout << endl; cout << "After operator: " << endl; printVerifyInformation(S, para, F);*/
}

// 2-opt*: select two routes and exchange the tails from selected nodes to the end.
void opt_star(routeInformation& S) {
	operatorPara para;
	para.route1_index = randomInt(S.route.size());
	para.route2_index = randomInt(S.route.size());
	while (para.route1_index == para.route2_index) {
		para.route2_index = randomInt(S.route.size());
	}

	if (S.route[para.route1_index].size() < 3 || S.route[para.route2_index].size() < 3) {
		cout << "opt_star random number is infeasible!" << endl;
		return;
	}

	para.node1_index = randomInt(1, S.route[para.route1_index].size() - 2);
	
	para.node2_index = randomInt(1, S.route[para.route2_index].size() - 2);
	

	if (para.node1_index == 1 && para.node2_index == 1) { return; }

	/*cout << "Before operator: " << endl; printVerifyInformationForTwoRoute(S, para, F);*/

	tempInformation single1, single2;
	single1.single_route = S.route[para.route1_index];
	single2.single_route = S.route[para.route2_index];

	// Save the two segments.
	vector<int> segment1(single1.single_route.begin() + para.node1_index, single1.single_route.end());
	vector<int> segment2(single2.single_route.begin() + para.node2_index, single2.single_route.end());

	// Delete the original segment.
	single1.single_route.erase(single1.single_route.begin() + para.node1_index, single1.single_route.end());
	single2.single_route.erase(single2.single_route.begin() + para.node2_index, single2.single_route.end());

	// Insert the new segment.
	single1.single_route.insert(single1.single_route.end(), segment2.begin(), segment2.end());
	single2.single_route.insert(single2.single_route.end(), segment1.begin(), segment1.end());


	/*cout << "The movement is to exchange: " << endl; for (auto x : segment1) { cout << x << " "; } cout << endl; for (auto x : segment2) { cout << x << " "; } cout << endl; printVerifySingleRoute(single1); cout << endl; printVerifySingleRoute(single2);*/

	evaluationFunction new_F = F;

	int old_route1_capacity = S.used_capacity[para.route1_index];
	int old_route2_capacity = S.used_capacity[para.route2_index];
	int new_route1_capacity = 0;
	int new_route2_capacity = 0;

	int l = single1.single_route.size();
	for (auto i = 1; i < l - 1; i++) {
		new_route1_capacity += demand[single1.single_route[i]];
	}
	l = single2.single_route.size();
	for (auto i = 1; i < l - 1; i++) {
		new_route2_capacity += demand[single2.single_route[i]];
	}

	// Load delta: new overload minus old overload.
	int old_extra1 = max(0, old_route1_capacity - load_capacity);
	int old_extra2 = max(0, old_route2_capacity - load_capacity);
	int new_extra1 = max(0, new_route1_capacity - load_capacity);
	int new_extra2 = max(0, new_route2_capacity - load_capacity);
	int delta_extra_capacity = (new_extra1 - old_extra1) + (new_extra2 - old_extra2);

	new_F.extra_capacity = F.extra_capacity + delta_extra_capacity;  // Global total overload delta.
	new_F.extra_vehicle = F.extra_vehicle;

	deltaEvaluationForTwoSingle(single1, single2, new_F, para.route1_index, para.route2_index);

	if (new_F.evaluation_obj < F.evaluation_obj) {

		S.route[para.route1_index] = single1.single_route;
		S.arrival_time[para.route1_index] = single1.single_arrival_time;
		S.single_cumulative_time[para.route1_index] = single1.cumulative_time;
		S.used_capacity[para.route1_index] = new_route1_capacity;


		S.route[para.route2_index] = single2.single_route;
		S.arrival_time[para.route2_index] = single2.single_arrival_time;
		S.single_cumulative_time[para.route2_index] = single2.cumulative_time;
		S.used_capacity[para.route2_index] = new_route2_capacity;

		F = new_F;
	}
	/*cout << endl; cout << "After operator: " << endl; printVerifyInformationForTwoRoute(S, para, F);*/

}

// Crossover.
void GPX(const routeInformation& S1, const routeInformation& S2, routeInformation& child, evaluationFunction& childF) {
	// MoE crossover strategy: implement five efficient expert strategies to balance diversity and efficiency.
	int expert_strategy = randomInt(5);

	routeInformation p_S1, p_S2;                              // Temporarily store parent solutions S1 and S2 to avoid damaging the original parent information.
	p_S1 = S1;
	p_S2 = S2;
	vector<int> l_node_S1(customer_num + 1, -1);                                    // Record the route position of each node in parents S1 and S2.
	vector<int> l_node_S2(customer_num + 1, -1);
	for (auto i = 0; i < S1.route.size(); i++) {
		for (auto j = 1; j < S1.route[i].size() - 1; j++) {
			l_node_S1[S1.route[i][j]] = i;
		}
	}

	for (auto i = 0; i < S2.route.size(); i++) {
		for (auto j = 1; j < S2.route[i].size() - 1; j++) {
			l_node_S2[S2.route[i][j]] = i;
		}
	}

	vector<vector<int>> similar(S1.route.size(), vector<int>(S2.route.size()));    // Record the similarity between S1 and S2, namely the number of similar nodes between each pair of routes.
	for (auto i = 1; i < customer_num + 1; i++) {
		similar[l_node_S1[i]][l_node_S2[i]]++;
	}

	vector<int> route_index1;                 // Candidate route indices from parent S1.
	vector<int> route_index2;                 // Candidate route indices from parent S2.
	vector<bool> isSelected_route1(p_S1.route.size(), false);           // Whether each route of parent S1 has already been selected.
	vector<bool> isSelected_route2(p_S2.route.size(), false);           // Whether each route of parent S2 has already been selected.
	unordered_set<int> node_pool_set;         // Node-index candidate pool; use unordered_set to improve lookup and deletion efficiency.
	vector<int> node_pool;                    // Backup vector form for subsequent insertion.
	for (auto i = 0; i < customer_num; i++) {
		int node = i + 1;
		node_pool_set.insert(node);
		node_pool.push_back(node);
	}

	// Precompute the lengths of all routes to reduce repeated calculations.
	vector<double> p_S1_route_lengths(p_S1.route.size(), 0);
	vector<double> p_S2_route_lengths(p_S2.route.size(), 0);
	for (int i = 0; i < p_S1.route.size(); i++) {
		for (int j = 1; j < p_S1.route[i].size(); j++) {
			p_S1_route_lengths[i] += travelTime[p_S1.route[i][j - 1]][p_S1.route[i][j]];
		}
	}
	for (int i = 0; i < p_S2.route.size(); i++) {
		for (int j = 1; j < p_S2.route[i].size(); j++) {
			p_S2_route_lengths[i] += travelTime[p_S2.route[i][j - 1]][p_S2.route[i][j]];
		}
	}

	for (auto k = 0; k < vehicle_num; k++) {
		int max_delta = -1;
		int delta;
		for (auto i = 0; i < p_S1.route.size(); i++) {
			for (auto j = 0; j < p_S2.route.size(); j++) {
				int si = similar[i][j];
				if (si >= max_delta && !isSelected_route1[i] && !isSelected_route2[j]) {
					if (si > max_delta) {
						max_delta = si;
						route_index1.clear();
						route_index2.clear();
					}
					route_index1.push_back(i);
					route_index2.push_back(j);
				}
			}
		}

		// Select parent routes based on the MoE strategy.
		int index = randomInt(route_index1.size());
		int r1 = route_index1[index];
		int r2 = route_index2[index];

		// Use precomputed route lengths to improve efficiency.
		double len1 = p_S1_route_lengths[r1];
		double len2 = p_S2_route_lengths[r2];

		// Decide which parent's route to select according to the expert strategy.
		bool select_from_S1 = false;

		if (expert_strategy == 0) {
			// 1. Cumulative-time priority: the highest-priority strategy, directly selecting the route with smaller cumulative time.
			double cum1 = S1.single_cumulative_time[r1];
			double cum2 = S2.single_cumulative_time[r2];
			select_from_S1 = (cum1 < cum2);
		}
		else if (expert_strategy == 1) {
			// 2. Combined cumulative-time and route-length strategy, emphasizing cumulative time.
			double combined1 = S1.single_cumulative_time[r1] + len1 * 0.3;
			double combined2 = S2.single_cumulative_time[r2] + len2 * 0.3;
			select_from_S1 = (combined1 < combined2);
		}
		else if (expert_strategy == 2) {
			// 3. Combined cumulative-time and capacity-utilization strategy, balancing cumulative time and capacity.
			double cum1 = S1.single_cumulative_time[r1];
			double cum2 = S2.single_cumulative_time[r2];
			int cap1 = load_capacity - S1.used_capacity[r1];
			int cap2 = load_capacity - S2.used_capacity[r2];
			// Cumulative time accounts for 70%, and capacity accounts for 30%.
			double score1 = cum1 * 0.7 + (1.0 / max(1, cap1)) * 0.3;
			double score2 = cum2 * 0.7 + (1.0 / max(1, cap2)) * 0.3;
			select_from_S1 = (score1 < score2);
		}
		else if (expert_strategy == 3) {
			// 4. Route-length priority: select the route with shorter length.
			select_from_S1 = (len1 < len2);
		}
		else {
			// 5. Cumulative-time-dominant random strategy: choose the route with smaller cumulative time with 80% probability and choose randomly with 20% probability.
			double cum1 = S1.single_cumulative_time[r1];
			double cum2 = S2.single_cumulative_time[r2];
			bool cum_better = (cum1 < cum2);
			select_from_S1 = (randomDouble() < 0.8) ? cum_better : (randomDouble() > 0.5);
		}

		// Select and process the route.
		int selected_route;
		if (select_from_S1) {
			selected_route = r1;
			isSelected_route1[selected_route] = true;
			child.route.push_back(p_S1.route[selected_route]);

			// Process nodes in the parent route.
			vector<int> tem = p_S1.route[selected_route];
			for (auto i = 1; i < tem.size() - 1; i++) {
				int node = tem[i];
				// Remove node from node_pool_set (O(1) operation).
				node_pool_set.erase(node);
				// Remove node from p_S1.
				int loc = l_node_S1[node];
				// Process only the current route because each node belongs to only one route.
				for (auto s1 = 1; s1 < p_S1.route[loc].size(); s1++) {
					if (p_S1.route[loc][s1] == node) {
						p_S1.route[loc].erase(p_S1.route[loc].begin() + s1);
						break;
					}
				}
				// Remove node from p_S2.
				loc = l_node_S2[node];
				for (auto s2 = 1; s2 < p_S2.route[loc].size(); s2++) {
					if (p_S2.route[loc][s2] == node) {
						p_S2.route[loc].erase(p_S2.route[loc].begin() + s2);
						break;
					}
				}
			}
		}
		else {
			selected_route = r2;
			isSelected_route2[selected_route] = true;
			child.route.push_back(p_S2.route[selected_route]);

			// Process nodes in the parent route.
			vector<int> tem = p_S2.route[selected_route];
			for (auto i = 1; i < tem.size() - 1; i++) {
				int node = tem[i];
				// Remove node from node_pool_set (O(1) operation).
				node_pool_set.erase(node);
				// Remove node from p_S1.
				int loc = l_node_S1[node];
				for (auto s1 = 1; s1 < p_S1.route[loc].size(); s1++) {
					if (p_S1.route[loc][s1] == node) {
						p_S1.route[loc].erase(p_S1.route[loc].begin() + s1);
						break;
					}
				}
				// Remove node from p_S2.
				loc = l_node_S2[node];
				// Process only the current route because each node belongs to only one route.
				for (auto s2 = 1; s2 < p_S2.route[loc].size(); s2++) {
					if (p_S2.route[loc][s2] == node) {
						p_S2.route[loc].erase(p_S2.route[loc].begin() + s2);
						break;
					}
				}
			}
		}
	}

	// Efficient customer insertion strategy: consider capacity constraints and insertion cost.
	if (!node_pool_set.empty()) {
		// Precompute the current capacity of each route.
		vector<int> current_capacity(child.route.size(), 0);
		for (int r = 0; r < child.route.size(); r++) {
			for (int j = 1; j < child.route[r].size() - 1; j++) {
				current_capacity[r] += demand[child.route[r][j]];
			}
		}

		// Create a temporary vector for traversal because unordered_set cannot be modified while iterating.
		vector<int> temp_node_pool(node_pool_set.begin(), node_pool_set.end());
		for (auto& node : temp_node_pool) {
			// Find the best insertion position for each remaining node.
			double best_insert_cost = numeric_limits<double>::max();
			int best_route = 0;
			int best_pos = 1;

			for (int r = 0; r < child.route.size(); r++) {
				// Quickly check the capacity constraint.
				if (current_capacity[r] + demand[node] > load_capacity) {
					continue;
				}

				// Check only key positions: the first three, last three, and three random positions to reduce computation.
				vector<int> check_positions;
				// Add the first three positions.
				for (int pos = 1; pos < min(4, (int)child.route[r].size()); pos++) {
					check_positions.push_back(pos);
				}
				// Add the last three positions.
				for (int pos = max(1, (int)child.route[r].size() - 3); pos < child.route[r].size(); pos++) {
					check_positions.push_back(pos);
				}
				// Add three random positions.
				if (child.route[r].size() > 6) {
					for (int i = 0; i < 3; i++) {
						check_positions.push_back(randomInt(child.route[r].size() - 2) + 1);
					}
				}

				// Improved insertion-cost calculation: consider route length and cumulative-time impact.
				for (int pos : check_positions) {
					if (pos <= 0 || pos >= child.route[r].size()) continue;

					int prev_node = child.route[r][pos - 1];
					int next_node = child.route[r][pos];

					// Calculate route-length change.
					double length_cost = travelTime[prev_node][node] + travelTime[node][next_node] - travelTime[prev_node][next_node];

					// Estimate cumulative-time impact by considering time changes before and after the insertion position.
					// Time from the previous node to the inserted node.
					double time_prev_to_node = travelTime[prev_node][node];
					// Time from the inserted node to the next node.
					double time_node_to_next = travelTime[node][next_node];
					// Original time from the previous node to the next node.
					double time_prev_to_next = travelTime[prev_node][next_node];

					// Cumulative-time impact estimate: assume the inserted node affects arrival times of all subsequent nodes.
					// Time delay = new route time - original route time.
					double time_delay = (time_prev_to_node + time_node_to_next) - time_prev_to_next;
					// Number of subsequent nodes.
					int num_following_nodes = child.route[r].size() - pos;
					// Cumulative-time cost: time delay multiplied by the number of subsequent nodes (simplified estimate).
					double cumulative_cost = time_delay * num_following_nodes;

					// Integrated insertion cost: weighted route length and cumulative time.
					double insert_cost = length_cost + 0.5 * cumulative_cost;

					if (insert_cost < best_insert_cost) {
						best_insert_cost = insert_cost;
						best_route = r;
						best_pos = pos;
					}
				}
			}

			// Execute the best insertion.
			child.route[best_route].insert(child.route[best_route].begin() + best_pos, node);
			current_capacity[best_route] += demand[node];
		}
	}

	// Calculate offspring information.
	calInformation(child, childF);

	// Efficient 2-opt local search: improve solution quality and reduce calInformation calls.
	for (int r = 0; r < child.route.size(); r++) {
		if (child.route[r].size() <= 4) continue; // Apply 2-opt to routes of medium or greater length to improve search coverage.

		bool improved = true;
		int iteration = 0;
		const int max_iterations = 5; // Increase the number of iterations to improve search depth.
		int no_improve_count = 0;
		const int max_no_improve = 3; // Add a non-improvement termination condition to allow more search.

		while (improved && iteration < max_iterations && no_improve_count < max_no_improve) {
			improved = false;
			iteration++;
			no_improve_count++;

			double best_delta = 0;
			int best_i = 1, best_j = 2;

			// First find the best exchange position and calculate the route-length change.
			const int route_size = child.route[r].size();
			// Further reduce the search range to improve efficiency.
			for (int i = 1; i < min(route_size - 3, 25); i++) { // Increase to the first 25 positions to expand the search range.
				// Further reduce exchange length to improve efficiency.
				for (int j = i + 2; j < min(i + 10, route_size - 1); j++) { // Increase the exchange length to 10 to improve search diversity.
					// Calculate the route-length change of the 2-opt exchange.
					int a = child.route[r][i - 1];
					int b = child.route[r][i];
					int c = child.route[r][j];
					int d = child.route[r][j + 1];

					double delta = travelTime[a][c] + travelTime[b][d] - travelTime[a][b] - travelTime[c][d];

					if (delta < best_delta - 1e-6) { // Consider floating-point error and record only the best exchange.
						best_delta = delta;
						best_i = i;
						best_j = j;
					}
				}
			}

			// If an improvement is found, execute the exchange.
			if (best_delta < -1e-6) {
				reverse(child.route[r].begin() + best_i, child.route[r].begin() + best_j + 1);
				improved = true;
				no_improve_count = 0;
			}
		}
	}

	// After the 2-opt operation is complete, update solution information.
	calInformation(child, childF);
}

void printVerifyInformation(const routeInformation& S, const operatorPara& para, const evaluationFunction& F) {
	std::cout << endl;
	std::cout << "S.route[para.select_route_index]: " << endl;
	for (auto i = 0; i < S.route[para.select_route_index].size(); i++) {
		std::cout << S.route[para.select_route_index][i] << " ";
	}
	std::cout << endl;


	std::cout << "S.arrival_time[para.select_route_index]: " << endl;
	for (auto i = 0; i < S.arrival_time[para.select_route_index].size(); i++) {
		std::cout << S.arrival_time[para.select_route_index][i] << " ";
	}
	std::cout << endl;

	std::cout << "S.single_cumulative_time[para.select_route_index]: " << S.single_cumulative_time[para.select_route_index] << endl;
	std::cout << endl;
	std::cout << "F: " << " ";
	std::cout << F.evaluation_obj << " " << F.cumulative_time << " " << F.extra_capacity << endl;
	std::cout << endl;
}

void printVerifySingleRoute(const tempInformation& single) {
	std::cout << endl;
	std::cout << "single.single_route: " << endl;
	for (auto i = 0; i < single.single_route.size(); i++) {
		std::cout << single.single_route[i] << " ";
	}
	std::cout << endl;


	std::cout << "single.single_arrival_time: " << endl;
	for (auto i = 0; i < single.single_arrival_time.size(); i++) {
		std::cout << single.single_arrival_time[i] << " ";
	}
	std::cout << endl;
}

void printVerifyInformationForTwoRoute(const routeInformation& S, const operatorPara& para, const evaluationFunction& F) {
	std::cout << endl;
	std::cout << "S.route[para.route1_index]: " << endl;
	for (auto i = 0; i < S.route[para.route1_index].size(); i++) {
		std::cout << S.route[para.route1_index][i] << " ";
	}
	std::cout << endl;


	std::cout << "S.arrival_time[para.route1_index]: " << endl;
	for (auto i = 0; i < S.arrival_time[para.route1_index].size(); i++) {
		std::cout << S.arrival_time[para.route1_index][i] << " ";
	}
	std::cout << endl;

	
	std::cout << "S.single_cumulative_time[para.route1_index]: " << S.single_cumulative_time[para.route1_index] << endl;
	std::cout << endl;
	std::cout << "S.used_capacity[para.route1_index]: " << S.used_capacity[para.route1_index] << endl;
	std::cout << endl;


	std::cout << endl;
	std::cout << "S.route[para.route2_index]: " << endl;
	for (auto i = 0; i < S.route[para.route2_index].size(); i++) {
		std::cout << S.route[para.route2_index][i] << " ";
	}
	std::cout << endl;

	std::cout << "S.arrival_time[para.route2_index]: " << endl;
	for (auto i = 0; i < S.arrival_time[para.route2_index].size(); i++) {
		std::cout << S.arrival_time[para.route2_index][i] << " ";
	}
	std::cout << endl;

	
	std::cout << "S.single_cumulative_time[para.route2_index]: " << S.single_cumulative_time[para.route2_index] << endl;
	std::cout << endl;
	
	std::cout << "S.used_capacity[para.route2_index]: " << S.used_capacity[para.route2_index] << endl;
	std::cout << endl;
	std::cout << "F: " << " ";
	std::cout << F.evaluation_obj << " " << F.cumulative_time << " " << F.extra_capacity << endl;
	std::cout << endl;
}

// Set the operator order as a static variable so success-rate-based adjustments can affect subsequent calls.
static vector<int> index_NL = { 8, 9, 1, 3, 4, 2, 5, 6, 7 };

// Fixed variable-neighborhood search process: CMT1 2237.1 in 82.125 seconds.
// Variable-neighborhood search strategy based on AVNS-LNS.
void RVNS() {
	int r_NL, index_r_NL;
	int count = 0;
	int l_index_NL = index_NL.size();

	// Paper-based adaptive strategy: record the success rate of each neighborhood.
	static vector<double> success_rate(op_num, 0.5);
	static vector<int> success_count(op_num, 0);
	static vector<int> total_count(op_num, 1);

	// Use the operator order adjusted by success rate instead of fully shuffling on every call.
	// Apply limited randomization only under specific conditions to preserve the success-rate ordering advantage.
	static int shuffle_count = 0;
	shuffle_count++;
	// Randomize the operator order once every 10 calls to balance exploration and exploitation.
	if (shuffle_count % 10 == 0) {
		// Randomize only the latter 50% of the operator order and preserve the success-rate advantage of the top 50%.
		int mid_point = l_index_NL / 2;
		shuffle(index_NL.begin() + mid_point, index_NL.end(), default_random_engine(randomInt(1000)));
	}

	while (count < l_index_NL) {
		r_NL = index_NL[count];
		evaluationFunction old_F = F;
		switch (r_NL) {
		case 1:
			relocate(S); // 1-insertion: insert a single customer.
			break;
		case 2:
			exchange(S); // 1-1 exchange: exchange two customers.
			break;
		case 3:
			swap_one_one(S); // 1-1 swap: exchange one pair of customers.
			break;
		case 4:
			shift(S); // 2-insertion: insert two consecutive customers.
			break;
		case 5:
			relocate_sec(S); // Another relocate operation.
			break;
		case 6:
			swap_one_two(S); // 1-2 swap: exchange one customer with two customers.
			break;
		case 7:
			swap_two_two(S); // 2-2 swap: exchange two pairs of customers.
			break;
		case 8: {
			// 2-opt: intra-route optimization; dynamically adjust the number of executions based on success rate.
			int opt_iterations = success_rate[8] > 0.3 ? 2 : 1;
			for (int i = 0; i < opt_iterations; i++) {
				opt(S);
			}
			break;
		}
		case 9: {
			// 2-opt*: inter-route optimization; dynamically adjust the number of executions based on success rate.
			int opt_star_iterations = success_rate[9] > 0.3 ? 2 : 1;
			for (int i = 0; i < opt_star_iterations; i++) {
				opt_star(S);
			}
			break;
		}
		default:
			break;
		}

		total_count[r_NL]++;
		op_used_num[r_NL]++;
		if (F.evaluation_obj < old_F.evaluation_obj) {
			success_count[r_NL]++;
			op_effec_num[r_NL]++;
			// Reset the counter and continue exploring the current neighborhood.
			count = 0;
		}
		else {
			count++;
		}

		// Update the success rate.
		success_rate[r_NL] = (double)success_count[r_NL] / total_count[r_NL];
	}

	// Dynamically adjust neighborhood order based on success rate, once every 50 calls for more frequent adjustment.
	static int adjust_count = 0;
	adjust_count++;
	if (adjust_count % 50 == 0) {
		// Sort neighborhood structures by success rate.
		vector<pair<double, int>> rate_op;
		for (int op : index_NL) {
			rate_op.emplace_back(success_rate[op], op);
		}
		// Sort in descending order.
		sort(rate_op.rbegin(), rate_op.rend());
		// Update neighborhood order, preserve the success-rate advantage of the top 70%, and randomize the remaining 30%.
		int keep_ratio = (int)(index_NL.size() * 0.7);
		for (int i = 0; i < keep_ratio; i++) {
			index_NL[i] = rate_op[i].second;
		}
		// Randomize the remaining operator order.
		vector<int> temp_ops;
		for (int i = keep_ratio; i < index_NL.size(); i++) {
			temp_ops.push_back(rate_op[i].second);
		}
		shuffle(temp_ops.begin(), temp_ops.end(), default_random_engine(randomInt(1000)));
		for (int i = 0; i < temp_ops.size(); i++) {
			index_NL[keep_ratio + i] = temp_ops[i];
		}
	}
}



// ALNS Parameters and Structures
struct ALNS_Parameters {
	double sigma1 = 50; // New global best
	double sigma2 = 20; // Better than current
	double sigma3 = 5;  // Accepted
	double reaction_factor = 0.1;

	vector<double> removal_weights;
	vector<double> insertion_weights;
	vector<double> removal_scores;
	vector<double> insertion_scores;
	vector<int> removal_counts;
	vector<int> insertion_counts;

	// Removals: 0: Random, 1: Worst, 2: Shaw(Time), 3: Shaw(Dist)
	// Insertions: 0: Greedy, 1: Regret-2
	int n_removal = 4;
	int n_insertion = 2;

	void init() {
		removal_weights.assign(n_removal, 1.0);
		insertion_weights.assign(n_insertion, 1.0);
		removal_scores.assign(n_removal, 0.0);
		insertion_scores.assign(n_insertion, 0.0);
		removal_counts.assign(n_removal, 0);
		insertion_counts.assign(n_insertion, 0);
	}

	// Roulette wheel selection
	int select_removal() {
		double total = 0;
		for (double w : removal_weights) total += w;
		double r = randomDouble() * total;
		double sum = 0;
		for (int i = 0; i < n_removal; i++) {
			sum += removal_weights[i];
			if (r <= sum) return i;
		}
		return n_removal - 1;
	}

	int select_insertion() {
		double total = 0;
		for (double w : insertion_weights) total += w;
		double r = randomDouble() * total;
		double sum = 0;
		for (int i = 0; i < n_insertion; i++) {
			sum += insertion_weights[i];
			if (r <= sum) return i;
		}
		return n_insertion - 1;
	}

	void update_scores(int r_op, int i_op, double score) {
		removal_scores[r_op] += score;
		insertion_scores[i_op] += score;
		removal_counts[r_op]++;
		insertion_counts[i_op]++;
	}

	void update_weights() {
		for (int i = 0; i < n_removal; i++) {
			if (removal_counts[i] > 0) {
				removal_weights[i] = removal_weights[i] * (1 - reaction_factor) + reaction_factor * (removal_scores[i] / removal_counts[i]);
				removal_scores[i] = 0;
				removal_counts[i] = 0;
			}
		}
		for (int i = 0; i < n_insertion; i++) {
			if (insertion_counts[i] > 0) {
				insertion_weights[i] = insertion_weights[i] * (1 - reaction_factor) + reaction_factor * (insertion_scores[i] / insertion_counts[i]);
				insertion_scores[i] = 0;
				insertion_counts[i] = 0;
			}
		}
	}
} alns_param;

bool alns_initialized = false;

// ALNS Helper Functions

// Calculate removal cost of a node (approximation for efficiency)
double calculate_removal_cost(const routeInformation& s, int route_idx, int node_idx) {
	// Cost reduction = current_obj - new_obj
	// Removing node i reduces obj by:
	// 1. Arrival time of i (t_i)
	// 2. Delay reduction for all subsequent nodes j: (t_i + t_{i->next}) - t_{prev->next}

	if (route_idx >= s.route.size() || node_idx <= 0 || node_idx >= s.route[route_idx].size() - 1) return 0;

	int node = s.route[route_idx][node_idx];
	int prev = s.route[route_idx][node_idx - 1];
	int next = s.route[route_idx][node_idx + 1];

	double t_i = s.arrival_time[route_idx][node_idx];
	double time_saved = t_i; // Node i is removed, so its arrival time is gone from sum

	// Calculate time shift for subsequent nodes
	double old_segment = travelTime[prev][node] + travelTime[node][next];


	double delta = (travelTime[prev][node] + travelTime[node][next]) - travelTime[prev][next];
	// Note: serviceTime[node] contributes to delay of next nodes.

	int nodes_after = s.route[route_idx].size() - node_idx - 2;
	/*int nodes_after = 0; for (int k = node_idx + 1; k < s.route[route_idx].size() - 1; k++) { nodes_after++; }*/
	return t_i + delta * nodes_after;
}

// ALNS Operators

void remove_random(routeInformation& s, int q, vector<int>& removed) {
	for (int k = 0; k < q; k++) {
		if (s.route.empty()) break;
		int r = randomInt(s.route.size());
		if (s.route[r].size() <= 2) { k--; continue; } // Empty route or only depot

		vector<int> valid_indices;
		for (int i = 1; i < s.route[r].size() - 1; i++) {
			valid_indices.push_back(i);
		}
		if (valid_indices.empty()) { k--; continue; }

		int idx = valid_indices[randomInt(valid_indices.size())];
		removed.push_back(s.route[r][idx]);
		s.route[r].erase(s.route[r].begin() + idx);
		s.arrival_time[r].erase(s.arrival_time[r].begin() + idx);
		s.used_capacity[r] -= demand[removed.back()];
		// Update arrival times later
	}
}

void remove_worst(routeInformation& s, int q, vector<int>& removed) {
	// Identify q nodes with highest removal cost (gain)
	vector<pair<double, pair<int, int>>> costs; // cost -> {route, index}

	// Recalculate arrival times first to be safe
	// Assuming s is up to date or we recalculate locally

	for (int r = 0; r < s.route.size(); r++) {
		for (int i = 1; i < s.route[r].size() - 1; i++) {
			double cost = calculate_removal_cost(s, r, i);
			// Add some noise for randomness (Paper 1 uses p >= 1 parameter)
			double noise = randomDouble() * 0.2 + 0.9; // 0.9 - 1.1
			costs.push_back({ cost * noise, {r, i} });
		}
	}

	sort(costs.rbegin(), costs.rend()); // Highest cost first

	// Remove top q, careful about indices shifting
	// Better strategy: mark for removal, then remove
	vector<pair<int, int>> to_remove;
	for (int k = 0; k < min((int)costs.size(), q); k++) {
		to_remove.push_back(costs[k].second);
		removed.push_back(s.route[costs[k].second.first][costs[k].second.second]);
	}

	// Sort to_remove by route then index descending to remove safely
	sort(to_remove.begin(), to_remove.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
		if (a.first != b.first) return a.first < b.first;
		return a.second > b.second;
		});

	for (auto& p : to_remove) {
		s.route[p.first].erase(s.route[p.first].begin() + p.second);
		s.arrival_time[p.first].erase(s.arrival_time[p.first].begin() + p.second);
		s.used_capacity[p.first] -= demand[removed.back()]; // Approximate, fixed later
	}
}

void remove_shaw(routeInformation& s, int q, vector<int>& removed, bool use_time) {
	// Shaw removal
	if (s.route.empty()) return;

	// Pick random seed
	int seed_r = randomInt(s.route.size());
	if (s.route[seed_r].size() <= 2) { remove_random(s, q, removed); return; }
	int seed_idx = randomInt(1, s.route[seed_r].size() - 2);
	

	int seed_node = s.route[seed_r][seed_idx];
	removed.push_back(seed_node);
	s.route[seed_r].erase(s.route[seed_r].begin() + seed_idx);
	s.arrival_time[seed_r].erase(s.arrival_time[seed_r].begin() + seed_idx);
	s.used_capacity[seed_r] -= demand[seed_node];

	while (removed.size() < q) {
		// Select node most related to a random node in removed
		int r_node = removed[randomInt(removed.size())];

		double best_rel = 1e9;
		int best_r = -1, best_i = -1;

		// Search for most related
		for (int r = 0; r < s.route.size(); r++) {
			for (int i = 1; i < s.route[r].size() - 1; i++) {
				int curr = s.route[r][i];
				double rel = 0;
				if (use_time) {
					// R = |t_i - t_j|
					// Need arrival time. Assuming s is somewhat valid.
					// Approximate arrival time from global arrays or s structure
					// Since we modify s, arrival times might be stale.
					// Using travelTime as proxy for distance-based Shaw if times are stale?
					// Let's use distance + demand diff
					rel = travelTime[r_node][curr] + abs(demand[r_node] - demand[curr]);
				}
				else {
					// R = d_ij
					rel = travelTime[r_node][curr];
				}

				if (rel < best_rel) {
					best_rel = rel;
					best_r = r;
					best_i = i;
				}
			}
		}

		if (best_r != -1) {
			removed.push_back(s.route[best_r][best_i]);
			s.route[best_r].erase(s.route[best_r].begin() + best_i);
			s.arrival_time[best_r].erase(s.arrival_time[best_r].begin() + best_i);
			s.used_capacity[best_r] -= demand[removed.back()];
		}
		else {
			break; // No more nodes?
		}
	}
}

// Optimized calculate_insertion_cost with pre-calculated nodes_after
double calculate_insertion_cost(const routeInformation& s, int route_idx, int pos_idx, int node, int nodes_after) {
	// Calculate cost increase
	if (route_idx >= s.route.size()) return 1e9;

	int prev = s.route[route_idx][pos_idx - 1];
	int next = s.route[route_idx][pos_idx];

	double delta = travelTime[prev][node] + travelTime[node][next] - travelTime[prev][next];

	// Arrival time of inserted node
	double t_prev = s.arrival_time[route_idx][pos_idx - 1];
	double t_arrive_node = t_prev + travelTime[prev][node]; // Approx
	double cost_increase = t_arrive_node + delta * nodes_after;

	return cost_increase;
}

// Wrapper for backward compatibility (if needed, but we should update calls)
double calculate_insertion_cost(const routeInformation& s, int route_idx, int pos_idx, int node) {
	int nodes_after = 0;
	for (int k = pos_idx; k < s.route[route_idx].size(); k++) {
		if (s.route[route_idx][k] != 0) nodes_after++;
	}
	return calculate_insertion_cost(s, route_idx, pos_idx, node, nodes_after);
}

// Helper to update arrival times for a single route
void update_route_arrival_time(routeInformation& S, int r) {
	S.arrival_time[r].clear();
	double current_time = 0;
	S.arrival_time[r].push_back(current_time);

	for (int j = 0; j < S.route[r].size() - 1; j++) {
		int u = S.route[r][j];
		int v = S.route[r][j + 1];
		current_time += travelTime[u][v];
		S.arrival_time[r].push_back(current_time);
	}
}

static default_random_engine rng(randomInt(1000));

void insert_greedy(routeInformation& s, vector<int>& removed) {
	// Insert one by one, best position
	// Shuffle removed to avoid bias
	shuffle(removed.begin(), removed.end(), rng);

	for (int node : removed) {
		double best_cost = 1e18;
		int best_r = -1;
		int best_p = -1;

		for (int r = 0; r < s.route.size(); r++) {
			if (s.used_capacity[r] + demand[node] > load_capacity) continue;

			// Pre-calculate total customers in route
			int total_customers = 0;
			for (size_t k = 0; k < s.route[r].size(); ++k) {
				if (s.route[r][k] != 0) total_customers++;
			}

			int customers_passed = 0;
			for (int p = 1; p < s.route[r].size(); p++) {
				int nodes_after = total_customers - customers_passed;
				double cost = calculate_insertion_cost(s, r, p, node, nodes_after);
				if (cost < best_cost) {
					best_cost = cost;
					best_r = r;
					best_p = p;
				}

				// Update customers_passed for next iteration
				// If current node at p is a customer, it is part of "nodes_after" for current p.
				// For next p (p+1), the node at current p is now "passed".
				if (s.route[r][p] != 0) {
					customers_passed++;
				}
			}
		}

		if (best_r != -1) {
			s.route[best_r].insert(s.route[best_r].begin() + best_p, node);
			s.used_capacity[best_r] += demand[node];
			update_route_arrival_time(s, best_r);
		}
		else {
			// Create new route
			s.route.push_back({ 0, node, 0 });
			s.arrival_time.emplace_back();
			update_route_arrival_time(s, s.route.size() - 1);
			s.used_capacity.push_back(demand[node]);
			s.single_cumulative_time.push_back(0); // Reset later
		}
	}
}

void insert_regret(routeInformation& s, vector<int>& removed, int k_regret) {
	// Regret-k insertion
	while (!removed.empty()) {
		int best_node_idx = -1;
		double max_regret = -1e18;
		int best_r_global = -1;
		int best_p_global = -1;

		for (int i = 0; i < removed.size(); i++) {
			int node = removed[i];
			vector<pair<double, pair<int, int>>> moves; // cost -> {r, p}
			moves.reserve(s.route.size() * 20); // Pre-reserve to reduce allocation

			for (int r = 0; r < s.route.size(); r++) {
				if (s.used_capacity[r] + demand[node] > load_capacity) continue;

				// Pre-calculate total customers
				int total_customers = 0;
				for (size_t k = 0; k < s.route[r].size(); ++k) {
					if (s.route[r][k] != 0) total_customers++;
				}

				int customers_passed = 0;
				for (int p = 1; p < s.route[r].size(); p++) {
					int nodes_after = total_customers - customers_passed;
					double cost = calculate_insertion_cost(s, r, p, node, nodes_after);
					moves.push_back({ cost, {r, p} });

					if (s.route[r][p] != 0) {
						customers_passed++;
					}
				}
			}

			// If no moves, handled separately (new route)
			if (moves.empty()) {
				// Treat as infinite cost, forcing immediate handling or deferred
				// For simplicity, skip calculation and let it be handled if selected (which it won't be if others have valid moves)
				// Or add a move with high cost
				moves.push_back({ 1e9, {-1, -1} });
			}

			sort(moves.begin(), moves.end());

			double regret = 0;
			if (moves.size() >= k_regret) {
				regret = moves[k_regret - 1].first - moves[0].first;
			}
			else if (moves.size() > 0) {
				regret = moves.back().first - moves[0].first;
			}

			if (regret > max_regret) {
				max_regret = regret;
				best_node_idx = i;
				best_r_global = moves[0].second.first;
				best_p_global = moves[0].second.second;
			}
		}

		if (best_node_idx != -1 && best_r_global != -1) {
			int node = removed[best_node_idx];
			if (best_r_global == -1) { // Should not happen if moves not empty logic is correct
				// Fallback to new route logic below
				goto create_new_route;
			}
			s.route[best_r_global].insert(s.route[best_r_global].begin() + best_p_global, node);
			s.used_capacity[best_r_global] += demand[node];
			update_route_arrival_time(s, best_r_global);
			removed.erase(removed.begin() + best_node_idx);
		}
		else {
		create_new_route:
			// Cannot insert remaining nodes in existing routes
			// Add new route for the first remaining node
			int node = removed[0];
			s.route.push_back({ 0, node, 0 });
			s.used_capacity.push_back(demand[node]);
			// Need to resize vectors to avoid crash
			s.arrival_time.emplace_back();
			update_route_arrival_time(s, s.route.size() - 1);
			s.single_cumulative_time.push_back(0);
			removed.erase(removed.begin());
		}
	}
}

// Adaptive Large Neighborhood Search
void ALNS() {
	if (!alns_initialized) {
		alns_param.init();
		alns_initialized = true;
	}

	// 1. Select Operators
	int r_op = alns_param.select_removal();
	int i_op = alns_param.select_insertion();

	// 2. Determine number of customers to remove (gamma)
	// Random between 10% and 30% of customers, or min 4
	int min_rem = max(4, (int)(0.1 * customer_num));
	int max_rem = max(5, (int)(0.3 * customer_num));
	int q = randomInt(min_rem, max_rem + 1);

	routeInformation s_backup = S;
	evaluationFunction f_backup = F;

	// 3. Apply Removal
	vector<int> removed_nodes;

	// Recalculate full info before starting (to ensure valid arrival times for heuristics)
	// calInformation(S, F); // Optimization: S is assumed valid from main loop or previous iteration

	switch (r_op) {
	case 0: remove_random(S, q, removed_nodes); break;
	case 1: remove_worst(S, q, removed_nodes); break;
	case 2: remove_shaw(S, q, removed_nodes, true); break; // Time
	case 3: remove_shaw(S, q, removed_nodes, false); break; // Distance
	}

	// Recalculate arrival times after removal to ensure correct insertion costs
	// calInformation(S, F); // Too slow
	// Optimization: update only arrival times for all routes
	for (int r = 0; r < S.route.size(); ++r) {
		update_route_arrival_time(S, r);
	}

	// 4. Apply Insertion
	switch (i_op) {
	case 0: insert_greedy(S, removed_nodes); break;
	case 1: insert_regret(S, removed_nodes, 2); break;
	}

	// 5. Evaluation and Acceptance
	calInformation(S, F); // Full calculation of new solution

	double score = 0;
	if (F.evaluation_obj < optimal.obj) { // New Global Best
		score = alns_param.sigma1;
		optimal.route = S.route;
		optimal.obj = F.cumulative_time;
		// Accepted
	}
	else if (F.evaluation_obj < f_backup.evaluation_obj) { // Better than current
		score = alns_param.sigma2;
		// Accepted
	}
	else {
		// Simulated Annealing Acceptance
		double temp = 100.0 * pow(0.999, t); // Simple cooling
		double delta = F.evaluation_obj - f_backup.evaluation_obj;
		if (randomDouble() < exp(-delta / temp)) {
			score = alns_param.sigma3;
			// Accepted
		}
		else {
			// Rejected
			S = s_backup;
			F = f_backup;
		}
	}

	// 6. Update Scores
	alns_param.update_scores(r_op, i_op, score);

	// Periodically update weights (every 50 calls)
	static int alns_iter = 0;
	alns_iter++;
	if (alns_iter % 50 == 0) {
		alns_param.update_weights();
	}
}

// VNS function incorporating RVNS and ALNS
void VNS() {
	// RVNS for local intensification
	RVNS();
	// ALNS for large neighborhood diversification
	ALNS();
}



int main()
{
	int c = 1;
	int c_max = 1;
	vector<double> cumulative_t;
	vector<double> op_time;
	while (c <= c_max) {
		t = 0;
		//readInitial(inFile);
		readInitial_CMT(inFile);
		calDistance();
		routeInformation elit;
		evaluationFunction elitF;
		GRASP_initialization(elit);
		S = elit;
		int it = p.max_initial_LS;
		while (it) {
			RVNS();
			it--;
		}
		elit = S;
		elitF = F;

		routeInformation newS1;
		GRASP_initialization(newS1);
		S = newS1;
		it = p.max_initial_LS;
		while (it) {
			RVNS();
			it--;
		}

		verify(S);
		verify(S, F);
		vector<double> convergence;

		startTime = clock();//Timing starts.
		endTime = startTime;
		evaluationFunction best_F = F;
		routeInformation best_S = S;
		int no_improve = 0;
		int shake_no_improve = 0;
		int cycle = 0;

		double bestTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
		double global_best_time = 0;
		while (bestTime < timeMax) {

			VNS();

			if (F.evaluation_obj < best_F.evaluation_obj && S.route.size() <= vehicle_num) {
				if (best_F.evaluation_obj - F.evaluation_obj > 0.009) {
					endTime = clock();
					bestTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
					global_best_time = bestTime;
				}
				best_S = S;
				best_F = F;
				//endTime = clock();
				//bestTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
				std::cout << "Iteration: " << t << ", Current best cumulative waiting time: " << best_F.cumulative_time << ", Taking time: " << global_best_time << "s, Current best evaluation objective: " << best_F.evaluation_obj << endl;
				//global_best_time = bestTime;
				no_improve = 0;
				shake_no_improve = 0;

				// Adaptive parameter adjustment: when a better solution is found, adjust parameters to balance exploration and exploitation.
				p.omega = p.min_omega;                             // Reduce omega to strengthen local search.
				p.shake_s = max(2.0, p.shake_s - 0.5);             // Reduce perturbation to strengthen local search.
				p.max_length_RCL_percent = max(0.10, p.max_length_RCL_percent - 0.02); // Reduce RCL length to improve solution quality.
			}
			else {
				no_improve++;
				shake_no_improve++;

				// Adaptive parameter adjustment: increase exploration after a long period without improvement.
				if (no_improve % 500 == 0) {
					p.omega = min(p.max_omega, p.omega + 1);             // Increase omega to strengthen global search.
					p.shake_s = min(8.0, p.shake_s + 0.5);               // Increase perturbation to improve solution diversity.
					p.max_length_RCL_percent = min(0.30, p.max_length_RCL_percent + 0.02); // Increase RCL length to improve exploration.
				}
			}
			if (shake_no_improve >= p.max_shake_no_improve) {
				p.omega = p.max_omega;
				shake_no_improve = 0;
				// Further increase perturbation after a long period without improvement.
				p.shake_s = min(10.0, p.shake_s + 1.0);
			}
			else if (no_improve >= p.max_no_improve_new_S) {
				// Crossover: select the best offspring produced by crossover.
				routeInformation child1, child2, child;
				evaluationFunction child1F, child2F, childF;
				GPX(S, elit, child1, child1F);
				GPX(elit, S, child2, child2F);

				if (child1F.evaluation_obj < child2F.evaluation_obj) {
					child = child1;
					childF = child1F;
				}
				else {
					child = child2;
					childF = child2F;
				}

				if (F.evaluation_obj < elitF.evaluation_obj && S.route.size() <= vehicle_num) {
					elit = S;
					elitF = F;
				}

				S = child;
				F = childF;

				cycle++;
				if (cycle % p.GPX_no_improve_cycle == 0) {
					routeInformation newS;
					GRASP_initialization(newS);
					S = newS;
					it = p.max_initial_LS;
					while (it) {
						RVNS();
						it--;
					}
					//elit = S;
					//elitF = F;
				}

				no_improve = 0;
			}
			endTime = clock();
			bestTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
			//convergence.push_back(best_F.cumulative_time);
			no_improve++;
			shake_no_improve++;
			t++;

			
		}

		verify(best_S);
		verify(best_S, best_F);
		std::cout << endl;
		std::cout << "best_F.cumulative_time: " << best_F.cumulative_time << endl;
		std::cout << "best_F.evaluation_obj: " << best_F.evaluation_obj << endl;
		std::cout << "The number of used vehicles is: " << best_S.route.size() << endl;
		std::cout << "The best time for the optimal solution is: " << global_best_time << endl;
		std::cout << "best_F.extra_vehicle: " << best_F.extra_vehicle << endl;
		std::cout << "The total iteration is: " << t << endl;
		std::cout << endl;



		cumulative_t.push_back(best_F.cumulative_time);
		op_time.push_back(global_best_time);
		c++;

		std::cout << endl;
		std::cout << "The final route is: " << endl;
		for (auto i = 0; i < best_S.route.size(); i++) {
			for (auto j = 0; j < best_S.route[i].size(); j++) {
				std::cout << best_S.route[i][j] << " ";
			}
			std::cout << endl;
		}
		std::cout << endl;

		std::ofstream outfile("route.txt", std::ios::app);
		if (outfile) {
			outfile << "The final route of EX" << c << " is: \n" << endl;
			for (auto i = 0; i < best_S.route.size(); i++) {
				for (auto j = 0; j < best_S.route[i].size(); j++) {
					outfile << best_S.route[i][j] << " ";
				}
				outfile << endl;
			}
			outfile << endl;
		}
	}

	cout << "===================== Final results print =====================" << endl;
	cout << "Final optimal cumulative time of each run: " << endl;
	for (auto x : cumulative_t) {
		cout << x << " ";
	}
	cout << endl;
	cout << "Best operational time of each run: " << endl;
	for (auto x : op_time) {
		cout << x << " ";
	}
	cout << endl;

	cout << "The number of used times of operators:" << endl;
	for (auto i = 0; i < op_used_num.size(); i++) {
		cout << op_used_num[i] << " ";
	}
	cout << endl;
	cout << "The number of effective used times of operators:" << endl;
	for (auto i = 0; i < op_effec_num.size(); i++) {
		cout << op_effec_num[i] << " ";
	}
	cout << endl;

	std::ofstream outfile_time("obj_runtime.txt", std::ios::app);
	if (outfile_time) {
		outfile_time << "Final optimal cumulative time of each run: " << endl;
		for (auto x : cumulative_t) {
			outfile_time << x << endl;
		}
		outfile_time << endl;
		outfile_time << "Best operational time of each run: " << endl;
		for (auto x : op_time) {
			outfile_time << x << endl;
		}
		outfile_time << endl;
	}

	std::ofstream outfile_op("op_information.txt", std::ios::app);
	if (outfile_op) {
		outfile_op << "The number of used times of operators:" << endl;
		for (auto i = 0; i < op_used_num.size(); i++) {
			outfile_op << op_used_num[i] << endl;
		}
		outfile_op << endl;

		outfile_op << "The number of effective used times of operators:" << endl;
		for (auto i = 0; i < op_effec_num.size(); i++) {
			outfile_op << op_effec_num[i] << endl;
		}
		outfile_op << endl;
	}


	/*std::cout << endl; std::cout << "The final route is: " << endl; for (auto i = 0; i < best_S.route.size(); i++) { for (auto j = 0; j < best_S.route[i].size(); j++) { std::cout << best_S.route[i][j] << " "; } std::cout << endl; } std::cout << endl;*/

	/*cout << "The situation of the convergence is: " << endl; for (auto i = 0; i < convergence.size(); i++) { cout << convergence[i] << " "; } cout << endl;*/
}


