#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <xxhash/xxh3.h>
#include <sstream>

class Utils {
public:
    static std::string calculateFileHash(std::ifstream& file, size_t blockSize = 16384);

    static std::vector<std::string> split(const std::string& input, char delimiter);
};

#endif // UTILS_H
