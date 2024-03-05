#include "fasta_parser.h"

/**
 * @brief 获取一个文件的文件大小，单位字节
 * @param filename 文件路径的引用
 * @return 文件大小
*/
std::streamsize FastaParser::getFileSize(const std::string& filename) {
	try {
		std::filesystem::path filePath = filename;
		std::streamsize fileSize = std::filesystem::file_size(filePath);
		return fileSize;
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "Error accessing file: " << e.what() << std::endl;
		return -1; // 返回错误值
	}
}


/**
 * @brief 这是一个类初始化函数，用于打开fasta文件和新建对应的index levelDB数据库
 * @return 1 错误 0 正常
 */
int FastaParser::createIndex(const std::string& fafp, const std::optional<std::string>& dbfp ) {
	fasta_file_path = fafp;

	if (dbfp.has_value()) {
		leveldb_path = *dbfp;
	}
	else {
		// 如果未提供 LevelDB 文件路径，则默认使用 Fasta 文件路径生成
		// 根据 fasta 文件名，构造出 ldb 文件名
		std::filesystem::path fsPath(fasta_file_path);
		leveldb_path = fsPath.stem().string() + ".ldb";
	}
	
	// 打开fasta文件
	fasta_file.open(fasta_file_path, std::ios::in);
	if (!fasta_file.is_open()) {
		perror("Error opening fasta file");
		return 1;
	}
	// 获取fasta文件的字节数
	this->fasta_file_size = this->getFileSize(fasta_file_path);

	XXH3_64bits_reset(&xxhash_state);  // 文件哈希计算状态初始化
	// 打开或新建数据库
	leveldb::Options options;
	options.create_if_missing = true;  // 如果数据库文件不存在，则新建

	// 判断数据库文件是否存在
	if (isDatabaseExists(leveldb_path)) {
		// 如果数据库文件存在
		std::cout << "Database is existed." << std::endl;

		leveldb::Status status = leveldb::DB::Open(options, leveldb_path, &db);  // 打开数据库
		// 根据打开结果处理返回
		if (!status.ok()) {
			std::cerr << "Error opening LevelDB: " << status.ToString() << std::endl;
			return 1;
		}
		// 从数据库中获取对应fasta文件的hash值，key=hash
		std::string key = "hash";
		std::string valuehash;

		status = db->Get(leveldb::ReadOptions(), key, &valuehash);

		if (!status.ok()) {
			// 获取失败
			std::cerr << "Error getting hash from LevelDB: " << status.ToString() << std::endl;
			valuehash = "";  // 返回空字符串表示获取失败
		}
		else {
			// 如果能获取到hash值，则验证待建索引的文件的hash值是否与之相等
			std::string filehash = Utils::calculateFileHash(fasta_file);
			if (valuehash == filehash) {
				std::cout << valuehash << "//" << filehash << std::endl;
			}
			else {
				// 如果不等则重新建立索引
				std::cout << valuehash << "//" << filehash << std::endl;
				this->indexfa();
			}
		}
	}
	else {
		// 如果不存在则新建
		leveldb::Status status = leveldb::DB::Open(options, leveldb_path, &db);  // 打开数据库
		// 根据打开结果处理返回
		if (!status.ok()) {
			std::cerr << "Error opening LevelDB: " << status.ToString() << std::endl;
			return 1;
		}
		// 建立索引
		this->indexfa();
	}

	return 0;
}


/**
 * @brief 这是一个fatsa文件解析主函数
 * @return 1 错误 0 正常
 */
