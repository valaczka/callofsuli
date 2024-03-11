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

#include "isometricplayer.h"
#include "qsgtexture.h"
#include "tiledscene.h"
#include "tiledtransport.h"
#include <QQuickItem>


#include <QSerializer>



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

	Q_PROPERTY(TiledScene *currentScene READ currentScene WRITE setCurrentScene NOTIFY currentSceneChanged FINAL)
	Q_PROPERTY(QQuickItem *joystick READ joystick WRITE setJoystick NOTIFY joystickChanged FINAL)
	Q_PROPERTY(TiledObject *followedItem READ followedItem WRITE setFollowedItem NOTIFY followedItemChanged FINAL)
	Q_PROPERTY(IsometricPlayer *controlledPlayer READ controlledPlayer WRITE setControlledPlayer NOTIFY controlledPlayerChanged FINAL)
	Q_PROPERTY(JoystickState joystickState READ joystickState WRITE setJoystickState NOTIFY joystickStateChanged FINAL)
	Q_PROPERTY(bool debugView READ debugView WRITE setDebugView NOTIFY debugViewChanged FINAL)

public:
	explicit TiledGame(QQuickItem *parent = nullptr);
	virtual ~TiledGame();

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


	enum ObjectClass {
		Invalid = 0,
		Player,
		EnemyWerebear
	};

	Q_ENUM(ObjectClass);


	Q_INVOKABLE bool load();
	bool load(const TiledGameDefinition &def);


	bool addGate(const QString &name, TiledScene *scene, TiledObjectBase *object);

	Q_INVOKABLE void switchScene();

	void loadPlayer(TiledScene *scene, const QPointF &pos);






	Tiled::TileLayer *loadSceneLayer(TiledScene *scene, Tiled::Layer *layer, Tiled::MapRenderer *renderer);

	const std::shared_ptr<QSGTexture> &getTexture(const QString &path);

	TiledScene *findScene(const int &id) const;

	TiledScene *currentScene() const;
	void setCurrentScene(TiledScene *newCurrentScene);

	const TiledTransportList &transportList() const;

	QQuickItem *joystick() const;
	void setJoystick(QQuickItem *newJoystick);

	IsometricPlayer *controlledPlayer() const;
	void setControlledPlayer(IsometricPlayer *newControlledItem);

	JoystickState joystickState() const;
	void setJoystickState(const JoystickState &newJoystickState);

	bool debugView() const;
	void setDebugView(bool newDebugView);

	TiledObject *followedItem() const;
	void setFollowedItem(TiledObject *newFollowedItem);

signals:
	void currentSceneChanged();
	void joystickChanged();
	void followedItemChanged();
	void controlledPlayerChanged();
	void joystickStateChanged();
	void debugViewChanged();

protected:
	bool loadScene(const int sceneId, const QString &file);
	virtual bool loadObjectLayer(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer);
	bool loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	bool loadTransport(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);	//??


	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;


protected:
	struct Scene {
		QQuickItem *container = nullptr;
		TiledScene *scene = nullptr;
	};

	struct Object {
		ObjectClass objectClass = Invalid;
		int id = -1;
		int sceneId = -1;					// Original scene id
		TiledScene *scene = nullptr;		// Current scene (!)
		TiledObject *object = nullptr;
	};

	QVector<Scene> m_sceneList;
	TiledScene *m_currentScene = nullptr;

	QVector<Object> m_objectList;

	std::unique_ptr<IsometricPlayer> m_player;

	TiledTransportList m_transportList;

private:
	struct KeyboardJoystickState {
		qreal dx = 0.5;
		qreal dy = 0.5;
	};

	void joystickConnect(const bool &connect = true);
	Q_INVOKABLE void updateJoystick();
	void updateKeyboardJoystick(const KeyboardJoystickState &state);

	int findObject(const int &id, const int &sceneId) const;


	KeyboardJoystickState m_keyboardJoystickState;
	QPointer<QQuickItem> m_joystick = nullptr;
	QPointer<TiledObject> m_followedItem = nullptr;
	QPointer<IsometricPlayer> m_controlledPlayer = nullptr;
	JoystickState m_joystickState;
	QHash<QString, std::shared_ptr<QSGTexture>> m_sharedTextures;
	bool m_debugView = false;
};

Q_DECLARE_METATYPE(TiledGame::JoystickState)

#endif // TILEDGAME_H
