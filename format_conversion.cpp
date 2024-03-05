#include "tools.h"
#include "basetypes.h"

class FastaMergeTool : public ToolsBase
{
	public:
		FastaMergeTool() {
		this->toolName = "FastaMergeTool";
		this->toolCmd = "merge";
		this->toolDesc = "The FASTA merge tool can combine multiple FASTA files into a single sequence file.";
		this->setInputType("seqInput", "fasta");
		this->setOutputType("seqOutput", "fasta");

		Fasta tool()
	};


		
		
};

