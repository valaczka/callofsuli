/*
 * ---- Call of Suli ----
 *
 * rpggame.h
 *
 * Created on: 2026. 05. 08.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgGame
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

#ifndef RPGGAME_H
#define RPGGAME_H

#include "abstractlevelgame.h"
#include "qslistmodel.h"
#include "rpglogicclient.h"
#include "tiledgame.h"
#include "rpgstream.h"
#include "rpgmapplaytutorial.h"


class RpgGamePrivate;
class RpgGameItem;
class RpgPlayer;
class RpgUdpEngine;

#ifndef OPAQUE_PTR_RpgGameItem
#define OPAQUE_PTR_RpgGameItem
Q_DECLARE_OPAQUE_POINTER(RpgGameItem*)
#endif


#ifndef OPAQUE_PTR_RpgPlayer
#define OPAQUE_PTR_RpgPlayer
Q_DECLARE_OPAQUE_POINTER(RpgPlayer*)
#endif

#ifndef OPAQUE_PTR_RpgUdpEngine
#define OPAQUE_PTR_RpgUdpEngine
Q_DECLARE_OPAQUE_POINTER(RpgUdpEngine*)
#endif




/**
 * @brief The RpgHeatNpc class
 */

class RpgHeatNpc : public QSerializer
{
	Q_GADGET

public:
	RpgHeatNpc()
		: QSerializer()
		, num(1)
		, delay(0)
	{}

	QS_SERIALIZABLE

	QS_FIELD(QString, type)							// NPC típus
	QS_COLLECTION(QList, QString, entry)			// Lehetséges belépési pontok (tmx -> "entry" layer), ha üres, akkor random chunk
	QS_FIELD(int, num)								// Hány jön létre
	QS_FIELD(int, delay)							// Késleltetés (msec) a létrehozás között
};




/**
 * @brief The RpgHeat class
 */

class RpgHeat : public QSerializer
{
	Q_GADGET

public:
	RpgHeat() : QSerializer() {}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, RpgHeatNpc, npc)	// Létrehozandó npc-k
};





/**
 * @brief The RpgGameDefinition class
 */

class RpgGameDefinition : public TiledGameDefinition
{
	Q_GADGET

public:
	RpgGameDefinition()
		: TiledGameDefinition()
	{}

	QStringList getDynamicContent() const;

	QS_SERIALIZABLE

	// Base

	QS_FIELD(QString, name)
	QS_FIELD(QString, minVersion)

	// Heat

	QS_COLLECTION_OBJECTS(QList, RpgHeat, heat)		// A heat-ek

	// Required tileset

	QS_COLLECTION(QList, QString, required)
};







/**
 * @brief The RpgPlayerDefinition class
 */

class RpgPlayerDefinition : public QSerializer
{
	Q_GADGET

public:
	RpgPlayerDefinition() : QSerializer()
	  , power(1)
	  , hp(5)
	  , mp(12)
	  , walk(90)
	  , run(200)

	  , push(350)
	  , pushDistance(1000)
	  , resist(100)

	  , towerPlus(2)
	  , towerMinus(1)

	  , bullet(5)
	{}


	void updateSfxPath(const QString &prefix);

	RpgStream::PlayerConfig toPlayerConfig() const;
	void loadPlayerConfig(const RpgStream::PlayerConfig &cfg);

	QString prefixPath;

	QS_SERIALIZABLE

	QS_FIELD(QString, name)
	QS_FIELD(QString, image)
	QS_FIELD(QString, base)			// Based on character (e.g. sfx, inventory,...)

	// Sfx sounds

	QS_FIELD(QString, sfxDead)
	QS_COLLECTION(QList, QString, sfxPain)
	QS_COLLECTION(QList, QString, sfxFootStep)
	QS_COLLECTION(QList, QString, sfxAccept)
	QS_COLLECTION(QList, QString, sfxDecline)


	// Sprites

	QS_COLLECTION(QList, QString, idleSprites)


	// Config

	QS_FIELD(int, power)			// power level

	QS_FIELD(int, hp)
	QS_FIELD(int, mp)

	QS_FIELD(int, walk)				// walk speed
	QS_FIELD(int, run)				// run speed


	QS_FIELD(int, push)				// push power
	QS_FIELD(int, pushDistance)		// max. push distance
	QS_FIELD(int, resist)			// slide resist

	QS_FIELD(int, towerPlus)		// start step!!!
	QS_FIELD(int, towerMinus)		// start step!!!


	QS_COLLECTION(QList, RpgStream::BaseDefenderObject::Type, defender)				// available defenders
	QS_COLLECTION(QList, RpgStream::PlayerConfig::Utility, utility)					// available utilities


	// Weapon

	QS_FIELD(int, bullet)
};







/**
 * @brief The RpgNpcDefinition class
 */

