#pragma once
#include <string>

class Fasta {
private:
	std::string header;
	std::string sequence;

public:
	// 构造函数，用于初始化对象
	Fasta(const std::string& header, const std::string& sequence) : header(header), sequence(sequence) {}

	// 成员函数，用于获取头部信息
	std::string getHeader() const {
		return header;
	}

	// 成员函数，用于获取序列信息
	std::string getSequence() const {
		return sequence;
	}

	// 成员函数，用于设置头部信息
	void setHeader(const std::string& newHeader) {
		header = newHeader;
	}

	// 成员函数，用于设置序列信息
	void setSequence(const std::string& newSequence) {
		sequence = newSequence;
	}

	// 成员函数，用于输出FASTA格式字符串
	std::string toFastaString() const {
		return ">" + header + "\n" + sequence + "\n";
	}
};

