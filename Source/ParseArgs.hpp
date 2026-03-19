#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <stdexcept>
#include <math.h>
#include <typeinfo>

/**************************************************************************************************/
/* class ParseArgs is a command line argument or switch parser.                                   */
/* Usage                                                                                          */
/* 1) Define the variables that you want the switches to modify. They can be 1 of 4 types:        */
/*    bool for boolean switches, int64_t for integer switches,                                    */
/*    double for real switches and std::string_view for text switches                             */
/*    They can also be 1 of 4 vector types which are std::vector<base types>                      */
/* 2) Create an instance of ParseArgs (i.e. ParseArgs parseArgs;)                                 */
/* 3) For each desired ordered arg, call the function                                             */
/*    addArg(const int argNum, const char* desc,  <type>* const valPtr, bool req = false)         */
/*       int argNum is the index of the ordered argument on the command line.                     */
/*       char* desc is the description that will be printed with the                              */
/*       <type> valPtr is a pointer to a user variable to be modified by the command line arg     */
/*       bool req is an optional flag which indicates this argument or switch is required.        */
/*    For each desired switch, call the function                                                  */
/*    addArg(const char* arg, const char* desc,  <type>* const valPtr, bool req = false)          */
/*       char* arg is the switch or argument name on the command line.                            */
/*       char* desc is the description that will be printed with the                              */
/*       <type> valPtr is a pointer to a user variable to be modified by the command line arg     */
/*       bool req is an optional flag which indicates this argument or switch is required.        */
/* 3a) Optionally add a lambda that performs some check on the value of the argument. The lambda  */
/*     is of type void and should take as input, a pointer to the variable provided in step 3. If */
/*     the check fails, the lambda should throw any std::exception with text explaining the error.*/
/*     check fails, the lambda should throw any std::exception with text explaining the error.    */
/*     addCheck(const char* argName, std::function<void(<type>*)> check)                          */
/*       char* arg is the switch or argument name on the command line.                            */
/*       std::function<void(ArgValue_t*)> check is a reference to a lambda that does the check.   */
/* 4) Call ParseArgs.parse(const int argc, const char* argv[], int start = 1)                     */
/*       int argc is the number is char* in argv                                                  */
/*       char* argv[] is an array of pointers to char string whish are the args.                  */
/*       int start the index of argv that parsing should start on                                 */
/*       if parse() returns true then no errors we found.                                         */
/*       if parse() returns false, then the program should clean up and exit().                   */
/*                                                                                                */
/* Regardless of how the arguments types defined, the user can enter the switch arguments in 2    */
/* formats:                                                                                       */
/* 1. the command line switch and value separated by a space                                      */
/* 2. the command line switch and value separated by an '='                                       */
/* A boolean switch without a value will set the boolean variable to true.                        */
/* There is one predefined switch -h which will print the list of user defined switches           */
/* with the provided descriptions.                                                                */
/* The integer and real values can be an equation that will be evaluated, and the result will     */
/* copied copied to the variable.  The equation is evaluated using standard order and precedence  */
/* rules. Operators include + - * / % and ^.  Parentheses can be used to change calculation order.*/
/* For the integer  type, the numbers can be entered as binary, octal, or hex by prefixing the    */
/* number with '0b', '0' or '0x' respectively. Otherwise, the number is interpreted as decimal.   */
/*                                                                                                */
/* Example:                                                                                       */
/*        :                                                                                       */
/*        :                                                                                       */
/* #include "ParseArgs.hpp"                                                                       */
/*                                                                                                */
/* int main(int argc, char* argv[]) {                                                             */
/*                                                                                                */
/*   //Define variables to be modified by the cmd line args                                       */
/*   bool aval = false;                                                                           */
/*   bool bval = true;                                                                            */
/*   int64_t ival = 0;                                                                            */
/*   double dval = 0.0;                                                                           */
/*   std::string_view sVal = "";                                                                  */
/*   std::string_view osVal = "";                                                                 */
/*   std::vector<bool> vbVals{ false, false };                                                    */
/*                                                                                                */
/*   // Create an instance of ParseArgs and add the desired cmd line args or switches             */
/*   // Optionally add a lambda to perform a check on the parsed value.                           */
/*   ParseArgs parseArgs;                                                                         */
/*   parseArgs.addHelpPretext("Example program to demonstrate ParseArgs usage.\n");               */
/*   parseArgs.addArg(0,    "ordered or non-switch arg", &osVal);                                 */
/*   parseArgs.addArg("-a", "Flag only arg", &aval);                                              */
/*   parseArgs.addArg("-b", "Boolean arg", &bval);                                                */
/*   parseArgs.addArg("-i", "Integer arg", &ival);                                                */
/*   parseArgs.addArg("-r", "Real arg", &dval);                                                   */
/*   parseArgs.addCheck([](double* const value) {                                                 */
/*     if (*value > 10.0 || *value < -10.0)                                                       */
/*       throw std::range_error("Value must be in the range of -10 to +10 inclusive");            */
/*     });                                                                                        */
/*   parseArgs.addArg("-s", "String arg", &sVal, true);                                           */
/*   parseArgs.addArg("-vb", "Vector of bools arg", &vbVals);                                     */
/*                                                                                                */
/*   // Call parse with argc and argv.  Exit if there was an error                                */
/*   if (!parseArgs.parse(argc, argv)) exit(0);                                                   */
/*                                                                                                */
/*   // print the current values of all the arguments                                             */
/*   std::cout << parseArgs.getValuesString();                                                    */
/*                                                                                                */
/*   // Since arg parsing is done, the data in parseArgs can optionally be deleted to save space. */
/*   parseArgs.clear();                                                                           */
/*   return 0;                                                                                    */
/* }                                                                                              */
/*                                                                                                */
/* The above example would parse the next 4 lines with without error.                             */
/* ordered -a -b=false -i 5 "-r 6.7 -s abc -vb true,true                                          */
/* ordered -a -b=false -i=5 "-r=6.7 -s=abc -vb=true,true                                          */
/* ordered -a -b=false -i=3+2 "-r=(10+3.4)/2 -s=abc -vb=true,true                                 */
/* ordered -a -b=false -i=0b101 "-r=(10+3.4)/2 -s=abc -vb=true,true                               */
/**************************************************************************************************/

