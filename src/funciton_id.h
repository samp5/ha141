#ifndef FUNCTION_IDENTIFIER
#define FUNCTION_IDENTIFIER
#define MIN_FUNC_ID 0
#define MAX_FUNC_ID 1

enum FunctionIdentifier {
  Initiaize,
  Construct,
  UNKNOWN_FUNC
}; // if you change this change the MAX_FUNC_ID and MIN_FUNC_ID accordingly
   // please it will not work if you do not

#endif // !FUNCTION_IDENTIFIER
