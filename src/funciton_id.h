#ifndef FUNCTION_IDENTIFIER
#define FUNCTION_IDENTIFIER
#define MIN_FUNC_ID 0
#define MAX_FUNC_ID 4

enum FunctionIdentifier {
  /*
   * Analagous to pySNN::initiaize(AdjDict dict, NUMBER_INPUT_NEURONS)
   */
  Initiaize,
  /*
   * Analagous to pySNN::pySNN(ConfigDict)
   */
  Construct,
  /*
   * pySNN::runBatch(ConfigDict)
   */
  RunBatch,
  /*
   * pySNN::getActivation(ConfigDict)
   */
  GetActivation,
  /*
   * pySNN::batchReset(ConfigDict)
   */
  BatchReset,
  UNKNOWN_FUNC
}; // if you change this change the MAX_FUNC_ID and MIN_FUNC_ID accordingly
   // please it will not work if you do not

enum Response {
  Sucesss,
  Fail,
};

#endif // !FUNCTION_IDENTIFIER
