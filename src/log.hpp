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

  void add_data(int id, double data);

  void write_data(const char *filesname = "./logs/%ld.log");

  void log(LogLevel level, const char *message, std::ostream &os = std::cout);

  void log_neuron_state(LogLevel level, const char *message, int id);

  void log_neuron_interaction(LogLevel level, const char *message, int id1,
                              int id2);
  void log_neuron_interaction(LogLevel level, const char *message, int id1,
                              int id2, double value);
  void log_neuron_value(LogLevel level, const char *message, int id,
                        double accumulated);
  void log_neuron_type(LogLevel level, const char *message, int id,
                       const char *type);

private:
  vector<double> time;
  vector<double> data;
  vector<int> id;
};

#endif // !LOG
