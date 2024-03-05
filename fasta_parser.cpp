#include "fasta_parser.h"

/**
 * @brief ��ȡһ���ļ����ļ���С����λ�ֽ�
 * @param filename �ļ�·��������
 * @return �ļ���С
*/
std::streamsize FastaParser::getFileSize(const std::string& filename) {
	try {
		std::filesystem::path filePath = filename;
		std::streamsize fileSize = std::filesystem::file_size(filePath);
		return fileSize;
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "Error accessing file: " << e.what() << std::endl;
		return -1; // ���ش���ֵ
	}
}


/**
 * @brief ����һ�����ʼ�����������ڴ�fasta�ļ����½���Ӧ��index levelDB���ݿ�
 * @return 1 ���� 0 ����
 */
int FastaParser::createIndex(const std::string& fafp, const std::optional<std::string>& dbfp ) {
	fasta_file_path = fafp;

	if (dbfp.has_value()) {
		leveldb_path = *dbfp;
	}
	else {
		// ���δ�ṩ LevelDB �ļ�·������Ĭ��ʹ�� Fasta �ļ�·������
		// ���� fasta �ļ���������� ldb �ļ���
		std::filesystem::path fsPath(fasta_file_path);
		leveldb_path = fsPath.stem().string() + ".ldb";
	}
	
	// ��fasta�ļ�
	fasta_file.open(fasta_file_path, std::ios::in);
	if (!fasta_file.is_open()) {
		perror("Error opening fasta file");
		return 1;
	}
	// ��ȡfasta�ļ����ֽ���
	this->fasta_file_size = this->getFileSize(fasta_file_path);

	XXH3_64bits_reset(&xxhash_state);  // �ļ���ϣ����״̬��ʼ��
	// �򿪻��½����ݿ�
	leveldb::Options options;
	options.create_if_missing = true;  // ������ݿ��ļ������ڣ����½�

	// �ж����ݿ��ļ��Ƿ����
	if (isDatabaseExists(leveldb_path)) {
		// ������ݿ��ļ�����
		std::cout << "Database is existed." << std::endl;

		leveldb::Status status = leveldb::DB::Open(options, leveldb_path, &db);  // �����ݿ�
		// ���ݴ򿪽��������
		if (!status.ok()) {
			std::cerr << "Error opening LevelDB: " << status.ToString() << std::endl;
			return 1;
		}
		// �����ݿ��л�ȡ��Ӧfasta�ļ���hashֵ��key=hash
		std::string key = "hash";
		std::string valuehash;

		status = db->Get(leveldb::ReadOptions(), key, &valuehash);

		if (!status.ok()) {
			// ��ȡʧ��
			std::cerr << "Error getting hash from LevelDB: " << status.ToString() << std::endl;
			valuehash = "";  // ���ؿ��ַ�����ʾ��ȡʧ��
		}
		else {
			// ����ܻ�ȡ��hashֵ������֤�����������ļ���hashֵ�Ƿ���֮���
			std::string filehash = Utils::calculateFileHash(fasta_file);
			if (valuehash == filehash) {
				std::cout << valuehash << "//" << filehash << std::endl;
			}
			else {
				// ������������½�������
				std::cout << valuehash << "//" << filehash << std::endl;
				this->indexfa();
			}
		}
	}
	else {
		// ������������½�
		leveldb::Status status = leveldb::DB::Open(options, leveldb_path, &db);  // �����ݿ�
		// ���ݴ򿪽��������
		if (!status.ok()) {
			std::cerr << "Error opening LevelDB: " << status.ToString() << std::endl;
			return 1;
		}
		// ��������
		this->indexfa();
	}

	return 0;
}


/**
 * @brief ����һ��fatsa�ļ�����������
 * @return 1 ���� 0 ����
 */
