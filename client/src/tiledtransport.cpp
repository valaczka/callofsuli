/*
 * ---- Call of Suli ----
 *
 * tiledtransport.cpp
 *
 * Created on: 2024. 03. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledTransport
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

#include "tiledtransport.h"

TiledTransport::TiledTransport(const TransportType &type, const QString &name,
							   TiledScene *sceneA, TiledObject *objectA,
							   TiledScene *sceneB, TiledObject *objectB)
	: QObject()
	, m_type(type)
	, m_name(name)
	, m_sceneA(sceneA)
	, m_objectA(objectA)
	, m_sceneB(sceneB)
	, m_objectB(objectB)
{

}




/**
 * @brief TiledTransport::typeFromString
 * @param str
 * @return
 */

TiledTransport::TransportType TiledTransport::typeFromString(const QString &str)
{
	static const QHash<QString, TransportType> hash = {
		{ QStringLiteral("gate"), TransportGate },
		{ QStringLiteral("door"), TransportDoor },
		{ QStringLiteral("market"), TransportMarket }
	};

	return hash.value(str, TransportInvalid);
}



/**
 * @brief TiledTransport::addObject
 * @param scene
 * @param object
 * @return
 */

bool TiledTransport::addObject(TiledScene *scene, TiledObject *object)
{
	if (!m_sceneA) {
		m_sceneA = scene;
		m_objectA = object;
		return true;
	}

	if (!m_sceneB) {
		m_sceneB = scene;
		m_objectB = object;
		return true;
	}

	return false;
}



/**
 * @brief TiledTransport::otherScene
 * @param scene
 * @return
 */

TiledScene *TiledTransport::otherScene(TiledScene *scene) const
{
	if (m_sceneA == scene)
		return m_sceneB;
	else if (m_sceneB == scene)
		return m_sceneA;

	return nullptr;
}



/**
 * @brief TiledTransport::otherScene
 * @param object
 * @return
 */

TiledScene *TiledTransport::otherScene(TiledObject *object) const
{
	if (m_objectA == object)
		return m_sceneB;
	else if (m_objectB == object)
		return m_sceneA;

	return nullptr;
}



/**
 * @brief TiledTransport::otherObject
 * @param scene
 * @return
 */

TiledObject *TiledTransport::otherObject(TiledScene *scene) const
{
	if (m_sceneA == scene)
		return m_objectB;
	else if (m_sceneB == scene)
		return m_objectA;

	return nullptr;
}



/**
 * @brief TiledTransport::otherObject
 * @param object
 * @return
 */

TiledObject *TiledTransport::otherObject(TiledObject *object) const
{
	if (m_objectA == object)
		return m_objectB;
	else if (m_objectB == object)
		return m_objectA;

	return nullptr;
}


/**
 * @brief TiledTransport::otherDirection
 * @param object
 * @return
 */

int TiledTransport::otherDirection(TiledObject *object) const
{
	if (m_objectA == object)
		return m_directionB;
	else if (m_objectB == object)
		return m_directionA;

	return -1;
}



QString TiledTransport::name() const
{
	return m_name;
}

void TiledTransport::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

TiledScene *TiledTransport::sceneA() const
{
	return m_sceneA;
}

void TiledTransport::setSceneA(TiledScene *newSceneA)
{
	if (m_sceneA == newSceneA)
		return;
	m_sceneA = newSceneA;
	emit sceneAChanged();
}

TiledObject *TiledTransport::objectA() const
{
	return m_objectA;
}

void TiledTransport::setObjectA(TiledObject *newObjectA)
{
	if (m_objectA == newObjectA)
		return;
	m_objectA = newObjectA;
	emit objectAChanged();
}

TiledScene *TiledTransport::sceneB() const
{
	return m_sceneB;
}

void TiledTransport::setSceneB(TiledScene *newSceneB)
{
	if (m_sceneB == newSceneB)
		return;
	m_sceneB = newSceneB;
	emit sceneBChanged();
}

TiledObject *TiledTransport::objectB() const
{
	return m_objectB;
}

void TiledTransport::setObjectB(TiledObject *newObjectB)
{
	if (m_objectB == newObjectB)
		return;
	m_objectB = newObjectB;
	emit objectBChanged();
}

bool TiledTransport::isOpen() const
{
	return m_isOpen;
}

void TiledTransport::setIsOpen(bool newIsOpen)
{
	if (m_isOpen == newIsOpen)
		return;
	m_isOpen = newIsOpen;
	emit isOpenChanged();
}

