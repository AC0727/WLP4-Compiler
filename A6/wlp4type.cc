#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>

const std::vector<std::string> TERMS = {"WAIN", "NULL", "INT", "BOF", "EOF", "LPAREN", "RPAREN", "LBRACE", "RBRACE", "RETURN", "SEMI",
                                        "COMMA", "ID", "IF", "ELSE", "WHILE", "PRINTLN", "BECOMES", "EQ", "NE", "LT", "GT", "LE", 
                                        "GE", "PLUS", "MINUS", "STAR", "SLASH", "PCT", "NEW", "DELETE", "LBRACK", "RBRACK", "AMP", "NUM"};

const std::vector<std::string> FUNCTION_RULES = {"procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE",
                                                "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE"};

const std::string WAIN_RULE = "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE";
const std::string PROCEDURES_RULE = "procedures procedure procedures";
const std::string VAR_DECLARE_RULE_NUM = "dcls dcls dcl BECOMES NUM SEMI";
const std::string VAR_DECLARE_RULE_NULL = "dcls dcls dcl BECOMES NULL SEMI";

enum Type {INT, INT_STAR};

struct Token {
    std::string type;
    std::string lexeme;

    Token(std::string str) {
      std::istringstream iss(str);
      iss >> type;
      iss >> lexeme;
    }
    std::string toString() {return type +  " " + lexeme;}
};

