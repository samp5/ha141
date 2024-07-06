#include "file_reader.hpp"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

void AdjListParser::parseLine(const std::string &line, AdjList &mutAdjList) {
  switch (format) {
  case AdjListFormat::UnknownFormat: {
    lg.log(ERROR, "AdjsListParse::parseLine : Attempt to parse line with "
                  "AdjListFormat::UnknownFormat");
    break;
  }
  case AdjListFormat::Standard: {
    std::stringstream stream(line);
    int from;
    stream >> from;
    mutAdjList[from] = {};
    while (!stream.eof()) {
      int to;
      stream >> to;
      mutAdjList[from].push_back(to);
    }
    break;
  }
  case AdjListFormat::DirectedGrid: {
    if (!numColumns.has_value()) {
      lg.log(LogLevel::ERROR, "AdjListParser::parseLine : Attempt to parse "
                              "AdjListFormat::DirectedGrid with an "
                              "AdjListParser::numColumns optional");
    }
    // base substr is (x, y)
    size_t cursor = line.find('(', 0);
    size_t match = line.find(')', 0);
    std::string point = line.substr(cursor + 1, (match - 1) - cursor);
    size_t comma = point.find(',', 0);
    std::string shouldBeX = point.substr(0, comma);
    int x = std::stoi(point.substr(0, comma));
    int y = std::stoi(point.substr(comma + 2, std::string::npos));
    int from = y * numColumns.value_or(0) + x;
    mutAdjList[from] = {};

    cursor = line.find('(', match);
    while (cursor != std::string::npos) {
      size_t parenMatch = line.find(')', cursor);
      std::string point = line.substr(cursor + 1, (parenMatch - 1) - cursor);
      size_t comma = point.find(',', 0);
      int x = std::stoi(point.substr(0, comma));
      int y = std::stoi(point.substr(comma + 1, std::string::npos));
      int index = y * numColumns.value() + x;
      mutAdjList[from].push_back(index);
      cursor = line.find('(', parenMatch);
    }
    break;
  }
  }
}
void AdjListParser::openFile(std::ifstream &file) {
  file.open(file_path);

  // error check
  if (!file.is_open()) {
    lg.log(ERROR,
           "AdjListParser::AdjListParser : Could not open file... Quitting");
    exit(1);
  }
}
void AdjListParser::assignFormat() {
  std::ifstream file;
  openFile(file);

  std::string line;
  std::getline(file, line);
  while (!line.size() || line.at(0) == '#') {
    std::getline(file, line);
    // error check
    if (file.fail()) {
      lg.log(ERROR,
             "AdjListParser::AdjListParser : Error reading file... Quitting");
      exit(1);
    }
  }
  char firstChar = line.at(0);
  if (std::isdigit(firstChar)) {
    format = AdjListFormat::Standard;
  } else if (firstChar == '(') {
    format = AdjListFormat::DirectedGrid;
    numColumns = maxColumn();
  } else {
    format = AdjListFormat::UnknownFormat;
  }
}
std::istream &AdjListParser::ignoreLine(std::ifstream &in,
                                        std::ifstream::pos_type &pos) {
  pos = in.tellg();
  return in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
std::string AdjListParser::getLastLine() {
  // open file
  std::ifstream file;
  openFile(file);

  std::ifstream::pos_type pos = file.tellg();
  std::ifstream::pos_type lastPos;
  while (file >> std::ws && ignoreLine(file, lastPos)) {
    pos = lastPos;
  }
  file.clear();
  file.seekg(pos);
  std::string lastLine;
  std::getline(file, lastLine);
  file.close();
  return lastLine;
}
int AdjListParser::maxColumn() {
  if (format != AdjListFormat::DirectedGrid) {
    lg.log(LogLevel::ERROR,
           "AdjListParser::maxColumn() attempted to read file format with type "
           "other than AdjListFormat::DirectedGrid... quitting");
    exit(1);
  }
  std::stringstream stream = std::stringstream(getLastLine());
  char discard;
  int x = 0;
  stream >> discard >> x;

  if (!x) {
    lg.log(ERROR, "AdjListParser::parseAdjList : Number of nodes is 0");
  }

  return x + 1;
}

std::unordered_map<int, std::vector<int>> AdjListParser::parseAdjList() {
  assignFormat();
  if (format == AdjListFormat::UnknownFormat) {
    lg.log(LogLevel::ERROR,
           "AdjListParser::parseAdjList() failed to assignFormat "
           "... quitting");
    exit(1);
  }
  AdjList AdjacencyListRet;
  std::ifstream file;
  openFile(file);
  std::string line;
  while (std::getline(file, line)) {
    // error check
    if (file.fail()) {
      lg.log(ERROR,
             "AdjListParser::AdjListParser : Error reading file... Quitting");
      exit(1);
    }
    if (!line.size() || line.at(0) == '#') {
      continue;
    }
    parseLine(line, AdjacencyListRet);
  }
  file.close();
  return AdjacencyListRet;
}
