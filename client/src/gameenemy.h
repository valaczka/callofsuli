/*
 * ---- Call of Suli ----
 *
 * gameenemy.h
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemy
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

#ifndef GAMEENEMY_H
#define GAMEENEMY_H

#include "gameentity.h"
#include "gameterrain.h"
#include "gamepickable.h"
#include "actiongame.h"
#include <QPointer>

class GamePlayer;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_GamePlayer
#define OPAQUE_PTR_GamePlayer
  Q_DECLARE_OPAQUE_POINTER(GamePlayer*)
#endif

#endif

class GameEnemy : public GameEntity
{
	Q_OBJECT

	Q_PROPERTY(bool aimedByPlayer READ aimedByPlayer WRITE setAimedByPlayer NOTIFY aimedByPlayerChanged)
	Q_PROPERTY(qreal castAttackFraction READ castAttackFraction WRITE setCastAttackFraction NOTIFY castAttackFractionChanged)
	Q_PROPERTY(qreal msecBeforeAttack READ msecBeforeAttack WRITE setMsecBeforeAttack NOTIFY msecBeforeAttackChanged)
	Q_PROPERTY(qreal msecBetweenAttack READ msecBetweenAttack WRITE setMsecBetweenAttack NOTIFY msecBetweenAttackChanged)
	Q_PROPERTY(EnemyState enemyState READ enemyState WRITE setEnemyState NOTIFY enemyStateChanged)
	Q_PROPERTY(GamePlayer* player READ player WRITE setPlayer NOTIFY playerChanged)
	Q_PROPERTY(qreal msecLeftToAttack READ msecLeftToAttack NOTIFY msecLeftToAttackChanged)
	Q_PROPERTY(bool hasQuestion READ hasQuestion NOTIFY questionChanged)
	Q_PROPERTY(bool hasPickable READ hasPickable NOTIFY pickableChanged)

public:
	explicit GameEnemy(QQuickItem *parent = nullptr);
	virtual ~GameEnemy();

	enum EnemyState {
		Invalid = 0,
		Idle,
		Move,
		WatchPlayer,
		Attack,
		Dead
	};

	Q_ENUM(EnemyState);

	Q_INVOKABLE void startMovingAfter(const int &msec);


	const GameTerrain::EnemyData &terrainEnemyData() const;
	void setTerrainEnemyData(const GameTerrain::EnemyData &newTerrainEnemyData);

	bool aimedByPlayer() const;
	void setAimedByPlayer(bool newAimedByPlayer);

	qreal castAttackFraction() const;
	void setCastAttackFraction(qreal newCastAttackFraction);

	qreal msecBeforeAttack() const;
	void setMsecBeforeAttack(qreal newMsecBeforeAttack);

	qreal msecBetweenAttack() const;
	void setMsecBetweenAttack(qreal newMsecBetweenAttack);

	const EnemyState &enemyState() const;
	void setEnemyState(const EnemyState &newEnemyState);

	GamePlayer *player() const;
	void setPlayer(GamePlayer *newPlayer);

	qreal msecLeftToAttack() const;
	void setMsecLeftToAttack(qreal newMsecLeftToAttack);

	ActionGame::QuestionLocation *question() const;
	void setQuestion(ActionGame::QuestionLocation *newQuestion);
	bool hasQuestion() const { return m_question; }

	const GamePickable::GamePickableData &pickable() const;
	void setPickable(const GamePickable::GamePickableData &newPickable);
	bool hasPickable() const { return m_pickable.type != GamePickable::PickableInvalid; }

public slots:
	void attackByPlayer(GamePlayer *player, const bool &questionEmpty = true);
	void missedByPlayer(GamePlayer *player);
	void turnToPlayer(GamePlayer *player);

signals:
	void attack();
	void killMissed();
	void aimedByPlayerChanged();
	void castAttackFractionChanged();
	void msecBeforeAttackChanged();
	void msecBetweenAttackChanged();
	void enemyStateChanged();
	void playerChanged();
	void msecLeftToAttackChanged();
	void questionChanged();
	void pickableChanged();

private slots:
	void onSceneConnected();

protected:
	virtual void enemyStateModified() {}
	virtual void attackedByPlayerEvent(GamePlayer *player, const bool &isQuestionEmpty);
	void playAttackSound();

	GameTerrain::EnemyData m_terrainEnemyData;
	int m_startMovingAfter = 0;

	qreal m_castAttackFraction = 0.5;
	qreal m_msecBeforeAttack = 1500;
	qreal m_msecBetweenAttack = 1500;

	EnemyState m_enemyState = Invalid;
	bool m_aimedByPlayer = false;
	QPointer<GameEntity> m_player = nullptr;
	qreal m_msecLeftToAttack = -1;
	ActionGame::QuestionLocation *m_question = nullptr;
	GamePickable::GamePickableData m_pickable;

};

#endif // GAMEENEMY_H