class RpgNpcDefinition : public QSerializer
{
	Q_GADGET

public:
	RpgNpcDefinition() : QSerializer()
	  , type(RpgStream::NpcData::None)
	  , hp(5)
	  , mp(0)
	  , walk(90)
	  , run(200)

	  , push(350)
	  , pushDistance(1000)
	  , resist(100)
	{}


	void updateSfxPath(const QString &prefix);
	QString prefixPath;

	RpgStream::EntityConfig toEntityConfig() const;
	RpgStream::NpcData toNpcData() const;

	QS_SERIALIZABLE

	QS_FIELD(RpgStream::NpcData::Type, type)

	// Sfx sounds

	QS_FIELD(QString, sfxDead)
	QS_COLLECTION(QList, QString, sfxPain)
	QS_COLLECTION(QList, QString, sfxFootStep)

	// Config

	QS_FIELD(int, hp)
	QS_FIELD(int, mp)				// gained mp on npc's death

	QS_FIELD(int, walk)				// walk speed
	QS_FIELD(int, run)				// run speed

	QS_FIELD(int, push)				// push power
	QS_FIELD(int, pushDistance)		// max. push distance
	QS_FIELD(int, resist)			// slide resist

	// Details

	QS_FIELD(QJsonObject, data)
};













class RpgObject;

/**
 * @brief The RpgLogicObjectMapper class
 */

struct RpgLogicObjectMapper
{
	QHash<quint32, RpgObject* > map;

	static quint32 getId(const TiledObjectBody::ObjectId &id);
	static quint32 getId(const RpgObject *object);
	static TiledObjectBody::ObjectId toObjectId(const quint32 &id);

	quint32 set(RpgObject *object);
	RpgObject *get(const quint32 &id) { return map.value(id); }

	template <class T, typename = std::enable_if<std::is_base_of<RpgObject, T>::value>::type>
	T* get(const quint32 &id) { return qobject_cast<T*>(map.value(id)); }
};









/**
 * @brief The RpgGame class
 */

class RpgGame : public AbstractLevelGame
{
	Q_OBJECT

	Q_PROPERTY(bool isEmpty READ isEmpty CONSTANT FINAL)
	Q_PROPERTY(bool isTutorial READ isTutorial CONSTANT FINAL)
	Q_PROPERTY(GameState gameState READ gameState WRITE setGameState NOTIFY gameStateChanged FINAL)
	Q_PROPERTY(GameMode gameMode READ gameMode WRITE setGameMode NOTIFY gameModeChanged FINAL)
	Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged FINAL)
	Q_PROPERTY(RpgGameItem *gameItem READ gameItem WRITE setGameItem NOTIFY gameItemChanged FINAL)
	Q_PROPERTY(RpgPlayer *controlledPlayer READ controlledPlayer WRITE setControlledPlayer NOTIFY controlledPlayerChanged FINAL)
	Q_PROPERTY(int campaignId READ campaignId WRITE setCampaignId NOTIFY campaignIdChanged FINAL)
	Q_PROPERTY(int gameId READ gameId WRITE setGameId NOTIFY gameIdChanged FINAL)

	Q_PROPERTY(int ptsTeam READ ptsTeam WRITE setPtsTeam NOTIFY ptsTeamChanged FINAL)
	Q_PROPERTY(int ptsOpponent READ ptsOpponent WRITE setPtsOpponent NOTIFY ptsOpponentChanged FINAL)
	Q_PROPERTY(int heat READ heat WRITE setHeat NOTIFY heatChanged FINAL)
	Q_PROPERTY(QColor colorTeam READ colorTeam CONSTANT FINAL)
	Q_PROPERTY(QColor colorOpponent READ colorOpponent CONSTANT FINAL)
	Q_PROPERTY(QColor colorNeutral READ colorNeutral CONSTANT FINAL)
	Q_PROPERTY(QColor colorGlow READ colorGlow CONSTANT FINAL)

	Q_PROPERTY(RpgUserData rpgUserData READ rpgUserData WRITE setRpgUserData NOTIFY rpgUserDataChanged FINAL)
	Q_PROPERTY(QSListModel* modelLobby READ modelLobby CONSTANT FINAL)
	Q_PROPERTY(QSListModel* modelPlayer READ modelPlayer CONSTANT FINAL)
	Q_PROPERTY(QSListModel* modelCharacters READ modelCharacters CONSTANT FINAL)
	Q_PROPERTY(RpgUdpEngine* engine READ engine NOTIFY engineChanged FINAL)

	Q_PROPERTY(QString readableRoom READ readableRoom NOTIFY readableRoomChanged FINAL)
	Q_PROPERTY(QString terrain READ terrain WRITE setTerrain NOTIFY terrainChanged FINAL)

	Q_PROPERTY(int questQuestion READ questQuestion WRITE setQuestQuestion NOTIFY questQuestionChanged FINAL)
	Q_PROPERTY(int questQuestionRq READ questQuestionRq WRITE setQuestQuestionRq NOTIFY questQuestionRqChanged FINAL)
	Q_PROPERTY(int questStreak READ questStreak WRITE setQuestStreak NOTIFY questStreakChanged FINAL)
	Q_PROPERTY(int questStreakRq READ questStreakRq WRITE setQuestStreakRq NOTIFY questStreakRqChanged FINAL)
	Q_PROPERTY(int questPtsRq READ questPtsRq WRITE setQuestPtsRq NOTIFY questPtsRqChanged FINAL)

	Q_PROPERTY(QVariantMap questSelectData READ questSelectData WRITE setQuestSelectData NOTIFY questSelectDataChanged FINAL)
	Q_PROPERTY(QVariantMap questResultData READ questResultData WRITE setQuestResultData NOTIFY questResultDataChanged FINAL)
	Q_PROPERTY(QVariantMap gameResultData READ gameResultData WRITE setGameResultData NOTIFY gameResultDataChanged FINAL)

	Q_PROPERTY(bool isRoomCompleted READ isRoomCompleted WRITE setIsRoomCompleted NOTIFY isRoomCompletedChanged FINAL)
	Q_PROPERTY(bool isCharacterSelect READ isCharacterSelect WRITE setIsCharacterSelect NOTIFY isCharacterSelectChanged FINAL)
	Q_PROPERTY(bool isAllOnboard READ isAllOnboard WRITE setIsAllOnboard NOTIFY isAllOnboardChanged FINAL)

