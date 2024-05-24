#include <filesystem>
#include <fstream>
#include <iostream>
class FileReader {
public:
  FileReader(const std::string &file_path, int start_pos) : file(file_path) {
    for (int i = 0; i < start_pos; i++) {
      file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    current_position = file.tellg();
  }

  std::string nextLine() {
    std::string line;
    std::getline(file, line);
    current_position = file.tellg();
    return line;
  }

  bool endOfFile() const { return file.eof(); }

private:
  std::ifstream file;
  std::streampos current_position;
};
