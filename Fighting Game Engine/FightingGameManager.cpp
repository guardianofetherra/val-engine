#include "FightingGameManager.h"
#include "GameSceneManager.h"
#include "ServiceManager.h"
#include "Object.h"
#include "PlayerManager.h"
#include "GameScene.h"
#include "GameCharacter.h"
#include "Screen.h"
#include "GameCharacter.h"
#include "GLIncludes.hpp"
#include "FightingStageBehaviour.h"


void FightingGameManager::HandleSceneLoaded(const GameScene* scene)
{
	//PLACEHOLDER
	switch(_currentState)
	{
	case FightingGameState::None:
		if(scene->name() == "Menu")
			ChangeState(FightingGameState::InGame);
		break;
	case FightingGameState::InputDeviceSelection:
		break;
	case FightingGameState::CharacterSelection:
		break;
	case FightingGameState::StageSelection:
		break;
	case FightingGameState::InGame:
		break;
	default:
		break;
	}
}

void FightingGameManager::ChangeState(FightingGameState state)
{
	if(state == _currentState)
	{
		return;
	}

	switch(state)
	{
	case FightingGameState::None:
		_playerManager->ClearPlayers();
		break;
	case FightingGameState::InputDeviceSelection:
		break;
	case FightingGameState::CharacterSelection:
		break;
	case FightingGameState::StageSelection:
		break;
	case FightingGameState::InGame:
	{
		//PLACEHOLDER
		Object* char1 = _gameSceneManager->currentScene()->LoadObject("Characters/Fritz/Prefab.json");
		GameCharacter* char1CharacterBehaviour = char1->GetBehaviour<GameCharacter>("GameCharacter");
		char1CharacterBehaviour->SetOwner(_playerManager->AddPlayer(0, -1));

		_stageBehaviour = dynamic_cast<FightingStageBehaviour*>(_gameSceneManager->currentScene()->FindBehaviour("FightingStageBehaviour"));
		break;
	}
	default:
		break;
	}
	_currentState = state;
	FightingGameStateChanged(state);
}

void FightingGameManager::Init()
{
	_playerManager = _serviceManager->PlayerManager();
	_gameSceneManager = _serviceManager->GameSceneManager();
	_gameSceneManager->SceneLoaded += VE_DELEGATE_FUNC(GameSceneManager::GameSceneEventHandler, HandleSceneLoaded);

	ChangeState(FightingGameState::None);
}

void FightingGameManager::Update()
{
	//PLACEHOLDER
	if(_currentState == FightingGameState::InGame && glfwGetKey(_serviceManager->Screen()->window, GLFW_KEY_B) == GLFW_PRESS)
	{
		ChangeState(FightingGameState::None);
		_gameSceneManager->LoadScene("Intro");
	}
}

void FightingGameManager::Cleanup()
{
	ChangeState(FightingGameState::None);
}

int FightingGameManager::AddCharacter(const std::string& path)
{
	if(currentState() == FightingGameState::None)
	{
		return -1;
	}

	Object* object = _gameSceneManager->currentScene()->LoadObject(path);
	if(object == nullptr)
	{
		return -1;
	}

	GameCharacter* characterBehaviour = object->GetBehaviour<GameCharacter>("GameCharacter");
	if(characterBehaviour == nullptr)
	{
		return -1;
	}

	_characters.emplace(object->id(), characterBehaviour);
	return object->id();
}

void FightingGameManager::RemoveCharacter(int id)
{
	auto& iter = _characters.find(id);
	if(iter == _characters.end())
	{
		return;
	}

	_characters.erase(iter);
	_gameSceneManager->currentScene()->DestroyObject(id);
}

FightingGameState FightingGameManager::currentState() const
{
	return _currentState;
}

FightingStageBehaviour* FightingGameManager::stageBehaviour() const
{
	return _stageBehaviour;
}

FightingGameManager::FightingGameManager(ServiceManager* serviceManager) : BaseService(serviceManager, 500)
{
	_allowServiceUpdate = true;
}