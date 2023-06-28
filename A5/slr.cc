#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>


const std::string CFG    = ".CFG";
const std::string INPUT      = ".INPUT";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string REDUCTIONS = ".REDUCTIONS";
const std::string EMPTY       = ".EMPTY";
const std::string END = ".END";

struct cfgRule {
  std::string lhs;
  std::vector<std::string> rhs;
};

struct transitionInput {
  int state;
  std::string input;

  std::string toString() {
    std::ostringstream oss;
    oss << state;
    return oss.str() + " " + input;
  }

  /*bool operator<(const transitionInput& rhs) const {
    return state < rhs.state && input.compare(rhs.input) < 0;
  }*/
};

struct reductionInput {
  int state;
  std::string lookahead;

  std::string toString() {
    std::ostringstream oss;
    oss << state;
    return oss.str() + " " + lookahead;
  }

  /*bool operator<(const reductionInput& rhs) const {
    return state < rhs.state && lookahead.compare(rhs.lookahead) < 0;
  }*/
};


struct parseError {
  int index;
  void printMessage() {std::cerr << "ERROR at " << index << std::endl;}
};


void print(std::vector<std::string> input, std::vector<std::string> symStack, int i) {
  for (std::string s : symStack) std::cout << s << " ";
  std::cout << ". ";
  for (int j = i; j < input.size(); j++) std::cout << input[j] << " ";
  std::cout << std::endl;
}


int main() {
  std::istream& in = std::cin;
  std::string line;
  std::string s;

  std::vector<std::string> symStack;
  std::vector<std::string> input;
  std::vector<int> stateStack;
  std::vector<cfgRule> cfgRules; 
  std::map<std::string, int> transitions;
  std::map<std::string, int> reductionRules; //value is rule

  std::getline(in, line); // cfg section (skip header)
  while(std::getline(in, line)) {
    if (line == INPUT) { 
      break; 
    } else {
      std::istringstream lineReader(line);
      std::vector<std::string> rhs;
      std::string lhs;
      lineReader >> lhs;
      while (lineReader >> s) rhs.emplace_back(s);
      cfgRules.emplace_back(cfgRule{lhs, rhs});
    }
  }


  // Input section
  while(std::getline(in, line)) {
    if (line == TRANSITIONS) break;

    std::istringstream lineReader(line);
    while(lineReader >> s) {
      input.emplace_back(s);
    }
  }

  //transition section
  while(std::getline(in, line)) {
    if (line == REDUCTIONS) break;

    std::istringstream lineReader(line);
    std::string curState;
    std::string strInput;
    int newState;
    lineReader >> curState;
    lineReader >> strInput;
    lineReader >> newState;

    std::string transition = curState + " " + strInput;
    transitions[transition] = newState;
  }

  //for (auto& p : transitions) std::cout << "T " << p.first << ' ' << p.second << std::endl;

  //reductions section
  while(std::getline(in, line)) {
    if (line == END) break;
    std::istringstream lineReader(line);
    std::string state;
    std::string lookahead;
    int rule;
    lineReader >> state;
    lineReader >> rule;
    lineReader>> lookahead;
    if (lookahead == ".ACCEPT") lookahead = "";

    std::string redInput = state + " " +  lookahead;
    reductionRules[redInput] = rule;
  }
  //for (auto& p : reductionRules) std::cout << "R " << p.first  << ' ' << p.second << std::endl;
  

  stateStack.emplace_back(0);
  print(input, symStack, 0);
  try {
    for (int i = 0; i < input.size(); i++) {
      reductionInput redInput = {stateStack[stateStack.size() - 1], input[i]};

      //reduce
      while(reductionRules.count(redInput.toString())) {
        int rule = reductionRules[redInput.toString()];
        int numPops;
        if (cfgRules[rule].rhs[0] == EMPTY) numPops = 0;
        else numPops = cfgRules[rule].rhs.size();

        for(int j = 0; j < numPops; j++) {
          symStack.pop_back();
          stateStack.pop_back();
        }

        symStack.emplace_back(cfgRules[rule].lhs);
        transitionInput transition = {stateStack[stateStack.size() - 1], cfgRules[rule].lhs};
        stateStack.emplace_back(transitions[transition.toString()]);
        redInput = {stateStack[stateStack.size() - 1], input[i]};
        print(input, symStack, i);
      }

      //shift
      symStack.emplace_back(input[i]);
      print(input, symStack, i + 1);

      //validity check
      transitionInput transition = {stateStack[stateStack.size() - 1], input[i]};
      if(!transitions.count(transition.toString())) {
        throw parseError{i + 1};
      }
      else {
        stateStack.emplace_back(transitions[transition.toString()]);
        //for(int s : stateStack) std::cout << "HE " << s << " " << input[i] <<std::endl;
      }
    }


    //reduce one last time
    int rule = 0;
    int numPops = cfgRules[rule].rhs.size();
    for(int j = 0; j < numPops; j++) {
      symStack.pop_back();
      stateStack.pop_back();
    }
    symStack.emplace_back(cfgRules[rule].lhs);
    transitionInput transition = {stateStack[stateStack.size() - 1], cfgRules[rule].lhs};
    stateStack.emplace_back(transitions[transition.toString()]);
    print(input, symStack, input.size());



  } catch (parseError & err) {
    err.printMessage();
  }
  
  
}