class ParseEq {
public:
  /// @brief parseRealEquation parses an arithmetic string of operations of real numbers and returns a double value
  /// @param in std::string containing the equation
  /// @param strIdx a point in the string of where to start parsing.  Should start with 0.
  /// @param precedence the level of precedence of the current operator.  Must start with 0.
  /// @param parenCnt reference to a value of the current number of open parens.  The parenCnt should start=0.  Must also be 0 after the call or throw an error
  /// @return the value resulting from evaluating the equation.
  /// @throws std::invalid_argument other exceptions
  static double parseRealEquation(const std::string_view& in, size_t& strIdx, size_t precedence, int64_t& parenCnt) {
    double currentValue = 0;
    std::string digits;

    // skip over leading spaces
    while (strIdx < in.length() && in[strIdx] == ' ') {
      strIdx++;
    }
    // then check for open paren and push 
    if (strIdx < in.length() && in[strIdx] == '(') {
      strIdx++;
      parenCnt++;
      currentValue = handleParen(in, strIdx, parenCnt);
    }
    else { // else must be a number
      digits = in.substr(strIdx);
      size_t numDigits = 0;
      currentValue = std::stod(digits, &numDigits);
      strIdx += numDigits;
    }

    // next process an operator.
    while (strIdx < in.length()) {

      if (in[strIdx] == '+' || in[strIdx] == '-') {
        if (precedence >= 1) return currentValue;
        char op = in[strIdx++];
        double returnValue = parseRealEquation(in, strIdx, 1, parenCnt);
        if (op == '+') currentValue += returnValue;
        else currentValue -= returnValue;
        continue;
      }

      if (in[strIdx] == '*' || in[strIdx] == '/') {
        if (precedence >= 2) return currentValue;
        char op = in[strIdx++];
        double returnValue = parseRealEquation(in, strIdx, 2, parenCnt);
        if (op == '*') currentValue *= returnValue;
        else if (op == '/') currentValue /= returnValue;
        continue;
      }

      if (in[strIdx] == '^') {
        if (precedence >= 3) return currentValue;
        char op = in[strIdx++];
        double returnValue = parseRealEquation(in, strIdx, 2, parenCnt);
        if (op == '^') currentValue = std::pow(currentValue, returnValue);
        continue;
      }

      if (in[strIdx] == ')') {
        if (parenCnt <= 0)
          throw std::invalid_argument("mismatched parentheses");
        return currentValue;
      }

      if (in[strIdx] == ' ') {
        strIdx++; // skip over spaces
        continue;
      }

      throw std::invalid_argument("unexpected character");
      strIdx++;
    }
    return(currentValue);
  }

  /// @brief handleParen is a helper function for parseRealEquation
  /// @param in input string
  /// @param strIdx index into the string where the parser is working
  /// @param parenCnt reference to the variable holding the paren count
  /// @return result of a lower level call to parseRealEquation
  static double handleParen(const std::string_view& in, size_t& strIdx, int64_t& parenCnt) {
    double currentValue = parseRealEquation(in, strIdx, 0, parenCnt);
    if (strIdx < in.length() && in[strIdx] == ')') parenCnt--;
    strIdx++;
    return currentValue;
  }

  /// @brief parseIntEquation parses an arithmetic string of operations of integer numbers and returns a integer result value
  /// @param in std::string containing the equation
  /// @param strIdx a point in the string of where to start parsing.  Should start with 0.
  /// @param precedence the level of precedence of the current operator.  Must start with 0.
  /// @param parenCnt reference to a value of the current number of open parens.  The parenCnt should start=0.  Must also be 0 after the call or throw an error
  /// @return the value resulting from evaluating the equation.
  /// @throws std::invalid_argument other exceptions
  static int64_t parseIntEquation(const std::string_view& in, size_t& strIdx, size_t precedence, int64_t& parenCnt) {
    int64_t currentValue = 0;
    std::string digits;

    // skip over leading spaces
    while (strIdx < in.length() && in[strIdx] == ' ') {
      strIdx++;
    }
    // then check for open paren and push 
    if (strIdx < in.length() && in[strIdx] == '(') { 
      strIdx++;
      parenCnt++;
      currentValue = handleIntParen(in, strIdx, parenCnt);
    }
    else { // else must be a number
      int radix= 0;
      if (strIdx < in.length() && in[strIdx] == '0') { // is radix other than 10?
        strIdx++;
        if (strIdx < in.length() && (in[strIdx] == 'b' || in[strIdx] == 'B')) { //is radix binary?
          radix = 2;
          strIdx++;
        } else if (strIdx < in.length() && (in[strIdx] == 'x' || in[strIdx] == 'X')) { //is radix hex? 
          radix = 16;
          strIdx++;
        } else if (strIdx < in.length() && (isdigit(in[strIdx]) != 0)) { //is radix octal? 
          radix = 8;
        } else { // then must be decimal 0
          radix = 10;
          strIdx--;
        }
      }
      else { // then radix must be 10
        radix = 10;
      }
      digits = in.substr(strIdx);
      size_t numDigits = 0;
      currentValue = std::stoll(digits, &numDigits, radix);
      strIdx += numDigits;
    }

    // next process an operator.
    while (strIdx < in.length()) {

      if (in[strIdx] == '+' || in[strIdx] == '-') {
        if (precedence >= 1) return currentValue;
        char op = in[strIdx++];
        int64_t returnValue = parseIntEquation(in, strIdx, 1, parenCnt);
        if (op == '+') currentValue += returnValue;
        else currentValue -= returnValue;
        continue;
      }

      if (in[strIdx] == '*' || in[strIdx] == '/' || in[strIdx] == '%') {
        if (precedence >= 2) return currentValue;
        char op = in[strIdx++];
        int64_t returnValue = parseIntEquation(in, strIdx, 2, parenCnt);
        if (op == '*') currentValue *= returnValue;
        else if (op == '/') currentValue /= returnValue;
        else if (op == '%') currentValue %= returnValue;
        continue;
      }

      if (in[strIdx] == '^') {
        if (precedence >= 3) return currentValue;
        char op = in[strIdx++];
        int64_t returnValue = parseIntEquation(in, strIdx, 2, parenCnt);
        if (op == '^') currentValue = (int64_t)std::pow(currentValue, returnValue);
        continue;
      }

      if (in[strIdx] == ')') {
        if (parenCnt <= 0)
          throw std::invalid_argument("mismatched parentheses");
        return currentValue;
      }

      if (in[strIdx] == ' ') {
        strIdx++; // skip over spaces
        continue;
      }

      throw std::invalid_argument("unexpected character");
      strIdx++;
    }
    return(currentValue);
  }

