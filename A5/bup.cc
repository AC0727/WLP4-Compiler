#include <iostream>
#include <sstream>
#include <string>
#include <vector>


const std::string CFG    = ".CFG";
const std::string INPUT      = ".INPUT";
const std::string ACTIONS = ".ACTIONS";
const std::string EMPTY       = ".EMPTY";
const std::string END = ".END";


struct scanError {
  int index;
};


int main() {
  std::istream& in = std::cin;
  std::string line;
  std::string s;

  std::vector<std::string> reduction;
  std::vector<std::string> input;
  std::vector<std::vector<std::string>> productionRules; //first index is LHS

  std::getline(in, line); // cfg section (skip header)
  while(std::getline(in, line)) {
    if (line == INPUT) { 
      break; 
    } else {
      std::istringstream lineReader(line);
      std::vector<std::string> rule;
      while (lineReader >> s) rule.emplace_back(s);
      productionRules.emplace_back(rule);
    }
  }


  // Input section
  while(std::getline(in, line)) {
    if (line == ACTIONS) break;
    std::istringstream lineReader(line);
    while(lineReader >> s) {
      input.emplace_back(s);
    }
  }
  


  int inputLocation = 0; //keep track of input location
  while(std::getline(in, line)) {
    //std::cout << "HELLO " << line << std::endl;
    if (line == "shift") {
        reduction.emplace_back(input[inputLocation]);
        inputLocation++;
    }
    else if (line == "print") {
        for (std::string s : reduction) std::cout << s << " ";
        std::cout << ". ";
        for (int i = inputLocation; i < input.size(); i++) std::cout << input[i] << " ";
        std::cout << std::endl;
    }
    else if (line == END) break;
    else {
        std::string ruleStr = line.substr(line.find(" ", 0) + 1);
        std::istringstream iss{ruleStr};
        int rule;
        iss >> rule;
        int numPops;
        if(productionRules[rule][1] == ".EMPTY") numPops = 0;
        else numPops = productionRules[rule].size() - 1;
        for(int i = 0; i < numPops; i++) reduction.pop_back();
        reduction.emplace_back(productionRules[rule][0]);
    }
  }
}


