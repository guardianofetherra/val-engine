#pragma once
#include <string>
#include <vector>

enum class ScriptVariableType;

struct ScriptVariableSignature
{
	ScriptVariableType type;
	std::string name;

	ScriptVariableSignature(ScriptVariableType type, const std::string& name);
};

struct ScriptFunctionSignature
{
	std::string name;

	int start;
	int end;

	std::vector<ScriptVariableSignature> arguments;
	ScriptVariableType returnType;

	ScriptFunctionSignature(const std::string& name, int start, int end, std::vector<ScriptVariableSignature>& arguments, ScriptVariableType returnType);
	ScriptFunctionSignature();
	~ScriptFunctionSignature();
};