  /// @brief handleParen is a helper function for parseIntEquation
  /// @param in input string
  /// @param strIdx index into the string where the r is working
  /// @param parenCnt reference to the variable holding the paren count
  /// @return result of a lower level call to parseIntEquation
  static int64_t handleIntParen(const std::string_view& in, size_t& strIdx, int64_t& parenCnt) {
    int64_t currentValue = parseIntEquation(in, strIdx, 0, parenCnt);
    if (strIdx < in.length() && in[strIdx] == ')') parenCnt--;
    strIdx++;
    return currentValue;
  }
};


class ParseArgs {

  /// @brief ArgBase is base struct for structs that hold information about the specific arguments 
  struct ArgBase {
    const char* argName;      // name of the argument or switch if switch type
    const int64_t argNum;     // number of arg if not switch type.
    const char* description;  // description of the argument or switch
    const bool required;      // boolean indicating whether this arg or switch is required
    bool found;               // Boolean indicating if this switch was found during parsing


    /// @brief constructor used for creating switched type arguments
    /// @param arg c-string of the switch
    /// @param desc c-string of the description of the argument
    /// @param req bool indicating whther the argument must be entered
    ArgBase(const char* arg, const char* desc, bool req) : 
            argName{ arg }, argNum (-1), description{ desc }, required{ req }, found{ false } {}
    
    /// @brief constructor used for creating switched type arguments
    /// @param argNum integer indication the place in the ordered arguments
    /// @param desc c-string of the description of the argument
    /// @param req bool indicating whther the argument must be entered
    ArgBase(const int64_t argNum, const char* desc, bool req) : 
            argName{ "" }, argNum (argNum), description{ desc }, required{ req }, found{ false } {}

    /// @brief pure virtual function that will parse the input for this specific type
    /// @param s input string
    virtual void  parse(std::string_view s) = 0;
 
    /// @brief pure virtual function that will convert the specific type to a string
    /// @return output string
    virtual std::string toString() = 0;
 
    /// @brief pure virtual function return the type
    /// @return text string of the type.
    virtual std::string type() = 0;
 
    /// @brief lambda used for the checking the variables.
    virtual ~ArgBase() {};
  };

  /// @brief ArgBool is a derivitive of ArgBase that holds information about specific single boolean arguments
  struct ArgBool : ArgBase {
    bool* const value;  // pointer to the value
    std::function<void(bool*)> check = nullptr; // pointer to a potential value check

    ArgBool(const char* argName, const char* desc, bool* valPtr, bool req) : 
            ArgBase(argName, desc, req), value{ valPtr } {} // constructor
    ArgBool(const int64_t argNum, const char* desc, bool* valPtr, bool req) : 
            ArgBase(argNum, desc, req), value{ valPtr } {} // constructor

    void parse(std::string_view s) override {  // parse for the specific type
      if (s == "")
        *value = true;
      else if (s == "true")
        *value = true;
      else if (s == "false")
        *value = false;
      else throw std::invalid_argument("Must be 'true' or 'false'");
      if (check != nullptr) check(value);
    }

    std::string toString() override { return *value ? "true" : "false"; }  // conversion to a string

    std::string type() override { return "boolean"; }  // string indication the type.
  };

  // @brief ArgInt is a derivitive of ArgBase that holds information about specific single integer arguments
  struct ArgInt : ArgBase {
    int64_t* const value;
    std::function<void(int64_t*)> check = nullptr;

    ArgInt(const char* argName, const char* desc, int64_t* const valPtr, bool req) : 
           ArgBase(argName, desc, req), value{ valPtr } { }
    ArgInt(const int64_t argNum, const char* desc, int64_t* const valPtr, bool req) : 
           ArgBase(argNum, desc, req), value{ valPtr } { }

    void parse(std::string_view s) override {
      if (s == "")
        throw std::invalid_argument("Must include an integer value");
      else {
        size_t strIdx = 0;
        int64_t parenCount = 0;
        *value = ParseEq::parseIntEquation(std::string(s), strIdx, 0, parenCount);
        if (parenCount != 0)
          throw std::invalid_argument("mismatched parentheses");
        else if (strIdx != s.length())
          throw std::invalid_argument("parse error");
        if (check != nullptr) check(value);
      }
    }

    std::string toString() override { return std::to_string(*value); }

    std::string type()  override { return "integer number"; }
  };

  // @brief ArgReal is a derivitive of ArgBase that holds information about specific single double arguments
  struct ArgReal : ArgBase {
    double* const value;
    std::function<void(double*)> check = nullptr;

    ArgReal(const char* argName, const char* desc, double* const valPtr, bool req) : 
            ArgBase(argName, desc, req), value{ valPtr } {}
    ArgReal(const int64_t argNum, const char* desc, double* const valPtr, bool req) : 
            ArgBase(argNum, desc, req), value{ valPtr } {}

    void parse(std::string_view s) override {
      if (s == "")
        throw std::invalid_argument(std::string("Must include a real number value"));
      else {
        size_t strIdx = 0;
        int64_t parenCount = 0;
        *value = ParseEq::parseRealEquation(s, strIdx, 0, parenCount);
        if (parenCount != 0)
          throw std::invalid_argument("mismatched parentheses");
        else if (strIdx != s.length())
          throw std::invalid_argument("parse error");
        if (check != nullptr) check(value);
      }
    }

    std::string toString() override { return std::to_string(*value); }

    std::string type() override { return "real number"; }
  };

  // @brief ArgText is a derivitive of ArgBase that holds information about specific single string_view arguments
  struct ArgText : ArgBase {
    std::string_view* const value;
    std::function<void(std::string_view*)> check = nullptr;

