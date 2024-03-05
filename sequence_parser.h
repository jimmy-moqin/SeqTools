#ifndef SEQUENCE_PARSER_H
#define SEQUENCE_PARSER_H
#pragma once
#include <iostream>
#include <leveldb/db.h>
#include <leveldb/status.h>
#include <fstream>
#include <sstream>
#include "utils.h"
#include <algorithm>



class SeqParser {
public:
	/**
	 * @brief 初始化函数，判断文件是否能打开，索引文件和序列文件是否对应
	 * @param indexFile leveldb索引文件
	 * @return 0
	*/
	std::string init(const std::string& indexFile, bool hashCheck = false);

	std::tuple<size_t, size_t> parseSeqIdValue(std::string seqIdValue);

	std::string readSequenceFromFasta(const std::string& fastaFile, std::tuple<size_t, size_t> sequenceInfo);

	/**
	 * @brief 根据索引文件获取指定seqid的序列或序列片段
	 * @param indexFile 索引文件路径
	 * @param seqId 目标序列id
	 * @param withDesc 是否包含描述信息
	 * @param lineWidth 行宽
	 * @param outputFile 输出文件路径
	 * @return 获取到的序列
	*/
	std::string getSequenceById(
		const std::string& indexFile,
		std::string seqId,
		bool withDesc,
		size_t lineWidth = 60,
		const std::string& outputFile = ""
	);

	/**
	 * @brief 根据索引文件和序列id列表批量获取序列
	 * @param indexFile 索引文件路径
	 * @param seqIdList txt文件，批量获取序列id列表
	 * @param withDesc 是否包含描述信息
	 * @return 批量获取到的序列
	*/
	std::string getSequenceByIdList(
		const std::string& indexFile,
		const std::string& seqIdList,
		bool withDesc
	);

	/**
	 * @brief 将序列写入输出文件
	 * @param sequence 要写入的序列
	 * @param outputFile 输出文件路径
	 * @param linewidth 输出序列的行宽
	*/
	void writeSequenceToFile(
		Fasta sequence,
		const std::string& outputFile,
		size_t linewidth
	);

	std::string formatSeq(Fasta sequence, size_t linewidth);


private:
	std::ifstream fastaFile;
	std::string fastaFilePath;
	leveldb::DB* db;
	leveldb::Options options;
};


#include <iostream>
#include <string>


#endif