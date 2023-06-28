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

void inputRangeAlpha(char start, char end, std::vector<char> alphabet) {
  for(char c = start; c <= end; ++c) {
    alphabet.emplace_back(c);
  }
}

void addTransitionRange(char start, char end, std::map<std::string, std::string>  & transitions, 
                        std::string fromState, std::string toState) {
  for (char c = start; c <= end; ++c) {
    //// There is a transition from 'fromState' to 'toState' on 'c'
    std::string input = fromState + " " + " ";
    input[input.size() - 1] = c;
    transitions[input] = toState;
  }
}

bool invisibleState(std::string state) {
  return state == "SPACE" || state == "NEWLINE" || state == "TAB" || state == "COMMENT";
}


void checkIDstate(std::string str, std::string & state) {
  if (str == "wain") state = "WAIN";
  else if (str == "int") state = "INT";
  else if (str == "if") state = "IF";
  else if (str == "else") state = "ELSE";
  else if (str == "while") state = "WHILE";
  else if (str == "println") state = "PRINTLN";
  else if (str == "return") state = "RETURN";
  else if (str == "NULL") state = "NULL";
  else if (str == "new") state = "NEW";
  else if (str == "delete") state = "DELETE";
}


struct scanError {
  std::string message;
};


int main() {
  std::istream& in = std::cin;
  std::string s;

  std::vector<char> alphabet = {'*', '+', '-', '/', '&', '%', '=', '!', '<', '>', '(', ')', '{', '}', '[', ']', ';'}; 
  inputRangeAlpha('a', 'z', alphabet);
  inputRangeAlpha('A', 'Z', alphabet);
  inputRangeAlpha('0', '9', alphabet);

  std::string initialState = "start";

  std::map<std::string, bool> states; //true means it is accept state
  states["start"] = false;
  states["EXCLAMATION"] = false;
  states["ID"] = true;
  states["NUM"] = true;
  states["LPAREN"] = true;
  states["RPAREN"] = true;
  states["LBRACE"] = true;
  states["RBRACE"] = true;
  states["BECOMES"] = true;
  states["EQ"] = true;
  states["NE"] = true;
  states["LT"] = true;
  states["GT"] = true;
  states["LE"] = true;
  states["GE"] = true;
  states["PLUS"] = true;
  states["MINUS"] = true;
  states["STAR"] = true;
  states["SLASH"] = true;
  states["PCT"] = true;
  states["COMMA"] = true;
  states["SEMI"] = true;
  states["RBRACK"] = true;
  states["LBRACK"] = true;
  states["AMP"] = true;
  states["SPACE"] = true;
  states["TAB"] = true;
  states["NEWLINE"] = true;
  states["COMMENT"] = true;


  std::string inputSpace = "start  ";
  std::string inputTab = "start " + 9;
  std::string inputNewline = "start " + 10;
  std::string commentSpace = "COMMENTBEGIN  ";
  std::string commentTab = "COMMENTBEGIN " + 9;
  std::string commentNewLine = "COMMENTBEGIN " + 10;
  std::map<std::string, std::string> transitions;
  addTransitionRange('a', 'z', transitions, "start", "ID");
  addTransitionRange('A', 'Z', transitions, "start", "ID");
  addTransitionRange('a', 'z', transitions, "ID", "ID");
  addTransitionRange('A', 'Z', transitions, "ID", "ID");
  addTransitionRange('0', '9', transitions, "ID", "ID");
  addTransitionRange('0', '9', transitions, "start", "NUM");
  addTransitionRange('0', '9', transitions, "NUM", "NUM");
  transitions["start !"] = "EXCLAMATION";
  transitions["start ("] = "LPAREN";
  transitions["start )"] = "RPAREN";
  transitions["start {"] = "LBRACE";
  transitions["start }"] = "RBRACE";
  transitions["start ="] = "BECOMES";
  transitions["BECOMES ="] = "EQ";
  transitions["EXCLAMATION ="] = "NE";
  transitions["start <"] = "LT";
  transitions["start >"] = "GT";
  transitions["LT ="] = "LE";
  transitions["GT ="] = "GE";
  transitions["start +"] = "PLUS";
  transitions["start -"] = "MINUS";
  transitions["start *"] = "STAR";
  transitions["start /"] = "SLASH";
  transitions["start %"] = "PCT";
  transitions["start ,"] = "COMMA";
  transitions["start ;"] = "SEMI";
  transitions["start ["] = "LBRACK";
  transitions["start ]"] = "RBRACK";
  transitions["start &"] = "AMP";
  transitions[inputSpace] = "SPACE";
  transitions[inputTab] = "TAB";
  transitions[inputNewline] = "NEWLINE";
  transitions["SLASH /"] = "COMMENT";

  
  
  std::string state = initialState;
  std::string lexeme;

  try { 

    while(std::getline(in, s)) {
      //std::cout << s << std::endl;
        int i = 0;

      while(i < s.size()) {
        std::string input = state + " " + " ";
        input[input.size() - 1] = s[i];
        //std::cout << "HELLO " << input << std::endl;

        if (transitions[input] == "") {

          if(invisibleState(state)) {
            lexeme = "";
            state = initialState;
          }
          else if(states[state]) { //if state is num, check range

            if (state == "NUM") {
              int64_t num = 0;
              std::istringstream iss{lexeme};
              iss >> num;
              if (num > 2147483647) throw scanError{"Last check of num range."};
            }
            else if (state == "ID") { // if state is ID, update to correct state
              checkIDstate(lexeme, state);
            }

            std::cout << state << " " << lexeme << std::endl;

            lexeme = "";
            state = initialState;
          } else {
            std::cerr << input << std::endl << s << std::endl;
            throw scanError{"State not accept state."};
          }

        } 
        else if (transitions[input] == "COMMENT") { //skip comment
          lexeme = "";
          state = initialState;
          i = 0;
          break;
        } else {
          lexeme += s[i];
          state = transitions[input];
          i++;
        }
      }
    }

    if (!invisibleState(state) && states[state]) {
        
        if (state == "NUM") { //if token is NUM, check range
          int64_t num = 0;
          std::istringstream iss{lexeme};
          iss >> num;
          if (num > 2147483647) throw scanError{"Last check of num range."};
        }
        else if (state == "ID") { // if state is ID, update to correct state
          checkIDstate(lexeme, state);
        }

        std::cout << state << " " << lexeme << std::endl;
    }
    else if (!invisibleState(state)) throw scanError{"Last check of accept state."};

  } catch (scanError & err) {
  std::cerr << "ERROR: " << err.message << std::endl;
  }
  
}


