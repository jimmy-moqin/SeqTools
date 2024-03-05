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
    CLI::App app{ "���д�����" };
    FastaParser fasta_parser;
    SeqParser seq_parser;

    // ���������� index
    CLI::App* indexTool = app.add_subcommand("index", "Ϊ�����ļ���������");
    std::string fastaFile;
    std::string indexFile;
    indexTool->add_option("-i,--input", fastaFile, "�����ļ���")->required();
    indexTool->add_option("-o,--output", indexFile, "����ļ���")->required();
    indexTool->callback([&fasta_parser, &fastaFile, &indexFile]() {
        fasta_parser.createIndex(fastaFile, indexFile);
        });

    // ���������� getseqbyid
    CLI::App* getSeqTool = app.add_subcommand("getseqbyid", "��������ID��ȡ����");
    std::string indexFile_setseq;
    std::string seqId;
    std::string withDesc;
    std::string lineWidth;
    std::string outputFile;

    getSeqTool->add_option("-i,--index", indexFile_setseq, "�����ļ���")->required();
    getSeqTool->add_option("-s,--seqid", seqId, "����ID")->required();
    getSeqTool->add_option("-l,--linewidth", lineWidth, "������е��п�");
    getSeqTool->add_option("-o,--output", outputFile, "����ļ���")->required();
    getSeqTool->add_option("-d,--withdesc", withDesc, "�Ƿ��������������Ϣ");


    getSeqTool->callback([&seq_parser, &indexFile_setseq, &seqId, &withDesc, &lineWidth, &outputFile]() {

		seq_parser.getSequenceById(indexFile_setseq, seqId, transString2Bool(withDesc), std::stoull(lineWidth), outputFile);
		});
    
    CLI11_PARSE(app, argc, argv);

    return 0;
}