public:
	RpgGame(GameMapMissionLevel *missionLevel, Client *client, const bool &multiplayer,
			std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> tutorial = nullptr);
	virtual ~RpgGame();

	enum GameState {
		GameStateInvalid = 0,
		GameStateDownloadContent,
		GameStateConnect,
		GameStateLobby,
		GameStateCharacterSelect,
		GameStatePrepare,
		GameStateInit,
		GameStatePlay,
		GameStateFinished,
		GameStateError,
		GameStateResult,
		GameStateAbort
	};

	Q_ENUM(GameState)


	enum GameMode {
		SinglePlayer = 0,
		MultiPlayer
	};

	Q_ENUM(GameMode)


	Q_INVOKABLE void gameAbort() override;
	Q_INVOKABLE void loadGameItem();
	Q_INVOKABLE void gameItemPrepared();
	Q_INVOKABLE void downloadAccepted();
	Q_INVOKABLE void reloadRpgData();

	Q_INVOKABLE void menuBgMusicPlay();
	Q_INVOKABLE void menuBgMusicStop();

	Q_INVOKABLE void reloadLobby();
	Q_INVOKABLE void connectLobby(const QVariantMap &data);
	Q_INVOKABLE void characterSelect(const QVariantMap &data);
	Q_INVOKABLE void questSelect(const QVariantMap &data);

	Q_INVOKABLE QUrl getCharacterImage(const QString &character) const;

	Q_INVOKABLE bool loadTutorial(const QString &character);

	Q_INVOKABLE QVariantMap getCharactersMetric() const;
	Q_INVOKABLE QVariantMap getCharacterMetricAtLevel(const QString &character, const int &level) const;

	Q_INVOKABLE bool hasActiveTargetUtility(const RpgStream::PlayerConfig::Utility &utility,
											const RpgStream::Team &target = RpgStream::TeamNone) const;


	static RpgGame *createEmptyGame(Client *client);

	static const QHash<QString, RpgGameDefinition> &terrains() { return m_terrains; }
	static void reloadTerrains();

	static std::optional<RpgGameDefinition> readGameDefinition(const QString &map);

	static const QHash<QString, RpgPlayerDefinition> &characters() { return m_characters; }
	static void reloadCharacters();

	static std::optional<RpgNpcDefinition> readNpcDefinition(const QString &name);

	static void reloadWorld();

	const Tiled::Map* commonMap(const QString &scene) const;
	Tiled::MapRenderer *commonRenderer(const QString &scene) const;

	Rpg::RpgLogicClient* rpgLogicClient();

	bool loadNextQuestion();

	virtual int msecLeft() const override;

	const QJsonObject &getFinishResult() const;

	GameState gameState() const;
	void setGameState(const GameState &newGameState);

	QString errorString() const;
	void setErrorString(const QString &newErrorString);

	RpgGameItem *gameItem() const;
	void setGameItem(RpgGameItem *newGameItem);

	RpgPlayer *controlledPlayer() const;
	void setControlledPlayer(RpgPlayer *newControlledPlayer);

	GameMode gameMode() const;
	void setGameMode(const GameMode &newGameMode);

	int ptsTeam() const;
	void setPtsTeam(int newPtsTeam);

	int ptsOpponent() const;
	void setPtsOpponent(int newPtsOpponent);

	QSListModel* modelLobby() const;

	QSListModel* modelPlayer() const;

	QSListModel* modelCharacters() const;

	QString readableRoom() const;

	QString terrain() const;
	void setTerrain(const QString &newTerrain);

	static QColor colorTeam();
	static QColor colorOpponent();
	static QColor colorNeutral();

	QColor getColor(const RpgStream::Team &team, const QColor &neutral = m_colorOpponent) const;

	static QColor colorGlow();

	std::optional<QPointF> entryPoint(const QString &entry) const;

	int heat() const;
	void setHeat(int newHeat);

	int questQuestion() const;
	void setQuestQuestion(int newQuestQuestion);

	int questQuestionRq() const;
	void setQuestQuestionRq(int newQuestQuestionRq);

	int questStreak() const;
	void setQuestStreak(int newQuestStreak);

	int questStreakRq() const;
	void setQuestStreakRq(int newQuestStreakRq);

	int questPtsRq() const;
	void setQuestPtsRq(int newQuestPtsRq);

	QVariantMap questResultData() const;
	void setQuestResultData(const QVariantMap &newQuestResultData);

	QVariantMap questSelectData() const;
	void setQuestSelectData(const QVariantMap &newQuestSelectData);

	int gameId() const;
	void setGameId(int newGameId);

	int campaignId() const;
	void setCampaignId(int newCampaignId);

	QVariantMap gameResultData() const;
	void setGameResultData(const QVariantMap &newGameResultData);

	bool isEmpty() const;

	bool isTutorial() const;

	RpgUserData rpgUserData() const;
	void setRpgUserData(const RpgUserData &newRpgUserData);

	bool isRoomCompleted() const;
	void setIsRoomCompleted(bool newIsRoomCompleted);

	bool isCharacterSelect() const;
	void setIsCharacterSelect(bool newIsCharacterSelect);

	RpgUdpEngine *engine() const;

	bool isAllOnboard() const;
	void setIsAllOnboard(bool newIsAllOnboard);

