#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>

using namespace std;

class Result {
public:
	int value;
	vector<bool> selected;
	Result(int x, vector<bool> y) {
		value = x;
		selected = y;
	}
};

map<string, int> itemWeight;
map<string, int> itemValue;
vector<string> items;
vector<string> rules;
vector<string> combined;

int readFile(string filepath, vector<string>* names, vector<string>* rules, map<string, int>* itemWeight, map<string, int>* itemValue) {
	ifstream readFile(filepath);
	string input;
	string temp;
	if (!readFile.is_open()) {
		cout << "failed";
	}
	getline(readFile, input);    //read ContainerSize:
	getline(readFile, input);
	int capacity = stoi(input);

	getline(readFile, input);   //read Items:

	while (getline(readFile, input)) {
		if (input == "Rules:") break;
		stringstream ss;
		ss << input;
		ss >> input;
		temp = input;
		names->push_back(temp);
		ss >> input;
		itemWeight->insert({ temp, stoi(input) });
		ss >> input;
		itemValue->insert({ temp, stoi(input) });
	}
	while (getline(readFile, input)) {
		stringstream ss;
		ss << input;
		ss >> input;
		temp = input;
		rules->push_back(temp);
		ss >> input;
		itemWeight->insert({ temp, stoi(input) });
		ss >> input;
		itemValue->insert({ temp, stoi(input) });
	}
	readFile.close();
	return capacity;
}

void writeFile(string filepath, vector<string> selected) {
	ofstream out(filepath);
	if (!out.is_open()) {
		return;
	}
	for (int i = 0; i < selected.size(); i++) {
		out << selected[i] << endl;
	}
	out.close();
	return;
}

bool inRules(string rule, string item, int length) {
	if (length == 1) {
		for (char x : rule) {
			if (x == item[0]) {
				return true;
			}
		}
	}
	else{
		for (char x : rule) {
			if (x == item[0] || x == item[1]) {
				return true;
			}
		}
	}
	return false;
}

void generateDependency(bool* row, int n, int col_id) {
	for (int i = 0; i < n; i++) {
		if (inRules(combined[col_id], combined[i], combined[i].size())) {
			row[i] = true;
		}
		else {
			row[i] = false;
		}
	}
}

// n is the index to go through all the items
// record is to store the item in current path
Result knapsack(int capacity, int n, int size, bool** dependency, vector<bool> record) {
	// base case
	if (n == 0 || capacity == 0) {
		vector<bool> x(size, false);
		return Result(0, x);
	}
	// cannot take current item
	if (itemWeight[combined[n - 1]] > capacity) {
		return knapsack(capacity, n - 1, size, dependency, record);
	}
	else {
		// check dependency
		for (int i = 0; i < size; i++) {
			// cannot take current item, got conflict with previous record
			if (dependency[n - 1][i] && record[i]) {
				return knapsack(capacity, n - 1, size, dependency, record);
			}
		}
		vector<bool> recordUpdated = record;
		recordUpdated[n - 1] = true;
		Result include = knapsack(capacity - itemWeight[combined[n - 1]], n - 1, size, dependency, recordUpdated);
		Result exclude = knapsack(capacity, n - 1, size, dependency, record);

		include.value += itemValue[combined[n - 1]];
		include.selected[n - 1] = true;

		if (include.value > exclude.value) return include;
		else return exclude;
	}
}

int main() {
	string inputPath = "problem.txt";
	string outputPath = "output.txt";

	int capacity = readFile(inputPath, &items, &rules, &itemWeight, &itemValue);
	int n1 = items.size();
	int n2 = rules.size();

	// compute the total value and weight for each rules
	for (int i = 0; i < n2; i++) {    // loop through the rules
		for (int j = 0; j < n1; j++) {      // loop the items that match with rule
			if (inRules(rules[i], items[j], 1)) {
				itemWeight[rules[i]] += itemWeight[items[j]];
				itemValue[rules[i]] += itemValue[items[j]];
			}
		}
	}

	// combined = items + rules
	combined = items;
	combined.insert(combined.end(), rules.begin(), rules.end());
	int n = combined.size();
	
	// generate dependency
	bool** dependency = new bool* [n];
	for (int i = 0; i < n; i++) {
		dependency[i] = new bool[n]; 
	}

	//vector<thread> threads;
#pragma omp parallel for 
	for (int i = 0; i < n; i++) {
		generateDependency(dependency[i], n, i);
	}

	vector<bool> record(n, false);

	Result result = knapsack(capacity, n, n, dependency, record);
	vector<string> selected;
	for (int i = 0; i < n; i++) {
		if (result.selected[i]) {
			selected.push_back(combined[i]);
		}
	}

	writeFile(outputPath, selected);

	for (int i = 0; i < n; i++) {
		delete[] dependency[i];
	}
	delete[] dependency;

	return 0;
}




