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
#include "actiongame.h"

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


/**
 * @brief The GameObject class
 */

class GameObject : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(GameScene *scene READ scene WRITE setScene NOTIFY sceneChanged)
	Q_PROPERTY(Box2DBody *body READ body CONSTANT)
	Q_PROPERTY(ActionGame *game READ game NOTIFY gameChanged)

public:
	GameObject(QQuickItem *parent = nullptr);
	virtual ~GameObject();

	static GameObject* createFromFile(QString file, GameScene *scene);

	GameScene *scene() const;
	void setScene(GameScene *newScene);

	ActionGame *game() const;
	Box2DBody *body() const;

	Q_INVOKABLE void bodyComplete();

private slots:
	void onSceneChanged();

signals:
	void sceneChanged();
	void gameChanged();

protected:
	GameScene *m_scene = nullptr;
	Box2DBody *m_body = nullptr;
};

#endif // GAMEOBJECT_H
