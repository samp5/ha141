#ifndef LOG
#define LOG

#include <iostream>
#include <vector>

using std::cout;
using std::ostream;
using std::vector;

enum LogLevel {
  DATA,
  ERROR,
  WARNING,
  INFO,
  DEBUG,
  DEBUG2,
};

extern LogLevel level;

class Log {
public:
  // constructor

  // DATA Functions
  void add_data(int id, double data);

  void write_data(const char *filesname = "./logs/%ld.log");

  void add_data(int group_id, int curr_id, double curr_data);

  // General Log function
  void log(LogLevel level, const char *message, ostream &os = cout);

  // Neuron Logs
  void log_neuron_state(LogLevel level, const char *message, int id);

  void log_neuron_interaction(LogLevel level, const char *message, int id1,
                              int id2);
  void log_neuron_interaction(LogLevel level, const char *message, int id1,
                              int id2, double value);
  void log_neuron_value(LogLevel level, const char *message, int id,
                        double accumulated);
  void log_neuron_type(LogLevel level, const char *message, int id,
                       const char *type);

  // Neuron Logs for Neurons in Groups
  void log_group_neuron_state(LogLevel level, const char *message, int group_id,
                              int id);
  void log_group_neuron_value(LogLevel level, const char *message, int group_id,
                              int id, double value);

  void log_group_neuron_type(LogLevel level, const char *message, int group_id,
                             int id, const char *type);

  void log_group_neuron_interaction(LogLevel level, const char *message,
                                    int group_id1, int id1, int group_id2,
                                    int id2);

  void log_group_neuron_interaction(LogLevel level, const char *message,
                                    int group_id1, int id1, int group_id2,
                                    int id2, double value);

  // Neuron Group Logs
  void log_group_state(LogLevel level, const char *message, int group_id);

  void log_group_value(LogLevel level, const char *message, int group_id,
                       int value);

  void print(const char *message, bool newline = true,
             std::ostream &os = std::cout);

private:
  vector<double> time;
  vector<double> data;
  vector<int> id;
  vector<int> group_id;
};

#endif // !LOG
