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

#include "libtcod/fov.hpp"
#include "qsgtexture.h"
#include "tiledscene.h"
#include "abstractgame.h"
#include <QQuickItem>
#include <QSerializer>



class TiledGamePrivate;
class TiledDebugDraw;


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

	Q_PROPERTY(QQuickItem* joystickA READ joystickA WRITE setJoystickA NOTIFY joystickAChanged FINAL)
	Q_PROPERTY(QQuickItem* joystickB READ joystickB WRITE setJoystickB NOTIFY joystickBChanged FINAL)
	Q_PROPERTY(QQuickItem* joystickC READ joystickC WRITE setJoystickC NOTIFY joystickCChanged FINAL)
	Q_PROPERTY(QQuickItem* joystickD READ joystickD WRITE setJoystickD NOTIFY joystickDChanged FINAL)

	Q_PROPERTY(TiledObject *followedItem READ followedItem WRITE setFollowedItem NOTIFY followedItemChanged FINAL)
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

	enum Joystick {
		JoystickA = 0,
		JoystickB,
		JoystickC,
		JoystickD,

		JoystickCount
	};

	Q_ENUM(Joystick)


	bool load(const TiledGameDefinition &def);

	static std::optional<QStringList> getDynamicTilesets(const TiledGameDefinition &def);

	Tiled::TileLayer *loadSceneLayer(TiledScene *scene, Tiled::Layer *layer, Tiled::MapRenderer *renderer);

	static QSGTexture *getTexture(const QString &path, QQuickWindow *window);
	static void clearSharedTextures();

	QVector<TiledScene*> sceneList() const;

	TiledScene *findScene(const quint32 &id) const;

	TiledScene *currentScene() const;
	void setCurrentScene(TiledScene *newCurrentScene);

	Q_INVOKABLE virtual void onMouseClick(const qreal &x, const qreal &y, const int &buttons, const int &modifiers);

	void playSfx(const QString &source, TiledScene *scene, const float &baseVolume = 1.) const;
	void playSfx(const QString &source, TiledScene *scene, const QPointF &position, const float &baseVolume = 1.) const;

	static std::optional<qreal> getSfxVolume(TiledScene *scene, const QPointF &position, const float &baseVolume = 1.,
											 const qreal &baseScale = 1.);


	Q_INVOKABLE void messageColor(const QString &text, const QColor &color, const bool &priority = false);
	Q_INVOKABLE void message(const QString &text, const bool &priority = false) {
		messageColor(text, m_defaultMessageColor, priority);
	}
	void setMessageEnabled(const bool &enabled = true);




	template <typename T, typename = std::enable_if<std::is_base_of<TiledObjectBody, T>::value>::type,
			  class... Args>
	T* createObject(const TiledObjectBody::ObjectId &id, TiledScene *scene, Args&&... args) {
		Q_ASSERT(scene);
		std::unique_ptr<T> dptr(new T(std::forward<Args>(args)...));
		initSpace(dptr.get(), scene);
		std::unique_ptr<TiledObjectBody> b(std::move(dptr));
		T* obj = dynamic_cast<T*>(addObject(b, id));
		obj->initialize();
		return obj;
	}

	bool removeObject(TiledObjectBody *body);

	void onShapeAboutToDelete(cpShape *shape);


	virtual TiledObjectBody *loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);
	bool loadDynamicZ(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer);


	// Sprite texture helper

	struct TextureSpriteMapper
	{
		QString name;
		TiledObject::Direction direction = TiledObject::Invalid;
		int width = 0;
		int height = 0;
		int duration = 0;
		int loops = 0;
		bool baked = false;
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


	struct TcodMapData {
		std::unique_ptr<TCODMap> map;
		QRectF viewport;
		qreal chunkSize = 0.;
		qreal chunkWidth = 0.;
		qreal chunkHeight = 0.;

		QPoint getChunk(const cpVect &pos) const;
		QPoint getChunk(const qreal &x, const qreal &y) const;

		QPointF chunkMiddle(const int &x, const int &y) const;

		std::optional<QPolygonF> findShortestPath(const cpVect &from, const cpVect &to) const;
		std::optional<QPolygonF> findShortestPath(const qreal &x1, const qreal &y1, const qreal &x2, const qreal &y2) const;
	};


	TcodMapData* reloadTcodMap(cpSpace *space, const cpBitmask &categories, const qreal chunkSize = 0.f);
	TcodMapData* reloadTcodMap(cpSpace *space, const qreal chunkSize = 0.f) {
		return reloadTcodMap(space, m_groundCategory, chunkSize);
	}
	TcodMapData* reloadTcodMap(TiledScene *scene, const cpBitmask &categories, const qreal chunkSize) {
		if (scene && scene->m_space)
			return reloadTcodMap(scene->m_space, categories, chunkSize);
		else
			return nullptr;
	}
	TcodMapData* reloadTcodMap(TiledScene *scene, const qreal chunkSize = 0.f) {
		return reloadTcodMap(scene, m_groundCategory, chunkSize);
	}


	void iterateOverBodies(const std::function<void(TiledObjectBody*)> &func);

	std::optional<QPolygonF> findShortestPath(TiledObjectBody *body, const cpVect &to) const;
	std::optional<QPolygonF> findShortestPath(TiledObjectBody *body, const qreal &x2, const qreal &y2) const;

	AbstractGame::TickTimer *tickTimer() const { return m_tickTimer.get(); }
	void setTickTimer(std::unique_ptr<AbstractGame::TickTimer> &timer) { m_tickTimer = std::move(timer); }

	bool debugView() const;
	void setDebugView(bool newDebugView);

	TiledObject *followedItem() const;
	void setFollowedItem(TiledObject *newFollowedItem);

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

	const cpBitmask &groundCategory() const;
	void setGroundCategory(cpBitmask newGroundCategory);

	QQuickItem *joystickA() const { return joystickGet(JoystickA); }
	void setJoystickA(QQuickItem *item) { if (joystickSet(JoystickA, item)) emit joystickAChanged(); }

	QQuickItem *joystickB() const { return joystickGet(JoystickB); }
	void setJoystickB(QQuickItem *item) { if (joystickSet(JoystickB, item)) emit joystickAChanged(); }

	QQuickItem *joystickC() const { return joystickGet(JoystickC); }
	void setJoystickC(QQuickItem *item) { if (joystickSet(JoystickC, item)) emit joystickAChanged(); }

	QQuickItem *joystickD() const { return joystickGet(JoystickD); }
	void setJoystickD(QQuickItem *item) { if (joystickSet(JoystickD, item)) emit joystickAChanged(); }


	JoystickState joystickState(const Joystick &joystick) const;
	void setJoystickState(const Joystick &joystick, const JoystickState &newJoystickState);

	Q_INVOKABLE bool joystickInteractive(const Joystick &joystick) const {
		return m_joystick[joystick].joystickState.hasKeyboard ||
				m_joystick[joystick].joystickState.hasTouch;
	}


signals:
	void gameLoaded();
	void gameLoadFailed(const QString &errorString);
	void gameSynchronized();
	void currentSceneChanged();
	void followedItemChanged();
	void debugViewChanged();
	void gameModeChanged();
	void messageListChanged();
	void defaultMessageColorChanged();
	void baseScaleChanged();
	void mouseNavigationChanged();
	void mouseAttackChanged();
	void flickableInteractiveChanged();
	void pausedChanged();

	void joystickAChanged();
	void joystickBChanged();
	void joystickCChanged();
	void joystickDChanged();
	void joystickStateChanged(TiledGame::Joystick joystick);

protected:
	TiledObjectBody *addObject(std::unique_ptr<TiledObjectBody> &body, const TiledObjectBody::ObjectId &id);
	bool initSpace(TiledObjectBody *body, TiledScene *scene);
	virtual void onShapeAboutToDeletePrivate(cpShape *shape) { Q_UNUSED(shape); }

	bool loadScene(const TiledSceneDefinition &def, const QString &basePath);
	virtual bool loadObjectLayer(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer);
	virtual bool loadLights(TiledScene *scene, const QList<Tiled::MapObject *> &objects, Tiled::MapRenderer *renderer);
	void synchronize();

	const qint64 &currentFrame() const;
	void overrideCurrentFrame(const qint64 &frame);


	virtual void loadTileLayer(TiledScene *scene, Tiled::TileLayer *layer, Tiled::MapRenderer *renderer);
	virtual void loadObjectLayer(TiledScene *scene, Tiled::MapObject *object, const QString &groupClass, Tiled::MapRenderer *renderer);
	virtual void loadGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer);
	virtual void loadImageLayer(TiledScene *scene, Tiled::ImageLayer *image, Tiled::MapRenderer *renderer);

	virtual void timerEvent(QTimerEvent *event) override final;
	virtual void prepareEvent() {  }
	virtual void timeStepPrepareEvent() {  }
	virtual void timeBeforeWorldStepEvent(const qint64 &tick);
	virtual void worldStep(TiledObjectBody *body);
	virtual void worldStep() { }
	virtual void timeAfterWorldStepEvent(const qint64 &tick) { Q_UNUSED(tick); }
	virtual void timeSteppedEvent(const std::vector<TiledObjectBody *> &aboutDestruction) { Q_UNUSED(aboutDestruction); }

	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void joystickStateEvent(const Joystick &joystick, const JoystickState &state) { Q_UNUSED(joystick); Q_UNUSED(state);}

	virtual void sceneDebugDrawEvent(TiledDebugDraw *debugDraw, TiledScene *scene);

