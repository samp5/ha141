#ifndef REMOTE_SNN_H
#define REMOTE_SNN_H
#include "../network.hpp"
#include <map>
#include <string>
#include <tuple>
#include <vector>

using AdjDict =
    std::map<std::tuple<int, int>,
             std::map<std::tuple<int, int>, std::map<std::string, float>>>;

typedef std::map<std::string, double> ConfigDict;

class rpcSNN : public SNN {
private:
  std::vector<std::vector<double>> data;
  std::vector<double> dataToRun;
  AdjDict adjList;
  size_t maxLayer; // maximum "layer", aka maximum number of columns
  ConfigDict configDict;
  bool rpc = false;

public:
  rpcSNN(ConfigDict dict = {});
  void initialize(char *packed_adj_list, int number_input_neurons);
  void generateImage();
  static ConfigDict getDefaultConfig();
  // so cursed
  int getNumberNeuronFromPackedData(char *packed_data);
  int addEdgesFromPackedData(char *packed_adj_dict);
};
#endif // !REMOTE_SNN_H
