#ifndef FILEREADER
#define FILEREADER
#include "log.hpp"
#include <fstream>
#include <limits>
#include <optional>
#include <unordered_map>
#include <vector>

/* The two output formats of the AdjList include:
 *
 * STANDARD
 * #networkGen.py
 * # GMT Sat Jul  6 14:59:20 2024
 * #
 * 0 2 3
 * 1 2 3
 * 2
 * 3
 *
 * DIRECTED_GRID
 * #networkGen.py
 * # GMT Sat Jul  6 14:59:20 2024
 * #
 * (0, 0) (0, 1) (1, 0)
 * (0, 1) (0, 0) (1, 1)
 * (1, 0) (0, 0) (1, 1) (0, 1)
 * (1, 1) (0, 1) (1, 0) (1, 1)
 *
 */

enum AdjListFormat { UnknownFormat, DirectedGrid, Standard };
class AdjListParser {
private:
  std::string file_path;
  std::streampos current_position;
  AdjListFormat format;
  std::optional<int> numColumns;

public:
  Log lg;
  using AdjList = std::unordered_map<int, std::vector<int>>;
  using AdjListInfo = struct _adjListInfo {
    AdjList adjList;
    int numberNodes;
    int numberEdges;
  };
  int parseLine(const std::string &line, AdjList &mutAdjList);
  void openFile(std::ifstream &file);
  AdjListParser(const std::string &file_path)
      : file_path(file_path), format(AdjListFormat::UnknownFormat),
        numColumns({}), lg() {}
  std::istream &ignoreLine(std::ifstream &in, std::ifstream::pos_type &pos);
  std::string getLastLine();
  int maxColumn();
  AdjListInfo parseAdjList();
  void assignFormat();
};
class InputFileReader {
public:
  /**
   * @brief Constructor for FileReader.
   *
   * @param file_path Input file file path
   * @param start_pos Line to start reading
   */
  InputFileReader(const std::string &file_path, int start_pos)
      : file(file_path) {
    for (int i = 0; i < start_pos; i++) {
      file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    current_position = file.tellg();
  }

  /**
   * @brief Retrieve the next line of the input file.
   *
   * Return the next line of the input file
   *
   * @return string containing line
   */
  std::string nextLine() {
    std::string line;
    std::getline(file, line);
    current_position = file.tellg();
    return line;
  }

  void setToLine(int targetLine) {
    file.clear();
    file.seekg(0);
    for (int i = 0; i < targetLine; i++) {
      file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    current_position = file.tellg();
  }

private:
  std::ifstream file;
  std::streampos current_position;
};

#endif // !FILEREADER
