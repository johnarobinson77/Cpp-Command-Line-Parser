// CmdLineArgs.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define ENABLE_TEST_ERROR_ACCESS

#include <iostream>
#include <format>
#include <cstring>
#include "ParseArgs.hpp"

struct ArgTest {
  static const int argc = 6;
  std::string description;
  std::string expected;
  const char* argv[argc];
  ArgTest( const char* desc, const char* exp, const char* arg0, const char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5 ) {
    description = desc;
    expected = exp;
    argv[0] = arg0;
    argv[1] = arg1;
    argv[2] = arg2;
    argv[3] = arg3;
    argv[4] = arg4;
    argv[5] = arg5;
  }
};

struct OrderedArgTest {
  static const int argc = 6;
  std::string_view description;
  std::string_view expected;
  const char* argv[argc];
  OrderedArgTest( const char* desc, const char* exp, const char* arg0, const char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5 ) {
    description = desc;
    expected = exp;
    argv[0] = arg0;
    argv[1] = arg1;
    argv[2] = arg2;
    argv[3] = arg3;
    argv[4] = arg4;
    argv[5] = arg5;
  }
};

struct IntEqParseTest {
  const char* eq;
  int64_t ex;
  std::string_view erm;
  IntEqParseTest (const char* eqIn, const int64_t exIn) {
    eq = eqIn;
    ex = exIn;
    erm = "should not have errored";
  }
  IntEqParseTest (const char* eqIn, const char* ermIn) {
    eq = eqIn;
    ex = std::numeric_limits<int64_t>::min();
    erm = ermIn;
  }
};

struct DblEqParseTest {
  const char* eq;
  double ex;
  std::string_view erm;
  DblEqParseTest (const char* eqIn, const double exIn) {
    eq = eqIn;
    ex = exIn;
    erm = "could not have errored";
  }
  DblEqParseTest (const char* eqIn, const char* ermIn) {
    eq = eqIn;
    ex = std::numeric_limits<double>::min();
    erm = ermIn;
  }
};


