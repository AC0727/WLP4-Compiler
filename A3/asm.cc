#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "scanner.h"
#include <map>
#include <memory>
#include <set>

class InvalidInst {
    std::vector<Token> &tokenLine;

    public:
        InvalidInst(std::vector<Token> &tokenLine) : tokenLine(tokenLine) {}

        void printMessage() const {
          std::cerr << "ERROR Not a valid instruction: ";
          for (auto &token : tokenLine) {
            std::cerr << token << ' ';
          }
          std::cerr << std::endl;
        }

};

// .word
bool validWord(std::vector<Token> & tokenLine, std::map<std::string, int64_t> &symbolTable) {
  if (tokenLine.size() != 2 || tokenLine[0].getKind() != Token::WORD) return false;

  if (tokenLine[1].getKind() != Token::INT && 
      tokenLine[1].getKind() != Token::HEXINT && tokenLine[1].getKind() != Token::ID) {
    return false;
  }

  if (tokenLine[1].getKind() == Token::INT || tokenLine[1].getKind() == Token::HEXINT) {
    int64_t num = tokenLine[1].toNumber();
    if (num > 4294967295 || num < -2147483648) return false;
  } 
  else if (!symbolTable.count(tokenLine[1].getLexeme())) return false; // label doesn't exist

  return true;
}

// .word
void word(std::vector<Token> & tokenLine, int64_t * num, std::map<std::string, int64_t> &symbolTable) {

  if (tokenLine[1].getKind() == Token::INT) {
    *num = tokenLine[1].toNumber();

  } else if (tokenLine[1].getKind() == Token::HEXINT) {
    *num = tokenLine[1].toNumber();

  } else if (tokenLine[1].getKind() == Token::ID) {
    *num = symbolTable[tokenLine[1].getLexeme()];
  }

}

// checks if isntruction is in valid format
bool validBasicNumFomat(std::vector<Token> & tokenLine) {
  if (tokenLine.size() != 6) return false;

  if (tokenLine[0].getLexeme() != "add" && tokenLine[0].getLexeme() != "sub" && 
      tokenLine[0].getLexeme() != "slt" && tokenLine[0].getLexeme() != "sltu") return false;

  for (int i = 1; i < 6; i++) {
    
    if (i % 2 == 0 && tokenLine[i].getKind() != Token::COMMA) {
      return false;
    }
    else if (i % 2 == 1 && tokenLine[i].getKind() != Token::REG) return false;

    //check if register between 1 and 31
    if (i == 1 || i == 3 || i == 5) {
      if (tokenLine[i].toNumber() < 0 || tokenLine[i].toNumber() > 31) return false;
    }
  }
  

  return true;
}

// add, sub, stl, stlu
void basicNumOps(std::vector<Token> & tokenLine, int64_t * num) {
  std::string id = tokenLine[0].getLexeme();
  int destReg = tokenLine[1].toNumber();
  int sourceRegOne = tokenLine[3].toNumber();
  int sourceRegTwo = tokenLine[5].toNumber();
  int filler;


  if (id == "add") {
    filler = 32;
  } 
  else if (id == "sub") {
    filler = 34;
  } 
  else if (id == "slt") {
    filler = 42;
  }
  else if (id == "sltu") {
    filler = 43;
  }

  *num = (sourceRegOne << 21) | (sourceRegTwo << 16) | (destReg << 11) | (filler);
}

// bne, beq validity
bool validBranch(std::vector<Token> & tokenLine, std::map<std::string, int64_t> &symbolTable) {
  if (tokenLine.size() != 6) return false;

  if (tokenLine[0].getLexeme() != "beq" && tokenLine[0].getLexeme() != "bne") return false;

  for (int i = 1; i < 6; i++) {
    
    if (i % 2 == 0 && tokenLine[i].getKind() != Token::COMMA) {
      return false;
    }
    else if (i == 5 && (tokenLine[i].getKind() != Token::INT && tokenLine[i].getKind() != Token::HEXINT && tokenLine[i].getKind() != Token::ID)) {
      return false;
    }
    else if (i != 5 && i % 2 == 1 && tokenLine[i].getKind() != Token::REG) return false;

    //check if register between 1 and 31
    if (i == 1 || i == 3) {
      if (tokenLine[i].toNumber() < 0 || tokenLine[i].toNumber() > 31) return false;
    }
  }

  //number within bounds of a 32 bit signed/unsigned integer
  if (tokenLine[5].getKind() == Token::INT || tokenLine[5].getKind() == Token::HEXINT) {
    int64_t num = tokenLine[5].toNumber();
    if (num > 65535 || num < -32768) return false;
  } 
  else if (!symbolTable.count(tokenLine[5].getLexeme())) return false; // label doesn't exist

  return true;
}

