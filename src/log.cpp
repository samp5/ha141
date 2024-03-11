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
    _level = "[%d:%d] [Info] ";
    break;
  case LogLevel::DEBUG:
    _level = "[%d:%d] [Debug] ";
    break;
  case LogLevel::DEBUG2:
    _level = "[%d:%d] [Debug2] ";
    break;
  case LogLevel::ERROR:
    _level = "[%d:%d] [Error] ";
    break;
  case LogLevel::WARNING:
    _level = "[%d:%d] [Warning] ";
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

  this->time.push_back(time_stamp);
  this->data.push_back(curr_data);
  this->id.push_back(curr_id);
  this->group_id.push_back(0);
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

  if (this->id.size() != this->data.size()) {
    this->log(ERROR, "write_data: Data size and ID size do not match?");
    delete[] file_name;
    return;
  }

  std::ofstream file;
  file.open(file_name);

  if (!file.is_open()) {
    this->log(ERROR, "write_data: Unable to open file");
    delete[] file_name;
    return;
  }

  for (vector<int>::size_type i = 0; i < id.size(); i++) {
    file << std::fixed << this->time[i] << " " << this->group_id[i] << " "
         << this->id[i] << " " << this->data[i] << '\n';
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

void Log::add_data(int group_id, int curr_id, double curr_data) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double time_stamp = (double)tv.tv_sec + (double)tv.tv_usec / 100000;

  this->time.push_back(time_stamp);
  this->data.push_back(curr_data);
  this->id.push_back(curr_id);
  this->group_id.push_back(group_id);
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