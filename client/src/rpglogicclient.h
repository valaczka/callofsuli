/*
 * ---- Call of Suli ----
 *
 * rpglogicclient.h
 *
 * Created on: 2026. 05. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLogicClient
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

#ifndef RPGLOGICCLIENT_H
#define RPGLOGICCLIENT_H

#include "chipmunk/chipmunk_types.h"
#include "rpgmapplaytutorial.h"
#include <rpglogic.h>
#include <QElapsedTimer>

class RpgObject;
class RpgTower;
class RpgUdpEngine;
class RpgGamePrivate;
class RpgEntity;
class RpgPlayer;
class TiledObjectBody;


namespace Rpg {


/**
 * @brief The RpgEntityStatePull class
 */



typedef BaseStatePull<RpgStream::PlayerState, 10> RpgPlayerStatePull;
typedef BaseStatePull<RpgStream::NpcState, 10> RpgNpcStatePull;




/**
 * @brief The RpgLogicClient class
 */

class RpgLogicClient : public RpgLogic
{
public:
	RpgLogicClient(const quint32 &lastAuthDiff, const quint32 &jitterDiff);

	void addLocalIdTag(entt::entity entity);

	void removeFromMapper(RpgObject *object);
	QPoint getChunkFromVector(const cpVect &point, cpVect *centerPtr = nullptr);
	QPoint getChunkFromVector(const cpVect &point, const float &angle, cpVect *centerPtr = nullptr);

	quint32 jitterTick(const quint32 &tick) const {
		return tick > (m_jitterDiff+m_lastAuthTickDiff) ? (tick-m_jitterDiff-m_lastAuthTickDiff) : 0;
	}

	const quint32 &lastAuthDiff() const { return m_lastAuthTickDiff; }
	const quint32 &jitterDiff() const { return m_jitterDiff; }

	quint32 estimatedServerTick() const;

	qint64 serverRtt() const { return m_serverRtt; }
	void setServerRtt(qint64 newServerRtt) { m_serverRtt = newServerRtt; }


	entt::entity addNpc(const RpgStream::NpcData &data, entt::entity owner);


	template <typename T, std::size_t PULL_SIZE = DEFAULT_PULL_SIZE,
			  typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	std::map<quint32, T> getSimulatedStates(entt::entity ent, const BaseStatePull<T, PULL_SIZE> &local,
											const T** latestPtr = nullptr) const
	{
		QMutexLocker locker(&m_mutex);

		std::map<quint32, T> ret;

		const BaseStatePull<T> *pull = m_registry.try_get<BaseStatePull<T> >(ent);

		if (!pull)
			return ret;

		const T* latest = pull->latest();

		if (!latest)
			return ret;

		if (latestPtr)
			*latestPtr = latest;

		return local.extractToMap(latest->tick());
	}


protected:
	qint64 m_serverRtt = 0;
	const quint32 m_jitterDiff = 0;

	QElapsedTimer m_lastInputTimer;
};







/**
 * @brief The RpgLogicClientSingle class
 */

class RpgLogicClientSingle : public RpgLogicClient
{
public:
	RpgLogicClientSingle(RpgGame *game);

	virtual RpgStream::GameConfig start();
	RpgStream::GameConfig startGame();

	virtual void overrideMapData(RpgStream::MapData &data);
	virtual void overrideResultData(QVariantMap &data);

	RpgGame *game() const { return m_game; }

	virtual RpgStream::Result getResult() override;

protected:
	virtual void eventRealized(entt::entity entity) override;
	virtual void rewindStage(const RpgStream::GameConfig::Stage &oldStage) override;
	virtual Rpg::QuestList getQuestList() const override;
	virtual bool checkReadyToFinish() override;

protected:
	RpgGame *const m_game;

	bool m_readyToFinish = false;
};







/**
 * @brief The RpgLogicClientTutorial class
 */

class RpgLogicClientTutorial : public RpgLogicClientSingle
{
public:
	struct Tutorial {
		QString character;
		QString terrain;
		int power = 1;
		quint32 duration = 0;										// 0 = default


