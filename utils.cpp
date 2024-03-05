#include "utils.h"

/**
 * @brief ������ļ��� XXH3 ��ϣֵ��
 * @param filePath �ļ�·��
 * @param blockSize ÿ�ζ�ȡ�ļ���Ĵ�С (Ĭ��Ϊ 66536)
 * @return �ļ��� XXH3 ��ϣֵ
 */
std::string Utils::calculateFileHash(std::ifstream& file, size_t blockSize) {
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return "";
    }

    XXH3_state_t xxhash3_state;
    XXH3_64bits_reset(&xxhash3_state);

    std::vector<char> buffer(blockSize);
    size_t bytesRead;
    file.seekg(0);
    while ((bytesRead = file.read(buffer.data(), blockSize).gcount()) > 0) {
        XXH3_64bits_update(&xxhash3_state, buffer.data(), bytesRead);
    }

    uint64_t checksum = XXH3_64bits_digest(&xxhash3_state);
    return std::to_string(checksum);
}

std::vector<std::string> Utils::split(const std::string& input, char delimiter) {
    std::vector<std::string> result;
    std::istringstream stream(input);
    std::string token;

    while (std::getline(stream, token, delimiter)) {
        if (!token.empty()) {  // Check if token is not empty before adding to result
            result.push_back(token);
        }
    }

    return result;
}
