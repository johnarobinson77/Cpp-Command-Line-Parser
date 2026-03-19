# ParseArgs

ParseArgs is a feature-rich command-line parser written in C++ that requires a minimum of user developer input to invoke.  The main features are:  

1. Supports four input types, including boolean (bool, integer (int64\_t), real (double), and text (std\:\:string\_view)  
2. Supports array inputs of the 4 above types  
3. Supports both ordered and switch arguments  
4. Supports extensive error checking that can be extended by the developer  
5. Performs arithmetic on inputs to integer and real input types using standard operators and precedence.  
6. Supports binary, octal, and hex inputs to integer inputs.  
7. Automatic formatted help message output with the \-h switch.
8. Header only.  No separate comple needed.

# Usage

ParseArgs is invoked using the following basic steps:

1. Define the variables you want the command line arguments to modify.  Each variable can be one of 4 basic types: bool, int64\_t, double, or  std\:\:string\_view.  Those variables can also be one of 4 array types: std\:\:vector\<bool\>, std\:\:vector\<int64\_t\>, std\:\:vector\<double\>, or  std\:\:vector\<std\:\:string\_view\>  
2. Define an object of type ParsArgs.  
3. For each of the variables defined, call the addArg function with it’s order or switch and a description.  
4. Optionally create any additional checks on the inputs by defining a lambda that does the check and adding it to the argument definition.  
5. Optionally add a program description that will be displayed with the help message.  
6. Call the parse function with the argc and argv variables passed in my main.  Optionally exit the program if the return is false.

## Adding Argument Definitions.

Command-line arguments can be defined in two main categories: ordered and switched.  An ordered argument is one that is defined by the order in which it occurs on the command line.  A switched arrangement is one that is described by a switch that precedes it, such as ‘-d’ or ‘%f”

To define an ordered argument, use the following ParsArgs function:
```cpp
  /// @brief This addArg function creates <type> ordered argument entry  
  /// @param argNum index of the argument on the command line.  
  /// @param desc char* desc is the description that will be printed with the help message  
  /// @param valPtr pointer to the <type> variable that will be modified by the switch  
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false  
  void addArg(const int argNum, const char* desc, <type>* const valPtr, bool req = false);  
  /// Where <type> can be bool, int64_t, double, or std::string_view
 ```
ParseArgs will place the first argument not preceded by a switch in argNum 0, the second in argNum 1, etc.  Those arguments must match the type of variable pointed to in the call to addArgs.

To define a switch argument, use the following parseArgs function:  
```cpp
  /// @brief This addArg function creates <type> switch argument entry  
  /// @param arg char* arg is the switch or argument name on the command line.  
  /// @param desc char* desc is the description that will be printed with the help message  
  /// @param valPtr pointer to the <type> variable that will be modified by the switch  
  /// @param req is a flag that indicates whether this argument or switch is required. Default is false  
  void addArg(const char* arg, const char* desc, <type>* const valPtr, bool req = false);  
  /// Where <type> can be bool, int64_t, double, std::string_view, std::vector<boo., std::vector<int64_t>, std::vector<double> or std::vector<string_view> 
```

Switch arguments use a switch to indicate which variable to modify, typically used for options.  When parsArgs recognises the switch added in the addArg function call, it modifies the variable with the next item on the command line.  As with the ordered  arguments that item must match the type of variable pointed to 

## Adding Optional Additional Checks

An additional check on the argument can be added by calling the addCheck function.  
```cpp
 /// @brief This addCheck function adds a "check" lambda to the last argument definition created by addArg  
  /// @param check lambda of type void(<type>*)  
  void addCheck(std::function<void(<type>*)> check);

  /// @brief This addCheck function adds a "check" lambda to a previously defined <<type> ordered arg.  
  /// @param argNum index of the ordered arg.  
  /// @param check lambda of type void(<type>*)  
  void addCheck(const int argNum, std::function<void(<type>*)> check);

  /// @brief This addCheck function adds a "check" lambda to a previously defined <type> switch arg.  
  /// @param argName switch or arg name of a previously created switch arg.  
  /// @param check lambda of type void(<type>*)  
  void addCheck(const char* argName, std::function<void(<type>*)> check);
```
The call to addCheck must come after the call to addArg to which the check is being added.  The type of the pointer in the lambda must match the type of the variable pointer to in the addArgs function.  If an error is found, the the check lambda should trow any std::exception.  This will cause ParseArgs to display the exception error message and the help message.

## Optional Help Display

ParseArgs will display a formatted list of arguments.  By default, it checks for  “-h: switch on the command line do invade that display.  However, the help switch can be changed with the following function:
```cpp
  /// @breif setHelpSwitch changes the default "-h" help switch to a developer-supplied string  
  /// @param helpString is the developer-supplied help switch string.  
  void addHelpPretext(((std::string_view helpString);
```
An optional text string, such as a program explanation, can be added to the help display with a call to:  
```cpp
  /// @brief addHelpPretext allows a program description text to be added at the top of the help message  
  /// @param pt description string.  
  void addHelpPretext(std::string_view pt);
```
## Parsing the Commandline Arguments

After all the argument definitions have been entered, the command line arguments are parsed with: 
```cpp
  /// @brief The parse member function parses the standard C command line arguments and  
  ///        modifies the variables specified in the calls to addArg  
  /// @param argc int argc is the number of args.  Same as main  
  /// @param argv char* argv[] array of pointers to char strings holding the actual arguments.  Typically directly from main(argc, argv)  
  /// @param start optional int indicating the starting argv.  Default is 1 or the first arg after the program name.  
  /// @return false if parse errors, otherwise true  
  bool parse(const int argc, const char* argv[], int start = 1);
```

Note that parse’s argc and argv are defined as const, so main’s must be defined with const.

## Extras

A call to the following function will return an std\:\:string that contains all of the defined argument values in a format that could be used as a future call to the program.
```cpp
  /// @brief  getValuesString returns the command line string for all the args with their current settings.  
  /// @return The resulting string  
  std::string getValuesString();
```

After all the arguments have been parsed, the memory allocated within ParsArgs can be deallocated with a call to clear.
```cpp
 /// @brief clear member function deletes all the data stored in the args maps.  
  void cler();
```

## Usage Example 
```cpp
#include "ParseArgs.hpp"

int main(const int argc, const char* argv[]) {   
    
  //Define variables to be modified by the command-line args  
  bool aval = false;  
  bool bval = true;   
  int64_t ival = 0;   
  double dval = 0.0;  
  std::string_view sVal = "";  
  std::string_view osVal = "";    
  std::vector<bool> vbVals{ false, false };   
    
  // Create an instance of ParseArgs and add the desired command-line args or switches   
  // Optionally add a lambda to perform a check on the parsed value.  
  ParseArgs parseArgs;   
  parseArgs.addHelpPretext("Example program to demonstrate ParseArgs usage");  
  parseArgs.addArg(0, "ordered or non-switch arg", &osVal);  
  parseArgs.addArg("-a", "Flag only boolean arg", &aval);   
  parseArgs.addArg("-b", "Boolean arg", &bval);  
  parseArgs.addArg("-i", "Integer arg", &ival);  
  parseArgs.addArg("-r", "Real arg", &dval);  
  parseArgs.addCheck([](double* const value) {   
    if (*value > 10.0 || *value < -10.0)   
    throw std::range_error("Value must be in the range of -10 to +10 inclusive");  
  });   
  parseArgs.addArg("-s", "String arg", &sVal, true);   
  parseArgs.addArg("-vb", "Vector of bools arg", &vbVals);   
    
  // Call parse with argc and argv.  Exit if there was an error    
  if (!parseArgs.parse(argc, argv)) exit(0);  
    
  // print the current values of all the arguments  
  std::cout << parseArgs.getValuesString();   
    
  // Since arg parsing is done, the data in parseArgs can optionally be deleted to save space.   
  parseArgs.clear();  
  return 0;  
}   
```
# Command-line Argument Formats

ParseArgs supports a variety of formats. The argument must conform to the type, or an error will be thrown.

## Argument Type Boolean
The following are acceptable inputs for arguments defined as a boolean.

| Boolean argument | Description |
| -------- | ----------- |
| \[true\|false\]                  | \# sets an ordered argument to true or false |
| \-b                             | \# the switch by itself will set the boolean variable to true |
| \-b \[true\|false\]              | \# sets the boolean variable to true or false  
| \-b=\[true\|false\]              | # sets the boolean variable to true or false |
## Argument Type Integer
The following are acceptable inputs for arguments defined  integer  

| Integer Argument | Description |
| -------- | ----------- |
| \<integer expression\>         | \# for ordered integer type  arguments |  
| \-i \<integer expression\>      | \# for switch integer type argument  |
| \-i=\<integer expression\>     | \# for switch integer type argument  |

Where \< integer expression\> is an expression of arithmetic operations on whole numbers.  Operators include ‘+’, ‘-’, ‘ ‘\*’, ‘/’, ‘%’, ‘^’, ‘(‘, ‘)’. Standard arithmetic precedence applies.  For example, the expression  \-i 5+10\*11 will set the integer to 115.  The numbers can be expressed as:  
 0bnnnn - a binary value   
 0nnnnn - an octal number    
 nnnnnn - a decimal number  
 0xnnnn - a hexadecimal number 

## Argument Type Real
The following are acceptable inputs for arguments defined as real.  

| Real Argument | Description |
| -------- | ----------- |
| \<real expression\> | for ordered arguments | 
| \-r \<real expression\> | for switch real type argument |  
| \-r=\<real expression\> | \# for switch real type argument|

Where \<real expression\> can be an expression of arithmetic operations on real numbers.  Operators include ‘+’, ‘-’, ‘ ‘\*’, ‘/’, ‘%’, ‘^’, ‘(‘, ‘)’. Standard arithmetic precedence applies. For example, \-r (5.5\+10.1)\*11.0 will set the the double variable to 171.6.

## Arquement Type Text
The following are acceptable inputs for arguments defined as text.  

| Text Argument | Description |
| -------- | ----------- |
| \<anyText\> | for ordered type text arguments | 
| \-i  \<anyText\> | for switch text type  argements |

## Vector Argument Types
The input format for vector argument types is the same as the above for basic types with multiple values where the values are separated by commas. The total number of inputs must match the size of the vector used in the argument definiion.  

| Text Argument | Description |
| -------- | ----------- |
| \-vb false,true | for a boolean vector of size 2 |  
| -vb=true,false | for a boolean vector of size 2 | 

# Test Code
Included with this submit is a test program called CmdLineArgs.cpp.  It is for regression testing only and should not be added to any code base.  CmdLineArgs.cpp requires compilation with -std=c++20 or later.  Howevver, ParseArgs.hpp should work with earlier standards.
