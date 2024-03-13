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
  DEBUG3,
  DEBUG4,
};

typedef struct {
  int neuron_id;
  int group_id;
  double timestamp;
  double membrane_potentail;
} LogData;

extern LogLevel level;
extern ostream &STREAM;
extern pthread_mutex_t log_mutex;

class Log {
public:
  // constructor

  // DATA Functions
  void add_data(int id, double data);

  void write_data(const char *filesname = "./logs/%ld.log");

  void add_data(int group_id, int curr_id, double curr_data);
  void add_data(int group_id, int curr_id, double curr_data, double time);

  // General Log function
  void log(LogLevel level, const char *message, ostream &os = STREAM);
  double get_time_stamp();

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
             std::ostream &os = STREAM);

  void log_message(LogLevel level, const char *message, double timestamp,
                   int group_id, int id, double value);
  void log_value(LogLevel level, const char *message, int value);

private:
  // make this a struct
  vector<LogData> log_data;
};

#endif // !LOG