int FastaParser::indexfa() {
	
	const size_t BLOCK_SIZE = 16384;      // ÿ�ζ�ȡ���ļ���Ĵ�С
	char* buffer = new char[BLOCK_SIZE];  // �ڶ��Ͽ����ڴ�

	// ָ�����ļ��ĳ�ʼƫ����
	size_t global_seek = 0;
	this->fasta_file.seekg(global_seek, std::ios::beg);

	leveldb::WriteBatch batch;     // ����levelDB������
	const int BATCH_SIZE = 10000;  // ��������Ŀ

	int count = 0;       // ��������������������
	std::string seq_id;  // ����id
	// ��Ϊ�յ��ݴ�Ԫ�飬�����ݴ���һ�ֶ�ȡ����seqid���׸������ƫ����
	std::optional<std::tuple<std::string, size_t>> tempTuple;

	while (1) {
		this->fasta_file.read(buffer, BLOCK_SIZE);
		size_t bytesRead = this->fasta_file.gcount();
		std::string block_content(buffer, bytesRead);

		size_t pos = 0;  // ��ʼ�� > �ַ��ڿ��ڵ�ƫ����
		while (1) {
			pos = block_content.find('>', pos);
			if (pos != std::string::npos) {
				// ������ҵ� >
				// ˵����ȡ���ļ������ٰ���һ��id���ڵ��У���һ������������ᴦ��
				size_t end_pos = block_content.find('\n', pos);
				if (end_pos != std::string::npos) {
					// ������ҵ����з���˵��block�Ķ�ȡ�߽�û�нض�id������
					size_t space_pos = block_content.find(' ', pos);
					if (space_pos != std::string::npos) {
						// ������ҵ��ո��Ǿͻ�ȡ�ո�ǰ���ַ���Ϊid
						seq_id = block_content.substr(pos + 1, space_pos - pos - 1);
						if (tempTuple.has_value()) {
							// ˵�����ǵ�һ�ζ�ȡ
							// ��ȡ��һ�����еĽ�βƫ������ͨ����һ������ > ����λ�ü�1�õ�
							size_t before_sign_pos = global_seek + pos - 1;
							// ���ݴ�Ԫ��ĵ�2λ��ȡ��һ�����е��׸������ƫ���������βƫ��������õ����г��ȣ��������У�
							size_t base_num = before_sign_pos - std::get<1>(*tempTuple);
							// �����ݴ��ַ�����������һ�����е�id����ʼƫ���������г���
							temp_string = temp_string + ',' + std::to_string(base_num);
							// ���ݶ��ŷָ��ַ��������λ�ȡ
							// seqid��ʣ������ݣ��ֱ���Ϊkey��value
							size_t seqid_pos = temp_string.find(',', 0);
							std::string key = temp_string.substr(0, seqid_pos);
							size_t strlenth = temp_string.length();
							std::string value = temp_string.substr(seqid_pos + 1, strlenth);
							// �������ݿ���������
							batch.Put(key, value);
						}
						else {
							// �����һ�ζ�ȡ
							temp_string = std::to_string(global_seek + end_pos + 1);
						}
					}
					else {
						// ����Ҳ����ո񣬾͵����з�ǰ���ַ���Ϊid
						seq_id = block_content.substr(pos + 1, end_pos - pos - 1);
						if (tempTuple.has_value()) {
							// ˵�����ǵ�һ�ζ�ȡ
							// ��ȡ��һ�����еĽ�βƫ������ͨ����һ������ > ����λ�ü�1�õ�
							size_t before_sign_pos = global_seek + pos - 1;
							// ���ݴ�Ԫ��ĵ�2λ��ȡ��һ�����е��׸������ƫ���������βƫ��������õ����г��ȣ��������У�
							size_t base_num = before_sign_pos - std::get<1>(*tempTuple);
							// �����ݴ��ַ�����������һ�����е�id����ʼƫ���������г���
							temp_string = temp_string + ',' + std::to_string(base_num);
							// ���ݶ��ŷָ��ַ��������λ�ȡ
							// seqid��ʣ������ݣ��ֱ���Ϊkey��value
							size_t seqid_pos = temp_string.find(',', 0);
							std::string key = temp_string.substr(0, seqid_pos);
							size_t strlenth = temp_string.length();
							std::string value = temp_string.substr(seqid_pos + 1, strlenth);
							// �������ݿ���������
							batch.Put(key, value);
						}
						else {
							// �����һ�ζ�ȡ
							temp_string =
								seq_id + ',' + std::to_string(global_seek + end_pos + 1);
						}
					}
					// �����ݴ�Ԫ������ݣ��������ݴ��ַ���
					tempTuple = std::make_optional(
						std::make_tuple(seq_id, global_seek + end_pos + 1));
					temp_string =
						seq_id + ',' + std::to_string(global_seek + end_pos + 1);
					// ������һ�β��� > ��λ��
					pos = end_pos + 1;
				}
				else {
					// ���û���ҵ����з���˵���ض����ˣ��ض�λ�ļ���ȡƫ����������ѭ��
					global_seek += pos;
					break;
				}
			}
			else {
				// ����Ҳ��� >
				// ˵����ȡ���ļ�����û��id���ڵ��У���ô�ض�λ�ļ���ȡƫ���������¶�һ���ļ���
				global_seek += bytesRead;
				break;
			}

			count++;

			if (count >= BATCH_SIZE) {
				// һ����д�����ݿ�
				db->Write(leveldb::WriteOptions(), &batch);
				// ������������ͼ�����
				batch.Clear();
				count = 0;
			}
		}

		if (bytesRead < BLOCK_SIZE) {
			// ʵ�ʶ��������ݴ�СС��ָ���ļ����Сʱ��˵�����ļ�ĩβ�����˴���������
			size_t before_sign_pos = global_seek + pos - 1;
			size_t base_num = before_sign_pos - std::get<1>(*tempTuple);
			temp_string = temp_string + ',' + std::to_string(base_num);
			batch.Put(seq_id, temp_string);
			break;
		}
		// �ڲ�ѭ��������ָ���´ζ�ȡ�ļ���ƫ��ֵ
		fasta_file.seekg(global_seek, std::ios::beg);
	}
	// �ļ���ȡ����
	fasta_file.close();

	// ��ʼ�����ļ�У���
	std::cout << "Start calculating the file checksum" << std::endl;
	// ���´�fasta�ļ�
	fasta_file.open(fasta_file_path, std::ios::in);
	if (!fasta_file.is_open()) {
		perror("Error opening fasta file");
		return 1;
	}
	// �����ļ���ȡƫ����
	global_seek = 0;
	this->fasta_file.seekg(global_seek, std::ios::beg);
	std::string hash = Utils::calculateFileHash(this->fasta_file);
	batch.Put("hash", hash);


	// ��ȡfasta�ļ��ľ���·��
	std::filesystem::path absolutePath = std::filesystem::absolute(this->fasta_file_path);
	batch.Put("abspath", absolutePath.string());

	if (count > 0) {
		// ���������û�����㣬Ҳ�������������ڻ������������ݣ��������һ����д��
		db->Write(leveldb::WriteOptions(), &batch);
	}

	cleanup();        // �������������ر�fasta�ļ������ݿ�
	delete[] buffer;  // �ͷ��ڶ��ϵ��ڴ�
	return 0;
}