int FastaParser::indexfa() {
	
	const size_t BLOCK_SIZE = 16384;      // 每次读取的文件块的大小
	char* buffer = new char[BLOCK_SIZE];  // 在堆上开辟内存

	// 指定读文件的初始偏移量
	size_t global_seek = 0;
	this->fasta_file.seekg(global_seek, std::ios::beg);

	leveldb::WriteBatch batch;     // 定义levelDB批处理
	const int BATCH_SIZE = 10000;  // 批处理数目

	int count = 0;       // 解析的序列条数计数器
	std::string seq_id;  // 序列id
	// 可为空的暂存元组，用于暂存上一轮读取到的seqid和首个碱基的偏移量
	std::optional<std::tuple<std::string, size_t>> tempTuple;

	while (1) {
		this->fasta_file.read(buffer, BLOCK_SIZE);
		size_t bytesRead = this->fasta_file.gcount();
		std::string block_content(buffer, bytesRead);

		size_t pos = 0;  // 初始化 > 字符在块内的偏移量
		while (1) {
			pos = block_content.find('>', pos);
			if (pos != std::string::npos) {
				// 如果能找到 >
				// 说明读取的文件块至少包含一个id所在的行（不一定完整，下面会处理）
				size_t end_pos = block_content.find('\n', pos);
				if (end_pos != std::string::npos) {
					// 如果能找到换行符，说明block的读取边界没有截断id所在行
					size_t space_pos = block_content.find(' ', pos);
					if (space_pos != std::string::npos) {
						// 如果能找到空格，那就获取空格前的字符作为id
						seq_id = block_content.substr(pos + 1, space_pos - pos - 1);
						if (tempTuple.has_value()) {
							// 说明不是第一次读取
							// 获取上一条序列的结尾偏移量，通过下一条序列 > 所在位置减1得到
							size_t before_sign_pos = global_seek + pos - 1;
							// 从暂存元组的第2位获取上一条序列的首个碱基的偏移量，与结尾偏移量相减得到序列长度（包含换行）
							size_t base_num = before_sign_pos - std::get<1>(*tempTuple);
							// 构造暂存字符串，包含上一条序列的id，起始偏移量，序列长度
							temp_string = temp_string + ',' + std::to_string(base_num);
							// 根据逗号分隔字符串，依次获取
							// seqid和剩余的内容，分别作为key和value
							size_t seqid_pos = temp_string.find(',', 0);
							std::string key = temp_string.substr(0, seqid_pos);
							size_t strlenth = temp_string.length();
							std::string value = temp_string.substr(seqid_pos + 1, strlenth);
							// 存入数据库批处理流
							batch.Put(key, value);
						}
						else {
							// 如果第一次读取
							temp_string = std::to_string(global_seek + end_pos + 1);
						}
					}
					else {
						// 如果找不到空格，就到换行符前的字符作为id
						seq_id = block_content.substr(pos + 1, end_pos - pos - 1);
						if (tempTuple.has_value()) {
							// 说明不是第一次读取
							// 获取上一条序列的结尾偏移量，通过下一条序列 > 所在位置减1得到
							size_t before_sign_pos = global_seek + pos - 1;
							// 从暂存元组的第2位获取上一条序列的首个碱基的偏移量，与结尾偏移量相减得到序列长度（包含换行）
							size_t base_num = before_sign_pos - std::get<1>(*tempTuple);
							// 构造暂存字符串，包含上一条序列的id，起始偏移量，序列长度
							temp_string = temp_string + ',' + std::to_string(base_num);
							// 根据逗号分隔字符串，依次获取
							// seqid和剩余的内容，分别作为key和value
							size_t seqid_pos = temp_string.find(',', 0);
							std::string key = temp_string.substr(0, seqid_pos);
							size_t strlenth = temp_string.length();
							std::string value = temp_string.substr(seqid_pos + 1, strlenth);
							// 存入数据库批处理流
							batch.Put(key, value);
						}
						else {
							// 如果第一次读取
							temp_string =
								seq_id + ',' + std::to_string(global_seek + end_pos + 1);
						}
					}
					// 更新暂存元组的内容，并构造暂存字符串
					tempTuple = std::make_optional(
						std::make_tuple(seq_id, global_seek + end_pos + 1));
					temp_string =
						seq_id + ',' + std::to_string(global_seek + end_pos + 1);
					// 更新下一次查找 > 的位置
					pos = end_pos + 1;
				}
				else {
					// 如果没有找到换行符，说明截断行了，重定位文件读取偏移量，跳出循环
					global_seek += pos;
					break;
				}
			}
			else {
				// 如果找不到 >
				// 说明读取的文件块中没有id所在的行，那么重定位文件读取偏移量，重新读一个文件块
				global_seek += bytesRead;
				break;
			}

			count++;

			if (count >= BATCH_SIZE) {
				// 一次性写入数据库
				db->Write(leveldb::WriteOptions(), &batch);
				// 清空批处理流和计数器
				batch.Clear();
				count = 0;
			}
		}

		if (bytesRead < BLOCK_SIZE) {
			// 实际读到的数据大小小于指定文件块大小时，说明在文件末尾处，此处继续处理
			size_t before_sign_pos = global_seek + pos - 1;
			size_t base_num = before_sign_pos - std::get<1>(*tempTuple);
			temp_string = temp_string + ',' + std::to_string(base_num);
			batch.Put(seq_id, temp_string);
			break;
		}
		// 内部循环结束，指定下次读取文件的偏移值
		fasta_file.seekg(global_seek, std::ios::beg);
	}
	// 文件读取结束
	fasta_file.close();

	// 开始计算文件校验和
	std::cout << "Start calculating the file checksum" << std::endl;
	// 重新打开fasta文件
	fasta_file.open(fasta_file_path, std::ios::in);
	if (!fasta_file.is_open()) {
		perror("Error opening fasta file");
		return 1;
	}
	// 重置文件读取偏移量
	global_seek = 0;
	this->fasta_file.seekg(global_seek, std::ios::beg);
	std::string hash = Utils::calculateFileHash(this->fasta_file);
	batch.Put("hash", hash);


	// 获取fasta文件的绝对路径
	std::filesystem::path absolutePath = std::filesystem::absolute(this->fasta_file_path);
	batch.Put("abspath", absolutePath.string());

	if (count > 0) {
		// 如果计数器没有清零，也就是批处理流内还存有少量数据，则在最后一次性写入
		db->Write(leveldb::WriteOptions(), &batch);
	}

	cleanup();        // 调用清理函数，关闭fasta文件和数据库
	delete[] buffer;  // 释放在堆上的内存
	return 0;
}


int FastaParser::read() {
	// 打开 LevelDB 数据库
	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = false;  // 设置为 true 如果你想创建数据库
	leveldb::Status status = leveldb::DB::Open(options, "CDC_ccs.ldb", &db);

	if (!status.ok()) {
		std::cerr << "Error opening LevelDB: " << status.ToString() << std::endl;
		return 1;
	}

	// 创建一个迭代器
	leveldb::ReadOptions readOptions;
	leveldb::Iterator* it = db->NewIterator(readOptions);

	// 打开一个文本文件以写入模式
	std::ofstream outputFile("output.txt");
	if (!outputFile.is_open()) {
		std::cerr << "Error opening output file." << std::endl;
		delete it;
		delete db;
		return 1;
	}

	// 迭代数据库并输出键值对到文本文件
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		std::string key = it->key().ToString();
		std::string value = it->value().ToString();

		// 将键值对输出到文本文件
		outputFile << "Key: " << key << ", Value: " << value << std::endl;
	}

	// 检查迭代器是否出错
	if (!it->status().ok()) {
		std::cerr << "Iterator error: " << it->status().ToString() << std::endl;
	}

	// 关闭文件和数据库，释放资源
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

