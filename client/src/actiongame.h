/*
 * ---- Call of Suli ----
 *
 * actiongame.h
 *
 * Created on: 2022. 12. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ActionGame
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ACTIONGAME_H
#define ACTIONGAME_H

#include <QPointer>
#include "abstractlevelgame.h"
#include <QObject>
#include "gameentity.h"
#include "gameterrain.h"
#include "gamescene.h"
#include "gamepickable.h"

class GamePlayer;
class GameEnemy;


/**
 * @brief The ActionGame class
 */


class ActionGame : public AbstractLevelGame
{
	Q_OBJECT

	Q_PROPERTY(GamePlayer* player READ player WRITE setPlayer NOTIFY playerChanged)
	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
	Q_PROPERTY(GameScene* scene READ scene WRITE setScene NOTIFY sceneChanged)
	Q_PROPERTY(int activeEnemies READ activeEnemies NOTIFY activeEnemiesChanged)

public:
	ActionGame(GameMapMissionLevel *missionLevel, Client *client);
	virtual ~ActionGame();

	class EnemyLocation;
	class QuestionLocation;

	void createQuestions();
	void createEnemyLocations();
	void createFixEnemies();
	void recreateEnemies();
	void createInventory();

	void linkQuestionToEnemies(QList<GameEnemy *> enemies);
	void linkPickablesToEnemies(QList<GameEnemy *> enemies);

	void tryAttack(GamePlayer *player, GameEnemy *enemy);

	GamePlayer *player() const;
	void setPlayer(GamePlayer *newPlayer);

	bool running() const;
	void setRunning(bool newRunning);

	GameScene* scene() const;
	void setScene(GameScene* newScene);

	int activeEnemies() const;

	const QVector<int> &closedBlocks() const;

public slots:
	void onPlayerDied(GameEntity *);
	void onEnemyDied(GameEntity *entity);

protected:
	virtual QQuickItem *loadPage() override;

signals:
	void missionCompleted();
	void missionFailed();
	void playerChanged();
	void runningChanged();
	void sceneChanged();
	void activeEnemiesChanged();

private:
	QPointer<GameScene> m_scene = nullptr;
	QPointer<GameEntity> m_player = nullptr;
	bool m_running = true;
	QVector<QuestionLocation*> m_questions;
	QVector<EnemyLocation*> m_enemies;
	QVector<int> m_closedBlocks;

	typedef QPair<GamePickable::GamePickableData, int> Inventory;
	QVector<Inventory> m_inventory;
};




/**
 * @brief The ActionGame::QuestionLocation class
 */

class ActionGame::QuestionLocation {
public:
	const Question &question() const;
	void setQuestion(const Question &newQuestion);

	GameEnemy* enemy() const;
	void setEnemy(GameEnemy *newEnemy);

	int used() const;
	void setUsed(int newUsed);

private:
	Question m_question = Question(nullptr);
	QPointer<GameEntity> m_enemy = nullptr;
	int m_used = 0;
};




/**
 * @brief The ActionGame::EnemyLocation class
 */

class ActionGame::EnemyLocation {
	Q_GADGET

	Q_PROPERTY(GameTerrain::EnemyData enemyData READ enemyData)
	Q_PROPERTY(GameEnemy* enemy READ enemy)

public:
	EnemyLocation(const GameTerrain::EnemyData &enemyData = GameTerrain::EnemyData());
	~EnemyLocation();

	GameEnemy* enemy() const;
	void setEnemy(GameEnemy *newEnemy);

	const GameTerrain::EnemyData &enemyData() const;
	void setEnemyData(const GameTerrain::EnemyData &newEnemyData);

private:
	GameTerrain::EnemyData m_enemyData = GameTerrain::EnemyData();
	QPointer<GameEntity> m_enemy = nullptr;
};



#endif // ACTIONGAME_H