bool TiledTransport::isActive() const
{
	return m_isActive;
}

void TiledTransport::setIsActive(bool newIsActive)
{
	if (m_isActive == newIsActive)
		return;
	m_isActive = newIsActive;
	emit isActiveChanged();
}

TiledTransport::TransportType TiledTransport::type() const
{
	return m_type;
}

void TiledTransport::setType(const TransportType &newType)
{
	if (m_type == newType)
		return;
	m_type = newType;
	emit typeChanged();
}

QString TiledTransport::lockName() const
{
	return m_lockName;
}

void TiledTransport::setLockName(const QString &newLockName)
{
	if (m_lockName == newLockName)
		return;
	m_lockName = newLockName;
	emit lockNameChanged();
}



/**
 * @brief TiledTransport::directionA
 * @return
 */

int TiledTransport::directionA() const
{
	return m_directionA;
}

void TiledTransport::setDirectionA(int newDirectionA)
{
	if (m_directionA == newDirectionA)
		return;
	m_directionA = newDirectionA;
	emit directionAChanged();
}

int TiledTransport::directionB() const
{
	return m_directionB;
}

void TiledTransport::setDirectionB(int newDirectionB)
{
	if (m_directionB == newDirectionB)
		return;
	m_directionB = newDirectionB;
	emit directionBChanged();
}



/**
 * @brief TiledTransportList::find
 * @param name
 * @return
 */

TiledTransport *TiledTransportList::add(const TiledTransport::TransportType &type, const QString &name, const QString &lockName, const int &direction,
							 TiledScene *scene, TiledObject *object)
{
	if (name.isEmpty()) {
		LOG_CERROR("scene") << "Transport name missing";
		return nullptr;
	}

	if (find(object)) {
		LOG_CERROR("scene") << "Transport object already registered";
		return nullptr;
	}

	TiledTransport *base = this->find(name);

	if (!base) {
		TiledTransport *t = new TiledTransport(type, name, scene, object);
		t->setLockName(lockName);
		t->setIsOpen(lockName.isEmpty());
		if (direction != -1.)
			t->setDirectionA(direction);
		this->emplace_back(t);
		return t;
	} else {
		if (direction != -1.)
			base->setDirectionB(direction);
		return base->addObject(scene, object) ? base : nullptr;
	}
}



/**
 * @brief TiledTransportList::add
 * @param type
 * @param name
 * @param scene
 * @param object
 * @return
 */

TiledTransport *TiledTransportList::add(const TiledTransport::TransportType &type, const QString &name, const int &direction, TiledScene *scene, TiledObject *object)
{
	return add(type, name, QStringLiteral(""), direction, scene, object);
}




/**
 * @brief TiledTransportList::add
 * @param name
 * @param scene
 * @param object
 * @return
 */

TiledTransport *TiledTransportList::add(const QString &name, TiledScene *scene, TiledObject *object)
{
	return add (TiledTransport::TransportInvalid, name, -1, scene, object);
}



/**
 * @brief TiledTransportList::find
 * @param name
 * @return
 */

TiledTransport* TiledTransportList::find(const QString &name) const
{
	if (name.isEmpty())
		return nullptr;

	auto it = std::find_if(this->cbegin(), this->cend(), [&name](const std::unique_ptr<TiledTransport> &t){
		return t->name() == name;
	});

	if (it == this->cend())
		return nullptr;
	else
		return it->get();
}


/**
 * @brief TiledTransportList::find
 * @param scene
 * @return
 */

TiledTransport* TiledTransportList::find(const TiledScene *scene) const
{
	if (!scene)
		return nullptr;

	auto it = std::find_if(this->cbegin(), this->cend(), [&scene](const std::unique_ptr<TiledTransport> &t){
		return t->sceneA() == scene || t->sceneB() == scene;
	});

	if (it == this->cend())
		return nullptr;
	else
		return it->get();
}



/**
 * @brief TiledTransportList::find
 * @param object
 * @return
 */

TiledTransport* TiledTransportList::find(const TiledObject *object) const
{
	if (!object)
		return nullptr;

	auto it = std::find_if(this->cbegin(), this->cend(), [&object](const std::unique_ptr<TiledTransport> &t){
		return t->objectA() == object || t->objectB() == object;
	});

	if (it == this->cend())
		return nullptr;
	else
		return it->get();
}
