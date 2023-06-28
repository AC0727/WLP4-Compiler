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

Type stringToType(std::string typeStr) {
  if (typeStr == "INT" || typeStr == "NUM" || typeStr == "int") return INT;
  else return INT_STAR;
}


std::string typeToString(Type type) {
  if (type == INT) return "int";
  else return "int*";
}

struct Token {
    std::string type;
    std::string lexeme;
    Type dataType = INT;

    Token(std::string str) {
      type = str.substr(0, str.find(' '));
      lexeme = str.substr(str.find(' ') + 1, str.size());
      
      if (str.find(':') != std::string::npos) {
        lexeme = lexeme.substr(0, lexeme.find(':'));
        lexeme.pop_back();
        dataType = stringToType(str.substr(str.find(':') + 2, str.size()));
      }

      

    }

    std::string toString() {return type +  " " + lexeme;}
};

struct varInfo {
  Type type;
  int offset;
};

struct treeNode {
  Token value;
  std::vector<treeNode *> children;

  treeNode(Token str): value{str} {}

  treeNode(Token str, std::vector<treeNode *> newChildren): value{str}, children{newChildren} {}

  void print() {
    std::cout << value.toString() << std::endl;
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


std::string intToStr(int i) {
  std::ostringstream oss;
  oss << i;
  return oss.str();
}


int rhsSize(std::string rule) {
    int num = 0;
    if (rule == ".EMPTY") return num;
    for (int i = 0; i < rule.size() - 1; i++) {
      if (rule[i + 1] == ':') break;
      else if (rule[i] == ' ') num++;
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
        treeNode * node{createTree(inputs, newChildren, i)};
        int linesSkip = subTreeSize(node);
        children.emplace_back(node);

        i += linesSkip;
        counter++;
    }

    if (!children.empty()) tree = new treeNode(value, children);
    else tree = new treeNode(value);

    return tree;
}


void pushThree() {
  std::cout << "sw $3, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
}


void popFive() {
  std::cout << "add $30, $30, $4" << std::endl << "lw $5, -4($30)" << std::endl;
}



void evaluateExpr(treeNode * tree, std::map<std::string, varInfo> & symbolTable) {
  if (tree->value.lexeme == "NUM") {
    std::cout << "lis $3" << std::endl;
    std::cout << ".word " + tree->children[0]->value.lexeme << std::endl;
  }
  else if (tree->value.lexeme == "ID") {
    std::string name = tree->children[0]->value.lexeme;
    int offset = symbolTable[name].offset;
    std::cout << "lw $3, " + intToStr(offset) + "($29)" << std::endl;
  }
  else if (tree->value.lexeme == "LPAREN expr RPAREN" || tree->value.lexeme == "LPAREN lvalue RPAREN") {
    evaluateExpr(tree->children[1], symbolTable);
  }
  else if (tree->value.lexeme == "factor" || tree->value.lexeme == "term") {
    evaluateExpr(tree->children[0], symbolTable);
  }
  else if (tree->value.lexeme == "STAR factor") {
    evaluateExpr(tree->children[1], symbolTable);
    std::cout << "lw $3, 0($3)" << std::endl; 
  }
  else if (tree->value.lexeme == "NULL") {
    std::cout << "add $3, $0, $11 " << std::endl;
  }
  else if (tree->value.lexeme == "expr PLUS term") {
    treeNode * exprNode = tree->children[0];
    treeNode * termNode = tree->children[2];

    if (exprNode->value.dataType == INT && termNode->value.dataType == INT) {
      evaluateExpr(exprNode, symbolTable);
      pushThree();
      evaluateExpr(termNode, symbolTable);
      popFive();
      std::cout << "add $3, $5, $3" << std::endl;
    }
    else if (exprNode->value.dataType == INT_STAR && termNode->value.dataType == INT) {
      evaluateExpr(exprNode, symbolTable);
      pushThree();
      evaluateExpr(termNode, symbolTable);
      std::cout << "mult $3, $4" << std::endl << "mflo $3" << std::endl;
      popFive();
      std::cout << "add $3, $5, $3" << std::endl;
    }
    else if (exprNode->value.dataType == INT && termNode->value.dataType == INT_STAR) {
      evaluateExpr(exprNode, symbolTable);
      std::cout << "mult $3, $4" << std::endl << "mflo $3" << std::endl;
      pushThree();
      evaluateExpr(termNode, symbolTable);
      popFive();
      std::cout << "add $3, $5, $3" << std::endl;
    } 
  } 
  else if (tree->value.lexeme == "expr MINUS term") {
    treeNode * exprNode = tree->children[0];
    treeNode * termNode = tree->children[2];

    if (exprNode->value.dataType == INT && termNode->value.dataType == INT) {
      evaluateExpr(exprNode, symbolTable);
      pushThree();
      evaluateExpr(termNode, symbolTable);
      popFive();
      std::cout << "sub $3, $5, $3" << std::endl;
    }
    else if (exprNode->value.dataType == INT_STAR && termNode->value.dataType == INT) {
      evaluateExpr(exprNode, symbolTable);
      pushThree();
      evaluateExpr(termNode, symbolTable);
      std::cout << "mult $3, $4" << std::endl << "mflo $3" << std::endl;
      popFive();
      std::cout << "sub $3, $5, $3" << std::endl;
    }
    else if (exprNode->value.dataType == INT_STAR && termNode->value.dataType == INT_STAR) {
      evaluateExpr(exprNode, symbolTable);
      pushThree();
      evaluateExpr(termNode, symbolTable);
      popFive();
      std::cout << "sub $3, $5, $3" << std::endl << "div $3, $4" << std::endl << "mflo $3" << std::endl;
    }
  }
  else if (tree->value.lexeme == "term STAR factor") {
    evaluateExpr(tree->children[0], symbolTable);
    pushThree();
    evaluateExpr(tree->children[2], symbolTable);
    popFive();
    std::cout << "mult $3, $5" << std::endl;
    std::cout << "mflo $3" << std::endl;
  }
  else if (tree->value.lexeme == "term SLASH factor") {
    evaluateExpr(tree->children[0], symbolTable);
    pushThree();
    evaluateExpr(tree->children[2], symbolTable);
    popFive();
    std::cout << "div $5, $3" << std::endl;
    std::cout << "mflo $3" << std::endl;
  }
  else if (tree->value.lexeme == "term PCT factor") {
    evaluateExpr(tree->children[0], symbolTable);
    pushThree();
    evaluateExpr(tree->children[2], symbolTable);
    popFive();
    std::cout << "div $5, $3" << std::endl;
    std::cout << "mfhi $3" << std::endl;
  }
  else if (tree->value.lexeme == "ID LPAREN RPAREN") {
    std::string funcName = "F" + tree->children[0]->value.lexeme;
    std::cout << "sw $31, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
    std::cout << "sw $29, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
    std::cout << "lis $5" << std::endl << ".word " + funcName << std::endl;
    std::cout << "jalr $5" << std::endl;
    std::cout << "add $30, $30, $4" << std::endl << "lw $29, -4($30)" << std::endl;
    std::cout << "add $30, $30, $4" << std::endl << "lw $31, -4($30)" << std::endl;
  }
  else if (tree->value.lexeme == "ID LPAREN arglist RPAREN") {
    std::string funcName = "F" + tree->children[0]->value.lexeme;
    std::cout << "sw $31, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
    std::cout << "sw $29, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;

    treeNode * args = tree->children[2];
    int numArgs = 0;
    while(args->children.size() == 3) {
      treeNode * expNode = args->children[0];
      evaluateExpr(expNode, symbolTable);
      pushThree();
      numArgs++;
      args = args->children[2];
    }
    evaluateExpr(args->children[0], symbolTable);
    pushThree();
    numArgs++;

    std::cout << "lis $5" << std::endl << ".word " + funcName << std::endl;
    std::cout << "jalr $5" << std::endl;
    for (int i = 0; i < numArgs; i++) std::cout << "add $30, $30, $4" << std::endl;
    std::cout << "add $30, $30, $4" << std::endl << "lw $29, -4($30)" << std::endl;
    std::cout << "add $30, $30, $4" << std::endl << "lw $31, -4($30)" << std::endl;
    
  }
  else if (tree->value.lexeme == "AMP lvalue") {
    treeNode * lval = tree->children[1];

    while (true) {
      if (lval->value.lexeme == "ID") {
        std::string id = lval->children[0]->value.lexeme;
        int offset = symbolTable[id].offset;
        std::cout << "lis $3" << std::endl << ".word " + intToStr(offset) << std::endl;
        std::cout << "add $3, $3, $29" << std::endl;
        break;
      }
      else if (lval->value.lexeme == "STAR factor" ) {
        evaluateExpr(lval->children[1], symbolTable);
        break;
      }
      else if (lval->value.lexeme == "LPAREN lvalue RPAREN") {
        lval = lval->children[1];
      }
    }
  }
  else if (tree->value.lexeme == "NEW INT LBRACK expr RBRACK") {
    evaluateExpr(tree->children[3], symbolTable);
    std::cout << "add $1, $3, $0" << std::endl;
    std::cout << "sw $31, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
    std::cout << "lis $5" << std::endl << ".word new" << std::endl << "jalr $5" << std::endl;
    std::cout << "add $30, $30, $4" << std::endl << "lw $31, -4($30)" << std::endl;
    std::cout << "bne $3, $0, 1" << std::endl;
    std::cout << "add $3, $11, $0" << std::endl;
  }
}


void printFuncVars(treeNode * tree, int & curOffset, std::map<std::string, varInfo> & symbolTable, int & numVars) {
  if (tree->value.lexeme == ".EMPTY") {
    return;
  } else {
    treeNode * dcl = tree->children[1];
    std::string name = dcl->children[1]->value.lexeme;
    Type type = stringToType(dcl->children[0]->value.lexeme);
    curOffset -= 4;
    symbolTable[name] = varInfo{type, curOffset};
    std::cout << ";; VAR DECLARATION" << std::endl;
    std::cout << "lis $3" << std::endl;
    if (tree->children[3]->value.lexeme == "NULL") {
      std::cout << ".word 1" << std::endl;
    } else {
      std::cout << ".word " + tree->children[3]->value.lexeme << std::endl;
    }
    std::cout <<  "sw $3, " + intToStr(curOffset) + "($29)" << std::endl;
    std::cout << "sub $30,$30,$4" << std::endl;
    numVars++;
    printFuncVars(tree->children[0], curOffset, symbolTable, numVars);
  }
}


std::string generateLabel(int & i) {
  std::string str = "L" + intToStr(i);
  i++;
  return str;
}


void printFuncBody(treeNode * tree, std::map<std::string, varInfo> & symbolTable, int & label) {
  if (tree->value.lexeme == "statements statement") {
    for (treeNode * node : tree->children) printFuncBody(node, symbolTable, label);
  }
  else if (tree->value.lexeme == "lvalue BECOMES expr SEMI") {
    evaluateExpr(tree->children[2], symbolTable);
    //get id from lvalue, WORKS FOR ID VARS ONLY RN
    std::string id = tree->children[0]->children[0]->value.lexeme;
    if (tree->children[0]->value.lexeme == "STAR factor") {
      pushThree();
      evaluateExpr(tree->children[0]->children[1], symbolTable);
      popFive();
      std::cout << "sw $5, 0($3)" << std::endl;
    }
    else {
      std::string offset = intToStr(symbolTable[id].offset);
      std::cout << "sw $3, " + offset + "($29)" << std::endl;
    }
  }
  else if (tree->value.lexeme == "expr EQ expr") {
    evaluateExpr(tree->children[0], symbolTable);
    pushThree();
    evaluateExpr(tree->children[2], symbolTable);
    popFive();
    std::cout << "slt $6, $3, $5" << std::endl << "slt $7, $5, $3" << std::endl << "add $3, $6, $7"; 
    std::cout << std::endl << "sub $3, $11, $3" << std::endl;
  }
  else if (tree->value.lexeme == "expr NE expr") {
    evaluateExpr(tree->children[0], symbolTable);
    pushThree();
    evaluateExpr(tree->children[2], symbolTable);
    popFive();
    std::cout << "slt $6, $3, $5" << std::endl << "slt $7, $5, $3" << std::endl << "add $3, $6, $7" << std::endl;
  }
  else if (tree->value.lexeme == "expr LT expr") {
    evaluateExpr(tree->children[0], symbolTable);
    pushThree();
    evaluateExpr(tree->children[2], symbolTable);
    popFive();
    if (tree->children[0]->value.dataType == INT) std::cout << "slt $3, $5, $3" << std::endl;
    else std::cout << "sltu $3, $5, $3" << std::endl;
  }
  else if (tree->value.lexeme == "expr GT expr") {
    evaluateExpr(tree->children[0], symbolTable);
    pushThree();
    evaluateExpr(tree->children[2], symbolTable);
    popFive();
    if (tree->children[0]->value.dataType == INT) std::cout << "slt $3, $3, $5" << std::endl;
    else std::cout << "sltu $3, $3, $5" << std::endl;
  }
  else if (tree->value.lexeme == "expr LE expr") {
    evaluateExpr(tree->children[0], symbolTable);
    pushThree();
    evaluateExpr(tree->children[2], symbolTable);
    popFive();
    if (tree->children[0]->value.dataType == INT) std::cout << "slt $3, $3, $5" << std::endl;
    else std::cout << "sltu $3, $3, $5" << std::endl;
    std::cout << "sub $3, $11, $3" << std::endl;
  }
  else if (tree->value.lexeme == "expr GE expr") {
    evaluateExpr(tree->children[0], symbolTable);
    pushThree();
    evaluateExpr(tree->children[2], symbolTable);
    popFive();
    if (tree->children[0]->value.dataType == INT) std::cout << "slt $3, $5, $3" << std::endl;
    else std::cout << "sltu $3, $5, $3" << std::endl;
    std::cout << "sub $3, $11, $3" << std::endl;
  }
  else if (tree->value.lexeme == "IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
    std::cout << std::endl << ";; IF STATEMENT" << std::endl;
    std::string firstLabel = generateLabel(label);
    std::string secLabel = generateLabel(label);
    
    printFuncBody(tree->children[2], symbolTable, label);
    std::cout << "beq $3, $0, " << firstLabel << std::endl;
    printFuncBody(tree->children[5], symbolTable, label);
    std::cout << "beq $0, $0, " << secLabel << std::endl;
    std::cout << firstLabel << ":" << std::endl;
    printFuncBody(tree->children[9], symbolTable, label);
    std::cout << secLabel << ":" << std::endl;
    std::cout << std::endl << ";; END IF STATEMENT" << std::endl;
  }
  else if (tree->value.lexeme == "WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
    std::cout << std::endl << ";; WHILE LOOP" << std::endl;
    std::string firstLabel = generateLabel(label);
    std::string secLabel = generateLabel(label);

    std::cout << firstLabel << ":" << std::endl;
    printFuncBody(tree->children[2], symbolTable, label);
    std::cout << "beq $3, $0, " << secLabel << std::endl;
    printFuncBody(tree->children[5], symbolTable, label);
    std::cout << "beq $0, $0, " << firstLabel << std::endl;
    std::cout << secLabel << ":" << std::endl;
    std::cout << ";; END WHILE LOOP" << std::endl;
  }
  else if (tree->value.lexeme == "PRINTLN LPAREN expr RPAREN SEMI") {
    std::cout << ";; PRINT" << std::endl;
    std::cout << "sw $1, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
    evaluateExpr(tree->children[2], symbolTable);
    std::cout << "add $1, $3, $0" << std::endl;
    std::cout << "sw $31, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
    std::cout << "lis $5" << std::endl << ".word print" << std::endl << "jalr $5" << std::endl;
    std::cout << "add $30, $30, $4" << std::endl << "lw $31, -4($30)" << std::endl;
    std::cout << "add $30, $30, $4" << std::endl << "lw $1, -4($30)" << std::endl;
    std::cout << ";; END PRINT" << std::endl;
  }
  else if (tree->value.lexeme == "DELETE LBRACK RBRACK expr SEMI") {
    evaluateExpr(tree->children[3], symbolTable);
    std::string deleteLabel = generateLabel(label);
    std::cout << "beq $3, $11, " << deleteLabel << std::endl;
    std::cout << "add $1, $3, $0" << std::endl;
    std::cout << "sw $31, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
    std::cout << "lis $5" << std::endl << ".word delete" << std::endl << "jalr $5" << std::endl;
    std::cout << "add $30, $30, $4" << std::endl << "lw $31, -4($30)" << std::endl;
    std::cout << deleteLabel << ":" << std::endl;
  }

  
}


void printWain(treeNode * tree, int & curOffset, int &label, std::map<std::string, varInfo> & symbolTable) {

  //print prolog
  std::cout << ";; PROLOG" << std::endl;
  std::cout << "lis $4" << std::endl;
  std::cout << ".word 4" << std::endl;
  std::cout << "lis $11" << std::endl;
  std::cout << ".word 1" << std::endl;
  std::cout << "sub $29,$30,$4" << std::endl;
  std::cout << "sw $1,0($29)" << std::endl;
  std::cout << "sub $30,$30,$4" << std::endl;
  std::cout << "sw $2,-4($29)" << std::endl;
  std::cout << "sub $30,$30,$4" << std::endl << std::endl;

  //add params to symbol table
  Token firstParam = tree->children[3]->children[1]->value;
  Token secParam = tree->children[5]->children[1]->value;
  symbolTable[firstParam.lexeme] = varInfo{INT, 0};
  symbolTable[secParam.lexeme] = varInfo{INT, -4};

  // performing init
  std::cout << "sw $2, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
  if (firstParam.dataType == INT_STAR) std::cout << "sw $2, -4($29)" << std::endl;
  else std::cout << "add $2, $0, $0" << std::endl;
  std::cout << "sw $31, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
  std::cout << "lis $5" << std::endl << ".word init" << std::endl << "jalr $5" << std::endl;
  std::cout << "add $30, $30, $4" << std::endl << "lw $31, -4($30)" << std::endl;
  std::cout << "add $30, $30, $4" << std::endl << "lw $2, -4($30)" << std::endl;
  

  int numVars = 2;
  printFuncVars(tree->children[8], curOffset, symbolTable, numVars);
  std::cout << std::endl << ";; STATEMENTS" << std::endl;
  printFuncBody(tree->children[9], symbolTable, label);
  std::cout << std::endl << ";; RETURN" << std::endl;
  evaluateExpr(tree->children[11], symbolTable); //return
  

  //print epilog
  std::cout << std::endl << ";; EPILOG" << std::endl;
  std::cout << "add $30,$29,$4" << std::endl;
  std::cout << "jr $31" << std::endl;
}



void generateCode(treeNode * tree, int &label) {

  if (tree->value.lexeme == "procedure procedures") {
    for (treeNode * child : tree->children) generateCode(child, label);
  }
  else if (tree->value.lexeme == "INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
    std::map<std::string, varInfo> symbolTable;
    std::string funcName = "F" + tree->children[1]->value.lexeme;
    std::cout << funcName << ":" << std::endl;
    std::cout << "sub $29, $30, $4" << std::endl;

    //parameters
    int numVars = 0;
    std::vector<std::string> paramIDs;
    std::vector<Type> paramTypes;
    treeNode * params = tree->children[3];
    // collecting them
    if (params->children.size() > 0) {
      treeNode * paramList = params->children[0];
      while(paramList->children.size() == 3) {
        treeNode * id = paramList->children[0];
        paramTypes.emplace_back(stringToType(id->children[0]->value.lexeme));
        paramIDs.emplace_back(id->children[1]->value.lexeme);
        numVars++;
        paramList = paramList->children[2];
      }
      treeNode * id = paramList->children[0];
      paramTypes.emplace_back(stringToType(id->children[0]->value.lexeme));
      paramIDs.emplace_back(id->children[1]->value.lexeme);
      numVars++;
    }

    // add to symbol table
    int paramOffset = 4;
    if (paramIDs.size() > 0) {
      for (int i = paramIDs.size() - 1; i >= 0; i--) {
        varInfo info{paramTypes[i], paramOffset};
        symbolTable[paramIDs[i]] = info;
        paramOffset += 4;
      }
    }
    
    //declarations
    int curOffset = 4;
    printFuncVars(tree->children[6], curOffset, symbolTable, numVars); // make sure $29 pointing past arguments

    // save registers
    std::cout << "sw $5, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
    std::cout << "sw $6, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;
    std::cout << "sw $7, -4($30)" << std::endl << "sub $30, $30, $4" << std::endl;

    // statements
    std::cout << std::endl << ";; STATEMENTS" << std::endl;
    printFuncBody(tree->children[7], symbolTable, label);

    // return expression
    std::cout << std::endl << ";; RETURN" << std::endl;
    evaluateExpr(tree->children[9], symbolTable);

    // pop registers
    std::cout << "add $30, $30, $4" << std::endl << "lw $7, -4($30)" << std::endl;
    std::cout << "add $30, $30, $4" << std::endl << "lw $6, -4($30)" << std::endl;
    std::cout << "add $30, $30, $4" << std::endl << "lw $5, -4($30)" << std::endl;

    //pop local variables
    for (int i = 0; i < numVars; i++) std::cout << "add $30, $30, $4" << std::endl;

    std::cout << "add $30, $29, $4" << std::endl;
    std::cout << "jr $31" << std::endl << std::endl;
  }
  else if (tree->value.lexeme == "main") {
    std::map<std::string, varInfo> symbolTable;
    int curOffset = -4;
    std::cout << "wain:" << std::endl;
    printWain(tree->children[0], curOffset, label, symbolTable);
  }
}


int main() {
    std::istream& in = std::cin;
    std::vector<std::string> inputs;
    std::string line;
    int label = 0;

    while(std::getline(in, line)) {
        inputs.emplace_back(line);
    }

    treeNode * tree = createTree(inputs, 3, 0);
    std::cout << ".import print" << std::endl;
    std::cout << ".import init" << std::endl;
    std::cout << ".import new" << std::endl;
    std::cout << ".import delete" << std::endl;
    std::cout << "beq $0, $0, wain" << std::endl;
    generateCode(tree->children[1], label);

    delete tree;   
}

