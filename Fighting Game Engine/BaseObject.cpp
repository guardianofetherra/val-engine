#include "BaseObject.h"
#include "ObjectReferenceManager.h"
#include "GameInstance.h"

VE_OBJECT_DEFINITION(BaseObject)

void BaseObject::RegisterReference(const ObjectReference<BaseObject>& reference)
{
	_references.emplace(&reference);
}

void BaseObject::UnregisterReference(const ObjectReference<BaseObject>& reference)
{
	_references.erase(&reference);
}

json BaseObject::Serialize() const
{
	json outJson = IReflectable::Serialize();
	outJson.emplace(_owningInstance->configData().serializationConfigData.objectClassPropertyName, className());
	return outJson;
}

BaseObject::BaseObject()
	: _owningInstance(nullptr)
	, _outer(nullptr)
	, enabled(true)
{
	ObjectReferenceManager::Get().AddObject(this);
}

BaseObject::~BaseObject()
{
	for(auto& iter : _references)
	{
		iter->Invalidate();
	}
	ObjectReferenceManager::Get().RemoveObject(this);
}