		std::function<void(RpgLogicClientTutorial *)> fnInit;
		std::function<void(RpgLogicClientTutorial *)> fnFirst;

		std::optional<std::unordered_set<quint32> > towers;			// nullopt: default
		std::optional<std::unordered_set<quint32> > emitters;		// nullopt: default
		std::optional<std::unordered_set<QString> > chests;			// nullopt: default

		QuestList questList;

		struct Step {
			QString message;

			QString infoTitle;
			QString infoText;
			QString infoIcon;

			std::vector<std::unique_ptr<RpgStream::BaseTickState> > inputEvents;

			std::function<void(RpgLogicClientTutorial *, const quint32 &)> fnNext;

			void addTargetEntityEvent(const std::function<bool(RpgEntity*)> &fn);
			void addTargetControlEvent(const std::function<bool(TiledObjectBody*)> &fn);
			void addMpEmitterEmptyEvent(const int &tmxId);
			void addPlayerEvent(const std::function<bool (RpgPlayer *, const RpgStream::EventPlayer &)> &fn);
			void addTowerEvent(const std::function<bool(RpgLogicClientTutorial *logic, const EventTowerActiveChanged &)> &fn);
		};

		std::vector<Step> steps;

		int currentStep = -1;

		void addTower(const quint32 &tmxId) {
			if (!towers) towers = std::unordered_set<quint32>{};
			towers->insert(getId(tmxId));
		}

		void noTowers() { towers = std::unordered_set<quint32>{}; }

		void addEmitter(const quint32 &tmxId) {
			if (!emitters) emitters = std::unordered_set<quint32>{};
			emitters->insert(getId(tmxId));
		}

		void noEmitters() { emitters = std::unordered_set<quint32>{}; }

		void addChest(const QString &entryPoint) {
			if (!chests) chests = std::unordered_set<QString>{};
			chests->insert(entryPoint);
		}

		void noChests() { chests = std::unordered_set<QString>{}; }
	};

	RpgLogicClientTutorial(RpgGame *game, std::unique_ptr<Tutorial> tutorial);
	virtual ~RpgLogicClientTutorial();

	virtual void overrideMapData(RpgStream::MapData &data) override;
	virtual void overrideResultData(QVariantMap &data) override;

	bool loadGameData(RpgStream::CharacterSelectClient *dest);

	void initialize();

	static quint32 getId(const quint32 &tmxId) { return RpgLogic::packId(1, 0, tmxId); }

	RpgPlayer *player() const;

	void npcAddToPoint(const QString &character, const QStringList &entryPoint, const RpgStream::Team &team = RpgStream::TeamNone,
					   const int &num = 1, const int &delay = 0);

	void npcAddToPoint(const QString &character, const QString &entryPoint, const RpgStream::Team &team = RpgStream::TeamNone,
					   const int &num = 1, const int &delay = 0) {
		npcAddToPoint(character, QStringList{entryPoint}, team, num, delay);
	}

	void npcAddToPoint(const QString &character, const QStringList &entryPoint, const int &num, const int &delay = 0) {
		npcAddToPoint(character, entryPoint, RpgStream::TeamNone, num, delay);
	}

	void npcAddToPoint(const QString &character, const QString &entryPoint, const int &num, const int &delay = 0) {
		npcAddToPoint(character, QStringList{entryPoint}, RpgStream::TeamNone, num, delay);
	}

	bool towerSet(const quint32 &tmxId, const RpgStream::Team &team, const quint32 &load);

	bool defenderAddToTower(const quint32 &tmxId, const RpgStream::BaseDefenderObject::Type &type, const RpgStream::Team &team);
	bool defenderAddToPoint(const QString &entryPoint, const RpgStream::BaseDefenderObject::Type &type, const RpgStream::Team &team);

	bool chestAddToPoint(const QString &entryPoint);

protected:
	virtual void eventRealized(entt::entity entity) override;
	virtual void onTargetEntityChanged();
	virtual void onTargetControlChanged();

