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
#include "rpglogic.h"
#include "rpglogicclient.h"
#include "tiledgame.h"


class RpgGamePrivate;
class RpgGameItem;
class RpgPlayer;

#ifndef OPAQUE_PTR_RpgGameItem
#define OPAQUE_PTR_RpgGameItem
Q_DECLARE_OPAQUE_POINTER(RpgGameItem*)
#endif


#ifndef OPAQUE_PTR_RpgPlayer
#define OPAQUE_PTR_RpgPlayer
Q_DECLARE_OPAQUE_POINTER(RpgPlayer*)
#endif


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

	QS_SERIALIZABLE

	// Base

	QS_FIELD(QString, name)
	QS_FIELD(QString, minVersion)

	// Required tileset

	QS_COLLECTION(QList, QString, required)
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
};




/**
 * @brief The RpgGame class
 */

class RpgGame : public AbstractLevelGame
{
	Q_OBJECT

	Q_PROPERTY(GameState gameState READ gameState WRITE setGameState NOTIFY gameStateChanged FINAL)
	Q_PROPERTY(GameMode gameMode READ gameMode WRITE setGameMode NOTIFY gameModeChanged FINAL)
	Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged FINAL)
	Q_PROPERTY(RpgGameItem *gameItem READ gameItem WRITE setGameItem NOTIFY gameItemChanged FINAL)
	Q_PROPERTY(RpgPlayer *controlledPlayer READ controlledPlayer WRITE setControlledPlayer NOTIFY controlledPlayerChanged FINAL)

public:
	RpgGame(GameMapMissionLevel *missionLevel, Client *client, const bool &multiplayer);
	virtual ~RpgGame();

	enum GameState {
		GameStateInvalid = 0,
		GameStateConnect,
		GameStateDownloadStatic,
		GameStateLobby,
		GameStateCharacterSelect,
		GameStateDownloadContent,
		GameStatePrepare,
		GameStateInit,
		GameStatePlay,
		GameStateFinished,
		GameStateError
	};

	Q_ENUM(GameState)


	enum GameMode {
		SinglePlayer = 0,
		MultiPlayerGuest,
		MultiPlayerHost
	};

	Q_ENUM(GameMode)


	Q_INVOKABLE void gameAbort() override;
	Q_INVOKABLE void loadGameItem();
	Q_INVOKABLE void gameItemPrepared();

	Q_INVOKABLE void menuBgMusicPlay();
	Q_INVOKABLE void menuBgMusicStop();



	static const QHash<QString, RpgGameDefinition> &terrains() { return m_terrains; }
	static void reloadTerrains();

	static std::optional<RpgGameDefinition> readGameDefinition(const QString &map);

	/*static const QHash<QString, RpgPlayerCharacterConfig> &characters();
	static void reloadCharacters();*/

	static void reloadWorld();

	Rpg::RpgLogicClient* rpgLogicClient();

	virtual int msecLeft() const override;

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

signals:
	void gameStateChanged();
	void errorStringChanged();
	void gameItemChanged();
	void controlledPlayerChanged();
	void gameModeChanged();

protected:
	virtual void timerEvent(QTimerEvent *) override;
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;

	void setError(const QString &str = {});

	bool load(const RpgGameDefinition &def);


private:
	RpgGamePrivate *d = nullptr;
	RpgGameItem *m_gameItem = nullptr;
	RpgPlayer *m_controlledPlayer = nullptr;

	GameMode m_gameMode;
	GameState m_gameState = GameStateInvalid;
	QString m_errorString;


	static QHash<QString, RpgGameDefinition> m_terrains;
	///static QHash<QString, RpgPlayerCharacterConfig> m_characters;

	friend class RpgGamePrivate;
	friend class RpgGameItem;
};

#endif // RPGGAME_H
