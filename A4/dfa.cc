#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>


const std::string ALPHABET    = ".ALPHABET";
const std::string STATES      = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT       = ".INPUT";
const std::string EMPTY       = ".EMPTY";

bool isChar(std::string s) {
  return s.length() == 1;
}
bool isRange(std::string s) {
  return s.length() == 3 && s[1] == '-';
}


int main() {
  std::istream& in = std::cin;
  std::string s;

  std::vector<char> alphabet;
  std::string initialState;
  std::map<std::string, int> states; //true means it is accept state
  std::map<std::string, std::string> transitions;

  std::getline(in, s); // Alphabet section (skip header)
  while(in >> s) {
    if (s == STATES) { 
      break; 
    } else {
      if (isChar(s)) {
        alphabet.emplace_back(s[0]);

      } else if (isRange(s)) {
        for(char c = s[0]; c <= s[2]; ++c) {
          alphabet.emplace_back(c);
        }
      } 
    }
  }


  std::getline(in, s); // States section (skip header)
  bool initial = true;
  while(in >> s) {
    if (s == TRANSITIONS) { 
      break; 
    } else {
      bool accepting = false;

      if (s.back() == '!' && !isChar(s)) {
        accepting = true;
        s.pop_back();
      }

      //// Variable 's' contains the name of a state
      if (initial) {
        initialState = s;
        initial = false;
      }

      if (accepting) {
        states[s] = 1;
        //std::cout << "HELLO " << states[s] << std::endl;
      } else {
        states[s] = 0;
      }
    }
  }


  std::getline(in, s); // Transitions section (skip header)
  while(std::getline(in, s)) {

    if (s == INPUT) {
      break; 

    } else {
      std::string fromState, symbols, toState;
      std::istringstream line(s);
      line >> fromState;

      while(line >> s) {

        if(line.peek() == EOF) {  // Then it's the to-state
          toState = s;
        } else {  // We expect a character or range
          if (isChar(s)) {
            symbols += s;
          } else if (isRange(s)) {
            for(char c = s[0]; c <= s[2]; ++c) {
              symbols += c;
            }
          }
        }
      }
      for ( char c : symbols ) {
        //// There is a transition from 'fromState' to 'toState' on 'c'
        std::string input = fromState + " " + " ";
        input[input.size() - 1] = c;
        transitions[input] = toState;
      }
    }
  }
  

  while(in >> s) {//// Variable 's' contains an input string for the DFA
    int stringLength = s.size();
    std::string state = initialState;

    if (s != ".EMPTY") {
      for (int i = 0; i <  stringLength; i++) {
        std::string input = state + " " + " ";
        input[input.size() - 1] = s[i];
        state = transitions[input];
      }
    }

    if (states[state]) {
      std::cout << s << " " << "true" << std::endl;
    } else {
      std::cout << s << " " << "false" << std::endl;
    }
  }
}