// bne, beq
void branch(std::vector<Token> & tokenLine, int64_t * num, int64_t curAdd, std::map<std::string, int64_t> &symbolTable) {
              
  std::string id = tokenLine[0].getLexeme();
  int opcode;
  int64_t index;

  if (tokenLine[5].getKind() == Token::INT || tokenLine[5].getKind() == Token::HEXINT) {
    index = tokenLine[5].toNumber();
  } else { // then it's a label
    index = ((symbolTable[tokenLine[5].getLexeme()] - 4 - curAdd)/4);
  }
  
  int regOne = tokenLine[1].toNumber();
  int regTwo = tokenLine[3].toNumber();

  if (id == "beq") {
    opcode = 4;
  } 
  else if (id == "bne") {
    opcode = 5;
  } 

  *num = (opcode << 26) | (regOne << 21) | (regTwo << 16) | (index & 0xffff);

}

// mult, div, multu, divu validity
bool validMultDiv(std::vector<Token> & tokenLine) {
  if (tokenLine.size() != 4) return false;

  if (tokenLine[0].getLexeme() != "mult" && tokenLine[0].getLexeme() != "div" && 
      tokenLine[0].getLexeme() != "multu" && tokenLine[0].getLexeme() != "divu") return false;

  for (int i = 1; i < 4; i++) {
    
    if (i % 2 == 0 && tokenLine[i].getKind() != Token::COMMA) {
      return false;
    }
    else if (i % 2 == 1 && tokenLine[i].getKind() != Token::REG) return false;

    //check if register between 1 and 31
    if (i == 1 || i == 3) {
      if (tokenLine[i].toNumber() < 0 || tokenLine[i].toNumber() > 31) return false;
    }
  }

  return true;
}

// mult, div, multu, divu 
void multDiv(std::vector<Token> & tokenLine, int64_t * num) {
  std::string id = tokenLine[0].getLexeme();
  int sourceRegOne = tokenLine[1].toNumber();
  int sourceRegTwo = tokenLine[3].toNumber();
  int filler;

  if (id == "mult") {
    filler = 24;
  } 
  else if (id == "div") {
    filler = 26;
  } 
  else if (id == "multu") {
    filler = 25;
  }
  else if (id == "divu") {
    filler = 27;
  }

  *num = (sourceRegOne << 21) | (sourceRegTwo << 16) | (filler);
}

// lis, mfhi, mflo, jalr, jr validity
bool validAssignJump(std::vector<Token> & tokenLine) {
  if (tokenLine.size() != 2) return false;
  else if (tokenLine[0].getLexeme() != "mfhi" && 
           tokenLine[0].getLexeme() != "mflo" && tokenLine[0].getLexeme() != "lis" &&
           tokenLine[0].getLexeme() != "jr" && tokenLine[0].getLexeme() != "jalr") return false;
  else if (tokenLine[1].getKind() != Token::REG) return false;
  else if (tokenLine[1].toNumber() < 0 || tokenLine[1].toNumber() > 31) return false;

  return true;
}

// lis, mfhi, mflo, jalr, jr
void assignJump(std::vector<Token> & tokenLine, int64_t * num) {
  std::string id = tokenLine[0].getLexeme();
  int sourceRegOne = tokenLine[1].toNumber();
  int filler;

  if (id == "mfhi") {
    filler = 16;
    *num = (sourceRegOne << 11) | (filler);
  } 
  else if (id == "mflo") {
    filler = 18;
    *num = (sourceRegOne << 11) | (filler);
  } 
  else if (id == "lis") {
    filler = 20;
    *num = (sourceRegOne << 11) | (filler);
  }
  else if (id == "jr") {
    filler = 8;
    *num = (sourceRegOne << 21) | (filler);
  }
  else if (id == "jalr") {
    filler = 9;
    *num = (sourceRegOne << 21) | (filler);
  }

  
}

