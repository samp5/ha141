#include "log.hpp"
#include "message.hpp"
#include "neuron.hpp"
#include "runtime.hpp"
#include <bits/types/struct_timeval.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <pthread.h>
#include <stdio.h>
#include <string>
#include <sys/time.h>
extern Mutex mx;
extern RuntimConfig cf;

std::string io_type_to_string(Neuron_t type) {
  std::string ret;
  switch (type) {
  case None:
    ret = "None";
    break;
  case Input:
    ret = "Input";
    break;
  case Hidden:
    ret = "Hidden";
    break;
  case Output:
    ret = "Output";
    break;
  }
  return ret;
}

pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;

void Log::log(LogLevel level, const char *message,
              std::ostream &os) { // default output stream is standard output

  if (level > cf.DEBUG_LEVEL) {
    return;
  }

  // get time for log message

  const char *_level;

  switch (level) {
  case LogLevel::ESSENTIAL:
    _level = " -> ";
    break;
  case LogLevel::INFO:
    _level = "[%f] ⓘ  ";
    break;
  case LogLevel::DEBUG:
    _level = "[%f] ❶  ";
    break;
  case LogLevel::DEBUG2:
    _level = "[%f] ❷  ";
    break;
  case LogLevel::DEBUG3:
    _level = "[%f] ❸  ";
    break;
  case LogLevel::DEBUG4:
    _level = "[%f] ❹ ";
    break;
  case LogLevel::ERROR:
    _level = "[%f] |⛔ Error | ";
    break;
  case LogLevel::WARNING:
    _level = "[%f] |⚠ Warning| ";
    break;
  case LogLevel::DATA:
    this->log(ERROR, "DATA passed to Log Function?");
    return;
  case LogLevel::NONE:
    this->log(ERROR, "NONE passed to Log Function?");
    return;
  }

  double time_rn = lg.get_time_stamp();
  int length = snprintf(nullptr, 0, _level, time_rn);
  char *message_prefix = new char[length + 1];

  snprintf(message_prefix, length + 1, _level, time_rn);
  // pthread_mutex_lock(&log_mutex);
  os << message_prefix << message << '\n';
  // pthread_mutex_unlock(&log_mutex);
  delete[] message_prefix;
}

void Log::log_neuron_state(LogLevel level, const char *message, int id) {

  // length
  int length = snprintf(nullptr, 0, message, id);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, id);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_neuron_interaction(LogLevel level, const char *message, int id1,
                                 int id2) {
  // length
  int length = snprintf(nullptr, 0, message, id1, id2);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, id1, id2);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_neuron_interaction(LogLevel level, const char *message, int id1,
                                 int id2, double value) {
  // length
  int length = snprintf(nullptr, 0, message, id1, id2, value);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, id1, id2, value);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_neuron_value(LogLevel level, const char *message, int id,
                           double accumulated) {
  // length of message
  int length = snprintf(nullptr, 0, message, id);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, id, accumulated);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_neuron_type(LogLevel level, const char *message, int id,
                          const char *type) {
  // length
  int length = snprintf(nullptr, 0, message, id);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, id, type);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::add_data(int group_id, int curr_id, double curr_data, double time,
                   int type, Message_t message_type) {
  pthread_mutex_lock(&data_mutex);
  LogData this_data;

  this_data.timestamp = time;
  this_data.membrane_potentail = curr_data;
  this_data.group_id = group_id;
  this_data.neuron_id = curr_id;
  this_data.neuron_type = type;
  this_data.message_type = message_type;
  this_data.stimulus_number = *cf.STIMULUS;

  this->log_data.push_back(this_data);
  pthread_mutex_unlock(&data_mutex);
}

void Log::add_data(int group_id, int curr_id, double curr_data, double time,
                   int type, Message_t message_type, Neuron *origin) {
  LogData *this_data = new LogData;
  this_data->timestamp = time;
  this_data->membrane_potentail = curr_data;
  this_data->group_id = group_id;
  this_data->neuron_id = curr_id;
  this_data->neuron_type = type;
  this_data->message_type = message_type;
  this_data->stimulus_number = *cf.STIMULUS;
  origin->push_back_data(this_data);
}

void Log::add_data(LogData data) {
  pthread_mutex_lock(&data_mutex);
  this->log_data.push_back(data);
  pthread_mutex_unlock(&data_mutex);
}
void Log::write_data(const char *filename) {

  namespace fs = std::filesystem;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  int name = tv.tv_sec - 1'710'000'000;

  fs::path parent = "./logs/" + std::to_string(name);
  fs::create_directory(parent);

  // length
  int length = snprintf(nullptr, 0, filename, name, name);
  // allocate
  char *file_name = new char[length + 1];
  // format
  snprintf(file_name, length + 1, filename, name, name);

  std::ofstream file;
  file.open(file_name);

  if (!file.is_open()) {
    this->log(ERROR, "write_data: Unable to open file");
    delete[] file_name;
    return;
  }

  for (LogData log_data : this->log_data) {
    file << std::fixed << log_data.group_id << " " << log_data.neuron_id << " "
         << io_type_to_string((Neuron_t)log_data.neuron_type) << " "
         << log_data.timestamp << " " << log_data.membrane_potentail << " "
         << messageTypeToString(log_data.message_type) << " "
         << log_data.stimulus_number << '\n';
  }

  file.close();

  // deallocate
  delete[] file_name;
  this->log_runtime_config(std::to_string(name));
}

