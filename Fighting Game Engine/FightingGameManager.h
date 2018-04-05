#pragma once
#include "BaseService.h"
#include "FightingGameState.h"
#include "Delegate.h"
#include <unordered_map>

class GameCharacter;
class GameSceneManager;
class GameScene;
class GamePlayer;
class PlayerManager;
class FightingStageBehaviour;

class FightingGameManager : public BaseService
{
	friend class ScriptManager;
private:
	GameSceneManager* _gameSceneManager;
	PlayerManager* _playerManager;

private:
	FightingGameState _currentState;

	std::unordered_map<int, GameCharacter*> _characters;
	FightingStageBehaviour* _stageBehaviour;

	void HandleSceneLoaded(const GameScene* scene);
	void ChangeState(FightingGameState state);

	void Init() override;
	void Update() override;
	void Cleanup() override;

	int AddCharacter(const std::string& path);
	void RemoveCharacter(int id);

public:
	typedef Delegate<FightingGameState> FightingGameStateEventHandler;
	FightingGameStateEventHandler FightingGameStateChanged;

	FightingGameState currentState() const;
	FightingStageBehaviour* stageBehaviour() const;

	FightingGameManager(ServiceManager* serviceManager);
	~FightingGameManager() = default;
};