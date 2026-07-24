/*
 * ---- Call of Suli ----
 *
 * rpggame_p.h
 *
 * Created on: 2026. 05. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef RPGGAME_P_H
#define RPGGAME_P_H

#include <QObject>
#include <libtiledquick/tilelayeritem.h>
#include "rpggame.h"
#include "rpglogic.h"
#include "rpglogicclient.h"
#include "rpgobject.h"
#include "rpggameitem.h"
#include "rpgplayer.h"
#include "rpgtower.h"
#include "rpgudpengine.h"



#ifdef WITH_GAMEPAD
#include <QtGamepadLegacy/QGamepad>
#endif



/**
 * @brief The CommonGameDefinition class
 */

class CommonGameDefinition : public TiledGameDefinition {
	Q_GADGET

public:
	CommonGameDefinition() : TiledGameDefinition() {
		basePath = QStringLiteral("qrc:/map/map_common/");

		static const QStringList list = {
			QStringLiteral("def_pulse.tmx"),
			QStringLiteral("def_multiplier1.tmx"),
		};

		for (const QString &s : list) {
			TiledSceneDefinition sc;
			sc.file = s;
			scenes.emplace_back(std::move(sc));
		}
	}

};


/**
 * @brief The RpgGamePrivate class
 */

class RpgGamePrivate : public QObject
{
	Q_OBJECT

private:
	RpgGamePrivate(RpgGame *game, const bool &multi, std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> tutorial);
	virtual ~RpgGamePrivate();

	static QString toReadableRoomId(const RpgStream::Room &room);

	void clearSharedTextures();
	void vibrate();
	bool loadCommonMaps();


	// Connect

	void connectionPrepare();
	void connectionCheck();
	void connectionReady();

	void onServerConnected();
	void onServerDisconnected();
	void onConnectionFailed(const QString &err);
	void onConnectionLost();

	void contentPrepare();
	void onDownloaderStateChanged();
	void onContentDownloaded();
	void onContentError();


	// Character select

	void characterSelect(const QVariantMap &data);
	void updateCharacterSelect();

	void loadQuests(const Rpg::QuestList &list, const quint32 &msecLeft);
	void questSelect(const QVariantMap &data);
	void questSelectTimerSet(const qint64 msec);

	// Prepare

	void prepareGameItem();
	void onGameItemPrepared();
	void loadChunkGrid();
	void loadHeat();
	void playerPositionAdd(const QPointF &pos, const RpgStream::Team &team);
	void mpEmitterAdd(const QPointF &pos, const quint32 &tagId, QQuickItem *visualItem, const QList<TiledObjectBody *> &excludeList);
	void towerAdd(RpgTower *tower);
	void chestPositionAdd(const QPointF &pos);
	void entryPointAdd(const QString &name, const QPointF &pos);

	void addLocationSound(TiledObjectBody *object, const QString &sound,
						  const qreal &baseVolume = 1.,
						  const Sound::ChannelType &channel = Sound::Music2Channel);



	void connectJoysticks();
	void setJoystickState(RpgPlayer *player, const TiledGame::Joystick &joystick, const TiledGame::JoystickState &state);
	bool setFromGamepad(RpgMotorPlayerControlled *motor);

	Q_INVOKABLE void joystickClickedA(const bool &clicked);
	Q_INVOKABLE void joystickClickedB(const bool &clicked);
	Q_INVOKABLE void joystickClickedC(const bool &clicked);
	Q_INVOKABLE void joystickClickedD(const bool &clicked);


	// Questions

	void reloadQuestions();
	bool initializeQuestions();
	bool nextQuestion();
	void onQuestionSuccess(const QVariantMap &answer);
	void onQuestionFailed(const QVariantMap &answer);
	void onQuestionStarted();
	void onQuestionFinished();

	// Play

	void startGame(const quint32 &tick = 0);
	void onBeforeWorldStep(const qint64 &tick);
	void onAfterWorldStep(const RpgStream::FullState &full);
	void finishGame();
	QVariantMap getQuestResult(const RpgStream::Result &result, const RpgStream::PlayerResult &player) const;


