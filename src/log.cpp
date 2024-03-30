#include "log.hpp"
#include "functions.hpp"
#include "message.hpp"
#include <bits/types/struct_timeval.h>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <pthread.h>
#include <stdio.h>
#include <string>
#include <sys/time.h>

pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;

void Log::log(LogLevel level, const char *message,
              std::ostream &os) { // default output stream is standard output

  if (level > ::DEBUG_LEVEL) {
    return;
  }

  // get time for log message
  struct timeval tv;
  gettimeofday(&tv, NULL);

  const char *_level;

  switch (level) {
  case LogLevel::INFO:
    _level = "[%d:%d] ⓘ  ";
    break;
  case LogLevel::DEBUG:
    _level = "[%d:%d] ❶  ";
    break;
  case LogLevel::DEBUG2:
    _level = "[%d:%d] ❷  ";
    break;
  case LogLevel::DEBUG3:
    _level = "[%d:%d] ❸  ";
    break;
  case LogLevel::DEBUG4:
    _level = "[%d:%d] ❹ ";
    break;
  case LogLevel::ERROR:
    _level = "[%d:%d] |⛔ Error | ";
    break;
  case LogLevel::WARNING:
    _level = "[%d:%d] |⚠ Warning| ";
    break;
  case LogLevel::DATA:
    this->log(ERROR, "DATA passed to Log Function?");
    return;
  case LogLevel::NONE:
    this->log(ERROR, "NONE passed to Log Function?");
    return;
  }

  int length = snprintf(nullptr, 0, _level, tv.tv_sec, tv.tv_usec);
  char *message_prefix = new char[length + 1];

  snprintf(message_prefix, length + 1, _level, tv.tv_sec, tv.tv_usec);
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

  this->log_data.push_back(this_data);
  pthread_mutex_unlock(&data_mutex);
}
void Log::write_data(const char *filename) {

  namespace fs = std::filesystem;

  struct timeval tv;
  gettimeofday(&tv, NULL);

  fs::path parent = "./logs/" + std::to_string(tv.tv_sec);
  fs::create_directory(parent);

  // length
  int length = snprintf(nullptr, 0, filename, tv.tv_sec, tv.tv_sec);
  // allocate
  char *file_name = new char[length + 1];
  // format
  snprintf(file_name, length + 1, filename, tv.tv_sec, tv.tv_sec);

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
         << message_type_to_string(log_data.message_type) << '\n';
  }

  file.close();

  // deallocate
  delete[] file_name;
  this->log_runtime_config();
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
  if (DEBUG_LEVEL == NONE) {
    return;
  }
  if (newline)
    os << message << '\n';
  else
    os << message;
}

double Log::get_time_stamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double time_stamp = (double)tv.tv_sec + (double)tv.tv_usec / 100000;
  return time_stamp;
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

void Log::log_runtime_config() {

  struct timeval tv;
  gettimeofday(&tv, NULL);

  std::string time = std::to_string(tv.tv_sec);

  namespace fs = std::filesystem;
  fs::path config_source = CONFIG_FILE;
  fs::path input_source = INPUT_FILE;

  fs::path config_target = "./logs/" + time + "/" + time + ".toml";
  fs::path input_target = "./logs/" + time + "/" + time + ".txt";

  fs::copy_file(config_source, config_target);
  fs::copy_file(input_source, input_target);
}
