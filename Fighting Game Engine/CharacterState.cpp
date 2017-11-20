#include "CharacterState.h"
#include "Script.h"
#include "InputMotion.h"
#include "ScriptManager.h"

std::string CharacterState::name() const
{
	return _name;
}

int CharacterState::priority() const
{
	return _priority;
}

Script * CharacterState::script()
{
	return _script;
}

const std::unordered_set<std::string>& CharacterState::stateTypeFlags() const
{
	return _stateTypeFlags;
}

const InputMotion& CharacterState::associatedMotion() const
{
	return _associatedMotion;
}

CharacterState::CharacterState(const json& j):
	_associatedMotion(j["motion"])
{
	_name = JSON::Get<std::string>(j["name"]);
	_priority = JSON::Get<int>(j["priority"]);
	_script = ScriptManager::GetScript(JSON::Get<std::string>(j["script"]));
	_stateTypeFlags.reserve(j["flags"].size());

	for(auto& iter : j["flags"])
	{
		_stateTypeFlags.emplace(JSON::Get<std::string>(iter));
	}
}

CharacterState::~CharacterState()
{

}
