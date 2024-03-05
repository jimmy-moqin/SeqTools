#include <iostream>
#include <CLI11.hpp>

#include "sequence_parser.h"
#include "fasta_parser.h"

bool transString2Bool(std::string str){
    if (str == "no") {
        return 0;
    }
    else if (str == "yes") {
        return 1;
	}
    else {
        return 0;
    }
};


int main(int argc, char* argv[]) {
    CLI::App app{ "序列处理工具" };
    FastaParser fasta_parser;
    SeqParser seq_parser;

    // 创建子命令 index
    CLI::App* indexTool = app.add_subcommand("index", "为序列文件创建索引");
    std::string fastaFile;
    std::string indexFile;
    indexTool->add_option("-i,--input", fastaFile, "输入文件名")->required();
    indexTool->add_option("-o,--output", indexFile, "输出文件名")->required();
    indexTool->callback([&fasta_parser, &fastaFile, &indexFile]() {
        fasta_parser.createIndex(fastaFile, indexFile);
        });

    // 创建子命令 getseqbyid
    CLI::App* getSeqTool = app.add_subcommand("getseqbyid", "根据序列ID获取序列");
    std::string indexFile_setseq;
    std::string seqId;
    std::string withDesc;
    std::string lineWidth;
    std::string outputFile;

    getSeqTool->add_option("-i,--index", indexFile_setseq, "索引文件名")->required();
    getSeqTool->add_option("-s,--seqid", seqId, "序列ID")->required();
    getSeqTool->add_option("-l,--linewidth", lineWidth, "输出序列的行宽");
    getSeqTool->add_option("-o,--output", outputFile, "输出文件名")->required();
    getSeqTool->add_option("-d,--withdesc", withDesc, "是否输出序列描述信息");


    getSeqTool->callback([&seq_parser, &indexFile_setseq, &seqId, &withDesc, &lineWidth, &outputFile]() {

		seq_parser.getSequenceById(indexFile_setseq, seqId, transString2Bool(withDesc), std::stoull(lineWidth), outputFile);
		});
    
    CLI11_PARSE(app, argc, argv);

    return 0;
}