protected:
	cpBitmask m_groundCategory = 0;
	TiledScene *m_currentScene = nullptr;

	std::unique_ptr<AbstractGame::TickTimer> m_tickTimer;
	bool m_paused = false;


private:
	QQuickItem* joystickGet(const Joystick &joystick) const { return m_joystick[joystick].joystick; }
	bool joystickSet(const Joystick &joystick, QQuickItem *item);

	void joystickConnect(const Joystick &joystick, const bool &connect = true);
	Q_INVOKABLE void updateJoystick();
	void updateKeyboardJoystick();
	void updateStepTimer();

	struct JoystickData {
		QPointer<QQuickItem> joystick = nullptr;
		JoystickState joystickState;
	};

	std::array<JoystickData, JoystickCount> m_joystick;




	QPointer<TiledObject> m_followedItem = nullptr;
	bool m_debugView = false;
	bool m_mouseNavigation = false;
	bool m_mouseAttack = false;
	bool m_flickableInteractive = true;
	QQuickItem *m_messageList = nullptr;
	QColor m_defaultMessageColor = Qt::white;
	qreal m_baseScale = 1.;

	TiledGamePrivate *d = nullptr;

	friend class TiledGamePrivate;
	friend class TiledScene;
};







Q_DECLARE_METATYPE(TiledGame::JoystickState)

#endif // TILEDGAME_H