	virtual std::unordered_set<entt::entity> initializeTowers() override;
	virtual std::unordered_set<entt::entity> initializeEmitters()override;
	virtual std::vector<Chest> initializeChests() override;
	virtual void initializeStages() override;
	virtual Rpg::QuestList getQuestList() const override;
	virtual bool checkReadyToFinish() override;

	int stepForward();

	void onTimerTimeout();
	void onTutorialFinished();



	class EventTargetEntityChanged : public RpgStream::BaseTickState
	{
	public:
		EventTargetEntityChanged() : RpgStream::BaseTickState() {  }

		std::function<bool(RpgEntity*)> fnCmp;

		RpgEntity *m_target = nullptr;
	};


	class EventTargetControlChanged : public RpgStream::BaseTickState
	{
	public:
		EventTargetControlChanged() : RpgStream::BaseTickState() { }

		std::function<bool(TiledObjectBody*)> fnCmp;

		TiledObjectBody *m_target = nullptr;
	};


	class EventTmxMpEmitterEmpty : public RpgStream::BaseTickState
	{
	public:
		EventTmxMpEmitterEmpty() : RpgStream::BaseTickState() { }

		int tmxId = 0;
	};


	class EventPlayerEvent : public RpgStream::BaseTickState
	{
	public:
		EventPlayerEvent() : RpgStream::BaseTickState() { }

		std::function<bool(RpgPlayer *, const RpgStream::EventPlayer &)> fnCmp;
	};


	class EventTowerEvent : public RpgStream::BaseTickState
	{
	public:
		EventTowerEvent() : RpgStream::BaseTickState() { }

		std::function<bool(RpgLogicClientTutorial *logic, const EventTowerActiveChanged &)> fnCmp;
	};


private:
	void onControlledPlayerChanged();
	void checkEvent(entt::entity entity);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T2>::value>::type>
	bool compareEvent(const T &, const T &);


	bool compareEvent(const RpgStream::EventStageChanged &step, const RpgStream::EventStageChanged &event);
	bool compareEvent(const EventTmxMpEmitterEmpty &step, const EventMpEmitterEmpty &event);
	bool compareEvent(const EventPlayerEvent &step, const RpgStream::EventPlayer &event);
	bool compareEvent(const EventTowerEvent &step, const EventTowerActiveChanged &event);


	template <typename T,
			  typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	bool compareEvent(const RpgLogicScope &scope, entt::entity entity, const RpgStream::BaseTickState *state);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T2>::value>::type>
	bool compareEventDiff(const RpgLogicScope &scope, entt::entity entity, const RpgStream::BaseTickState *state);


protected:
	std::unique_ptr<Tutorial> m_tutorial;
	Tutorial::Step *m_currentStep = nullptr;

	QTimer m_messageTimer;

	friend struct Tutorial;
	friend struct Tutorial::Step;
};







/**
 * @brief The RpgLogicClientMulti class
 */

class RpgLogicClientMulti : public RpgLogicClient
{
public:
	RpgLogicClientMulti();

	void loadFull(const RpgStream::Full &full);
	void loadFullState(const RpgStream::FullState &full);

	void loadResult(RpgStream::Result &&result);

	RpgUdpEngine *engine() const;
	void setEngine(RpgUdpEngine *newEngine);

private:
	void loadPlayers(const std::vector<RpgStream::PlayerStateList> &list);
	void loadEvents(const std::vector<RpgStream::Events> &list);
	void loadTowers(const std::vector<RpgStream::TowerState> &list);
	void loadMp(const std::vector<RpgStream::MpData> &list);
	void loadDefenders(const std::vector<RpgStream::DefenderState> &list);
	void loadNpc(const std::vector<RpgStream::NpcStateList> &list);
	void loadControls(const std::vector<RpgStream::ControlStateList> &list);

	RpgUdpEngine *m_engine = nullptr;
};





/**
 * @brief The LocalIdTag class
 */

struct LocalIdTag {
	bool placeholder = false;
};

/**
 * @brief The RpgLogicControlledObjects class
 */

struct RpgLogicControlledObjects
{
	quint32 player = 0;
	std::unordered_set<quint32> entities;
};

}		// end of namespace

#endif // RPGLOGICCLIENT_H