// lw, sw validity
bool validLoadStore(std::vector<Token> & tokenLine) {
  if (tokenLine.size() != 7) return false;

  if (tokenLine[0].getLexeme() != "sw" && tokenLine[0].getLexeme() != "lw") return false;
  
  for (int i = 1; i < 7; i++) {
    if (i == 1 || i == 5) {
      if (tokenLine[i].getKind() != Token::REG) return false;
      else if (tokenLine[i].toNumber() < 0 || tokenLine[i].toNumber() > 31) return false;
    }
    else if (i == 4 && tokenLine[i].getLexeme() != "(") return false;
    else if (i == 6 && tokenLine[i].getLexeme() != ")") return false;
    else if (i == 2 && tokenLine[i].getKind() != Token::COMMA) return false;
    else if (tokenLine[3].toNumber() > 65535 || tokenLine[3].toNumber() < -32768) return false;
  }
  
  return true;
}

// lw, sw
void loadStore(std::vector<Token> & tokenLine, int64_t * num) {
  std::string id = tokenLine[0].getLexeme();
  int destReg = tokenLine[5].toNumber();
  int sourceRegOne = tokenLine[1].toNumber();
  int opcode;
  int index = tokenLine[3].toNumber();

  if (id == "lw") {
    opcode = 35;
  } 
  else if (id == "sw") {
    opcode = 43;
  }

  *num = (opcode << 26) |(destReg << 21) | (sourceRegOne << 16) | (index & 0xffff);
}



/*
 * C++ Starter code for CS241 A3
 *
 * This file contains the main function of your program. By default, it just
 * prints the scanned list of tokens back to standard output.
 */
int main() {
  int64_t address = 0;
  std::string line;
  std::map<std::string, int64_t> symbolTable;
  //all addresses and instructions
  std::vector<std::vector<Token>> program;

  try {
    //first pass through, collect labels
    while (getline(std::cin, line)) {
      std::vector<Token> tokenLine = scan(line);

      if (line[0] != ';')  { // not a comment
        int numLabels = 0;
        std::string label;

        // populate symbole table
        for (int i = 0; i < tokenLine.size(); i++) {
          if (tokenLine[i].getKind() == Token::LABEL) {
            label = tokenLine[i].getLexeme().substr(0, tokenLine[i].getLexeme().size() - 1);

            //make sure label doesn't appear more than once
            if (symbolTable.count(label)) {
              throw InvalidInst(tokenLine); 
            }
            else symbolTable[label] = address;
          }
          else {
            tokenLine.erase(tokenLine.begin(), tokenLine.begin() + i); //remove the label tokens
            break;
          }
          numLabels++;
        }

        // if it's an actual instruction
        if (numLabels != tokenLine.size()) {
          program.emplace_back(tokenLine); 
          address+=4;
        }
      }
    }

    //for (auto pair : symbolTable) std::cerr << pair.first << " " << pair.second << std::endl;

    // second pass through
    for(int64_t i = 0; i < program.size(); i++) { // i * 4 is address of instruction
      std::vector<Token> tokenLine = program[i];
      int64_t inst;

      if (validWord(tokenLine, symbolTable)) {
        word(tokenLine, &inst, symbolTable);
      } 
      else if (validBasicNumFomat(tokenLine)) {
        basicNumOps(tokenLine, &inst);
      }
      else if (validBranch(tokenLine, symbolTable)) {
        branch(tokenLine, &inst, i * 4, symbolTable);
      }
      else if (validAssignJump(tokenLine)) {
        assignJump(tokenLine, &inst);
      }
      else if (validMultDiv(tokenLine)) {
        multDiv(tokenLine, &inst);
      }
      else if (validLoadStore(tokenLine)) {
        loadStore(tokenLine, &inst);
      } 
      else { // throw exception
        throw InvalidInst(tokenLine);
      }

      std::cout << char(inst >> 24) << char(inst >> 16) << char(inst >> 8) << char(inst);
    }

  } catch (ScanningFailure &f) {
    std::cerr << f.what() << std::endl;
    return 1;
  } catch (InvalidInst &err) {
    err.printMessage();
  }
  
  return 0;
}

