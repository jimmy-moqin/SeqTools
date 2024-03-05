#include "sequence_parser.h"
#include "basetypes.h"

std::string SeqParser::init(const std::string& indexFile, bool hashCheck) {
	// 默认hash校验不打开
	// Step 1: Check if indexFile is provided
	if (indexFile.empty()) {
		std::cerr << "Error: No input index file provided." << std::endl;
		return "Error: No input index file provided.";
	}

	// Try to open LevelDB file

	options.create_if_missing = false;

	leveldb::Status status = leveldb::DB::Open(options, indexFile, &db);

	if (!status.ok()) {
		std::cerr << "Error opening LevelDB file: " << status.ToString() << std::endl;
		delete db;
		return "Error opening LevelDB file.";
	}

	// Step 2: Get hash and abspath values from LevelDB
	std::string hashValue;
	std::string abspathValue;

	status = db->Get(leveldb::ReadOptions(), "hash", &hashValue);
	if (!status.ok()) {
		std::cerr << "Error getting 'hash' value from LevelDB: " << status.ToString() << std::endl;
		delete db;
		return "Error getting 'hash' value from LevelDB.";
	}

	status = db->Get(leveldb::ReadOptions(), "abspath", &abspathValue);
	if (!status.ok()) {
		std::cerr << "Error getting 'abspath' value from LevelDB: " << status.ToString() << std::endl;
		delete db;
		return "Error getting 'abspath' value from LevelDB.";
	}

	// Step 3: Open and calculate hash of the fasta file
	this->fastaFilePath = abspathValue;
	this->fastaFile.open(fastaFilePath, std::ios::in);
	if (!fastaFile.is_open()) {
		std::cerr << "Error opening fasta file: " << abspathValue << std::endl;
		delete db;
		return "Error opening fasta file.";
	}

	if (hashCheck) {
		std::string calculatedHash = Utils::calculateFileHash(fastaFile);

		// Compare the calculated hash with the stored hash
		if (calculatedHash == hashValue) {
			std::string ret = "OK";
			return ret;
		}
		else {
			std::cerr << calculatedHash << "//" << hashValue << std::endl;
			std::cerr << "Error: Hash mismatch for fasta file." << std::endl;
			return "Error: Hash mismatch for fasta file.";
		}
	}
	else {
		std::string ret = "OK";
		return ret;
	}
	
}

std::tuple<size_t, size_t> SeqParser::parseSeqIdValue(std::string seqIdValue) {
	size_t commaPos = seqIdValue.find(',', 0);
	if (commaPos == std::string::npos) {
		throw std::runtime_error("Error parsing seqId value: " + seqIdValue);
	}
	size_t offset = std::stoul(seqIdValue.substr(0, commaPos));
	size_t length = std::stoul(seqIdValue.substr(commaPos + 1));
	return std::make_tuple(offset, length);
}

std::string SeqParser::readSequenceFromFasta(const std::string& fastaFile, std::tuple<size_t, size_t> sequenceInfo) {
	size_t offset, length;
	offset = std::get<0>(sequenceInfo);
	length = std::get<1>(sequenceInfo);
	std::ifstream file(fastaFile, std::ios::binary);

	if (!file.is_open()) {
		std::cerr << "Error opening fasta file: " << fastaFile << std::endl;
		return "";
	}

	// Set the file cursor to the specified offset
	file.seekg(offset);
	// Read the sequence of the specified length
	std::string sequence;
	sequence.resize(length);
	file.read(&sequence[0], length);

	file.close();

	return sequence;
}

std::string SeqParser::getSequenceById(
	const std::string& indexFile,
	std::string seqId,
	bool withDesc,
	size_t lineWidth,
	const std::string& outputFile)
{
	std::string ret = this->init(indexFile);
	if (ret == "OK") {
		// 从数据库中获取key为seqId的value
		std::string seqValue;
		db->Get(leveldb::ReadOptions(), seqId, &seqValue);
		std::tuple<size_t, size_t> seqInfo = this->parseSeqIdValue(seqValue);
		std::string seq = this->readSequenceFromFasta(this->fastaFilePath, seqInfo);
		
		Fasta seqFasta(seqId,seq); // 构造Fasta对象
		
		if (outputFile.empty()) {
			return seq;
		}
		else {
			this->writeSequenceToFile(seqFasta, outputFile, lineWidth);
			return "OK";
		}
	}
	else {
		return ret;
	}
}

void SeqParser::writeSequenceToFile(Fasta sequence, const std::string& outputFile, size_t linewidth)
{
	std::string formattedSeq = this->formatSeq(sequence, linewidth);
	std::ofstream file(outputFile);
	file << formattedSeq;
	file.close();
}

std::string SeqParser::formatSeq(Fasta sequence, size_t linewidth) {
	// Remove all newline characters from the sequence
	std::string seqWithoutNewlines  = sequence.getSequence();

	seqWithoutNewlines.erase(std::remove(seqWithoutNewlines.begin(), seqWithoutNewlines.end(), '\n'), seqWithoutNewlines.end());

	// Format seq content based on linewidth
	std::stringstream formattedSeq;
	formattedSeq << sequence.getHeader() << std::endl;
	for (size_t i = 0; i < seqWithoutNewlines.size(); i += linewidth) {
		formattedSeq << seqWithoutNewlines.substr(i, linewidth) << std::endl;
	}
	return formattedSeq.str();
}

//int main_() {
//	SeqParser seqparer;
//	seqparer.getSequenceById(
//		"H:\\PySMS\\PySMS\\SeqenceTools\\CDC_ccs.ldb",
//		"m64144_201111_150747/100007939/ccs",
//		false,
//		60,
//		"test.fasta"
//	);
//}