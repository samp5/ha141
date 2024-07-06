#include "file_reader.hpp"
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>

bool testAdjListParserParseAdjList() {
  bool pass = true;
  std::ofstream writeFile;
  writeFile.open("./parserTest.txt");
  writeFile << "#networkGen.py\n";
  writeFile << "# GMT Sat Jul  6 14:59:20 2024\n";
  writeFile << "#\n";
  writeFile << "0 2 3\n";
  writeFile << "1 2 3\n";
  writeFile << "2\n";
  writeFile << "3\n";
  writeFile.close();

  typedef std::unordered_map<int, std::vector<int>> AdjList;
  AdjList expected = {
      {0, {2, 3}},
      {1, {2, 3}},
      {2, {}},
      {3, {}},
  };
  AdjListParser parser("./parserTest.txt");
  auto list = parser.parseAdjList().adjList;
  for (auto pair : list) {
    if (expected.find(pair.first) == expected.end()) {
      parser.lg.value(ERROR,
                      "NodeID %d was in the output but not in expected ouput",
                      pair.first);
      pass = false;
    }
    if (expected[pair.first].size() != pair.second.size()) {
      parser.lg.value(ERROR, "Expected size was %d",
                      static_cast<int>(expected[pair.first].size()));
      parser.lg.value(ERROR, "Got size was %d",
                      static_cast<int>(pair.second.size()));
      pass = false;
    }
    for (std::vector<int>::size_type i = 0;
         i < std::min(expected[pair.first].size(), pair.second.size()); i++) {
      if (expected[pair.first].at(i) != pair.second.at(i)) {
        parser.lg.value(ERROR, "Expected value was %d",
                        expected[pair.first].at(i));
        parser.lg.value(ERROR, "Got value %d", pair.second.at(i));
        pass = false;
        break;
      }
    }
  }

  writeFile.open("./parserTest.txt");
  writeFile << "#networkGen.py\n";
  writeFile << "# GMT Sat Jul  6 14:59:20 2024\n";
  writeFile << "#\n";
  writeFile << "(0, 0) (0, 1) (1, 0)\n";
  writeFile << "(0, 1) (0, 0) (1, 1)\n";
  writeFile << "(1, 0) (0, 0) (1, 1) (0, 1)\n";
  writeFile << "(1, 1) (0, 1) (1, 0) (1, 1)\n";
  writeFile.close();
  AdjList expected1 = {
      {0, {2, 1}},
      {2, {0, 3}},
      {1, {0, 3, 2}},
      {3, {2, 1, 3}},
  };
  AdjListParser parser1("./parserTest.txt");
  AdjList list1 = parser1.parseAdjList().adjList;
  for (auto pair : list1) {
    if (expected1.find(pair.first) == expected.end()) {
      parser.lg.value(ERROR,
                      "NodeID %d was in the output but not in expected ouput",
                      pair.first);
      pass = false;
    }
    if (expected1[pair.first].size() != pair.second.size()) {
      parser.lg.value(ERROR, "Expected size was %d",
                      static_cast<int>(expected1[pair.first].size()));
      parser.lg.value(ERROR, "Got size was %d",
                      static_cast<int>(pair.second.size()));
      pass = false;
    }
    for (std::vector<int>::size_type i = 0; i < expected1[pair.first].size();
         i++) {
      if (expected1[pair.first].at(i) != pair.second.at(i)) {
        parser.lg.value(ERROR, "Expected value was %d",
                        expected1[pair.first].at(i));
        parser.lg.value(ERROR, "Got value %d", pair.second.at(i));
        pass = false;
        break;
      }
    }
  }
  std::filesystem::remove("./parserTest.txt");
  return pass;
}

typedef struct _function {
  bool (*func)();
  std::string name;
} Test;
int main() {
  std::vector<Test> tests = {
      {testAdjListParserParseAdjList, "AdjListParser::parseAdjList"}};
  for (auto f : tests) {
    if (!f.func()) {
      std::cout << " " << f.name << " Failed \n";
    } else {
      std::cout << " " << f.name << " Passed! \n";
    }
  }
}
