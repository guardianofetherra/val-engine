#include "ScriptBlock.h"
#include "CommonUtilIncludes.hpp"
#include "Script.h"
#include "ScriptParsingUtils.h"
#include "ScriptError.h"

//Cache line references to functions and variables.
//Not meant to be any kind of rigorous parsing quite yet, so we can get away with just checking if the indentation level is the same.
void ScriptBlock::PreProcess()
{
	for(size_t i = 0; i < _lines.size(); i++)
	{
		int indentation;
		std::string line = ScriptParsingUtils::TrimLine(_lines[i], indentation);
		if(indentation != _depth)
		{
			continue;
		}

		std::vector<std::string> spl;
		boost::split(spl, line, boost::is_any_of(" "), boost::token_compress_on);

		if(spl.size() < 2)
		{
			continue;
		}

		if(spl[0] == "function")
		{
			if(_functions.find(spl[1]) != _functions.end())
			{
				throw ScriptError("Parser error: Duplicate function definition on line: " + std::to_string(_lines.front() + i));
			}
			_functions[spl[1]] = i;
		}
	}

	Run();
}

void ScriptBlock::Run()
{
	for(size_t i = 0; i < _lines.size(); ++i)
	{
		int depth;
		std::string line = ScriptParsingUtils::TrimLine(_lines[i], depth);
		if(depth < 0)
		{
			continue;
		}
		else if(depth < _depth)
		{
			throw ScriptError("Parser error: Line of invalid depth within block: " + std::to_string(_lines.front() + i));
		}

		//TODO: Proper parsing
	}
	//run normally
	//if parent isn't null, throw exceptions on class and function declarations
	//otherwise skip to end of those blocks and continue
}

//Create a new function script block and add some variables to it based on the function signature.
void ScriptBlock::RunFunction(std::string name, ...)
{
	auto functionLine = _functions.find(name);
	if(functionLine == _functions.end())
	{
		throw ScriptError("Attempting to call invalid function '" + name + "'");
	}

	int functionEndLine = ScriptParsingUtils::FindBlockEnd(_lines, functionLine->second);
	if(functionEndLine < 0)
	{
		throw ScriptError("Function '" + name + "' not properly terminated.");
	}

	int fBegin = functionLine->second + 1;
	int fEnd = functionEndLine;

	ScriptLinesView functionLines = ScriptLinesView(_lines.lines(), functionLine->second + 1, functionEndLine);

	std::shared_ptr<ScriptBlock> scriptBlock = std::make_shared<ScriptBlock>(functionLines, _depth + 1, this, _owner);
	scriptBlock->Run();
}

ScriptBlock::ScriptBlock(ScriptLinesView lines, int depth, ScriptBlock* parent, Script* owner) : _lines(lines)
{
	_depth = depth;
	_parent = parent;
	_owner = owner;

	if(parent == nullptr)
	{
		PreProcess();
	}
}

ScriptBlock::~ScriptBlock()
{
}