int main() {
  bool doSwitchedArgTest = true;
  bool doOrderedArgTest = true;
  bool doEquationParsingTests = true;

  int64_t numFailures = 0;

  if (doSwitchedArgTest) {
    std::vector<ArgTest> argTests;
    
    argTests.emplace_back("This one should just print the help message", 
      "", 
      "cmdLineArgs", "-h", "", "", "", "");
    argTests.emplace_back( "This test should report missing required -t",
      "Required command line argument -t was not present.\n.\n", 
      "cmdLineArgs", "-a", "-a", "-b=false", "-i=5", "-r=6.7" );
    argTests.emplace_back("This test should report pass",
      "-a=true -b=false -i=5 -r=6.700000 -t=\"abc\" -vb=false,false -vi=-1,-1,-1 -vr=-1.000000,-1.000000,-1.000000,-1.000000 -vt=\"\",\"\",\"\",\"\",\"\" ",
      "cmdLineArgs", "-a", "-b=false", "-i=2+3", "-r=6+0.7", "-t=abc" );

    argTests.emplace_back("This test should report incorrect spelling of false",
      "Command line argument error on -b=flase.\nMust be 'true' or 'false'.\n", 
      "cmdLineArgs", "-a", "-b=flase", "-i=5", "-r=6.7", "-t=abc" );
    argTests.emplace_back("This test should report incorrect integer value on -i", 
      "Command line argument error on -i=z.\nexpected a value or open parenthesis.\n",
      "cmdLineArgs", "-a", "-b=false", "-i=z", "-r=6.7", "-t=abc" );
    argTests.emplace_back("This test should report incorrect real value on -r", 
      "Command line argument error on -r=z6.7.\nexpected a value or open parenthesis.\n", 
      "cmdLineArgs", "-a", "-b=false", "-i=5", "-r=z6.7", "-t=abc" );

    argTests.emplace_back("This test should report non-existant text on -t",
      "Command line argument error on -t.\nMust include a text value.\n", 
      "cmdLineArgs", "-a", "-b=false", "-i=5", "-r=6.7", "-t" );
    argTests.emplace_back("This test should report positive check on -i, confirming that check lambdas work",
      "Command line argument error on -i=-5.\nValue must be positive.\n", 
      "cmdLineArgs", "-a", "-b=false", "-i=-5", "-r=6.7", "-t=abc" );
    argTests.emplace_back("This test should report range check on -r, confirming that check lambdas work",
      "Command line argument error on -r=16.7.\nValue must be in the range of -10 to +10 inclusive.\n",
      "cmdLineArgs", "-a", "-b=false", "-i=5", "-r=16.7", "-t=abc" );

    argTests.emplace_back("This test should report passing vector cases",
      "-a=false -b=true -i=0 -r=0.000000 -t=\"test\" -vb=true,true -vi=5,6,7 -vr=6.700000,8.900000,10.110000,12.130000 -vt=\"one\",\"two\",\"three\",\"four\",\"five\" ",
      "cmdLineArgs", "-t=test", "-vb=true,true", "-vi=5,6,7", "-vr=6.7,8.9,10.11,12.13", "-vt=one,two,three,four,five" );
    argTests.emplace_back("This test should reportnon matching numbers of bool values",
      "Command line argument error on -vb=true.\nNumber of values doesn't match array size of 2.\n",
      "cmdLineArgs", "-t=test", "-vb=true", "-vi=5,6,7", "-vr=6.7,8.9,10.11,12.13", "-vt=one,two,three,four,five" );
    argTests.emplace_back("This one fails for non matching numbers of real values",
      "Command line argument error on -vr=6.7,8.9,10.11,12.13,14.15.\nNumber of values doesn't match array size of 4.\n",
      "cmdLineArgs", "-t=test", "-vb=true,true", "-vi=5,6,7", "-vr=6.7,8.9,10.11,12.13,14.15", "-vt = one,two,three,four,five" );

    argTests.emplace_back("This test should report failures on space separate arg names/values.",
      "-a=true -b=true -i=6 -r=0.000000 -t=\"test12\" -vb=false,false -vi=-1,-1,-1 -vr=-1.000000,-1.000000,-1.000000,-1.000000 -vt=\"\",\"\",\"\",\"\",\"\" ",
      "cmdLineArgs", "-t", "test12", "-i", "6", "-a" );
    argTests.emplace_back("This test should report failures on space separate arg names/values.",
      "-a=true -b=true -i=7 -r=0.000000 -t=\"test13\" -vb=false,false -vi=-1,-1,-1 -vr=-1.000000,-1.000000,-1.000000,-1.000000 -vt=\"\",\"\",\"\",\"\",\"\" ",
      "cmdLineArgs", "-a", "-t", "test13", "-i", "7" );
    argTests.emplace_back("This test should report failures on space separate arg names/values.",
      "-a=true -b=true -i=8 -r=1.200000 -t=\"test14\" -vb=false,false -vi=-1,-1,-1 -vr=-1.000000,-1.000000,-1.000000,-1.000000 -vt=\"\",\"\",\"\",\"\",\"\" ",
      "cmdLineArgs", "-a", "-t=test14", "-i=8", "-r", "1.2" );

    argTests.emplace_back("This test should report missing arg on -r",
      "Command line argument error on -r.\nMust include a real number value.\n",
      "cmdLineArgs", "-t", "test12", "-i", "6", "-r" );
    argTests.emplace_back("This test should report missing arg on -r",
      "Command line argument error on -r.\nexpected a value or open parenthesis.\n",
      "cmdLineArgs", "-r", "-t", "test13", "-i", "7" );
    argTests.emplace_back("This test should report missing arg on -r",
      "Command line argument error on -r.\nexpected a value or open parenthesis.\n",
      "cmdLineArgs", "-r", "-k=test14", "-i=8", "-t", "one.two" );

    //Define variables to be modified by the cmd line args
    // scalers
    bool aval = false;
    bool bval = true;
    int64_t ival = 0;
    double dval = 0.0;
    std::string_view tVal = "";
    // vectors
    std::vector<bool> vbVals{ false, false };
    std::vector<int64_t> viVals{ -1, -1, -1 };
    std::vector<double> vrVals{ -1.0, -1.0, -1.0, -1.0 };
    std::vector<std::string_view> vtVals{ "","","","","" };

    // Create an instance of ParseArgs and add the desired cmd line args or switches
    // Optionally add a lambda to performs a check on the parsed value
    ParseArgs parseArgs;
    parseArgs.addArg("-a", "Flag only arg", &aval);
    parseArgs.addArg("-b", "Boolean arg", &bval);
    parseArgs.addArg("-i", "Integer arg", &ival);
    parseArgs.addArg("-r", "Real arg", &dval);
    parseArgs.addCheck([](double* value) {
      if (*value > 10.0 || *value < -10.0) throw std::range_error("Value must be in the range of -10 to +10 inclusive");
      });
    parseArgs.addArg("-t", "Text arg", &tVal, true);
    parseArgs.addArg("-vb", "Vector of bools arg", &vbVals);
    parseArgs.addArg("-vi", "Vector of integers arg", &viVals);
    parseArgs.addArg("-vr", "Vector of reals arg", &vrVals);
    parseArgs.addArg("-vt", "Vector of text arg", &vtVals);

    // check lambdas can also be added later by specifying the arg or switch name.
    auto positiveIntCheck = [](int64_t* value) {
      if (*value < 0) {
        throw std::invalid_argument("Value must be positive");
      } };
    parseArgs.addCheck("-i", positiveIntCheck);

    parseArgs.addHelpPretext("CmdLineArgs [args] is a test of ParsArgs");

    std::cout << std::format("\nStart of {} command line arg tests\n", argTests.size());

    numFailures = 0;
    for (size_t it = 0; it < argTests.size(); it++) {
      // Call parse with argc and argv.  Exit if there was an error
      if (parseArgs.parse(ArgTest::argc, argTests[it].argv)) {
        // check the current values of all the arguments
        if (std::string(argTests[it].expected) != parseArgs.getValuesString()) {
          std::cout << std::format("Test {} {} failed\n Got {}\n Expected: {}\n", it, argTests[it].description, 
                                   std::string(argTests[it].expected), parseArgs.getValuesString());
          numFailures++;
        }
      } else {
        if (std::string(argTests[it].expected) != parseArgs.getErrorMessage()) {
          std::cout << std::format("Test {} {} failed\n Got {}\n Expected: {}\n", it, argTests[it].description, 
                                  std::string(argTests[it].expected) , parseArgs.getErrorMessage());
          numFailures++;
        }
      }
      // restore the defaults
      aval = false;
      bval = true;
      ival = 0;
      dval = 0.0;
      tVal = "";
      vbVals = { false, false };
      viVals = { -1, -1, -1 };
      vrVals = { -1.0, -1.0, -1.0, -1.0 };
      vtVals = { "","","","","" };
    }
    // Since arg parsing is done, the data in parseArgs can optionally be deleted to save space.
    parseArgs.clear();
    std::cout << std::format("{} failures found\n", numFailures);
  }

  if (doOrderedArgTest) {
    
    std::vector<OrderedArgTest> orderedArgTests;
    orderedArgTests.emplace_back("This one should just print the help message", 
      "", 
      "cmdLineArgs", "--help", "", "", "", "");
    orderedArgTests.emplace_back("This test should report passing",
      "1 2.200000 \"oArg3\" -b=false -i=4 -r=0.000000 -t=\"\" ",
      "cmdLineArgs", "1", "2.2", "oArg3", "-i=4", "");
    orderedArgTests.emplace_back("This test should report passing",
      "1 2.200000 \"oArg3\" -b=false -i=0 -r=4.000000 -t=\"\" ",
      "cmdLineArgs", "1", "2.2", "-r=4.0", "oArg3", "");
    orderedArgTests.emplace_back("This test should report passing",
      "1 2.200000 \"oArg3\" -b=true -i=0 -r=0.000000 -t=\"\" ",
      "cmdLineArgs", "1", "-b", "2.2", "oArg3", "");
    orderedArgTests.emplace_back("This test should report passing",
      "1 2.200000 \"oArg3\" -b=false -i=0 -r=0.000000 -t=\"switched Text\" ",
      "cmdLineArgs", "-t=switched Text", "1", "2.2", "oArg3", "");
    orderedArgTests.emplace_back("This test should report passing",
      "1 2.200000 \"oArg3\" -b=false -i=0 -r=0.000000 -t=\"switched Text\" ",
      "cmdLineArgs", "-t", "switched Text", "1", "2.2", "oArg3");
    orderedArgTests.emplace_back("This test should report failing.  -t is missing an argument",
      "Command line argument error on 2.2.\nunexpected character.\n",
      "cmdLineArgs", "-t", "1", "2.2", "oArg3", "");

    // Create an instance of ParseArgs and add the desired cmd line args or switches
    // Optionally add a lambda to performs a check on the parsed value
    bool bval = false;
    int64_t ival = 0;
    double dval = 0.0;
    std::string_view tVal = "";
    int64_t oiVal = 0;
    double orVal = 0.0;
    std::string_view otVal = "";
    ParseArgs parseOrderedArgs;
    parseOrderedArgs.addArg("-b", "boolean arg", &bval);
    parseOrderedArgs.addArg("-i", "Integer arg", &ival);
    parseOrderedArgs.addArg("-r", "Flag only arg", &dval, false);
    parseOrderedArgs.addArg("-t", "Text arg", &tVal, false);
    parseOrderedArgs.addArg(0, "Integer ordered arg", &oiVal);
    parseOrderedArgs.addArg(1, "Real ordered arg", &orVal);
    parseOrderedArgs.addArg(2, "text ordered arg", &otVal, true);

    // check lambdas can also be added later by specifying the arg or switch name.
    auto positiveIntCheck = [](int64_t* value) {
      if (*value < 0) {
        throw std::invalid_argument("Value must be positive");
      } };
    parseOrderedArgs.addCheck("-i", positiveIntCheck);
    parseOrderedArgs.addCheck(0, positiveIntCheck);

    parseOrderedArgs.setHelpSwitch("--help");
    parseOrderedArgs.addHelpPretext("CmdLineArgs [args] is a test of ParsArgs");

    std::cout << std::format("\nStart of {} ordered command line arg tests\n", orderedArgTests.size());

    numFailures = 0;
    for (size_t it = 0; it < orderedArgTests.size(); it++) {
      // Call parse with argc and argv.
      if (parseOrderedArgs.parse(OrderedArgTest::argc, orderedArgTests[it].argv)) {
        if (std::string(orderedArgTests[it].expected) != parseOrderedArgs.getValuesString()) {
          std::cout << std::format("Test {} {} failed\n Got {}\n Expected: {}\n", it, orderedArgTests[it].description, 
                                   std::string(orderedArgTests[it].expected), parseOrderedArgs.getValuesString());
         numFailures++;
        }
      } else {
        if (std::string(orderedArgTests[it].expected) != parseOrderedArgs.getErrorMessage()) {
          std::cout << std::format("Test {} {} failed\n Got {}\n Expected: {}\n", it, orderedArgTests[it].description, 
                                   std::string(orderedArgTests[it].expected) , parseOrderedArgs.getErrorMessage());
          numFailures++;
        }
      }
      // restore the defaults
      bval = false;
      ival = 0;
      dval = 0.0;
      tVal = "";
      oiVal = 0;
      orVal = 0.0;
      otVal = "";
    }
    // Since arg parsing is done, the data in parseOrderedArgs can optionally be deleted to save space.
    parseOrderedArgs.clear();
    std::cout << std::format("{} failures found\n", numFailures);
  }

  if (doEquationParsingTests) {
  
    std::vector<IntEqParseTest> intEqParseTests;
    intEqParseTests.emplace_back("10", 10);
    intEqParseTests.emplace_back("(11)", 11);
    intEqParseTests.emplace_back("10+5", 10+5);
    intEqParseTests.emplace_back("-10 + 5", -10 + 5);
    intEqParseTests.emplace_back("-66 / -11", -66 / -11);
    intEqParseTests.emplace_back(" 3*5+10", 25);
    intEqParseTests.emplace_back("3+5*10", 53);
    intEqParseTests.emplace_back("3*5 % 10", 3*5 % 10);
    intEqParseTests.emplace_back("(3+5)*10", (3+5)*10);
    intEqParseTests.emplace_back("(3*2+5)*10", (3*2+5)*10);
    intEqParseTests.emplace_back("(3+2)*(2+5)*10", (3+2)*(2+5)*10);
    intEqParseTests.emplace_back("((3+2)*2+5+6)*10", ((3+2)*2+5+6)*10);
    intEqParseTests.emplace_back("((3+2)*(2+5)+6)*10", ((3+2)*(2+5)+6)*10);
    intEqParseTests.emplace_back("10*(6+(3+2)*(2^2+5))", 10 * ( 6 + (3 + 2) * ( static_cast<int64_t>(pow(2,2)) + 5)));// 
    intEqParseTests.emplace_back("10,2", "unexpected character");
    intEqParseTests.emplace_back("(3+5*10", "mismatched parentheses");
    intEqParseTests.emplace_back("3+5)*10", "mismatched parentheses");
    intEqParseTests.emplace_back("/3+5)*10", "expected a value or open parenthesis");
    intEqParseTests.emplace_back("3+5*10/", "expected a value or open parenthesis");
    intEqParseTests.emplace_back("0b1010", 0b1010);
    intEqParseTests.emplace_back("01234567", 01234567);
    intEqParseTests.emplace_back("0xabcdef", 0xabcdef);
    intEqParseTests.emplace_back("0", 0);


    std::cout << std::format("\nStart of {} integer equation tests\n", intEqParseTests.size());

    numFailures = 0;
    for (size_t i = 0; i < intEqParseTests.size(); i++) {
      try {
        auto in = intEqParseTests[i].eq;
        size_t strIdx = 0;
        int64_t parenCount = 0;
        int64_t returnValue = ParseEq::parseIntEquation(in, strIdx, 0, parenCount);
        if (parenCount != 0)
          throw std::invalid_argument("mismatched parentheses");
        else if (strIdx != strlen(in))
          throw std::invalid_argument("parse error");
        else {
          if (returnValue != intEqParseTests[i].ex) {
            std::cout << std::format("Test {} failed. For input {}, got {}, expected {}\n", i, intEqParseTests[i].eq, returnValue, intEqParseTests[i].ex);
            numFailures++;
          }
        }
      }
      catch (const std::exception& e) {
        // catch any errors found in parsing.
        std::string newWhat = e.what();
        if (newWhat.find("stoll") != std::string::npos || newWhat.find("stod") != std::string::npos)
          newWhat = "expected a value or open parenthesis";
        //std::cerr << newWhat << std::endl;
        if (newWhat != intEqParseTests[i].erm) {
//          std::cout << std::format("Test {} failed. For input {}, got {}, expected {}\n", i, intEqParseTests[i].eq, newWhat, intEqParseTests[i].ex);
          numFailures++;
        }
      }
    }
    std::cout << std::format("{} failures found\n", numFailures);

    std::vector<DblEqParseTest> dblEqParseTest;

    dblEqParseTest.emplace_back("10.2", 10.2);
    dblEqParseTest.emplace_back("(10.3)", (10.3));
    dblEqParseTest.emplace_back("10.2 + 5.3", 10.2 + 5.3);
    dblEqParseTest.emplace_back("-10.2+5.3", -10.2+5.3);
    dblEqParseTest.emplace_back("-66.6 / -11.1", -66.6 / -11.1);
    dblEqParseTest.emplace_back(" 3.0*5+10", 3.0*5+10);
    dblEqParseTest.emplace_back("3+5.0*10", 3+5.0*10);
    dblEqParseTest.emplace_back("(3+5)*10.0", (3+5)*10.0);
    dblEqParseTest.emplace_back("(3.0*2+5)*10", (3.0*2+5)*10);
    dblEqParseTest.emplace_back("(3+2.0)*(2+5)*10", (3+2.0)*(2+5)*10);
    dblEqParseTest.emplace_back("((3+2)*2.0+5+6)*10", ((3+2)*2.0+5+6)*10);
    dblEqParseTest.emplace_back("((3+2)*(2+5.0)+6)*10", ((3+2)*(2+5.0)+6)*10);
    dblEqParseTest.emplace_back("10*(6+(3+2)*(2.0^2+5))", 10*(6+(3+2)*(pow(2.0,2)+5)));
    dblEqParseTest.emplace_back("10,2", "unexpected character");
    dblEqParseTest.emplace_back("(3+5*10", "mismatched parentheses");
    dblEqParseTest.emplace_back("3+5)*10", "mismatched parentheses");
    dblEqParseTest.emplace_back("/3+5)*10", "expected a value or open parenthesis");
    dblEqParseTest.emplace_back("3+5*10/", "expected a value or open parenthesis");

    std::cout << std::format("\nStart of {} real equation tests\n", dblEqParseTest.size());

    numFailures = 0;
    for (size_t i = 0; i < dblEqParseTest.size(); i++) {
      try {
        auto in = dblEqParseTest[i].eq;
        size_t strIdx = 0;
        int64_t parenCount = 0;
        double returnValue = ParseEq::parseRealEquation(in, strIdx, 0, parenCount);
        if (parenCount != 0)
          throw std::invalid_argument("mismatched parentheses");
        else if (strIdx != strlen(in))
          throw std::invalid_argument("parse error");
        //else std::cout << returnValue << std::endl;
        else {
          if (returnValue != dblEqParseTest[i].ex) {
            std::cout << std::format("Test {} failed. For input {}, got {}, expected {}\n",i, dblEqParseTest[i].eq, returnValue, dblEqParseTest[i].ex);
            numFailures++;
          }
        }
      }
      catch (const std::exception& e) {
        // catch any errors found in parsing.
        std::string newWhat = e.what();
        if (newWhat.find("stoll") != std::string::npos || newWhat.find("stod") != std::string::npos)
          newWhat = "expected a value or open parenthesis";
        if (newWhat != dblEqParseTest[i].erm) {
          std::cout << std::format("Test {} failed. For input {}, got {}, expected {}\n",i, dblEqParseTest[i].eq, newWhat, dblEqParseTest[i].erm);
          numFailures++;
        }
      }
    }
    std::cout << std::format("{} failures found\n", numFailures);  
  }
}