	// Synchronize

	void syncGameConfig(const RpgStream::GameConfig &config, const quint32 &tick);

	void syncGameState();

	void syncTowersAndEmitters();
	void syncObjects();
	void syncPlayers();
	void syncMp();
	void syncDefenders();
	void syncNpc();
	void syncControls();

	std::optional<ScatterPoint> addToScatter(const int &scatter);

	struct ObjectSet
	{
		QSet<quint32> player;
		QSet<quint32> mp;
		QSet<quint32> defender;
		QSet<quint32> control;
	};

	ObjectSet extractObjects(const RpgStream::FullState &full) const;

	void deleteMissingObjects(const ObjectSet &objects);

	void processEvents(const std::vector<RpgStream::Events> &list, const qint64 &tick);
	void processEvents(const std::vector<RpgStream::EventPlayer> &list);
	void processEvents(const std::vector<RpgStream::EventStageChanged> &list);
	void processEvents(const std::vector<RpgStream::EventMpEmitter> &list);
	void processEvents(const std::vector<RpgStream::EventNpc> &list);
	void processEvents(const std::vector<RpgStream::EventControl> &list);
	void processEvents(const std::vector<RpgStream::EventDefender> &list);

	void onTimeStepped(const std::vector<TiledObjectBody *> &aboutDestruction);

	/// RPG LOGIC LOCAL

	quint32 logicRegisterObject(RpgObject *object);

private:
	RpgGame *const q;

	RpgGameDefinition m_gameDefinition;
	RpgStream::QuestSelect m_questSelect;
	QDeadlineTimer m_questSelectTimer;
	QBasicTimer m_questBasicTimer;

	bool m_isMapLoaded = false;
	bool m_isMapSynchronized = false;

	RpgStream::CharacterSelectClient m_characterSelect;

	std::unique_ptr<Rpg::RpgLogicClient> m_logic;
	quint32 m_deadlineTick = 0;
	qint64 m_lastProcessedEventTick = -1;			// a szervertől jött utoljára feldolgozott eseménylista tick-je


	QVector<Question> m_questionList;
	QVector<Question>::const_iterator m_questionIterator;
	bool m_questionInitialized = false;
	quint32 m_questionDuration = 0;					// todo: csökkenteni a kérdések számát...


	RpgStream::MapData m_mapData;
	QHash<quint32, QPointer<RpgTower> > m_towerList;
	QHash<quint32, QPair<QPointer<QQuickItem>, QList<TiledObjectBody*> > > m_emitters;
	QHash<QString, QPointF> m_entryPoint;
	std::vector<std::unique_ptr<TiledGameSfxLocation>> m_sfxLocations;
	QList<QScatterSeries*> m_scatters;



	inline static RpgStream::HashFnv1A64 m_terrainHash = {};
	inline static RpgStream::HashFnv1A64 m_characterHash = {};
	inline static RpgStream::HashFnv1A64 m_npcHash = {};
	inline static QHash<QString, RpgNpcDefinition> m_npcDefinitions = {};
	inline static CommonGameDefinition m_commonGameDefinition = {};

	std::unordered_map<QString, std::pair< std::unique_ptr<Tiled::Map>, std::unique_ptr<Tiled::MapRenderer> > > m_commonMaps;


#ifdef WITH_GAMEPAD
	void onGamePadChanged(const double &);
	void onGamePadButtonL3Changed(const bool &pressed);
	void onGamePadButtonR3Changed(const bool &pressed);
	void onGamePadButtonL1Changed(const bool &pressed);
	void onGamePadButtonR1Changed(const bool &pressed);
	std::unique_ptr<QGamepad> m_gamePad;
#endif


	// Multiplayer

	QByteArray m_connectionToken;
	std::unique_ptr<RpgUdpEngine> m_engine;


	friend class RpgGame;
	friend class RpgGameItem;
	friend class RpgUdpEngine;
};







#endif // RPGGAME_P_H
