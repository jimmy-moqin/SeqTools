#pragma once
#include <string>

class Fasta {
private:
	std::string header;
	std::string sequence;

public:
	// ���캯�������ڳ�ʼ������
	Fasta(const std::string& header, const std::string& sequence) : header(header), sequence(sequence) {}

	// ��Ա���������ڻ�ȡͷ����Ϣ
	std::string getHeader() const {
		return header;
	}

	// ��Ա���������ڻ�ȡ������Ϣ
	std::string getSequence() const {
		return sequence;
	}

	// ��Ա��������������ͷ����Ϣ
	void setHeader(const std::string& newHeader) {
		header = newHeader;
	}

	// ��Ա��������������������Ϣ
	void setSequence(const std::string& newSequence) {
		sequence = newSequence;
	}

	// ��Ա�������������FASTA��ʽ�ַ���
	std::string toFastaString() const {
		return ">" + header + "\n" + sequence + "\n";
	}
};

