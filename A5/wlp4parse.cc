#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include "wlp4parse.h"


const std::string CFG    = ".CFG";
const std::string INPUT      = ".INPUT";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string REDUCTIONS = ".REDUCTIONS";
const std::string EMPTY       = ".EMPTY";
const std::string END = ".END";

struct cfgRule {
  std::string lhs;
  std::vector<std::string> rhs;

  std::string toString() {
    std::string rule = lhs;
    for(std::string word : rhs) rule = rule + " " + word;
    return rule;
  }
};

struct transitionInput {
  int state;
  std::string input;

  std::string toString() {
    std::ostringstream oss;
    oss << state;
    return oss.str() + " " + input;
  }
};

struct reductionInput {
  int state;
  std::string lookahead;

  std::string toString() {
    std::ostringstream oss;
    oss << state;
    return oss.str() + " " + lookahead;
  }
};

struct token {
    std::string type;
    std::string lexeme;
    std::string toString() {return type +  " " + lexeme;}
};

struct treeNode {
  std::string value;
  std::vector<treeNode *> children;

  treeNode(std::string str) {
    value = str;
  }

  treeNode(std::string str, std::vector<treeNode *> newChildren) {
    value = str;
    for (int i = 0; i < newChildren.size(); i++) {
      children.emplace_back(newChildren[i]);
    }
    for (int i = 0; i < newChildren.size(); i++) newChildren.pop_back();
  }

  void print() {
    std::cout << value << std::endl;
    for (treeNode * node : children) node->print();
  }

  ~treeNode() {
    for(int i = 0; i < children.size(); i++) {
      delete children[i];
      children[i] = nullptr;
    }
  }
};

struct parseError {
  int index;
  void printMessage() {std::cerr << "ERROR at " << index << std::endl;}
};


int main() {
  std::istringstream wlp4in{WLP4_COMBINED};
  std::istream& in = std::cin;
  std::string line;
  std::string s;

  std::stack<std::string> symStack;
  std::vector<treeNode *> treeStack;
  std::vector<token> input;
  std::stack<int> stateStack;
  std::vector<cfgRule> cfgRules; 
  std::map<std::string, int> transitions;
  std::map<std::string, int> reductionRules; //value is rule

  std::getline(wlp4in, line); // cfg section (skip header)
  while(std::getline(wlp4in, line)) {
    if (line == TRANSITIONS) { 
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


  //transition section
  while(std::getline(wlp4in, line)) {
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


  //reductions section
  while(std::getline(wlp4in, line)) {
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


  // Input section
  input.emplace_back(token{"BOF", "BOF"});
  while(std::getline(in, line)) {
    std::istringstream lineReader(line);
    std::string type;
    std::string lexeme;
    lineReader >> type;
    lineReader >> lexeme;

    input.emplace_back(token{type, lexeme});
  }
  input.emplace_back(token{"EOF", "EOF"});
  


  stateStack.push(0);
  try {
    for (int i = 0; i < input.size(); i++) {
      /*for(treeNode * node : treeStack) std::cout << "HHHH " << node->value << std::endl;
      std::cout << std::endl;*/

      reductionInput redInput = {stateStack.top(), input[i].type};

      //reduce
      while(reductionRules.count(redInput.toString())) {
        int rule = reductionRules[redInput.toString()];
        //std::cout << "RULE " << rule << std::endl;
        int numPops;
        std::vector<treeNode *> children;

        if (cfgRules[rule].rhs[0] == EMPTY) numPops = 0;
        else numPops = cfgRules[rule].rhs.size();

        for(int j = 0; j < numPops; j++) {
          symStack.pop();
          stateStack.pop();
          children.emplace_back(treeStack.back());
          treeStack.pop_back();
        }

        std::reverse(children.begin(), children.end());
        std::string ruleStr = cfgRules[rule].toString();
        treeNode * node = new treeNode{ruleStr, children};
        //for(treeNode * node : children) std::cout << "H " << node->value << std::endl;
        treeStack.emplace_back(node);

        symStack.push(cfgRules[rule].lhs);
        transitionInput transition = {stateStack.top(), cfgRules[rule].lhs};
        stateStack.push(transitions[transition.toString()]);
        redInput = {stateStack.top(), input[i].type};
      }

      //shift
      symStack.push(input[i].type);
      treeNode * node = new treeNode{input[i].toString()};
      treeStack.emplace_back(node);

      //validity check
      transitionInput transition = {stateStack.top(), input[i].type};
      if(!transitions.count(transition.toString())) {
        int k = i;
        throw parseError{k};
      }
      else {
        stateStack.push(transitions[transition.toString()]);
      }

      //std::cout << "BB " << treeStack.top()->value << " " << treeStack.size() << std::endl;
    }


    //reduce one last time
    std::vector<treeNode *> children;
    int rule = 0;
    int numPops = cfgRules[rule].rhs.size();
    for(int j = 0; j < numPops; j++) {
      symStack.pop();
      stateStack.pop();
      children.emplace_back(treeStack.back());
      treeStack.pop_back();
    }
    std::reverse(children.begin(), children.end());
    std::string ruleStr = cfgRules[rule].toString();
    treeNode * node = new treeNode{ruleStr, children};
    treeStack.emplace_back(node);

    symStack.push(cfgRules[rule].lhs);
    transitionInput transition = {stateStack.top(), cfgRules[rule].lhs};
    stateStack.push(transitions[transition.toString()]);


    //print tree
    for (treeNode * node : treeStack) node->print();

    //delete
    for(treeNode * node : treeStack) delete node;


  } catch (parseError & err) {
    err.printMessage();

    //delete tree
    for(treeNode * node : treeStack) delete node;
  }
  
}

