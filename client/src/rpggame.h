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
#include "tiledgame.h"


class RpgGamePrivate;
class RpgGameItem;

#ifndef OPAQUE_PTR_RpgGameItem
#define OPAQUE_PTR_RpgGameItem
Q_DECLARE_OPAQUE_POINTER(RpgGameItem*)
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






/**
 * @brief The RpgGame class
 */

class RpgGame : public AbstractLevelGame
{
	Q_OBJECT

	Q_PROPERTY(GameState gameState READ gameState WRITE setGameState NOTIFY gameStateChanged FINAL)
	Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged FINAL)
	Q_PROPERTY(RpgGameItem *gameItem READ gameItem WRITE setGameItem NOTIFY gameItemChanged FINAL)

public:
	RpgGame(GameMapMissionLevel *missionLevel, Client *client);
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




	GameState gameState() const;
	void setGameState(const GameState &newGameState);

	QString errorString() const;
	void setErrorString(const QString &newErrorString);

	RpgGameItem *gameItem() const;
	void setGameItem(RpgGameItem *newGameItem);

signals:
	void gameStateChanged();
	void errorStringChanged();
	void gameItemChanged();

protected:
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;

	void setError(const QString &str = {});

	bool load(const RpgGameDefinition &def);


private:
	RpgGamePrivate *d = nullptr;
	RpgGameItem *m_gameItem = nullptr;
	GameState m_gameState = GameStateInvalid;
	QString m_errorString;


	static QHash<QString, RpgGameDefinition> m_terrains;
	///static QHash<QString, RpgPlayerCharacterConfig> m_characters;

	friend class RpgGamePrivate;
	friend class RpgGameItem;
};

#endif // RPGGAME_H
