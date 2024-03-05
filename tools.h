#pragma once
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>

class
	ToolsBase
{
public:

	int setInputType(std::string param, std::string type) {
		this->inputTypeMap[param] = type;
		return 0;
	};

	int setOutputType(std::string param, std::string type) {
		this->outputTypeMap[param] = type;
		return 0;
	};

	int addOptions(std::vector<std::string> optList, std::string opt) {
		this->optionsList = optList;
		return 0;
	};

	int addToolOptions(std::string param, std::vector<std::string> options) {
		this->toolOptionsMap[param] = options;
		return 0;
	};
	
	std::string getToolName() {
		return this->toolName;
	};

	std::string getToolDesc() {
		return this->toolDesc;
	};

	std::unordered_map<std::string, std::string> getInputTypeMap() {
		return this->inputTypeMap;
	};

	std::unordered_map<std::string, std::string> getOutputTypeMap() {
		return this->outputTypeMap;
	};

	std::string getToolCmd() {
		return this->toolCmd;
	};


protected:
	std::string toolName;
	std::string toolCmd;
	std::string toolDesc;
	std::unordered_map<std::string, std::string> inputTypeMap;
	std::unordered_map<std::string, std::string> outputTypeMap;
	std::vector<std::string> optionsList;
	std::unordered_map<std::string, std::vector<std::string>> toolOptionsMap;
};

