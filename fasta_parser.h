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
 * @brief ���ڽ���fasta�ļ�����
 */
class FastaParser {
public:
	/**
	 * @brief ���캯�������ڳ�ʼ��FastaParser�Ķ���
	 *
	 * @param fafp : fasta�ļ����ļ�·��
	 * @param dbfp : ���ݿ��ļ����ļ�·��(��ѡ)��Ĭ�ϸ��� fasta �ļ�������
	 *
	 */

	// ��Ա����
	int createIndex(const std::string& fafp, const std::optional<std::string>& dbfp = std::nullopt);
	int indexfa();
	int read();
	std::streamsize getFileSize(const std::string& filename);
	/**
	 * @brief �ڴ����ݿ�֮ǰ����ļ��Ƿ����
	 * @param leveldbPath LevelDB �ļ���·��
	 * @return true ����ļ����ڣ�false ����ļ�������
	 */
	bool isDatabaseExists(const std::string& leveldbPath) {
		std::filesystem::path dbPath(leveldbPath);
		return std::filesystem::exists(dbPath);
	}

	/**
	 * @brief ����һ���������������ڹر�fasta�ļ��Ͷ�Ӧ��index levelDB���ݿ�
	 * @return void
	 */
	void cleanup() {
		fasta_file.close();
		delete db;
	}

private:
	std::string fasta_file_path;  // fasta�ļ���·��
	size_t fasta_file_size;       // fasta�ļ��ֽ���
	std::string leveldb_path;     // ���ݿ��ļ���·�����Զ�����
	std::ifstream fasta_file;     // �򿪵�fasta�ļ���
	leveldb::DB* db;              // ���ݿ����
	std::string temp_string;      // �ݴ��ַ���
	XXH3_state_t xxhash_state;    // �ļ���У��hashֵ
};
#endif