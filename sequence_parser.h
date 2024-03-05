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
	 * @brief ��ʼ���������ж��ļ��Ƿ��ܴ򿪣������ļ��������ļ��Ƿ��Ӧ
	 * @param indexFile leveldb�����ļ�
	 * @return 0
	*/
	std::string init(const std::string& indexFile, bool hashCheck = false);

	std::tuple<size_t, size_t> parseSeqIdValue(std::string seqIdValue);

	std::string readSequenceFromFasta(const std::string& fastaFile, std::tuple<size_t, size_t> sequenceInfo);

	/**
	 * @brief ���������ļ���ȡָ��seqid�����л�����Ƭ��
	 * @param indexFile �����ļ�·��
	 * @param seqId Ŀ������id
	 * @param withDesc �Ƿ����������Ϣ
	 * @param lineWidth �п�
	 * @param outputFile ����ļ�·��
	 * @return ��ȡ��������
	*/
	std::string getSequenceById(
		const std::string& indexFile,
		std::string seqId,
		bool withDesc,
		size_t lineWidth = 60,
		const std::string& outputFile = ""
	);

	/**
	 * @brief ���������ļ�������id�б�������ȡ����
	 * @param indexFile �����ļ�·��
	 * @param seqIdList txt�ļ���������ȡ����id�б�
	 * @param withDesc �Ƿ����������Ϣ
	 * @return ������ȡ��������
	*/
	std::string getSequenceByIdList(
		const std::string& indexFile,
		const std::string& seqIdList,
		bool withDesc
	);

	/**
	 * @brief ������д������ļ�
	 * @param sequence Ҫд�������
	 * @param outputFile ����ļ�·��
	 * @param linewidth ������е��п�
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