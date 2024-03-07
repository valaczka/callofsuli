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

TiledTransport::TiledTransport(const QString &name,
							   TiledScene *sceneA, TiledObjectBase *objectA,
							   TiledScene *sceneB, TiledObjectBase *objectB)
	: QObject()
	, m_name(name)
	, m_sceneA(sceneA)
	, m_objectA(objectA)
	, m_sceneB(sceneB)
	, m_objectB(objectB)
{

}



/**
 * @brief TiledTransport::addObject
 * @param scene
 * @param object
 * @return
 */

bool TiledTransport::addObject(TiledScene *scene, TiledObjectBase *object)
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

TiledScene *TiledTransport::otherScene(TiledObjectBase *object) const
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

TiledObjectBase *TiledTransport::otherObject(TiledScene *scene) const
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

TiledObjectBase *TiledTransport::otherObject(TiledObjectBase *object) const
{
	if (m_objectA == object)
		return m_objectB;
	else if (m_objectB == object)
		return m_objectA;

	return nullptr;
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

TiledObjectBase *TiledTransport::objectA() const
{
	return m_objectA;
}

void TiledTransport::setObjectA(TiledObjectBase *newObjectA)
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

TiledObjectBase *TiledTransport::objectB() const
{
	return m_objectB;
}

void TiledTransport::setObjectB(TiledObjectBase *newObjectB)
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



/**
 * @brief TiledTransportList::find
 * @param name
 * @return
 */

bool TiledTransportList::add(const QString &name, TiledScene *scene, TiledObjectBase *object)
{
	if (name.isEmpty()) {
		LOG_CERROR("scene") << "Transport name missing";
		return false;
	}

	if (find(object)) {
		LOG_CERROR("scene") << "Transport object already registered";
		return false;
	}

	TiledTransport *base = this->find(name);

	if (!base) {
		std::unique_ptr<TiledTransport> base(new TiledTransport(name, scene, object));
		this->push_back(std::move(base));
		return true;
	} else {
		return base->addObject(scene, object);
	}
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

	auto it = std::find_if(this->cbegin(), this->cend(), [scene](const std::unique_ptr<TiledTransport> &t){
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

TiledTransport* TiledTransportList::find(const TiledObjectBase *object) const
{
	if (!object)
		return nullptr;

	auto it = std::find_if(this->cbegin(), this->cend(), [object](const std::unique_ptr<TiledTransport> &t){
		return t->objectA() == object || t->objectB() == object;
	});

	if (it == this->cend())
		return nullptr;
	else
		return it->get();
}