void Log::log_group_neuron_state(LogLevel level, const char *message,
                                 int group_id, int id) {
  // length
  int length = snprintf(nullptr, 0, message, group_id, id);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, group_id, id);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_group_neuron_value(LogLevel level, const char *message,
                                 int group_id, int id, double accumulated) {
  // length of message
  int length = snprintf(nullptr, 0, message, group_id, id, accumulated);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, group_id, id, accumulated);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_group_neuron_interaction(LogLevel level, const char *message,
                                       int group_id1, int id1, int group_id2,
                                       int id2, double value) {
  // length
  int length =
      snprintf(nullptr, 0, message, group_id1, id1, group_id2, id2, value);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, group_id1, id1, group_id2, id2,
           value);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_group_neuron_interaction(LogLevel level, const char *message,
                                       int group_id1, int id1, int group_id2,
                                       int id2) {
  // length
  int length = snprintf(nullptr, 0, message, group_id1, id1, group_id2, id2);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, group_id1, id1, group_id2, id2);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::add_data(int group_id, int curr_id, double curr_data, double time) {

  pthread_mutex_lock(&data_mutex);
  LogData this_data;

  this_data.timestamp = time;
  this_data.membrane_potentail = curr_data;
  this_data.group_id = group_id;
  this_data.neuron_id = curr_id;

  this->log_data.push_back(this_data);
  pthread_mutex_unlock(&data_mutex);
}

void Log::add_data(int group_id, int curr_id, double curr_data) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double time_stamp = (double)tv.tv_sec + (double)tv.tv_usec / 100000;

  pthread_mutex_lock(&data_mutex);
  LogData this_data;

  this_data.timestamp = time_stamp;
  this_data.membrane_potentail = curr_data;
  this_data.group_id = group_id;
  this_data.neuron_id = curr_id;
  this_data.stimulus_number = *cf.STIMULUS;

  this->log_data.push_back(this_data);
  pthread_mutex_unlock(&data_mutex);
}

void Log::log_group_state(LogLevel level, const char *message, int group_id) {
  // length
  int length = snprintf(nullptr, 0, message, group_id);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, group_id);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_group_value(LogLevel level, const char *message, int group_id,
                          int value) {
  // length
  int length = snprintf(nullptr, 0, message, group_id, value);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, group_id, value);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_group_neuron_type(LogLevel level, const char *message,
                                int group_id, int id, const char *type) {

  // length
  int length = snprintf(nullptr, 0, message, group_id, id, type);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, group_id, id, type);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::print(const char *message, bool newline, std::ostream &os) {
  if (newline)
    os << message << '\n';
  else
    os << message;
}

void Log::set_offset(double value) { this->off_set += value; }

double Log::get_time_stamp() {

  hr_clock::time_point now = hr_clock::now();

  duration time_span = std::chrono::duration_cast<duration>(now - this->start);
  return time_span.count() - this->off_set;
}

void Log::log_message(LogLevel level, const char *message, double timestamp,
                      int group_id, int id, double value) {
  // length
  int length = snprintf(nullptr, 0, message, timestamp, group_id, id, value);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, timestamp, group_id, id, value);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_value(LogLevel level, const char *message, int value,
                    int value2) {
  // length
  int length = snprintf(nullptr, 0, message, value, value2);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, value, value2);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}
void Log::log_value(LogLevel level, const char *message, int value) {
  // length
  int length = snprintf(nullptr, 0, message, value);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, value);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_string(LogLevel level, const char *message, const char *string) {
  // length
  int length = snprintf(nullptr, 0, message, string);
  // allocate
  char *formatted_msg = new char[length + 1];
  // format
  snprintf(formatted_msg, length + 1, message, string);
  // log
  this->log(level, formatted_msg);
  // deallocate
  delete[] formatted_msg;
}

void Log::log_runtime_config(const std::string &name) {

  namespace fs = std::filesystem;
  fs::path config_source = cf.CONFIG_FILE;

  fs::path config_target = "./logs/" + name + "/" + name + ".toml";

  fs::copy_file(config_source, config_target);
}

const char *Log::get_active_status_string(bool active) {
  if (active) {
    const char *active = "active"; // NOLINT
    return active;
  } else {
    const char *inactive = "inactive"; // NOLINT
    return inactive;
  }
}

LogLevel Log::get_level_from_string(std::string level) {
  if (level == "NONE")
    return NONE;
  if (level == "INFO")
    return INFO;
  if (level == "DEBUG")
    return DEBUG;
  if (level == "DEBUG2")
    return DEBUG2;
  if (level == "DEBUG3")
    return DEBUG3;
  if (level == "DEBUG4")
    return DEBUG4;
  lg.log(WARNING, "\"level\" does not match any available options: setting "
                  "LogLevel to INFO");
  return INFO;
}
std::string Log::messageTypeToString(Message_t type) {
  std::string ret;
  switch (type) {
  case Stimulus:
    ret = "S";
    break;
  case Refractory:
    ret = "R";
    break;
  case From_Neighbor:
    ret = "N";
    break;
  case Decay:
    ret = "D";
    break;
  case Checked:
    ret = "C";
    break;
  }
  return ret;
}
