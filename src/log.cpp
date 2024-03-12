#include "log.hpp"
#include <bits/types/struct_timeval.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <stdio.h>
#include <sys/time.h>

void Log::log(LogLevel level, const char *message,
              std::ostream &os) { // default output stream is standard output

  if (level > ::level) {
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
  case LogLevel::ERROR:
    _level = "[%d:%d] |⛔ Error | ";
    break;
  case LogLevel::WARNING:
    _level = "[%d:%d] |⚠ Warning| ";
    break;
  case LogLevel::DATA:
    this->log(ERROR, "DATA passed to Log Function?");
    return;
  }

  int length = snprintf(nullptr, 0, _level, tv.tv_sec, tv.tv_usec);
  char *message_prefix = new char[length + 1];

  snprintf(message_prefix, length + 1, _level, tv.tv_sec, tv.tv_usec);
  os << message_prefix << message << '\n';
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

void Log::add_data(int curr_id, double curr_data) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double time_stamp = (double)tv.tv_sec + (double)tv.tv_usec / 100000;

  LogData this_data;

  this_data.timestamp = time_stamp;
  this_data.membrane_potentail = curr_data;
  this_data.group_id = 0;
  this_data.neuron_id = curr_id;

  this->log_data.push_back(this_data);
}

void Log::write_data(const char *filename) {

  struct timeval tv;
  gettimeofday(&tv, NULL);

  // length
  int length = snprintf(nullptr, 0, filename, tv.tv_sec);
  // allocate
  char *file_name = new char[length + 1];
  // format
  snprintf(file_name, length + 1, filename, tv.tv_sec);

  std::ofstream file;
  file.open(file_name);

  if (!file.is_open()) {
    this->log(ERROR, "write_data: Unable to open file");
    delete[] file_name;
    return;
  }

  for (LogData log_data : this->log_data) {
    file << std::fixed << log_data.group_id << " " << log_data.neuron_id << " "
         << log_data.timestamp << " " << log_data.membrane_potentail << '\n';
  }

  file.close();
  // deallocate
  delete[] file_name;
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

  LogData this_data;

  this_data.timestamp = time;
  this_data.membrane_potentail = curr_data;
  this_data.group_id = group_id;
  this_data.neuron_id = curr_id;

  this->log_data.push_back(this_data);
}

void Log::add_data(int group_id, int curr_id, double curr_data) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double time_stamp = (double)tv.tv_sec + (double)tv.tv_usec / 100000;

  LogData this_data;

  this_data.timestamp = time_stamp;
  this_data.membrane_potentail = curr_data;
  this_data.group_id = group_id;
  this_data.neuron_id = curr_id;

  this->log_data.push_back(this_data);
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
