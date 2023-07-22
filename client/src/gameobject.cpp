/*
 * ---- Call of Suli ----
 *
 * gameobject.cpp
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

#include "application.h"
#include "actiongame.h"
#include "gameobject.h"
#include "gamescene.h"



GameObject::GameObject(QQuickItem *parent)
	: QQuickItem(parent)
	, m_body(new Box2DBody(this))
{
	LOG_CDEBUG("scene") << "Create GameObject" << this;
	setScene(static_cast<GameScene*>(parent));

	m_body->setBodyType(Box2DBody::Static);
	m_body->setTarget(this);
	m_body->setActive(true);
	m_body->setSleepingAllowed(true);

	if (m_scene) {
		onSceneChanged();
	}

	connect(this, &GameObject::sceneChanged, this, &GameObject::onSceneChanged);
}




/**
 * @brief GameObject::~GameObject
 */

GameObject::~GameObject()
{
	delete m_body;

	qDeleteAll(m_childItems);

	LOG_CDEBUG("scene") << "Destroy GameObject" << this;
}


/**
 * @brief GameObject::createFromFile
 * @param file
 * @return
 */

GameObject *GameObject::createFromFile(QString file, GameScene *scene)
{
	if (file.startsWith(QStringLiteral(":")))
		file.replace(QStringLiteral(":"), QStringLiteral("qrc:"));
	else if (file.startsWith(QStringLiteral("/")))
		file.replace(QStringLiteral("/"), QStringLiteral("qrc:/"));
	else if (!file.startsWith(QStringLiteral("qrc:/")))
		file.prepend(QStringLiteral("qrc:/"));

	QQmlComponent component(Application::instance()->engine(), file, scene);

	LOG_CDEBUG("scene") << "Create object from file:" << file << component.isReady();

	return qobject_cast<GameObject*>(component.create());
}




GameScene *GameObject::scene() const
{
	return m_scene;
}

void GameObject::setScene(GameScene *newScene)
{
	if (m_scene == newScene)
		return;
	m_scene = newScene;
	emit sceneChanged();
	emit gameChanged();
}



Box2DBody *GameObject::body() const
{
	return m_body;
}


/**
 * @brief GameObject::bodyComplete
 */

void GameObject::bodyComplete()
{
	m_body->componentComplete();
}


/**
 * @brief GameObject::addChildItem
 * @param item
 */

void GameObject::addChildItem(QQuickItem *item)
{
	m_childItems.append(item);
}


/**
 * @brief GameObject::deleteSelf
 */

void GameObject::deleteSelf()
{
	this->deleteLater();
}


/**
 * @brief GameObject::onSceneChanged
 */

void GameObject::onSceneChanged()
{
	if (m_sceneConnected)
		return;

	if (m_scene) {
		if (m_scene->world()) {
			m_body->setWorld(m_scene->world());
		}

		if (m_scene) {
			connect(m_scene->timingTimer(), &QTimer::timeout, this, &GameObject::calculateTimeout);
			emit sceneConnected();
			m_sceneConnected = true;
		}
	}
}




/**
 * @brief GameObject::calculateTimeout
 */

void GameObject::calculateTimeout()
{
	if (!m_scene) {
		m_elapsedTimer.invalidate();
		return;
	}

	qreal factor = 1.0;

	if (!m_elapsedTimer.isValid())
		m_elapsedTimer.start();
	else {
		const qreal &msec = m_elapsedTimer.restart();
		const qreal &interval = m_scene->timingTimerTimeoutMsec();

		factor = qMax(1.0, msec/interval);
	}

	emit timingTimerTimeout(factor);
}


/**
 * @brief GameObject::game
 * @return
 */

ActionGame *GameObject::game() const
{
	return m_scene ? m_scene->game() : nullptr;
}


/**
 * @brief GameObject::objectType
 * @return
 */


const QString &GameObject::objectType() const
{
	return m_objectType;
}

void GameObject::setObjectType(const QString &newObjectType)
{
	if (m_objectType == newObjectType)
		return;
	m_objectType = newObjectType;
	emit objectTypeChanged();
}
