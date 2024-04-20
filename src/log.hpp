#ifndef LOG
#define LOG

#include "message.hpp"
#include <chrono>
#include <iostream>
#include <vector>

using std::cout;
using std::ostream;
using std::vector;

enum LogLevel {
  ESSENTIAL,
  NONE,
  DATA,
  ERROR,
  WARNING,
  INFO,
  DEBUG,
  DEBUG2,
  DEBUG3,
  DEBUG4,
};

typedef struct LogData {
  int neuron_id;
  int group_id;
  double timestamp;
  double membrane_potentail;
  int neuron_type = 0;
  Message_t message_type;
  int stimulus_number;
} LogData;

extern LogLevel DEBUG_LEVEL;
extern vector<int>::const_iterator STIMULUS;
extern ostream &STREAM;
extern pthread_mutex_t log_mutex;

using hr_clock = std::chrono::high_resolution_clock;
using duration = std::chrono::duration<double>;

class Log {

private:
  hr_clock::time_point start;
  double off_set;

public:
  // constructor
  Log() : start(hr_clock::now()), off_set(0.0f) {}
  // DATA Functions

  void start_clock() { this->start = hr_clock::now(); }
  void write_data(const char *filesname = "./logs/%ld/%ld.log");

  void add_data(int group_id, int curr_id, double curr_data);

  void add_data(int group_id, int curr_id, double curr_data, double time);

  void add_data(int group_id, int curr_id, double curr_data, double time,
                int type, Message_t message_type);

  void add_data(int group_id, int curr_id, double curr_data, double time,
                int type, Message_t message_type, Neuron *origin);

  void add_data(LogData data);

  void log_runtime_config(const std::string &name);

  // General Log function
  void log(LogLevel level, const char *message, ostream &os = STREAM);

  // Get time stamp from std::chrono::high_resolution_clock
  double get_time_stamp();

  // Sets the offset of all future timestamps
  void set_offset(double value);

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

  void log_string(LogLevel level, const char *message, const char *string);

  void print(const char *message, bool newline = true,
             std::ostream &os = STREAM);

  void log_message(LogLevel level, const char *message, double timestamp,
                   int group_id, int id, double value);
  void log_value(LogLevel level, const char *message, int value);
  void log_value(LogLevel level, const char *message, int value, int value2);

private:
  vector<LogData> log_data;
};

#endif // !LOG
