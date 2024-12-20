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
#include "tiledweapon.h"
#include "abstractgame.h"
#include <QQuickItem>
#include <QSerializer>
#include <unordered_map>



/**
 * @brief The TiledGameDefinition class
 */

class TiledGameDefinition : public QSerializer
{
	Q_GADGET

public:
	TiledGameDefinition()
		: firstScene(-1)
		, duration(0)
	{}

	QString basePath;

	QS_SERIALIZABLE
	QS_FIELD(int, firstScene)
	QS_FIELD(int, duration)
	QS_FIELD(QString, music)
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
	Q_PROPERTY(TiledObjectBase *followedItem READ followedItem WRITE setFollowedItem NOTIFY followedItemChanged FINAL)
	Q_PROPERTY(bool debugView READ debugView WRITE setDebugView NOTIFY debugViewChanged FINAL)
	Q_PROPERTY(QQuickItem *messageList READ messageList WRITE setMessageList NOTIFY messageListChanged FINAL)
	Q_PROPERTY(QColor defaultMessageColor READ defaultMessageColor WRITE setDefaultMessageColor NOTIFY defaultMessageColorChanged FINAL)
	Q_PROPERTY(qreal baseScale READ baseScale WRITE setBaseScale NOTIFY baseScaleChanged FINAL)
	Q_PROPERTY(bool mouseNavigation READ mouseNavigation WRITE setMouseNavigation NOTIFY mouseNavigationChanged FINAL)
	Q_PROPERTY(bool mouseAttack READ mouseAttack WRITE setMouseAttack NOTIFY mouseAttackChanged FINAL)
	Q_PROPERTY(bool flickableInteractive READ flickableInteractive WRITE setFlickableInteractive NOTIFY flickableInteractiveChanged FINAL)
	Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged FINAL)

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


	bool load(const TiledGameDefinition &def);

	static std::optional<QStringList> getDynamicTilesets(const TiledGameDefinition &def);

	Tiled::TileLayer *loadSceneLayer(TiledScene *scene, Tiled::Layer *layer, Tiled::MapRenderer *renderer);

	static QSGTexture *getTexture(const QString &path, QQuickWindow *window);
	static void clearSharedTextures() {  m_sharedTextures.clear(); }

	QVector<TiledScene*> sceneList() const;

	TiledScene *findScene(const int &id) const;

	TiledScene *currentScene() const;
	void setCurrentScene(TiledScene *newCurrentScene);

	const TiledTransportList &transportList() const;
	TiledTransportList &transportList();

	bool transport(TiledObject *object, TiledTransport *transport, TiledObjectBase *transportBase = nullptr);

	Q_INVOKABLE virtual void onMouseClick(const qreal &x, const qreal &y, const Qt::MouseButtons &buttons, const int &modifiers);

	virtual bool playerAttackEnemy(TiledObject *player, TiledObject *enemy, const TiledWeapon::WeaponType &weaponType) = 0;
	virtual bool enemyAttackPlayer(TiledObject *enemy, TiledObject *player, const TiledWeapon::WeaponType &weaponType) = 0;
	virtual bool playerPickPickable(TiledObject *player, TiledObject *pickable) = 0;

	virtual void onPlayerDead(TiledObject *player) = 0;
	virtual void onEnemyDead(TiledObject *enemy) = 0;
	virtual void onEnemySleepingStart(TiledObject *enemy) = 0;
	virtual void onEnemySleepingEnd(TiledObject *enemy) = 0;

	void playSfx(const QString &source, TiledScene *scene, const float &baseVolume = 1.) const;
	void playSfx(const QString &source, TiledScene *scene, const QPointF &position, const float &baseVolume = 1.) const;

	static std::optional<qreal> getSfxVolume(TiledScene *scene, const QPointF &position, const float &baseVolume = 1.,
											 const qreal &baseScale = 1.);


	Q_INVOKABLE void messageColor(const QString &text, const QColor &color);
	Q_INVOKABLE void message(const QString &text) { messageColor(text, m_defaultMessageColor); }

	Q_INVOKABLE bool joystickInteractive() const { return m_joystickState.hasKeyboard || m_joystickState.hasTouch; }

	int playerPositionsCount(const int &sceneId) const;
	int playerPositionsCount(TiledScene *scene) const;

	std::optional<QPointF> playerPosition(const int &sceneId, const int &num) const;
	std::optional<QPointF> playerPosition(TiledScene *scene, const int &num) const;


	virtual TiledObjectBasePolygon *loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer,
											   const QPointF &translate = {});


	// Sprite texture helper

	struct TextureSpriteMapper
	{
		QString name;
		TiledObject::Direction direction = TiledObject::Invalid;
		int width = 0;
		int height = 0;
		int duration = 0;
		int loops = 0;
	};

	struct TextureSpriteDirection {
		TextureSprite sprite;
		TiledObject::Direction direction = TiledObject::Invalid;
	};

	static bool loadTextureSprites(TiledSpriteHandler *handler,
								   const QVector<TextureSpriteMapper> &mapper,
								   const QString &path,
								   const QString &layer = QStringLiteral("default"));


	static TextureSprite spriteFromMapper(
			const QVector<TextureSpriteMapper> &mapper,
			const TextureSpriteDef &def,
			const QString &name,
			const TiledObject::Direction &direction = TiledObject::Invalid,
			const int &maxCount = 0);

	static QVector<TextureSpriteDirection> spritesFromMapper(
			const QVector<TextureSpriteMapper> &mapper,
			const TextureSpriteDef &def);

	static QStringList spriteNamesFromMapper(const QVector<TextureSpriteMapper> &mapper);
	static QVector<TiledObject::Direction> directionsFromMapper(const QVector<TextureSpriteMapper> &mapper, const QString &name);

	static bool appendToSpriteHandler(TiledSpriteHandler *handler,
									  const QVector<TextureSpriteDirection> &sprites,
									  const QString &source,
									  const QString &layer = QStringLiteral("default"));

	static bool appendToSpriteHandler(TiledSpriteHandler *handler,
									  const QVector<TextureSprite> &sprites,
									  const QString &source,
									  const QString &layer = QStringLiteral("default"));



	virtual void onSceneWorldStepped(TiledScene *scene);

	AbstractGame::TickTimer *tickTimer() const { return m_tickTimer.get(); }
	void setTickTimer(std::unique_ptr<AbstractGame::TickTimer> &timer) { m_tickTimer = std::move(timer); }

	QQuickItem *joystick() const;
	void setJoystick(QQuickItem *newJoystick);

	JoystickState joystickState() const;
	void setJoystickState(const JoystickState &newJoystickState);

	bool debugView() const;
	void setDebugView(bool newDebugView);

	TiledObjectBase *followedItem() const;
	void setFollowedItem(TiledObjectBase *newFollowedItem);

	QQuickItem *messageList() const;
	void setMessageList(QQuickItem *newMessageList);

	QColor defaultMessageColor() const;
	void setDefaultMessageColor(const QColor &newDefaultMessageColor);

	qreal baseScale() const;
	void setBaseScale(qreal newBaseScale);

	bool mouseNavigation() const;
	void setMouseNavigation(bool newMouseNavigation);

	bool mouseAttack() const;
	void setMouseAttack(bool newMouseAttack);

	bool flickableInteractive() const;
	void setFlickableInteractive(bool newFlickableInteractive);

	bool paused() const;
	void setPaused(bool newPaused);