int FastaParser::read() {
	// �� LevelDB ���ݿ�
	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = false;  // ����Ϊ true ������봴�����ݿ�
	leveldb::Status status = leveldb::DB::Open(options, "CDC_ccs.ldb", &db);

	if (!status.ok()) {
		std::cerr << "Error opening LevelDB: " << status.ToString() << std::endl;
		return 1;
	}

	// ����һ��������
	leveldb::ReadOptions readOptions;
	leveldb::Iterator* it = db->NewIterator(readOptions);

	// ��һ���ı��ļ���д��ģʽ
	std::ofstream outputFile("output.txt");
	if (!outputFile.is_open()) {
		std::cerr << "Error opening output file." << std::endl;
		delete it;
		delete db;
		return 1;
	}

	// �������ݿⲢ�����ֵ�Ե��ı��ļ�
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		std::string key = it->key().ToString();
		std::string value = it->value().ToString();

		// ����ֵ��������ı��ļ�
		outputFile << "Key: " << key << ", Value: " << value << std::endl;
	}

	// ���������Ƿ����
	if (!it->status().ok()) {
		std::cerr << "Iterator error: " << it->status().ToString() << std::endl;
	}

	// �ر��ļ������ݿ⣬�ͷ���Դ
	outputFile.close();
	delete it;
	delete db;

	return 0;
}


//int main_() {
//	clock_t start, finish;
//	double duration;
//
//	start = clock();
//	FastaParser parser;
//	parser.createIndex("G:/CDC_ccs.fasta");
//	finish = clock();
//
//	duration = (double)(finish - start) / CLOCKS_PER_SEC;
//	std::cout << duration << " seconds" << std::endl;
//
//	// parser.read();
//	return 0;
//}

