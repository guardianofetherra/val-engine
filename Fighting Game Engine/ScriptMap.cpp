#include "ScriptMap.h"
#include "ScriptError.h"
#include "IReflectable.h"
#include "ScriptVariableUtils.h"

std::shared_ptr<ScriptMap::value_type> ScriptMap::AddMember(const std::shared_ptr<key_type>& key, const std::shared_ptr<value_type>& value)
{
	return _storage.emplace(key->value(), value->Clone()).first->second;
}

std::shared_ptr<ScriptMap::value_type> ScriptMap::GetMember(const std::shared_ptr<key_type>& key)
{
	auto& iter = _storage.find(key->value());
	if(iter == _storage.end())
	{
		throw ScriptError("Attempting to index invalid member of collection [" + key->value() + "]");
	}

	return iter->second;
}

void ScriptMap::RemoveMember(const std::shared_ptr<key_type>& key)
{
	_storage.erase(key->value());
}

void ScriptMap::Clear()
{
	_storage.clear();
}

json ScriptMap::ToJSON() const
{
	json parentJson = Super::ToJSON();
	json& arrayJson = parentJson.emplace("collection", json::array()).first.value();
	for(auto& iter : _storage)
	{
		arrayJson.emplace(iter.first, iter.second->ToJSON());
	}

	return parentJson;
}

std::shared_ptr<BaseScriptVariable> ScriptMap::Clone() const
{
	return std::make_shared<ScriptMap>(*this);
}

ScriptMap::ScriptMap(bool isConst) 
	: Super(isConst)
{

}

ScriptMap::ScriptMap(const json& j)
	: Super(j)
{
	const json& containerJson = ScriptVariableUtils::JsonIsScriptVariableObject(j) ? j["ve_value"] : j;
	for(auto& iter = containerJson.begin(); iter != containerJson.end(); ++iter)
	{
		_storage.emplace(iter.key(), ScriptVariableUtils::FromJson(iter.value()));
	}
}

ScriptMap::ScriptMap(const ScriptMap& map) 
	: Super(false)
{
	for(auto& iter : map._storage)
	{
		_storage.emplace(iter.first, iter.second->Clone());
	}

	_initialized = true;
}