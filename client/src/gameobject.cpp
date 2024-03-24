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

	//connect(this, &GameObject::sceneChanged, this, &GameObject::onSceneChanged);
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

	onSceneChanged();
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
			onSceneConnected();
			m_sceneConnected = true;
		}
	}
}


/**
 * @brief GameObject::interpolate
 * @param t
 * @param toState
 */

ObjectStateBase GameObject::interpolate(const qreal &t, const ObjectStateBase &from, const ObjectStateBase &to)
{
	ObjectStateBase b = from;

	if (from.fields.testFlag(ObjectStateBase::FieldSize)) {
		b.size.setWidth(std::lerp(from.size.width(), to.size.width(), t));
		b.size.setHeight(std::lerp(from.size.height(), to.size.height(), t));
	}

	if (from.fields.testFlag(ObjectStateBase::FieldPosition)) {
		b.position.setX(std::lerp(from.position.x(), to.position.x(), t));
		b.position.setY(std::lerp(from.position.y(), to.position.y(), t));
	}

	return b;
}



/**
 * @brief GameObject::interpolateStates
 * @param currentTick
 * @param defaultState
 * @return
 */

ObjectStateBase GameObject::interpolateStates(const qint64 &currentTick, const ObjectStateBase *defaultState)
{
	if (m_authoritativeStates.size() < 2) {
		if (defaultState)
			return *defaultState;
		else
			return ObjectStateBase();
	}


	const qint64 virtualTick = currentTick-m_authoritativeStateInterval;

	qint64 fromTick=-1, toTick=-1;
	ObjectStateBase fromState, toState;

	for (auto it = m_authoritativeStates.cbegin(); it != m_authoritativeStates.cend(); ++it) {
		auto it2 = it;
		++it2;

		if (it2 == m_authoritativeStates.cend())
			break;

		if (fromTick != -1 && toTick > virtualTick)
			break;

		fromTick = it->first;
		fromState = it->second;
		toTick = it2->first;
		toState = it2->second;
	}

	if (fromTick > virtualTick || toTick < (currentTick-2*m_authoritativeStateInterval)) {
		if (defaultState)
			return *defaultState;
		else
			return ObjectStateBase();
	}

	return interpolate((qreal)(virtualTick-fromTick)/(qreal)(toTick-fromTick), fromState, toState);;
}




/**
 * @brief GameObject::stateReconciliation
 * @param from
 * @param to
 * @return
 */

bool GameObject::stateReconciliation(const ObjectStateBase &from, const ObjectStateBase &to)
{
	Q_UNUSED(to);

	if (from.state == ObjectStateBase::StateInactive)
		return false;

	return true;
}


/**
 * @brief GameObject::removeOldAuthoritativeStates
 * @param currentTick
 */

void GameObject::removeOldAuthoritativeStates(const qint64 &currentTick)
{
	qint64 diff = m_authoritativeStateInterval * AUTHORITATIVE_STATE_CACHE_FACTOR;

	for (auto it = m_authoritativeStates.cbegin(); it != m_authoritativeStates.cend(); ) {
		if (it->first < currentTick-diff)
			it = m_authoritativeStates.erase(it);
		else
			++it;
	}
}


/**
 * @brief GameObject::authoritativeStateInterval
 * @return
 */

qint64 GameObject::authoritativeStateInterval() const
{
	return m_authoritativeStateInterval;
}


/**
 * @brief GameObject::setAuthoritativeStateInterval
 * @param newAuthoritativeStateInterval
 */

void GameObject::setAuthoritativeStateInterval(qint64 newAuthoritativeStateInterval)
{
	if (newAuthoritativeStateInterval > 0)
		m_authoritativeStateInterval = newAuthoritativeStateInterval;
}





/**
 * @brief GameObject::stateReconciliation
 * @param state
 * @return
 */

std::optional<ObjectStateBase> GameObject::stateReconciliation(const ObjectStateBase &state)
{
	for (auto it = m_cachedStates.rbegin(); it != m_cachedStates.rend(); ++it) {
		if (it->tick < state.tick)
			m_cachedStates.erase(it.base()-1);
	}

	ObjectStateBase r = state;

	for (auto it = m_cachedStates.begin(); it != m_cachedStates.end(); ++it) {
		if (stateReconciliation(state, *it)) {
			r = *it;
		} else {
			return std::nullopt;
		}
	}

	return r;
}


/**
 * @brief GameObject::getCurrentState
 * @param ptr
 * @return
 */

ObjectStateBase GameObject::getCurrentState() const
{
	ObjectStateBase b;
	b.type = ObjectStateBase::TypeBase;

	b.fields.setFlag(ObjectStateBase::FieldPosition);
	b.fields.setFlag(ObjectStateBase::FieldSize);

	b.position = QPointF(x(), y()+height());
	b.size = QSizeF(width(), height());

	return b;
}




/**
 * @brief GameObject::setCurrentState
 * @param state
 * @return
 */

void GameObject::setCurrentState(const ObjectStateBase &state, const bool &force)
{
	Q_UNUSED(force);

	if (state.fields.testFlag(ObjectStateBase::FieldSize)) {
		setHeight(state.size.height());
		setWidth(state.size.width());
	}

	if (state.fields.testFlag(ObjectStateBase::FieldPosition)) {
		const qreal _x = state.position.x();
		const qreal _y = state.position.y()-height();

		if (_x != x() || _y != y()) {
			setX(state.position.x());
			setY(_y);
			m_body->setAwake(true);
		}
	}
}


/**
 * @brief GameObject::interpolateState
 * @param currentTick
 * @param defaultState
 */

void GameObject::interpolateState(const qint64 &currentTick, const ObjectStateBase *defaultState)
{
	const ObjectStateBase &b = interpolateStates(currentTick, defaultState);

	if (b.type != ObjectStateBase::TypeInvalid) {
		setCurrentState(b, true);
	} else if (defaultState) {
		setCurrentState(*defaultState, true);
	} else {
		LOG_CTRACE("game") << "SKIP..." << currentTick << this;
	}
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
 * @brief GameObject::onTimingTimerTimeoutMulti
 * @param hosted
 * @param msec
 * @param delayFactor
 */

void GameObject::onTimingTimerTimeoutMulti(const bool &hosted, const int &msec, const qreal &delayFactor)
{
	Q_UNUSED(msec)
	Q_UNUSED(delayFactor)
	Q_UNUSED(hosted)
}



/**
 * @brief GameObject::getStateSnapshot
 * @param snapshot
 * @param entityId
 * @return
 */

int GameObject::getStateSnapshot(ObjectStateSnapshot *snapshot, const qint64 &entityId)
{
	ActionGame *_game = game();

	if (!_game)
		return -1;

	if (snapshot) {
		ObjectStateBase prev;
		int num = 0;

		for (ObjectStateBase s : std::as_const(m_cachedStates)) {
			s.id = entityId;

			if (prev.type != ObjectStateBase::TypeInvalid) {
				const auto &d = prev.diff(s);

				if (!d) {
					LOG_CERROR("game") << "State diff error" << entityId << s.type << s.tick;
				} else {
					s = d.value();
				}
			}

			prev = s;
			snapshot->append(s);
			++num;
		}

		return num;
	}


	return -1;
}


/**
 * @brief GameObject::setStateFromSnapshot
 * @param ptr
 */

void GameObject::setStateFromSnapshot(const ObjectStateBase &ptr, const qint64 &currentTick, const bool &force)
{
	setCurrentState(ptr, force);
}

