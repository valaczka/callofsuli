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
#include "gamemap.h"

class GamePlayer;
class GameEnemy;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_GamePlayer
#define OPAQUE_PTR_GamePlayer
  Q_DECLARE_OPAQUE_POINTER(GamePlayer*)
#endif

#ifndef OPAQUE_PTR_GameEnemy
#define OPAQUE_PTR_GameEnemy
  Q_DECLARE_OPAQUE_POINTER(GameEnemy*)
#endif

#endif

#define ACTION_GAME_BASE_XP		10
#define ACTION_GAME_ENEMY_KILL_XP	5



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
	Q_PROPERTY(GamePickable *pickable READ pickable NOTIFY pickableChanged)
	Q_PROPERTY(QVariantList tools READ tools CONSTANT)
	Q_PROPERTY(QVariantList toolListIcons READ toolListIcons NOTIFY toolListIconsChanged)

public:
	explicit ActionGame(GameMapMissionLevel *missionLevel, Client *client, const GameMap::GameMode &mode);
	explicit ActionGame(GameMapMissionLevel *missionLevel, Client *client) : ActionGame(missionLevel, client, GameMap::Action) {}
	virtual ~ActionGame();

	class EnemyLocation;
	class QuestionLocation;

	virtual void onSceneReady();
	virtual void onSceneAnimationFinished();

	void createPickable(const GamePickable::GamePickableData &data, const QPointF &bottomPoint);

	void tryAttack(GamePlayer *player, GameEnemy *enemy);
	void operateReal(GamePlayer *player, GameObject *object);
	bool canOperate(const QString &type) const;
	bool canOperate(GameObject *object) const;

	GamePlayer *player() const;
	void setPlayer(GamePlayer *newPlayer);

	bool running() const;
	void setRunning(bool newRunning);

	GameScene* scene() const;
	void setScene(GameScene* newScene);

	int activeEnemies() const;

	const QVector<int> &closedBlocks() const;

	GamePickable *pickable() const;

	void resetKillStreak();
	void killAllEnemy();

	Q_INVOKABLE int toolCount(const GamePickable::PickableType &type) const;
	void toolAdd(const GamePickable::PickableType &type, const int &count = 1);
	void toolRemove(const GamePickable::PickableType &type, const int &count = 1);
	void toolClear(const GamePickable::PickableType &type);
	QVariantList toolListIcons() const;
	static QVariantList tools();
	Q_INVOKABLE void toolUse(const GamePickable::PickableType &type);
	const QHash<QString, QVector<GamePickable::PickableType> > &toolDependency() const;

	static void reloadAvailableCharacters();
	static const QStringList &availableCharacters();

	virtual QJsonObject getExtendedData() const override;

	static int tickInterval() { return TickTimer::interval(); }
	qint64 currentTick() const { return m_tickTimer.currentTick(); }

	virtual void sceneTimerTimeout(const int &msec, const qreal &delayFactor);


public slots:
	void pickableAdd(GamePickable *pickable);
	void pickableRemove(GamePickable *pickable);
	void pickableRemoveAll();
	void pickablePick();
	void message(const QString &text, const QColor &color = "white");
	void addMSec(const qint64 &msec);
	void dialogMessageTooltip(const QString &text, const QString &icon, const QString &title = tr("Tudtad?"));
	bool dialogMessageTooltipById(const QString &msgId);
	void dialogMessageFinish(const QString &text, const QString &icon, const bool &success);
	void gameAbort() override;

protected:
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;
	virtual bool gameFinishEvent() override;
	virtual void onSceneAboutToStart();

	void createPlayer();
	void createQuestions();
	void createEnemyLocations();
	void createFixEnemies();
	void recreateEnemies();
	void createInventory();
	void createPickable(GameEnemy *enemy);

	void linkQuestionToEnemies(QList<GameEnemy *> enemies);
	void linkPickablesToEnemies(QList<GameEnemy *> enemies);
	void relinkQuestionToEnemy(GameEnemy * enemy);

signals:
	void playerChanged();
	void runningChanged();
	void sceneChanged();
	void activeEnemiesChanged();
	void pickableChanged();
	void timeNotify();
	void toolChanged(GamePickable::PickableType type, int count);
	void toolListIconsChanged();

private:
	void onSceneStarted();
	void onMsecLeftChanged();
	void onGameQuestionSuccess(const QVariantMap &answer);
	void onGameQuestionFailed(const QVariantMap &answer);
	void onGameQuestionStarted();
	void onGameQuestionFinished();
	void onGameTimeout();
	void onGameSuccess();
	void onGameFailed();

	void onPlayerDied(GameEntity *);
	void onEnemyDied(GameEntity *entity);

protected:
	/**
	 * @brief The ActionGameTickTimer class
	 */

	class TickTimer {
	public:
		TickTimer() {}

		void start(ActionGame *game, const qint64 &startTick = 0) {
			m_reference.start();
			m_startTick = startTick > 0 ? startTick : QDateTime::currentMSecsSinceEpoch();
			m_timer.start(m_interval, Qt::PreciseTimer, game);
		}

		void stop() {
			m_reference.invalidate();
			m_timer.stop();
		}

		const qint64 &latency() const { return m_latency; }
		void setLatency(const qint64 &latency) { m_latency = latency; }

		qint64 currentTick() const {
			if (!m_reference.isValid())
				return 0;

			const qint64 &elapsed = m_reference.elapsed();
			return m_startTick+elapsed+m_latency;
		}

		static int interval() { return m_interval; }

	private:
		QBasicTimer m_timer;
		QElapsedTimer m_reference;
		qint64 m_startTick = 0;
		qint64 m_latency = 0;
		static const int m_interval;
	};

	TickTimer m_tickTimer;

	void timeNotifySendReset();

	QPointer<GameScene> m_scene = nullptr;
	QPointer<GameEntity> m_player = nullptr;
	bool m_running = true;
	std::vector<std::unique_ptr<QuestionLocation>> m_questions;
	std::vector<std::unique_ptr<EnemyLocation>> m_enemies;
	QVector<int> m_closedBlocks;

	typedef QPair<GamePickable::GamePickableData, int> Inventory;
	QVector<Inventory> m_inventory;

	QStack<GamePickable*> m_pickableStack;
	QPointer<GameEntity> m_attackedEnemy = nullptr;

	static const QHash<QString, QVector<GamePickable::PickableType>> m_toolDependency;
	QHash<GamePickable::PickableType, int> m_tools;

	static QStringList m_availableCharacters;

	int m_timeNotifySendNext = -1;
	int m_killStreak = 0;

private:


	/**
	 * @brief The Tooltip class
	 */

	struct Tooltip {
		QString icon;
		QString title;
		QString text;
	};

	static const QHash<QString, Tooltip> m_tooltips;
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
