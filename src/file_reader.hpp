#ifndef FILEREADER
#define FILEREADER
#include <fstream>
#include <limits>

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

private:
  std::ifstream file;
  std::streampos current_position;
};

#endif // !FILEREADER
