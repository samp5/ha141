#include "log.hpp"
#include "message.hpp"
#include "network.hpp"
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

pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Stringify Neuron_t.
 *
 *
 * @return String version of Neuron_t
 */
std::string neuronTypeString(Neuron_t type) {
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

/**
 * @brief Main log function.
 *
 * All log functions pass through this main log function
 * before going to the output stream. This function decorates
 * the message with the correct flag and time.
 *
 * @param level LogLevel that decides whether the message is printed
 * @param  os The output stream to be used default is standard output
 */

void Log::log(LogLevel level, const char *message,
              std::ostream &os) { // default output stream is standard output

  if (level > network->getConfig()->DEBUG_LEVEL) {
    return;
  }

  const char *_level;

  switch (level) {
  case LogLevel::ESSENTIAL:
    _level = "-> ";
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
  default:
    _level = "";
    return;
  }

  double time_rn = time();
  int length = snprintf(nullptr, 0, _level, time_rn);
  char *message_prefix = new char[length + 1];

  snprintf(message_prefix, length + 1, _level, time_rn);
  // pthread_mutex_lock(&log_mutex);
  os << message_prefix << message << '\n';
  // pthread_mutex_unlock(&log_mutex);
  delete[] message_prefix;
}

/**
 * @brief Add data to Log::log_data.
 *
 *
 * @param data LogData instance
 */
void Log::addData(LogData *data) {
  pthread_mutex_lock(&data_mutex);
  this->log_data.push_back(data);
  pthread_mutex_unlock(&data_mutex);
}

/**
 * @brief Write data to a log file.
 *
 * Write data to log file specified in RuntimConfig
 *
 * @param filename Name of file
 */
void Log::writeData() {

  log(ESSENTIAL, "Writing data to file...");
  namespace fs = std::filesystem;

  struct timeval tv;
  gettimeofday(&tv, NULL);

  fs::path parent;
  std::string name;
  if (network->getConfig()->OUTPUT_FILE == "") {
    name = std::to_string(tv.tv_sec - 1'710'000'000);
  } else {
    name = network->getConfig()->OUTPUT_FILE;
  }

  parent = "./logs/" + name;
  fs::create_directory(parent);
  std::string file_name = "./logs/" + name + "/" + name + ".log";

  std::ofstream file;
  file.open(file_name);

  if (!file.is_open()) {
    this->log(ERROR, "write_data: Unable to open file");
    return;
  }

  for (LogData *log_data : this->log_data) {
    file << std::fixed << log_data->group_id << " " << log_data->neuron_id
         << " " << neuronTypeString((Neuron_t)log_data->neuron_type) << " "
         << log_data->timestamp << " " << log_data->potential << " "
         << messageTypeToString(log_data->message_type) << " "
         << log_data->stimulus_number << '\n';
  }

  file.close();
  this->logConfig(name);
}

/**
 * @brief Log a message about the state of a neuron.
 *
 */
void Log::groupNeuronState(LogLevel level, const char *message, int group_id,
                           int id) {
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

/**
 * @brief Log a message about the value of a neuron.
 *
 */
void Log::neuronValue(LogLevel level, const char *message, int group_id, int id,
                      double accumulated) {
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

/**
 * @brief Log a message about the interaction between 2 neurons.
 *
 */
void Log::neuronInteraction(LogLevel level, const char *message, int group_id1,
                            int id1, int group_id2, int id2) {
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

/**
 * @brief Log a message about the state of a group.
 *
 */
void Log::state(LogLevel level, const char *message, int group_id) {
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

/**
 * @brief Log a message about the type of a neuron.
 *
 */
void Log::neuronType(LogLevel level, const char *message, int group_id, int id,
                     const char *type) {

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

/**
 * @brief Log a message
 *
 */
void Log::print(const char *message, bool newline, std::ostream &os) {
  if (newline)
    os << message << '\n';
  else
    os << message;
}

/**
 * @brief Update the Log::offset value
 *
 * @param value Value to be added
 *
 */
void Log::addOffset(double value) { this->offset += value; }

double Log::time() {

  hr_clock::time_point now = hr_clock::now();

  duration time_span = std::chrono::duration_cast<duration>(now - this->start);
  return time_span.count() - this->offset;
}

/**
 * @brief Log an integer
 *
 */
void Log::value(LogLevel level, const char *message, int value) {
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

/**
 * @brief Log a string
 *
 */
void Log::string(LogLevel level, const char *message, const char *string) {
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

/**
 * @brief Copy the configuration file used for a run into the log folder
 *
 */
void Log::logConfig(const std::string &name) {

  namespace fs = std::filesystem;
  fs::path config_source = network->getConfig()->CONFIG_FILE;

  fs::path config_target = "./logs/" + name + "/" + name + ".toml";

  fs::copy_file(config_source, config_target);
}

const char *Log::activeStatusString(bool active) {
  if (active) {
    const char *active = "active"; // NOLINT
    return active;
  } else {
    const char *inactive = "inactive"; // NOLINT
    return inactive;
  }
}

LogLevel Log::debugLevelString(std::string level) {
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
  log(WARNING, "\"level\" does not match any available options: setting "
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
  }
  return ret;
}

Log::~Log() {
  for (auto d : log_data) {
    delete d;
  }
}
