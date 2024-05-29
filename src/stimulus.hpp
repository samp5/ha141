#ifndef IMAGE
#define IMAGE
#include <climits>
#include <cmath>
#include <stdexcept>
struct Image {
  int width;
  int height;
  double max_latency;

  struct Point {
    float x;
    float y;
    Point(float x, float y) : x(x), y(y){};
  };

  Point center;

  double max_distance;

  Image(int w, int h, double max_latency = 0.025)
      : width(w), height(h), max_latency(max_latency),
        center(float(w - 1) / 2, float(h - 1) / 2),
        max_distance(getDistance(0, 0)){};

  Image(int pixels, double max_latency = 0.025)
      : width(std::sqrt(pixels)), height(std::sqrt(pixels)),
        max_latency(max_latency), center(float(width) / 2, float(height) / 2),
        max_distance(getDistance(0, 0)) {
    if (!isSquare(pixels)) {
      throw std::runtime_error(
          "Error intialzing Image struct with pixel constructor. Need a square "
          "image or use constructor with signature Image(int, int)");
    }
  };

  double getDistance(double x, double y) {
    double distance =
        std::sqrt(std::pow(x - center.x, 2) + std::pow(y - center.y, 2));
    return distance;
  };

  double getLatency(int index) {
    Point pos = calculateCoords(index);
    double distance = getDistance(pos.x, pos.y);
    double latency = distance / max_distance * max_latency;
    return latency;
  }

  Point calculateCoords(int index) {
    int col = index % width;
    int row = (index - col) / width;
    return Point(col, row);
  }

  static std::pair<int, int> bestRectangle(int pixels) {
    int min_per = INT_MAX;
    int w = 0;
    int h = 0;
    for (int i = 1; i < std::ceil(std::sqrt(pixels)) + 1; i++) {
      if (pixels % i == 0) {
        int width = pixels / i;
        int height = i;
        if (2 * (width + height) < min_per) {
          w = width;
          h = height;
        }
      }
    }
    std::pair<int, int> dim = std::make_pair(w, h);
    return dim;
  }

  static bool isSquare(double pixels) {
    if (int(std::sqrt(pixels)) * int(std::sqrt(pixels)) == pixels) {
      return true;
    }
    return false;
  }
};
#endif // !IMAGE