signals:
	void gameLoaded();
	void gameLoadFailed(const QString &errorString);
	void marketRequest();
	void currentSceneChanged();
	void joystickChanged();
	void followedItemChanged();
	void joystickStateChanged();
	void debugViewChanged();
	void gameModeChanged();
	void messageListChanged();
	void defaultMessageColorChanged();
	void baseScaleChanged();
	void mouseNavigationChanged();
	void mouseAttackChanged();
	void flickableInteractiveChanged();
	void pausedChanged();

protected:
	bool loadScene(const TiledSceneDefinition &def, const QString &basePath);
	virtual bool loadObjectLayer(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer);
	bool loadDynamicZ(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	bool loadTransport(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	void addPlayerPosition(TiledScene *scene, const QPointF &position);

	int findLoadedObject(const int &id, const int &sceneId) const;
	bool addLoadedObject(const int &id, const int &sceneId);

	void changeScene(TiledObject *object, TiledScene *from, TiledScene *to, const QPointF &toPoint);


	virtual void loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, const QString &groupClass, Tiled::MapRenderer *renderer);
	virtual void loadGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer);
	virtual void loadImageLayer(TiledScene *scene, Tiled::ImageLayer *image, Tiled::MapRenderer *renderer);
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void joystickStateEvent(const JoystickState &newJoystickState) { Q_UNUSED(newJoystickState);}

	virtual bool transportBeforeEvent(TiledObject *object, TiledTransport *transport);
	virtual bool transportAfterEvent(TiledObject *object, TiledScene *newScene, TiledObjectBase *newObject);
	virtual bool transportMarket();
	virtual bool transportGate(TiledObject *object, TiledTransport *transport, TiledObjectBase *transportBase);
	virtual bool transportDoor(TiledObject *object, TiledTransport *transport);


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

	QVector<Scene> m_sceneList;
	QVector<TiledObjectBase::ObjectId> m_loadedObjectList;
	QVector<PlayerPosition> m_playerPositionList;
	TiledTransportList m_transportList;

	TiledScene *m_currentScene = nullptr;

	std::unique_ptr<AbstractGame::TickTimer> m_tickTimer;
	bool m_paused = false;



private:
	struct KeyboardJoystickState {
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;

		bool upLeft = false;
		bool upRight = false;
		bool downLeft = false;
		bool downRight = false;

		bool shift = false;


		void clear() {
			left = false;
			right = false;
			up = false;
			down = false;

			upLeft = false;
			upRight = false;
			downLeft = false;
			downRight = false;

			shift = false;
		}
	};

	void joystickConnect(const bool &connect = true);
	Q_INVOKABLE void updateJoystick();
	void updateKeyboardJoystick();


	KeyboardJoystickState m_keyboardJoystickState;
	QPointer<QQuickItem> m_joystick = nullptr;
	QPointer<TiledObjectBase> m_followedItem = nullptr;
	JoystickState m_joystickState;
	static std::unordered_map<QString, std::unique_ptr<QSGTexture>> m_sharedTextures;
	bool m_debugView = false;
	bool m_mouseNavigation = false;
	bool m_mouseAttack = false;
	bool m_flickableInteractive = true;
	QQuickItem *m_messageList = nullptr;
	QColor m_defaultMessageColor = Qt::white;
	qreal m_baseScale = 1.;
};

Q_DECLARE_METATYPE(TiledGame::JoystickState)

#endif // TILEDGAME_H
