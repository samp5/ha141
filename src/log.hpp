#ifndef LOG
#define LOG

#include <iostream>
#include <vector>

using std::vector;

enum LogLevel {
  DATA,
  ERROR,
  WARNING,
  INFO,
  DEBUG,
};

extern LogLevel level;

class Log {
public:
  // constructor

  void write_data(vector<int> id, vector<double> data,
                  const char *filesname = "%d.log");

  void log(LogLevel level, const char *message, std::ostream &os = std::cout);

  void log_neuron_state(LogLevel level, const char *message, int id);

  void log_neuron_interaction(LogLevel level, const char *message, int id1,
                              int id2);
  void log_neuron_interaction(LogLevel level, const char *message, int id1,
                              int id2, double value);
  void log_neuron_value(LogLevel level, const char *message, int id,
                        double accumulated);
  void log_accumulated_stdout(LogLevel level, const char *message, int id,
                              double accumulated);

  void log_neuron_type(LogLevel level, const char *message, int id,
                       const char *type);
};

#endif // !LOG
