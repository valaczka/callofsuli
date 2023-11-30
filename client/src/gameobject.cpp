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
	LOG_CDEBUG("scene") << "Destroy GameObject" << this;
}


/**
 * @brief GameObject::createFromFile
 * @param file
 * @return
 */

GameObject *GameObject::createFromFile(QString file, GameScene *scene, const bool &synchronous)
{
	if (file.startsWith(QStringLiteral(":")))
		file.replace(QStringLiteral(":"), QStringLiteral("qrc:"));
	else if (file.startsWith(QStringLiteral("/")))
		file.replace(QStringLiteral("/"), QStringLiteral("qrc:/"));
	else if (!file.startsWith(QStringLiteral("qrc:/")))
		file.prepend(QStringLiteral("qrc:/"));


	QQmlComponent component(Application::instance()->engine(), file, scene);

	LOG_CDEBUG("scene") << "Create object from file:" << file << component.isReady();

	// Synchronous

	if (synchronous)
		return qobject_cast<GameObject*>(component.create());


	// Asynchronous

	QQmlIncubator incubator;
	component.create(incubator, Application::instance()->engine()->contextForObject(scene));

	while (!incubator.isReady()) {
		QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
	}

	return qobject_cast<GameObject*>(incubator.object());
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
	return m_body.get();
}


/**
 * @brief GameObject::bodyComplete
 */

void GameObject::bodyComplete()
{
	m_body->componentComplete();
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
			m_scene->gameObjectAdd(this);
			emit sceneConnected();
			m_sceneConnected = true;
		}
	}
}


/**
 * @brief GameObject::getCurrentState
 * @param ptr
 * @return
 */

bool GameObject::getCurrentState(ObjectStateBase *ptr) const
{
	if (!ptr)
		return false;

	ptr->position = QPointF(x(), y());
	ptr->size = QSizeF(width(), height());

	return true;
}




/**
 * @brief GameObject::setCurrentState
 * @param state
 * @return
 */

void GameObject::setCurrentState(const ObjectStateBase &state)
{
	setHeight(state.size.height());
	setWidth(state.size.width());
	setX(state.position.x());
	setY(state.position.y());
}


/**
 * @brief GameObject::updateStateQuickItem
 * @param state
 * @param item
 */

void GameObject::updateStateQuickItem(ObjectStateEntity *state, QQuickItem *item)
{
	if (!state || !item)
		return;

	state->position.setY(state->position.y()-item->height());
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



/**
 * @brief GameObject::onTimingTimerTimeout
 * @param msec
 * @param delayFactor
 */

void GameObject::onTimingTimerTimeout(const int &msec, const qreal &delayFactor)
{
	Q_UNUSED(msec)
	Q_UNUSED(delayFactor)
}


/**
 * @brief GameObject::setStateFromSnapshot
 * @param ptr
 */

void GameObject::setStateFromSnapshot(ObjectStateBase *ptr)
{
	if (!ptr)
		return;

	LOG_CTRACE("scene") << "Load state from snapshot" << ptr;

	setCurrentState(*ptr);
}

