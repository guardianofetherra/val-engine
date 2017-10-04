#include "ScriptExpression.h"
#include "ScriptBlock.h"
#include "ScriptToken.h"
#include "ScriptVariable.h"
#include "ScriptVariableUtils.h"
#include "ScriptParsingUtils.h"
#include "ScriptOperator.h"
#include "ScriptError.h"

#include <stack>
#include <boost\lexical_cast.hpp>

enum class ScriptExpression::State
{
	None,
	BinaryOperator,
	UnaryOperator,
	VariableDeclaration,
	VariableEvaluation,
};

std::shared_ptr<BaseScriptVariable> ScriptExpression::Evaluate()
{
	ScriptExpression::State state = ScriptExpression::State::None;
	ScriptExpression::State previousState = ScriptExpression::State::None;

	std::stack<std::shared_ptr<BaseScriptVariable>> variableStack;
	std::stack<const ScriptOperator*> operatorStack;

	for(size_t i = 0; i < _tokens.size(); ++i)
	{
		const ScriptToken& token = _tokens[i];

		switch(token.type)
		{
		case ScriptTokenType::Operator:
			if(previousState == ScriptExpression::State::BinaryOperator || previousState == ScriptExpression::State::UnaryOperator)
			{
				throw ScriptError("Unexpected Token " + token.token);
			}
			if(previousState == ScriptExpression::State::VariableEvaluation || previousState == ScriptExpression::State::VariableDeclaration)
			{
				state = ScriptExpression::State::BinaryOperator;
			}
			else
			{
				state = ScriptExpression::State::UnaryOperator;
			}
			break;
		case ScriptTokenType::Keyword:
			if(BaseScriptVariable::GetVariableType(token.token) != ScriptVariableType::Null)
			{
				state = ScriptExpression::State::VariableDeclaration;
				break;
			}
		case ScriptTokenType::NumericLiteral:
		case ScriptTokenType::StringLiteral:
		case ScriptTokenType::ParenthesisGroup:
			if(previousState != ScriptExpression::State::None && previousState != ScriptExpression::State::BinaryOperator && previousState != ScriptExpression::State::UnaryOperator)
			{
				throw ScriptError("Unexpected Token " + token.token);
			}
			state = ScriptExpression::State::VariableEvaluation;
			break;
		case ScriptTokenType::Whitespace:
			continue;
		default:
			throw ScriptError("Unexpected Token " + token.token);
		}

		switch(state)
		{
		case ScriptExpression::State::BinaryOperator:
		{
			const ScriptOperator* op = ScriptOperator::GetOperator(token.token);
			if(op == nullptr)
			{
				throw ScriptError("Unrecognized operator with signature " + token.token);
			}
			//check operator priority
			while(!operatorStack.empty() && (operatorStack.top()->priority() <= op->priority()))
			{
				std::shared_ptr<BaseScriptVariable> rhs = variableStack.top();
				variableStack.pop();
				std::shared_ptr<BaseScriptVariable> lhs = variableStack.top();
				variableStack.pop();

				std::shared_ptr<BaseScriptVariable> result = ScriptVariableUtils::Operate(lhs, rhs, operatorStack.top()->type());
				operatorStack.pop();

				variableStack.push(result);
			}

			operatorStack.push(op);
			break;
		}
		case ScriptExpression::State::UnaryOperator:
		{
			const ScriptOperator* op = ScriptOperator::GetOperator(token.token);
			if(op == nullptr)
			{
				throw ScriptError("Unrecognized operator with signature " + token.token);
			}

			operatorStack.push(op);
			break;
		}
		case ScriptExpression::State::VariableDeclaration:
		{
			if(i == _tokens.size() - 1)
			{
				throw ScriptError("Expected variable declaration name, found end of line.");
			}

			const ScriptToken& nameToken = _tokens[i + 1];
			if(nameToken.type != ScriptTokenType::Keyword)
			{
				throw ScriptError("Unexpected token " + nameToken.token);
			}

			ScriptVariableType variableType = BaseScriptVariable::GetVariableType(token.token);
			std::shared_ptr<BaseScriptVariable> newVariable;

			switch(variableType)
			{
			case ScriptVariableType::Bool:
				newVariable = std::make_shared<ScriptBool>();
				break;
			case ScriptVariableType::Int:
				newVariable = std::make_shared<ScriptInt>();
				break;
			case ScriptVariableType::String:
				newVariable = std::make_shared<ScriptString>();
				break;
			default:
				throw ScriptError("Unhandled script variable type " + std::to_string((int)variableType));
			}

			_parent->AddVariable(nameToken.token, newVariable);

			if(previousState == ScriptExpression::State::UnaryOperator)
			{
				newVariable = ScriptVariableUtils::Operate(newVariable, ScriptOperatorType::Subtraction);
			}

			variableStack.push(newVariable);
			i += 1;
			break;
		}
		case ScriptExpression::State::VariableEvaluation:
		{
			std::shared_ptr<BaseScriptVariable> evaluatedVar;

			if(token.type == ScriptTokenType::ParenthesisGroup)
			{
				std::vector<ScriptToken> parenthesisTokens;
				ScriptParsingUtils::ParseLineTokens(token.token, parenthesisTokens);
				std::shared_ptr<ScriptExpression> expression = std::make_shared<ScriptExpression>(_parent, parenthesisTokens);
				evaluatedVar = expression->Evaluate();
			}
			if(token.type == ScriptTokenType::NumericLiteral)
			{
				evaluatedVar = std::make_shared<ScriptInt>(boost::lexical_cast<int>(token.token), true);
			}
			else if(token.type == ScriptTokenType::StringLiteral)
			{
				evaluatedVar = std::make_shared<ScriptString>(token.token, true);
			}
			else if(token.token == ScriptToken::value_true)
			{
				evaluatedVar = std::make_shared<ScriptBool>(true, true);
			}
			else if(token.token == ScriptToken::value_false)
			{
				evaluatedVar = std::make_shared<ScriptBool>(false, true);
			}
			else
			{
				if(i == _tokens.size() - 1 || _tokens[i + 1].type != ScriptTokenType::ParenthesisGroup)
				{
					evaluatedVar = _parent->GetVariable(token.token);
				}
				else
				{
					std::vector<ScriptToken> parenthesisTokens;
					ScriptParsingUtils::ParseLineTokens(_tokens[i + 1].token, parenthesisTokens);
					std::vector<std::shared_ptr<BaseScriptVariable>> variables;

					for(size_t i = 0; i < parenthesisTokens.size(); ++i)
					{
						int nextToken = ScriptParsingUtils::GetNextTokenOfType(ScriptTokenType::Separator, parenthesisTokens, i);
						if(nextToken < 0)
						{
							nextToken = parenthesisTokens.size();
						}

						variables.push_back(
							std::make_unique<ScriptExpression>(_parent, std::vector<ScriptToken>(parenthesisTokens.begin() + i, parenthesisTokens.begin() + nextToken))->Evaluate()
						);

						i = nextToken;
					}

					evaluatedVar = _parent->RunFunction(token.token, variables);
					i += 1;
				}
			}

			if(evaluatedVar == nullptr)
			{
				throw ScriptError("Invalid value evaluation on token " + token.token);
			}

			if(previousState == ScriptExpression::State::UnaryOperator)
			{
				evaluatedVar = ScriptVariableUtils::Operate(evaluatedVar, ScriptOperatorType::Subtraction);
			}

			variableStack.push(evaluatedVar);
			break;
		}
		}

		previousState = state;
	}

	while(!operatorStack.empty())
	{
		std::shared_ptr<BaseScriptVariable> rhs = variableStack.top();
		variableStack.pop();
		std::shared_ptr<BaseScriptVariable> lhs = variableStack.top();
		variableStack.pop();

		std::shared_ptr<BaseScriptVariable> result = ScriptVariableUtils::Operate(lhs, rhs, operatorStack.top()->type());
		operatorStack.pop();

		variableStack.push(result);
	}

	if(variableStack.size() != 1)
	{
		throw ScriptError("Script expression parsing resulted in variable stack of size !=1. Alert all authorities.");
	}

	return variableStack.top();
}

ScriptExpression::ScriptExpression(ScriptBlock* parent, const std::vector<ScriptToken>& tokens) : _tokens(tokens)
{
	_parent = parent;
}

ScriptExpression::~ScriptExpression()
{
}
