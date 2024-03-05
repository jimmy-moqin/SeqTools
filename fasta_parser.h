#ifndef FASTA_PARSER_H
#define FASTA_PARSER_H

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <xxhash/xxh3.h>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <variant>

#include "utils.h"

/**
 * @brief 用于解析fasta文件的类
 */
class FastaParser {
public:
	/**
	 * @brief 构造函数，用于初始化FastaParser的对象。
	 *
	 * @param fafp : fasta文件的文件路径
	 * @param dbfp : 数据库文件的文件路径(可选)，默认根据 fasta 文件名构造
	 *
	 */

	// 成员函数
	int createIndex(const std::string& fafp, const std::optional<std::string>& dbfp = std::nullopt);
	int indexfa();
	int read();
	std::streamsize getFileSize(const std::string& filename);
	/**
	 * @brief 在打开数据库之前检查文件是否存在
	 * @param leveldbPath LevelDB 文件的路径
	 * @return true 如果文件存在，false 如果文件不存在
	 */
	bool isDatabaseExists(const std::string& leveldbPath) {
		std::filesystem::path dbPath(leveldbPath);
		return std::filesystem::exists(dbPath);
	}

	/**
	 * @brief 这是一个类清理函数，用于关闭fasta文件和对应的index levelDB数据库
	 * @return void
	 */
	void cleanup() {
		fasta_file.close();
		delete db;
	}

private:
	std::string fasta_file_path;  // fasta文件的路径
	size_t fasta_file_size;       // fasta文件字节数
	std::string leveldb_path;     // 数据库文件的路径，自动构造
	std::ifstream fasta_file;     // 打开的fasta文件流
	leveldb::DB* db;              // 数据库对象
	std::string temp_string;      // 暂存字符串
	XXH3_state_t xxhash_state;    // 文件块校验hash值
};
#endif