signals:
	void downloadRequest(QString size);
	void finishDataReceived(const QJsonObject &data);
	void questSelectCompleted();
	void gameStateChanged();
	void errorStringChanged();
	void gameItemChanged();
	void controlledPlayerChanged();
	void gameModeChanged();
	void ptsTeamChanged();
	void ptsOpponentChanged();
	void readableRoomChanged();
	void terrainChanged();
	void heatChanged();
	void questQuestionChanged();
	void questQuestionRqChanged();
	void questStreakChanged();
	void questStreakRqChanged();
	void questPtsRqChanged();
	void questResultDataChanged();
	void questSelectDataChanged();
	void gameIdChanged();
	void campaignIdChanged();
	void gameResultDataChanged();
	void rpgUserDataChanged();
	void isRoomCompletedChanged();
	void isCharacterSelectChanged();
	void engineChanged();
	void isAllOnboardChanged();

protected:
	virtual void timerEvent(QTimerEvent *) override;
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;
	virtual bool gameFinishEvent() override;

	void setError(const QString &str = {});

	bool load(const RpgGameDefinition &def);


private:
	RpgGamePrivate *d = nullptr;
	RpgGameItem *m_gameItem = nullptr;
	RpgPlayer *m_controlledPlayer = nullptr;

	GameMode m_gameMode;
	GameState m_gameState = GameStateInvalid;
	QString m_errorString;

	int m_campaignId = -1;
	int m_gameId = -1;

	int m_ptsTeam = 0;
	int m_ptsOpponent = 0;
	int m_heat = 0;

	int m_questQuestion = 0;
	int m_questQuestionRq = 0;
	int m_questStreak = 0;
	int m_questStreakRq = 0;
	int m_questPtsRq = 0;

	QVariantMap m_questResultData;
	QVariantMap m_questSelectData;
	QVariantMap m_gameResultData;

	static const QColor m_colorTeam;
	static const QColor m_colorOpponent;
	static const QColor m_colorNeutral;
	static const QColor m_colorGlow;

	QString m_terrain;

	std::unique_ptr<QSListModel> m_modelLobby;
	std::unique_ptr<QSListModel> m_modelPlayer;
	std::unique_ptr<QSListModel> m_modelCharacters;

	RpgUserData m_rpgUserData;
	bool m_isRoomCompleted = false;
	bool m_isCharacterSelect = false;
	bool m_isAllOnboard = false;

	static QHash<QString, RpgGameDefinition> m_terrains;
	static QHash<QString, RpgPlayerDefinition> m_characters;

	friend class RpgGamePrivate;
	friend class RpgGameItem;

};

#endif // RPGGAME_H