struct funcInfo {
  std::vector<Type> paramTypes;
  std::map<std::string, Type> varTable;
  Type returnType;
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

struct error {
  std::string message;
  void printMessage() {std::cerr << "ERROR: " << message << std::endl;}
};


int rhsSize(std::string rule) {
    int num = 0;
    for (int i = 0; i < rule.size(); i++) {
        if (rule[i] == ' ') num++;
    }
    return num;
}


int subTreeSize(treeNode * tree) {
  int num = 1;
  if (tree->children.empty()) return num;

  for(treeNode * child : tree->children) num += subTreeSize(child);

  return num;
}


treeNode * createTree(std::vector<std::string> inputs, int numChildren, int index) {
    treeNode * tree;
    std::vector<treeNode *> children;
    std::string value = inputs[index];
    int i = index + 1;
    int counter = 0;

    if (numChildren == 0) {
      return new treeNode(value);
    }

    while (counter < numChildren && i < inputs.size()) { //get children
        std::istringstream lineReader(inputs[i]);
        std::string lhs;
        std::string rhsFirstWord;
        lineReader >> lhs;
        lineReader >> rhsFirstWord;
        int newChildren = rhsSize(inputs[i]); 

        if (newChildren == 1) {
           if (std::find(TERMS.begin(), TERMS.end(), lhs) != TERMS.end() || rhsFirstWord == ".EMPTY") { //if a terminal or empty
            newChildren = 0;
          }
        }
        treeNode * node = createTree(inputs, newChildren, i);
        int linesSkip = subTreeSize(node);
        children.emplace_back(node);

        i += linesSkip;
        counter++;
    }

    if (!children.empty()) tree = new treeNode(value, children);
    else tree = new treeNode(value);

    return tree;
}


Type convertStringType(std::string typeStr) {
  if (typeStr == "INT" || typeStr == "NUM") return INT;
  else return INT_STAR;
}


std::string typeToString(Type type) {
  if (type == INT) return "int";
  else return "int*";
}


void propagateType(treeNode * tree, Type type) {
  Token token{tree->value};
  if (token.type == "ID" || token.type == "NUM" || token.type == "NULL" || token.type == "factor" || 
      token.type == "expr" || token.type == "lvalue" || token.type == "term") {

    if (tree->value.find(':') == std::string::npos) tree->value = tree->value + " : " + typeToString(type);
  }
  if (tree->value == "factor ID LPAREN arglist RPAREN : int") {
    for(int i = 1; i < tree->children.size(); i++) propagateType(tree->children[i], type);
  } 
  else if (tree->value != "factor ID LPAREN RPAREN : int") {
    for(treeNode * child : tree->children) propagateType(child, type);
  }
  //for(treeNode * child : tree->children) propagateType(child, type);
}


Type typeOf(treeNode * tree, std::string funcName, std::map<std::string, funcInfo> & table) { //on an expr rule
  Type type = INT;
  std::map<std::string, Type> curTable = table[funcName].varTable;

  if (tree->value == "factor NUM") {
    type = INT;
  } 
  else if (tree->value == "factor NULL") {
    type = INT_STAR;
  } 
  else if (tree->value == "factor ID" || tree->value == "lvalue ID") {
    Token token{tree->children[0]->value};
    //if (token.lexeme == "q") for (auto & pair: curTable) std::cout << funcName << " A " << pair.first << " " << pair.second << std::endl; 
    if (!curTable.count(token.lexeme)) throw error{"return variable not defined"};
    type = curTable[token.lexeme];
  }
  else if (tree->value == "factor LPAREN expr RPAREN" || tree->value == "lvalue LPAREN lvalue RPAREN") { //has brackets around expr
    type = typeOf(tree->children[1], funcName, table);
  }
  else if (tree->value == "factor AMP lvalue") {
    Type right = typeOf(tree->children[1], funcName, table);
    if (right == INT) type = INT_STAR;
    else throw error{"expression types not valid"};
  }
  else if (tree->value == "factor STAR factor" || tree->value == "lvalue STAR factor") {
    Type right = typeOf(tree->children[1], funcName, table);
    if (right == INT_STAR) type = INT;
    else throw error{"expression types not valid"};
  }
  else if (tree->value == "factor NEW INT LBRACK expr RBRACK") {
    Type expType = typeOf(tree->children[3], funcName, table);
    if (expType == INT) type = INT_STAR;
    else throw error{"expression types not valid"};
  }
  else if (tree->value == "factor ID LPAREN RPAREN") {
    Token token{tree->children[0]->value};
    if (!table.count(token.lexeme)) throw error{"return variable not defined"};
    std::vector<Type> accTypes = table[token.lexeme].paramTypes;
    if (accTypes.size() != 0) throw error{"param list not right length."};
    type = INT;
  }
  else if (tree->value == "factor ID LPAREN arglist RPAREN") {
    Token token{tree->children[0]->value};
    //std::cout << "HI " <<  funcName << " " << token.lexeme <<std::endl;
    if (!table.count(token.lexeme)) throw error{"function name not defined"};

    treeNode * args = tree->children[2];
    std::vector<Type> types;
    while(args->children.size() == 3) {
      treeNode * expNode = args->children[0];
      types.emplace_back(typeOf(expNode, funcName, table));
      args = args->children[2];
    }
    types.emplace_back(typeOf(args->children[0], funcName, table));
    

    std::vector<Type> accTypes = table[token.lexeme].paramTypes;
    if (accTypes.size() != types.size()) throw error{"param list not right length."};
    for (int i = 0; i < types.size(); i++) {
      //std::cout << "HI " <<  funcName << " " << token.lexeme << " " << accTypes[i] << types[i] << std::endl;
      if (types[i] != accTypes[i]) throw error{"param types not right."};
    }

    type = INT;
  }
  else if (tree->value == "term factor") {
    type = typeOf(tree->children[0], funcName, table);
  }
  else if (tree->value == "term term STAR factor" ||
           tree->value == "term term SLASH factor" || tree->value == "term term PCT factor") {
    Type left = typeOf(tree->children[0], funcName, table);
    Type right = typeOf(tree->children[2], funcName, table);

    if(left == INT && right == INT) type = INT;
    else throw error{"expression types not valid"};
  }
  else if (tree->value == "expr term") {
    type = typeOf(tree->children[0], funcName, table);
  }
  else if (tree->value == "expr expr PLUS term") {
    Type left = typeOf(tree->children[0], funcName, table);
    Type right = typeOf(tree->children[2], funcName, table);

    if(left == INT && right == INT) type = INT;
    else if ((left == INT_STAR && right == INT) || left == INT && right == INT_STAR) type = INT_STAR;
    else throw error{"expression types not valid"};
  }
  else if (tree->value == "expr expr MINUS term") {
    Type left = typeOf(tree->children[0], funcName, table);
    Type right = typeOf(tree->children[2], funcName, table);

    if((left == INT && right == INT) || (left == INT_STAR && right == INT_STAR)) type = INT;
    else if (left == INT_STAR && right == INT) type = INT_STAR;
    else throw error{"expression types not valid"};

  }
  
  propagateType(tree, type);
  
  return type;
}


void populateVariables(std::map<std::string, Type> * curTable, treeNode * tree) { //on a decls rule
  if (tree->children.size() > 0 && tree->value != "dcls .EMPTY") {
    std::string typeStr = tree->children[1]->children[0]->value.substr(tree->children[1]->children[0]->value.find(" ") + 1);
    Token nameToken = Token{tree->children[1]->children[1]->value};
    Token valueToken = Token{tree->children[3]->value};

    if (curTable->count(nameToken.lexeme)) throw error{"variable already exists"};
    else {
      Type type = convertStringType(typeStr);
      if (type != convertStringType(valueToken.type)) throw error{"variable value and type don't match"};
      (*curTable)[nameToken.lexeme] = type;
      //add type to node
      tree->children[1]->children[1]->value = tree->children[1]->children[1]->value + " : " + typeToString(type);
      tree->children[3]->value = tree->children[3]->value + " : " + typeToString(type);
    }
  }
  if (tree->children.size() > 0) populateVariables(curTable, tree->children[0]);
  
}


void parseStatements(std::string funcName, std::map<std::string, funcInfo> & table, treeNode * tree) {
  if (tree->value == "statements statements statement") {
    for (treeNode * node : tree->children) parseStatements(funcName, table, node);
  }
  else if (tree->value == "statement lvalue BECOMES expr SEMI") {
    Type lhs = typeOf(tree->children[0], funcName, table);
    Type rhs = typeOf(tree->children[2], funcName, table);
    if (lhs != rhs) throw error{"lhs and rhs don't match"};
  }
  else if (tree->value == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
    parseStatements(funcName, table, tree->children[2]);
    parseStatements(funcName, table, tree->children[5]);
    parseStatements(funcName, table, tree->children[9]);
  }
  else if (tree->value == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
    parseStatements(funcName, table, tree->children[2]);
    parseStatements(funcName, table, tree->children[5]);
  }
  else if (tree->value == "statement PRINTLN LPAREN expr RPAREN SEMI") {
    Type exprType = typeOf(tree->children[2], funcName, table);
    if (exprType != INT) throw error{"print type not int"};
  }
  else if (tree->value == "statement DELETE LBRACK RBRACK expr SEMI") {
    Type exprType = typeOf(tree->children[3], funcName, table);
    if (exprType != INT_STAR) throw error{"delete type not int*"};
  }
  else if (tree->value.substr(0, tree->value.find(" ")) == "test") {
    Type lhs = typeOf(tree->children[0], funcName, table);
    Type rhs = typeOf(tree->children[2], funcName, table);
    if (lhs != rhs) throw error{"lhs and rhs don't match"};
  }
}


void generalFuncParse(treeNode * tree, std::vector<Token> paramNames, std::vector<Type> paramTypes, int varDclsIndex, int resultIndex, 
                      int statementIndex, std::map<std::string, funcInfo> & table, std::string funcName) {
  //collect variables
  std::map<std::string, Type> curTable;
  for (int i = 0; i < paramNames.size(); i++) curTable[paramNames[i].lexeme] = paramTypes[i];
  populateVariables(&curTable, tree->children[varDclsIndex]);
  funcInfo func{paramTypes, curTable, INT};
  table[funcName] = func;

  //parse statements
  parseStatements(funcName, table, tree->children[statementIndex]);

  //check result type, add types to nodes
  treeNode * resultNode = tree->children[resultIndex]; 
  Type resultType = typeOf(resultNode, funcName, table);
  if (resultType != func.returnType) throw error{"return type of function not int"};
}


void collectFuncParams(treeNode * tree, std::map<std::string, funcInfo> & table, std::vector<Token> & paramNames, 
                       std::vector<Type> & paramTypes) { // paramlist rule
  treeNode * dclNode = tree->children[0];
  std::string paramTypeStr = dclNode->children[0]->value.substr(dclNode->children[0]->value.find(" ") + 1);
  Type paramType = convertStringType(paramTypeStr); 
  Token paramNameToken{dclNode->children[1]->value};

  //check if param defined twice
  for (Token token : paramNames) {
    if (token.lexeme == paramNameToken.lexeme)  throw error{"param name defined twice."};
  }

  paramNames.emplace_back(paramNameToken);
  paramTypes.emplace_back(paramType);

  //add type information
  dclNode->children[1]->value = dclNode->children[1]->value + " : " + typeToString(paramType);

  if (tree->children.size() == 3) collectFuncParams(tree->children[2], table, paramNames, paramTypes);
}


void populateFunctions(std::map<std::string, funcInfo> & table, treeNode * tree) { //wain or procedures rule
  if (tree->children[0]->value == WAIN_RULE) {
    treeNode * wainNode = tree->children[0];
    if (tree->value == WAIN_RULE) wainNode = tree;

    // get function name
    Token nameToken{wainNode->children[1]->value};

    if(table.count(nameToken.lexeme)) {
      throw error{"function already exists"};
    } else {
      //collecting parameters
      std::string paramOneTypeStr = wainNode->children[3]->children[0]->value.substr(wainNode->children[3]->children[0]->value.find(" ") + 1);
      Type paramOneType = convertStringType(paramOneTypeStr); 
      Token paramOneNameToken{wainNode->children[3]->children[1]->value};

      std::string paramTwoTypeStr = wainNode->children[5]->children[0]->value.substr(wainNode->children[5]->children[0]->value.find(" ") + 1);
      Type paramTwoType = convertStringType(paramTwoTypeStr); 
      Token paramTwoNameToken{wainNode->children[5]->children[1]->value};

      std::vector<Token> paramNames = {paramOneNameToken, paramTwoNameToken};
      std::vector<Type> params = {paramOneType, paramTwoType};

      //check type of second param and param names
      if (paramOneNameToken.lexeme == paramTwoNameToken.lexeme) throw error{"param names are equal"};
      if (paramTwoType != INT) throw error{"second wain param not an int"};

      //add types to params
      wainNode->children[3]->children[1]->value = wainNode->children[3]->children[1]->value + " : " + typeToString(paramOneType);
      wainNode->children[5]->children[1]->value = wainNode->children[5]->children[1]->value + " : " + typeToString(paramTwoType);

      //general function parsing
      generalFuncParse(wainNode, paramNames, params, 8, 11, 9, table, nameToken.lexeme);
    }
  } else {
    std::vector<Token> paramNames;
    std::vector<Type> paramTypes;
    treeNode * funcNode = tree->children[0];
    Token nameToken{funcNode->children[1]->value};

    if (table.count(nameToken.lexeme)) {
      throw error{"function already exists"};
    } else {
      //collect parameters
      if (funcNode->children[3]->value != "params .EMPTY") {
        collectFuncParams(funcNode->children[3]->children[0], table, paramNames, paramTypes);
      }

      //general function parsing
      generalFuncParse(funcNode, paramNames, paramTypes, 6, 9, 7, table, nameToken.lexeme);

      populateFunctions(table, tree->children[1]); // go look for more functions
    }
  }
}



int main() {
    std::istream& in = std::cin;
    std::vector<std::string> inputs;
    std::map<std::string, funcInfo> symbolTable;
    std::map<std::string, Type> * curTable;
    std::string line;
    int lineSkips = 0;

    while(std::getline(in, line)) {
        inputs.emplace_back(line);
    }

    treeNode * tree = createTree(inputs, 3, 0);

    try {
      populateFunctions(symbolTable, tree->children[1]);
      tree->print();
    } catch (error & e) {
      e.printMessage();
    }
    

    delete tree;

}

