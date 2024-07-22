/** @file */
#ifndef LOG
#define LOG

#include "message.hpp"
#include <chrono>
#include <iostream>
#include <vector>

class SNN;
using std::cout;
using std::ostream;
using std::vector;

/**
 * \enum LogLevel
 * @brief Controlls debug level.
 *
 * Set in RuntimConfig. Controlls level of
 * detail on standard output.
 *
 */
enum LogLevel {
  ERROR,
  ESSENTIAL,
  WARNING,
  NONE,
  DATA,
  INFO,
  DEBUG,
  DEBUG2,
  DEBUG3,
  DEBUG4,
};

/**
 *
 * \struct LogData
 *
 * @brief  Log data datastructure.
 *
 * Each Neuron holds their own vector of LogData * that gets transfered
 * to the main Log class upon thread joining.
 *
 */
struct LogData {
  int neuron_id;
  int group_id;
  int timestamp;
  double potential;
  int neuron_type = 0;
  Message_t message_type;
  int stimulus_number;
  LogData(){};
  LogData(int nID, int gID, int t, double p, int nt, Message_t mt, int sn)
      : neuron_id(nID), group_id(gID), timestamp(t), potential(p),
        neuron_type(nt), message_type(mt), stimulus_number(sn) {}
  LogData(const LogData &other) = default;
  std::string toTmpFileString() {
    char toReturn[81];
    sprintf(toReturn, "%d %d %d %lf %d %d %d\n", neuron_id, group_id, timestamp,
            potential, neuron_type, message_type, stimulus_number);
    return std::string(toReturn);
  }
  void storeDataIn(double ret[]) {
    ret[0] = static_cast<double>(neuron_id);
    ret[1] = static_cast<double>(group_id);
    ret[2] = static_cast<double>(timestamp);
    ret[3] = potential;
    ret[4] = static_cast<double>(neuron_type);
    ret[5] = static_cast<double>(message_type);
    ret[6] = static_cast<double>(stimulus_number);
  };
  friend std::ostream &operator<<(std::ostream &out, const LogData &rhs) {
    out << "DATA FOR N" << rhs.neuron_id << "G" << rhs.group_id
        << " TYPE: " << rhs.message_type << " POTENTIAL: " << rhs.potential
        << " STIMULUS: " << rhs.stimulus_number
        << " TIMESTAMP: " << rhs.timestamp << "\n";
    return out;
  }
};

struct LogDataArray {
  LogData *array;
  size_t arrSize;
};

using hr_clock = std::chrono::high_resolution_clock;
using duration = std::chrono::duration<double>;

/**
 * \class Log
 * @brief Logging class.
 */
class Log {
private:
  hr_clock::time_point start;
  double offset; /**< Global offset accouting for stimulus switching time */
  SNN *network;
  vector<LogData *> log_data;

public:
  Log(SNN *network) : start(hr_clock::now()), offset(0.0f), network(network) {}
  Log() : start(hr_clock::now()), offset(0.0f), network(NULL){};
  ~Log();
  void batchReset() {
    start = hr_clock::now();
    offset = 0.0f;
    for (auto &data : log_data) {
      delete data;
      data = nullptr;
    }
    log_data.clear();
  }
  void startClock() { this->start = hr_clock::now(); }
  void writeData();
  void writeToFD(int fd, const std::vector<NeuronGroup *> &neuronGroups);
  void writeCSV(const std::vector<std::vector<int>> &mat);
  void addData(LogData *data);
  const vector<LogData *> &getLogData() const { return log_data; }
  void logConfig(const std::string &name);
  void log(LogLevel level, const char *message, ostream &os = std::cout);
  double time();
  void addOffset(double value);
  void groupNeuronState(LogLevel level, const char *message, int group_id,
                        int id);
  void neuronValue(LogLevel level, const char *message, int group_id, int id,
                   double value);
  void neuronType(LogLevel level, const char *message, int group_id, int id,
                  const char *type);
  void neuronInteraction(LogLevel level, const char *message, int group_id1,
                         int id1, int group_id2, int id2);
  void message(LogLevel level, const char *message, int group_id1,
               int timestamp1, Message_t mt, int group_id2, int timestamp2);
  void message(LogLevel level, const char *message, int group_id1,
               int timestamp1, Message_t mt, int timestamp2);
  void state(LogLevel level, const char *message, int group_id);
  void string(LogLevel level, const char *message, const char *string);
  void print(const char *message, bool newline = true,
             std::ostream &os = std::cout);
  void value(LogLevel level, const char *message, int value);
  void value(LogLevel level, const char *message, double value);
  const char *activeStatusString(bool active);
  LogLevel debugLevelString(std::string level);
  std::string messageTypeToString(Message_t type);
  void setNetwork(SNN *_network) { network = _network; };
  void printNetworkInfo();
};

#endif // !LOG
