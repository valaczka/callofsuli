/*
 * ---- Call of Suli ----
 *
 * tiledgame.h
 *
 * Created on: 2024. 03. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledGame
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

#ifndef TILEDGAME_H
#define TILEDGAME_H

#include "qsgtexture.h"
#include "tiledscene.h"
#include "tiledtransport.h"
#include <QQuickItem>
#include <QSerializer>
#include <unordered_map>



/**
 * @brief The TiledSceneDefinition class
 */

class TiledSceneDefinition : public QSerializer
{
	Q_GADGET

public:
	TiledSceneDefinition()
		: id(-1)
	{}

	QS_SERIALIZABLE
	QS_FIELD(int, id)
	QS_FIELD(QString, file)
};


/**
 * @brief The TiledGameDefinition class
 */

class TiledGameDefinition : public QSerializer
{
	Q_GADGET

public:
	TiledGameDefinition()
		: firstScene(-1)
	{}

	QS_SERIALIZABLE
	QS_FIELD(int, firstScene)
	QS_COLLECTION_OBJECTS(QVector, TiledSceneDefinition, scenes)
};







/**
 * @brief The TiledGame class
 */

class TiledGame : public QQuickItem
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(GameMode gameMode READ gameMode WRITE setGameMode NOTIFY gameModeChanged FINAL)
	Q_PROPERTY(TiledScene *currentScene READ currentScene WRITE setCurrentScene NOTIFY currentSceneChanged FINAL)
	Q_PROPERTY(QQuickItem *joystick READ joystick WRITE setJoystick NOTIFY joystickChanged FINAL)
	Q_PROPERTY(TiledObject *followedItem READ followedItem WRITE setFollowedItem NOTIFY followedItemChanged FINAL)
	Q_PROPERTY(JoystickState joystickState READ joystickState WRITE setJoystickState NOTIFY joystickStateChanged FINAL)
	Q_PROPERTY(bool debugView READ debugView WRITE setDebugView NOTIFY debugViewChanged FINAL)

public:
	explicit TiledGame(QQuickItem *parent = nullptr);
	virtual ~TiledGame();


	enum GameMode {
		SinglePlayer = 0,
		MultiPlayerHost,
		MultiPlayerGuest
	};

	Q_ENUM(GameMode);


	struct JoystickState {
		qreal dx = 0;
		qreal dy = 0;
		qreal angle = 0;
		qreal distance = 0;
		bool hasTouch = false;
		bool hasKeyboard = false;

		inline friend bool operator==(const JoystickState &s1, const JoystickState &s2) {
			return s1.dx == s2.dx &&
					s1.dy == s2.dy &&
					s1.angle == s2.angle &&
					s1.distance == s2.distance &&
					s1.hasTouch == s2.hasTouch &&
					s1.hasKeyboard == s2.hasKeyboard;
		}
	};


	bool load(const TiledGameDefinition &def);

	Tiled::TileLayer *loadSceneLayer(TiledScene *scene, Tiled::Layer *layer, Tiled::MapRenderer *renderer);

	static QSGTexture *getTexture(const QString &path, QQuickWindow *window);

	TiledScene *findScene(const int &id) const;

	TiledScene *currentScene() const;
	void setCurrentScene(TiledScene *newCurrentScene);

	const TiledTransportList &transportList() const;

	Q_INVOKABLE bool transport(TiledObject *object, TiledTransport *transport);

	virtual void playerAttackEnemy(TiledObject *player, TiledObject *enemy) = 0;
	virtual void enemyAttackPlayer(TiledObject *enemy, TiledObject *player) = 0;


	void playSfx(const QString &source, TiledScene *scene);
	void playSfx(const QString &source, TiledScene *scene, const QPointF &position);

	QQuickItem *joystick() const;
	void setJoystick(QQuickItem *newJoystick);

	JoystickState joystickState() const;
	void setJoystickState(const JoystickState &newJoystickState);

	bool debugView() const;
	void setDebugView(bool newDebugView);

	TiledObject *followedItem() const;
	void setFollowedItem(TiledObject *newFollowedItem);

	GameMode gameMode() const;
	void setGameMode(const GameMode &newGameMode);

signals:
	void gameLoaded();
	void gameLoadFailed();
	void currentSceneChanged();
	void joystickChanged();
	void followedItemChanged();
	void joystickStateChanged();
	void debugViewChanged();
	void gameModeChanged();

protected:
	bool loadScene(const int sceneId, const QString &file);
	virtual bool loadObjectLayer(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer);
	TiledObjectBase *loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	bool loadTransport(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	void addPlayerPosition(TiledScene *scene, const QPointF &position);

	int findLoadedObject(const int &id, const int &sceneId) const;
	bool addLoadedObject(const int &id, const int &sceneId);

	void changeScene(TiledObject *object, TiledScene *from, TiledScene *to, const QPointF &toPoint);


	virtual void loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void joystickStateEvent(const JoystickState &newJoystickState) { Q_UNUSED(newJoystickState);}
	virtual bool transportBeforeEvent(TiledObject *object, TiledTransport *transport);
	virtual bool transportAfterEvent(TiledObject *object, TiledScene *newScene, TiledObjectBase *newObject);


protected:
	struct Scene {
		QQuickItem *container = nullptr;
		TiledScene *scene = nullptr;
	};

	struct PlayerPosition {
		int sceneId = -1;
		TiledScene *scene = nullptr;
		QPointF position;

		friend bool operator==(const PlayerPosition &p1, const PlayerPosition &p2) {
			return p1.sceneId == p2.sceneId &&
					p1.scene == p2.scene &&
					p1.position == p2.position;
		}
	};

	GameMode m_gameMode = SinglePlayer;
	QVector<Scene> m_sceneList;
	QVector<TiledObjectBase::Object> m_loadedObjectList;
	QVector<PlayerPosition> m_playerPositionList;
	TiledTransportList m_transportList;

	TiledScene *m_currentScene = nullptr;



private:
	struct KeyboardJoystickState {
		qreal dx = 0.5;
		qreal dy = 0.5;
	};

	void joystickConnect(const bool &connect = true);
	Q_INVOKABLE void updateJoystick();
	void updateKeyboardJoystick(const KeyboardJoystickState &state);


	KeyboardJoystickState m_keyboardJoystickState;
	QPointer<QQuickItem> m_joystick = nullptr;
	QPointer<TiledObject> m_followedItem = nullptr;
	JoystickState m_joystickState;
	static std::unordered_map<QString, std::unique_ptr<QSGTexture>> m_sharedTextures;
	bool m_debugView = false;
};

Q_DECLARE_METATYPE(TiledGame::JoystickState)

#endif // TILEDGAME_H
