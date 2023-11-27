/*
 * ---- Call of Suli ----
 *
 * gameobject.h
 *
 * Created on: 2020. 10. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameObject
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QObject>
#include <QQuickItem>
#include "box2dbody.h"
#include "objectstate.h"

/**
 * COLLISION CATEGORIES
 *
 * Category1: ground
 * Category2: player bound
 * Category3: player body (sensor)
 * Category4: item bound (sensor)
 * Category5: enemy bound
 * Category6: other sensor
 */


#define CATEGORY_GROUND			Box2DFixture::Category1
#define CATEGORY_PLAYER			Box2DFixture::Category2
#define CATEGORY_ITEM			Box2DFixture::Category4
#define CATEGORY_ENEMY			Box2DFixture::Category5
#define CATEGORY_OTHER			Box2DFixture::Category6


class GameScene;
class ActionGame;
class GameObject;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_GameScene
#define OPAQUE_PTR_GameScene
  Q_DECLARE_OPAQUE_POINTER(GameScene*)
#endif

#ifndef OPAQUE_PTR_ActionGame
#define OPAQUE_PTR_ActionGame
  Q_DECLARE_OPAQUE_POINTER(ActionGame*)
#endif

#endif





/**
 * @brief The GameObject class
 */

class GameObject : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(GameScene *scene READ scene WRITE setScene NOTIFY sceneChanged)
	Q_PROPERTY(Box2DBody *body READ body CONSTANT)
	Q_PROPERTY(ActionGame *game READ game NOTIFY gameChanged)
	Q_PROPERTY(QString objectType READ objectType WRITE setObjectType NOTIFY objectTypeChanged)

public:
	GameObject(QQuickItem *parent = nullptr);
	virtual ~GameObject();

	static GameObject* createFromFile(QString file, GameScene *scene, const bool &synchronous = true);

	GameScene *scene() const;
	void setScene(GameScene *newScene);

	ActionGame *game() const;
	Box2DBody *body() const;

	Q_INVOKABLE void bodyComplete();
	Q_INVOKABLE void deleteSelf();

	const QString &objectType() const;
	void setObjectType(const QString &newObjectType);

	virtual void onTimingTimerTimeout(const int &msec, const qreal &delayFactor);
	virtual void cacheCurrentState() {}
	virtual bool getStateSnapshot(ObjectStateSnapshot *snapshot, const qint64 &objectId = 1) {
		Q_UNUSED(snapshot); Q_UNUSED(objectId);
		return false;
	}

private slots:
	void onSceneChanged();

signals:
	void sceneChanged();
	void gameChanged();
	void sceneConnected();
	void objectTypeChanged();

protected:
	QPointer<GameScene> m_scene;
	std::unique_ptr<Box2DBody> m_body;
	QList<QPointer<QQuickItem>> m_childItems;

	bool getCurrentState(ObjectStateBase *ptr) const;
	void setCurrentState(const ObjectStateBase &state);

private:
	bool m_sceneConnected = false;
	QString m_objectType;
};

#endif // GAMEOBJECT_H