    ArgText(const char* argName, const char* desc, std::string_view* const valPtr, bool req) : 
            ArgBase(argName, desc, req), value{ valPtr } {}
    ArgText(const int64_t argNum, const char* desc, std::string_view* const valPtr, bool req) : 
            ArgBase(argNum, desc, req), value{ valPtr } {}

    void parse(std::string_view s) override {
      if (s == "")
        throw std::invalid_argument("Must include a text value");
      else
        *value = s;

      if (check != nullptr) check(value);
    }

    std::string toString() override { return '"' + std::string(*value) + '"'; }

    std::string type() override { return "text"; }
  };

public:

  /// @brief This addArg function creates boolean switch argument entry
  /// @param arg char* arg is the switch or argument name on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the boolean variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const char* arg, const char* desc, bool* const valPtr, bool req = false) {
    addArgT<ArgBool, bool>(arg, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to the last boolean created by addArg
  /// @param check lambda of type void(bool*)
  void addCheck(std::function<void(bool*)> check) {
    addCheckT<ArgBool, bool>(check);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined boolean switch arg.
  /// @param argName switch or arg name of a previously created switch arg.
  /// @param check lambda of type void(bool*)
  void addCheck(const char* argName, std::function<void(bool*)> check) {
    addCheckT<ArgBool, bool>(argName, check);
  }

  /// @brief This addArg function creates integer switch argument entry
  /// @param arg char* arg is the switch or argument name on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the int64_t variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const char* arg, const char* desc, int64_t* const valPtr, bool req = false) {
    addArgT<ArgInt, int64_t>(arg, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to the last integer created by addArg
  /// @param check lambda of type void(int64_t*)
  void addCheck(std::function<void(int64_t*)> check) {
    addCheckT<ArgInt, int64_t>(check);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined integer switch arg.
  /// @param argName switch or arg name of a previously created switch arg.
  /// @param check lambda of type void(int64_t*)
  void addCheck(const char* argName, std::function<void(int64_t*)> check) {
    addCheckT<ArgInt, int64_t>(argName, check);
  }

  /// @brief This addArg function creates real switch argument entry
  /// @param arg char* arg is the switch or argument name on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the double variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const char* arg, const char* desc, double* const valPtr, bool req = false) {
    addArgT<ArgReal, double>(arg, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to the last real created by addArg
  /// @param check lambda of type void(double*)
  void addCheck(std::function<void(double*)> check) {
    addCheckT<ArgReal, double>(check);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined real switch arg.
  /// @param argName switch or arg name of a previously created switch arg.
  /// @param check lambda of type void(double*)
  void addCheck(const char* argName, std::function<void(double*)> check) {
    addCheckT<ArgReal, double>(argName, check);
  }

  /// @brief This addArg function creates text switch argument entry
  /// @param arg char* arg is the switch or argument name on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the string_view variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const char* arg, const char* desc, std::string_view* const valPtr, bool req = false) {
    addArgT<ArgText, std::string_view>(arg, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to the last text created by addArg
  /// @param check lambda of type void(std::stringView*)
  void addCheck(std::function<void(std::string_view*)> check) {
    addCheckT<ArgText, std::string_view>(check);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined text switch arg.
  /// @param argName switch or arg name of a previously created switch arg.
  /// @param check lambda of type void(std::stringView*)
  void addCheck(const char* argName, std::function<void(std::string_view*)> check) {
    addCheckT<ArgText, std::string_view>(argName, check);
  }

  /// @brief This addArg function creates boolean ordered argument entry
  /// @param argNum index of the argument on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the bool variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const int argNum, const char* desc, bool* const valPtr, bool req = false) {
    addArgT<ArgBool, bool>(argNum, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined boolean ordered arg.
  /// @param argNum index of the ordered arg.
  /// @param check lambda of type void(bool*)
  void addCheck(const int argNum, std::function<void(bool*)> check) {
    addCheckT<ArgBool, bool>(argNum, check);
  }

   /// @brief This addArg function creates integer ordered argument entry
  /// @param argNum index of the argument on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the int54_t variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const int argNum, const char* desc, int64_t* const valPtr, bool req = false) {
    addArgT<ArgInt, int64_t>(argNum, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined integer ordered arg.
  /// @param argNum index of the ordered arg.
  /// @param check lambda of type void(int64_t*)
  void addCheck(const int argNum, std::function<void(int64_t*)> check) {
    addCheckT<ArgInt, int64_t>(argNum, check);
  }

  /// @brief This addArg function creates real ordered argument entry
  /// @param argNum index of the argument on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the double variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const int argNum, const char* desc, double* const valPtr, bool req = false) {
    addArgT<ArgReal, double>(argNum, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined real ordered arg.
  /// @param argNum index of the ordered arg.
  /// @param check lambda of type void(double*)
  void addCheck(const int argNum, std::function<void(double*)> check) {
    addCheckT<ArgReal, double>(argNum, check);
  }

   /// @brief This addArg creates text ordered argument entry
  /// @param argNum index of the argument on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the std::string_view variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const int argNum, const char* desc, std::string_view* const valPtr, bool req = false) {
    addArgT<ArgText, std::string_view>(argNum, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined text ordered arg.
  /// @param argNum index of the ordered arg.
  /// @param check lambda of type void(std::stringView*)
  void addCheck(const int argNum, std::function<void(std::string_view*)> check) {
    addCheckT<ArgText, std::string_view>(argNum, check);
  }

#ifndef REMOVE_VECTOR_ARGUMENT_CODE
private:

  /// @brief ArgBoolArray is a derivitive of ArgBase that holds information about specific array of boolean arguments
  struct ArgBoolArray : ArgBase {
    std::vector<bool>* const values;
    std::function<void(std::vector<bool>*)> check = nullptr; // pointer to a potential value check

    ArgBoolArray(const char* arg, const char* desc, std::vector<bool>* valPtr, bool req) : ArgBase(arg, desc, req), values{ valPtr } {}

    void parse(std::string_view s) override {
      if (s == "")
        throw std::invalid_argument("Must include a list of boolean values");
      else {
        auto pVals = split(s, ',');
        size_t len = values->size();
        if (pVals.size() != len)
          throw std::invalid_argument("Number of values doesn't match array size of " + std::to_string(len));
        for (size_t idx = 0; idx < len; idx++) {
          if (pVals[idx] == "true")
            values->at(idx) = true;
          else if (pVals[idx] == "false")
            values->at(idx) = false;
          else throw std::invalid_argument("Boolean values must be 'true' or 'false'");
        }
      }
      if (check != nullptr) check(values);
    }

    std::string toString() override {
      std::string tmp;
      for (size_t i = 0; i < values->size(); i++) {
        if (i != 0)  tmp += ',';
        tmp += values->at(i) ? "true" : "false";
      }
      return tmp;
    }

    std::string type() override { return "boolean array"; }
  };

  /// @brief ArgIntArray is a derivitive of ArgBase that holds information about specific array of integer arguments
  struct ArgIntArray : ArgBase {
    std::vector<int64_t>* const values;
    std::function<void(std::vector<int64_t>*)> check = nullptr; // pointer to a potential value check

    ArgIntArray(const char* arg, const char* desc, std::vector<int64_t>* valPtr, bool req) : ArgBase(arg, desc, req), values{ valPtr } {}

    void parse(std::string_view s) override {
      if (s == "")
        throw std::invalid_argument("Must include a list of integer values");
      else {
        auto pVals = split(s, ',');
        size_t len = values->size();
        if (pVals.size() != len)
          throw std::invalid_argument("Number of values doesn't match array size of " + std::to_string(len));
        for (size_t idx = 0; idx < len; idx++) {
          size_t last_c = 0;
          values->at(idx) = std::stoll(std::string(pVals[idx]), &last_c);
          if (last_c != pVals[idx].size())
            throw std::invalid_argument("Must include integer number values");
        }
      }
      if (check != nullptr) check(values);
    }

    std::string toString() override {
      std::string tmp;
      for (size_t i = 0; i < values->size(); i++) {
        if (i != 0)  tmp += ',';
        tmp += std::to_string(values->at(i));
      }
      return tmp;
    }

    std::string type() override { return "integer array"; }
  };

  /// @brief ArgRealArray is a derivitive of ArgBase that holds information about specific array of real (double) arguments
  struct ArgRealArray : ArgBase {
    std::vector<double>* const values;
    std::function<void(std::vector<double>*)> check = nullptr; // pointer to a potential value check

    ArgRealArray(const char* arg, const char* desc, std::vector<double>* valPtr, bool req) : ArgBase(arg, desc, req), values{ valPtr } {}

    void parse(std::string_view s) override {
      if (s == "")
        throw std::invalid_argument("Must include a list of real values");
      else {
        auto pVals = split(s, ',');
        size_t len = values->size();
        if (pVals.size() != len)
          throw std::invalid_argument("Number of values doesn't match array size of " + std::to_string(len));
        for (size_t idx = 0; idx < len; idx++) {
          size_t last_c = 0;
          values->at(idx) = std::stod(std::string(pVals[idx]), &last_c);
          if (last_c != pVals[idx].size())
            throw std::invalid_argument("Must include a real number values");
        }
      }
      if (check != nullptr) check(values);
    }

    std::string toString() override {
      std::string tmp;
      for (size_t i = 0; i < values->size(); i++) {
        if (i != 0)  tmp += ',';
        tmp += std::to_string(values->at(i));
      }
      return tmp;
    }

    std::string type() override { return "real array"; }
  };

  /// @brief ArgTextArray is a derivitive of ArgBase that holds information about specific array of text arguments
  struct ArgTextArray : ArgBase {
    std::vector<std::string_view>* const values;
    std::vector<std::string> localValues;
    std::function<void(std::vector<std::string_view>*)> check = nullptr; // pointer to a potential value check

    ArgTextArray(const char* arg, const char* desc, std::vector<std::string_view>* valPtr, bool req) : ArgBase(arg, desc, req), values{ valPtr } {}

    void parse(std::string_view s) override {
      if (s == "")
        throw std::invalid_argument("Must include a list of text values");
      else {
        auto pVals = split(s, ',');
        size_t len = values->size();
        if (pVals.size() != len)
          throw std::invalid_argument("Number of values doesn't match array size of " + std::to_string(len));
        for (size_t idx = 0; idx < len; idx++) {
          values->at(idx) = pVals[idx];
        }
      }
      if (check != nullptr) check(values);
    }

    std::string toString() override {
      std::string tmp;
      for (size_t i = 0; i < values->size(); i++) {
        if (i != 0)  tmp += ',';
        tmp += '"' + std::string(values->at(i)) + '"';
      }
      return tmp;
    }

    std::string type() override { return "text array"; }
  };

public:
  /// @brief This addArg function creates boolean array switch argument entry
  /// @param arg char* arg is the switch or argument name on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the std::vector<bool> variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const char* arg, const char* desc, std::vector<bool>* const valPtr, bool req = false) {
    addArgT<ArgBoolArray, std::vector<bool>>(arg, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to the last boolean array created by addArg
  /// @param check lambda of type void(std::vector<bool>*)
  void addCheck(std::function<void(std::vector<bool>*)> check) {
    addCheckT<ArgBoolArray, std::vector<bool>>(check);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined boolean  array switch arg.
  /// @param argName switch or arg name of a previously created switch arg.
  /// @param check lambda of type void(std::vector<bool>*)
  void addCheck(const char* argName, std::function<void(std::vector<bool>*)> check) {
    addCheckT<ArgBoolArray, std::vector<bool>>(argName, check);
  }

  /// @brief This addArg function creates integer array switch argument entry
  /// @param arg char* arg is the switch or argument name on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the std::vector<int64_t> variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const char* arg, const char* desc, std::vector<int64_t>* const valPtr, bool req = false) {
    addArgT<ArgIntArray, std::vector<int64_t>>(arg, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to the last integer array created by addArg
  /// @param check lambda of type void(std::vector<int64_t>*)
  void addCheck(std::function<void(std::vector<int64_t>*)> check) {
    addCheckT<ArgIntArray, std::vector<int64_t>>(check);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined integer array switch arg.
  /// @param argName switch or arg name of a previously created switch arg.
  /// @param check lambda of type void(std::vector<int64_t>*)
  void addCheck(const char* argName, std::function<void(std::vector<int64_t>*)> check) {
    addCheckT<ArgIntArray, std::vector<int64_t>>(argName, check);
  }

  /// @brief This addArg function creates real array switch argument entry
  /// @param arg char* arg is the switch or argument name on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the std::vector<double> variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
  void addArg(const char* arg, const char* desc, std::vector<double>* const valPtr, bool req = false) {
    addArgT<ArgRealArray, std::vector<double>>(arg, desc, valPtr, req);
  }
 /// @brief This addCheck function adds a "check" lambda to the last real array created by addArg
  /// @param check lambda of type void(std::vector<double>*)
   void addCheck(std::function<void(std::vector<double>*)> check) {
    addCheckT<ArgRealArray, std::vector<double>>(check);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined real array switch arg.
  /// @param argName switch or arg name of a previously created switch arg.
  /// @param check lambda of type void(std::vector<double>*)
  void addCheck(const char* argName, std::function<void(std::vector<double>*)> check) {
    addCheckT<ArgRealArray, std::vector<double>>(argName, check);
  }

   /// @brief This addArg function creates a text array switch argument entry
  /// @param arg char* arg is the switch or argument name on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the std::vector<std::string_view> variable that will be modified by the switch
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false
 void addArg(const char* arg, const char* desc, std::vector<std::string_view>* const valPtr, bool req = false) {
    addArgT<ArgTextArray, std::vector<std::string_view>>(arg, desc, valPtr, req);
  }
  /// @brief This addCheck function adds a "check" lambda to the last text array created by addArg
  /// @param check lambda of type void(std::vector<std::string_view>*)
  void addCheck(std::function<void(std::vector<std::string_view>*)> check) {
    addCheckT<ArgTextArray, std::vector<std::string_view>>(check);
  }
  /// @brief This addCheck function adds a "check" lambda to a previously defined text array switch arg.
  /// @param argName switch or arg name of a previously created switch arg.
  /// @param check lambda of type void(std::vector<std::string_view>*)
  void addCheck(const char* argName, std::function<void(std::vector<std::string_view>*)> check) {
    addCheckT<ArgTextArray, std::vector<std::string_view>>(argName, check);
  }

#endif // REMOVE_VECTOR_ARGUMENT_CODE

private:

  /// @brief split is a static utility  function that splits a string with separators into separate strings
  /// @param in tring in is the input string ti be slit
  /// @param sep char sep is the separator character about which the string will be separated
  /// @return a vector of individual separated strings
  static std::vector<std::string_view> split(std::string_view in, char sep) {
    std::vector<std::string_view> out;
    std::string_view word;
    size_t start_idx = 0;
    while (true) {
      size_t end_idx = in.find(sep, start_idx);
      word = in.substr(start_idx, end_idx - start_idx);
      out.push_back(word);
      if (end_idx == std::string::npos) break;
      start_idx = end_idx + 1;
    }
    return out;
  }

  /// @brief This addArgT template function builds a ArgValue_t object, and adds it to argMap
  /// @tparam ArgBase_t one of the classes derived from argBase
  /// @tparam ArgValue_t type of object that valPtr will point to.
  /// @param arg char* arg is the switch or argument name on the command line.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the variable that will be modified by the switch
  /// @param req req is a flag which indicates these argument or switch is required. Default is false
  template<class ArgBase_t, class ArgValue_t>
  void addArgT(const char* arg, const char* desc, ArgValue_t* const valPtr, bool req) {
    auto* tempArgBase = new ArgBase_t(arg, desc, valPtr, req);
    std::pair< std::string, ArgBase* > argEntry{ arg, (ArgBase*)tempArgBase };
    argsMap.push_back(argEntry);
    lastMapEntry = tempArgBase;
  }

  /// @brief This addCheck template function adds a "check" lambda to the last one defined by addArg
  /// @tparam ArgBase_t one of the classes derived from argBase
  /// @tparam ArgValue_t type of object that the paramater of the lambda points too.
  /// @param check lambda of type void(ArgValue_t*)
  template<class ArgBase_t, class ArgValue_t>
  void addCheckT(std::function<void(ArgValue_t*)> check) {
    if (typeid(ArgBase_t) == typeid(*lastMapEntry)) {
      static_cast<ArgBase_t*>(lastMapEntry)->check = check;
    }
    else {
      if (lastMapEntry->argNum == -1)
        std::cerr << "addCheck: check is the wrong type for switch " << lastMapEntry->argName << std::endl;
      else
        std::cerr << "addCheck: check is the wrong type for parameter " << lastMapEntry->argNum << std::endl;
    }
  }
 
  /// @brief This addCheck template function adds a "check" lambda to a previously defined switch arg.
  /// @tparam ArgBase_t one of the classes derived from argBase
  /// @tparam ArgValue_t type of object that the paramater of the lambda points too.
  /// @param argName switch or arg name of a previously defined arg.
  /// @param check lambda of type void(ArgValue_t*)
  template<class ArgBase_t, class ArgValue_t>
  void addCheckT(const char* argName, std::function<void(ArgValue_t*)> check) {
    std::string argString = std::string(argName);
    auto match = find(argsMap,argString);
    ArgBase* ptr = match->second;
    if (match != argsMap.end()) {
      // parse the value for this arg and the mark it found
      if (typeid(ArgBase_t) == typeid(*ptr)) {
        static_cast<ArgBase_t*>(match->second)->check = check;
      } else {
        std::cerr << "addCheck: check is the wrong type for " << argName << ".\n";
      }
    } else {
      // print error if arg name is not in the argsMap.
      std::cerr << "addCheck: Unrecognized argument name " << argName << ".\n";
    }
  }

  /// @brief This addArgT template function builds a ArgValue_t object, and adds it to nsArgMap
  /// @tparam ArgBase_t one of the classes derived from argBase
  /// @tparam ArgValue_t type of object that valPtr will point to.
  /// @param arg int is the index of ordered argument.
  /// @param desc char* desc is the description that will be printed with the help message
  /// @param valPtr pointer to the variable that will be modified by the switch
  /// @param req req is a flag which indicates these argument or switch is required. Default is false
  template<class ArgBase_t, class ArgValue_t>
  void addArgT(const int argNum, const char* desc, ArgValue_t* const valPtr, bool req) {
    if (argNum < 0) {
      std::cout << "addArg: error in specification of argument number\n";
      return;
    }
    if (static_cast<size_t>(argNum) >= nsArgsMap.size()) nsArgsMap.resize(argNum+1);
    auto* tempArgBase = new ArgBase_t(argNum, desc, valPtr, req);
    nsArgsMap[argNum] = tempArgBase;
    lastMapEntry = tempArgBase;
  }

  /// @brief This addCheck template function adds a "check" lambda to a previously defined ordered arg.
  /// @tparam ArgBase_t one of the classes derived from argBase
  /// @tparam ArgValue_t type of object that the paramater of the lambda points too.
  /// @param argNum integer index of a previously defined ordered arg.
  /// @param check lambda of type void(ArgValue_t*)
  template<class ArgBase_t, class ArgValue_t>
  void addCheckT(const int argNum, std::function<void(ArgValue_t*)> check) {
    if (argNum < 0 || static_cast<size_t>(argNum) >= nsArgsMap.size()) {
      std::cerr << "AddCheck: Argument number " << argNum << "is out of range\n";
      return;
    }
    ArgBase* ptr = nsArgsMap[argNum];
    if (typeid(ArgBase_t) == typeid(*ptr)) {
        static_cast<ArgBase_t*>(ptr)->check = check;
    } else {
      std::cerr << "addCheck: check is the wrong type for " << argNum << ".\n";
    }
  }

  // argsMap hold and provides the lookup of user defined arguments.
  // A vector is used here instead of a map to maintain the order the arguments were added.

  /// @brief argsMap hold defined switch arguments.
  std::vector<std::pair<std::string, ArgBase*>> argsMap;

  /// @brief find searches the a vector of argBase pointer for the argName
  /// @param argsMap std::vector argMap
  /// @param argName std::string of the switch are got search for.
  /// @return iterator pointing to the found item in the vector or end() if not found.
  std::vector<std::pair<std::string, ArgBase*>>::iterator find(std::vector<std::pair<std::string, ArgBase*>>& argsMap, std::string& argName) {
    for (std::vector<std::pair<std::string, ArgBase*>>::iterator it = argsMap.begin(); it != argsMap.end(); ++it) {
      if (it->first == std::string(argName)) return it;
    }
    return argsMap.end(); 
  }
  /// @brief find searches the a vector of argBase pointer for the argName
  /// @param argsMap std::vector argMap
  /// @param argName std::string_view of the switch are got search for.
  /// @return iterator pointing to the found item in the vector or end() if not found.
  std::vector<std::pair<std::string, ArgBase*>>::iterator find(std::vector<std::pair<std::string, ArgBase*>>& argsMap, std::string_view& argName) {
    for (std::vector<std::pair<std::string, ArgBase*>>::iterator it = argsMap.begin(); it != argsMap.end(); ++it) {
      if (it->first == std::string(argName)) return it;
    }
    return argsMap.end(); 
  }

  /// @brief nsArgsMap holds defined ordered argument
  std::vector<ArgBase*> nsArgsMap;

  /// @brief this holds a pointer to the last defined Argment object for use by functions that add additional info the Arg* objects 
  ArgBase* lastMapEntry = nullptr;

  /// @brief olds the string printed at the start of the help message
  std::string_view printHelpPretext = "";

  /// @breif switch that prints the help message
  std::string_view helpSwitch = "-h";

#ifdef ENABLE_TEST_ERROR_ACCESS
  /// @brief this string hold any error messages created during a pars operation.  Only used in a test  
  std::string error_message;
#endif // ENABLE_TEST_ERROR_ACCESS

public:

  /// @brief The parse member function parses the standard C command line arguments and
  ///        modifies the variables specified in the calls to addArg
  /// @param argc int argc is the number of args.  Sane as main
  /// @param argv char* argv[] array of pointers to char strings holding the actual arguments.  Typically directly from main(argc, argv)
  /// @param start optional int indicating the starting argv.  Default is 1 or the first arg after the program name.
  /// @return false if parse errors, otherwise true
  bool parse(const int argc, const char* argv[], int start = 1) {
    std::string_view argName;
    std::string_view argValue;
    int64_t orderedArgNum = 0;
#ifdef ENABLE_TEST_ERROR_ACCESS
    error_message.clear();
#endif // eNABLE_TEST_ERROR_ACCESS

    // loop through the strings in argv
    for (int i = start; i < argc; i++) {
      std::string_view s(argv[i]);
      if (s == "") continue; // skip if an empty argument
      if (s == helpSwitch) {
        printHelp();
        return false;
      }
      try {
        // parse out the argument name and the value text
        size_t eqi = s.find('=');
        if (eqi != std::string::npos) { // first check for the equals format and split the argument
          argName = s.substr(0, eqi);
          argValue = s.substr(eqi + 1);
          // look up the  argument name or switch in the argsMap
          auto match = find(argsMap,argName);
          if (match != argsMap.end()) {
            // parse the value for this arg and the mark it found
            match->second->parse(argValue);
            match->second->found = true;
          }  else {// throw an error if arg name is not in the argsMap.
            throw std::invalid_argument( std::string(argName) + " is not a recognized argument");
          }
        } else { // if not the equals case, see if the next argv is a valid switch and process it.
          argName = s;
          auto match = find(argsMap, s);
          if (match != argsMap.end()) { // if is a valid switch, process it
	          ArgBase& ArgBaseObj = *(match->second);
            if (typeid(ArgBaseObj) == typeid(ArgBool)) { // see if it is a bool type and handle the non-value case
              if (i+1 >= argc) {// first check that there is a next argv
                argValue = "";
              } else {  // if so, then check to see if it is a valid bool argument;
                std::string_view sp1(argv[i + 1]);
                if ((sp1 == "true" || sp1 == "false") ) {  // arg values for the bool case can only be true or false
                  argValue = sp1;
                  i = i + 1;
                } else { // else it's an empty argument
                  argValue = "";
                }
              }
            } else {  // if not a bool type get the next item from argv for the value
              if ( i+1 >= argc) {
                argValue = "";
              } else { 
                argValue = std::string_view(argv[i + 1]);
                i=i+1;
                }
            }
            // process the arguments.
            match->second->parse(argValue);
            match->second->found = true;
          } else { // see if it could be an ordered argument
            if (s[0] != '-' && s[0] != '%' && static_cast<size_t>(orderedArgNum) < nsArgsMap.size()) {
              nsArgsMap[orderedArgNum]->parse(s);
              orderedArgNum++;
            } else {
              throw std::invalid_argument( std::string(s) + " is not a recognized argument");
            }
          } // end of ordered argument
        } // end of equals / non-equals arg types
      } // end of try block
      catch (const std::exception& e) {
        // catch any errors found in parsing.
#ifdef ENABLE_TEST_ERROR_ACCESS
          error_message = "Command line argument error on " + std::string(s) + ".\n";
#else
        std::cerr << "Command line argument error on " << s << ".\n";
#endif // ENABLE_TEST_ERROR_ACCESS
        std::string newWhat = e.what();
        if (newWhat.find("stoll") != std::string::npos || newWhat.find("stod") != std::string::npos)
          newWhat = "expected a value or open parenthesis";
#ifdef ENABLE_TEST_ERROR_ACCESS
        error_message += newWhat + ".\n";
#else
        std::cerr << newWhat << std::endl;
        printHelp(false);
#endif // ENABLE_TEST_ERROR_ACCESS
        return false;
      }
    }
    // after parsing is done check to see the the required argument were present.
    for (auto& arg : argsMap) {
      if (arg.second->required && !arg.second->found) {
#ifdef ENABLE_TEST_ERROR_ACCESS
        error_message = "Required command line argument " + arg.first + " was not present.\n" + ".\n";
#else
        std::cout << "Required command line argument " << arg.first << " was not present.\n";
        printHelp(false);
#endif // ENABLE_TEST_ERROR_ACCESS
        return false;
      }
    }
    return true;
  }

  void setHelpSwitch(std::string_view helpString) {
      helpSwitch = helpString;
  }

  /// @brief addHelpPretext allows a program description text to be added at the top of the help message
  /// @param pt description string.
  void addHelpPretext(std::string_view pt) {
    printHelpPretext = pt;
  }

  /** The printHelp member function prints the list of user defined arguments with type and descriptions.
  *** It is called by parse() when -h is found in the arguments or when a parse error is identified.
  *** bool printDefaults : if true prints the default value for each of the args.**/

  /// @brief The printHelp member function prints the list of user defined arguments with type and descriptions.
  ///        It is called by parse() when helpSwitchF is found in the arguments or when a parse error is identified.
  /// @param printDefaults bool specifying whether to include the default values of the variables.  default is true.
  void printHelp(bool printDefaults = true) {
    // set up a lambda to add text strings to a specific width
    auto pw = [](std::string in, size_t w) -> std::string {
      std::string out = in;
      while (out.length() < w) out += ' ';
      return out;
      };
    int aTypeWidth = 15;
    // scan for maximum argument name (switch) width
    size_t aNameWidth = 3;
    if (helpSwitch.size() > aNameWidth)  aNameWidth = helpSwitch.size();
    for (auto& arg : argsMap) {
      if (arg.first.size() > aNameWidth)  aNameWidth = arg.first.size();
    }
    // start by printing the pretext if available
    if (printHelpPretext != "")
      std::cout << printHelpPretext << std::endl;
    // now print the table of defined args, header first
    if (nsArgsMap.size() > 0) {
      std::string orderedArgs = "<program> ";
      for (auto oArg : nsArgsMap) {
        orderedArgs += "<" + std::string(oArg->description) + "> ";
      }
      if (argsMap.size() > 0) orderedArgs += "[options]";
      std::cout << orderedArgs << "\n";
    }
    if (argsMap.size() > 0) {
      //std::cout << format("{:{}} : {:{}} {}\n", "arg", aNameWidth, "value type", aTypeWidth, "Description");
      std::cout << pw("arg", aNameWidth) << " : " << pw("value type", aTypeWidth) << " " << "Description" << std::endl;
      std::cout << "----------------------------------------------------------------------------------------\n";
      //std::cout << format("{:{}} : {:{}} {}\n", "-h", aNameWidth, " ", aTypeWidth, "Prints this help message");
      std::cout << pw(std::string(helpSwitch), aNameWidth) << " : " << pw(" ", aTypeWidth) << " " << "Prints this help message" << std::endl;
      // then each defined arg
      for (auto& arg : argsMap) {
        //std::cout << format("{:{}} : {:{}} {}\n", arg.second->argName, aNameWidth, arg.second->type(), aTypeWidth, arg.second->description);
        std::cout << pw(arg.second->argName, aNameWidth) << " : " << pw(arg.second->type(), aTypeWidth) << " " << arg.second->description << std::endl;
        //if (printDefaults) std::cout << format("{:{}}    default = {}\n", "", aNameWidth + aTypeWidth, arg.second->toString());
        if (printDefaults){
          if (arg.second->required) 
            std::cout << pw("", aNameWidth + aTypeWidth) << "    required argument\n";
          else        
            std::cout << pw("", aNameWidth + aTypeWidth) << "    default = " << arg.second->toString() << std::endl;
        }
      }
      std::cout << "Optional argument format: switch value or switch=value\n";
    }
  }

  /// @brief  getValuesString returns the command line string for all the args with their current settings.
  /// @return The resulting string
  std::string getValuesString() {
    std::string out;
    for (auto& oArg : nsArgsMap){
      out += oArg->toString() + " ";
    }
    for (auto& arg : argsMap) {
      out += std::string(arg.second->argName) + "=" + arg.second->toString() + " ";
    }
    return out;
  }


  /// @brief clear member function deletes all the data stored in the args maps.
  void clear() {
    // first delete all the arg* objects.
    for (auto& arg : argsMap) {
      delete arg.second;
    }
    argsMap.clear();
    for (auto& oArg : nsArgsMap){
      delete oArg;
    }
    nsArgsMap.clear();
  }

  // destructor
  ~ParseArgs() {
    clear();
  }

#ifdef ENABLE_TEST_ERROR_ACCESS
  /// @brief get any error messages produced by the pars function.  Only available when in test mode.
  /// @return std::string of the error message
  std::string getErrorMessage() {return error_message;} 
#endif // ENABLE_TEST_ERROR_ACCESS

};
