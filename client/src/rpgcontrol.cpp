/*
 * ---- Call of Suli ----
 *
 * rpgcontrol.cpp
 *
 * Created on: 2025. 05. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#include "rpgcontrol.h"
#include "rpgplayer.h"
#include "tiledscene.h"
#include <libtiled/grouplayer.h>

RpgControlBase::RpgControlBase(const RpgConfig::ControlType &type)
	: m_type(type)
{

}



/**
 * @brief RpgActiveControlObject::RpgActiveControlObject
 * @param iface
 * @param polygon
 * @param game
 * @param renderer
 * @param type
 */

RpgActiveControlObject::RpgActiveControlObject(RpgActiveIface *iface, const QPolygonF &polygon,
											   TiledGame *game, Tiled::MapRenderer *renderer, const cpBodyType &type)
	: QObject(nullptr)
	, TiledObjectBody(polygon, game, renderer, type)
	, m_iface(iface)
{
	Q_ASSERT(m_iface);
}


/**
 * @brief RpgActiveControlObject::RpgActiveControlObject
 * @param iface
 * @param center
 * @param radius
 * @param game
 * @param renderer
 * @param type
 */

RpgActiveControlObject::RpgActiveControlObject(RpgActiveIface *iface, const QPointF &center, const qreal &radius,
											   TiledGame *game, Tiled::MapRenderer *renderer, const cpBodyType &type)
	: QObject(nullptr)
	, TiledObjectBody(center, radius, game, renderer, type)
	, m_iface(iface)
{
	Q_ASSERT(m_iface);
}



/**
 * @brief RpgActiveControlObject::RpgActiveControlObject
 * @param iface
 * @param object
 * @param game
 * @param renderer
 * @param type
 */

RpgActiveControlObject::RpgActiveControlObject(RpgActiveIface *iface, const Tiled::MapObject *object,
											   TiledGame *game, Tiled::MapRenderer *renderer, const cpBodyType &type)
	: QObject(nullptr)
	, TiledObjectBody(object, game, renderer, type)
	, m_iface(iface)
{
	Q_ASSERT(m_iface);
}


/**
 * @brief RpgActiveControlObject::type
 * @return
 */

const RpgConfig::ControlType &RpgActiveControlObject::type() const
{
	return m_iface->activeType();
}



/**
 * @brief RpgActiveControlObject::useControl
 * @param player
 */

void RpgActiveControlObject::useControl(RpgPlayer *player)
{
	Q_ASSERT(m_iface);
	m_iface->use(player);
}




/**
 * @brief RpgActiveControlObject::keyLock
 * @return
 */

QString RpgActiveControlObject::keyLock() const
{
	Q_ASSERT(m_iface);
	return m_iface->m_keyLock;
}




/**
 * @brief RpgActiveControlObject::questionLock
 * @return
 */


bool RpgActiveControlObject::questionLock() const
{
	Q_ASSERT(m_iface);
	return m_iface->m_questionLock;
}



/**
 * @brief RpgActiveControlObject::isLocked
 * @return
 */

bool RpgActiveControlObject::isLocked() const
{
	Q_ASSERT(m_iface);
	return m_iface->m_isLocked;
}


/**
 * @brief RpgActiveControlObject::isActive
 * @return
 */

bool RpgActiveControlObject::isActive() const
{
	Q_ASSERT(m_iface);
	return m_iface->m_isActive;
}




/**
 * @brief RpgActiveControlObject::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgActiveControlObject::onShapeContactBegin(cpShape *self, cpShape *other)
{
	if (!m_iface)
		return;

	m_iface->onShapeContactBegin(self, other);
}



/**
 * @brief RpgActiveControlObject::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgActiveControlObject::onShapeContactEnd(cpShape *self, cpShape *other)
{
	if (!m_iface)
		return;

	m_iface->onShapeContactEnd(self, other);
}



/**
 * @brief RpgActiveIface::createVisualItem
 * @param layer
 * @return
 */

RpgActiveIface::~RpgActiveIface()
{
	for (RpgActiveControlObject *o : m_controlObjectList) {
		if (o)
			o->m_iface = nullptr;
	}

}



/**
 * @brief RpgActiveIface::createVisualItem
 * @param scene
 * @param layer
 * @return
 */

TiledVisualItem *RpgActiveIface::createVisualItem(TiledScene *scene, Tiled::GroupLayer *layer)
{
	if (!scene)
		return nullptr;

	TiledVisualItem *item = scene->addVisualItem();

	Q_ASSERT(item);

	item->setName(layer->name());
	item->setGlowColor(QStringLiteral("#FFF59D"));

	setVisualItem(item);

	return item;
}



/**
 * @brief RpgActiveIface::visualItem
 * @return
 */

TiledVisualItem *RpgActiveIface::visualItem() const
{
	return m_visualItem;
}


/**
 * @brief RpgActiveIface::setVisualItem
 * @param item
 */

void RpgActiveIface::setVisualItem(TiledVisualItem *item)
{
	if (m_visualItem == item)
		return;

	m_visualItem = item;

	refreshVisualItem();
}





/**
 * @brief RpgActiveIface::controlObjectAdd
 * @param object
 * @return
 */

RpgActiveControlObject *RpgActiveIface::controlObjectAdd(RpgActiveControlObject *object)
{
	if (!object)
		return nullptr;

	if (m_controlObjectList.contains(object))
		return nullptr;


	m_controlObjectList.append(object);

	return object;
}



/**
 * @brief RpgActiveIface::setKeyLock
 * @param newKey
 */

void RpgActiveIface::setKeyLock(const QString &newKey)
{
	m_keyLock = newKey;

	for (RpgActiveControlObject *o : m_controlObjectList) {
		if (o)
			emit o->keyLockChanged();
	}
}



/**
 * @brief RpgActiveIface::setQuestionLock
 * @param newLock
 */

void RpgActiveIface::setQuestionLock(const bool &newLock)
{
	m_questionLock = newLock;

	for (RpgActiveControlObject *o : m_controlObjectList) {
		if (o)
			emit o->questionLockChanged();
	}
}



/**
 * @brief RpgActiveIface::setIsLocked
 * @param newLocked
 */

void RpgActiveIface::setIsLocked(const bool &newLocked)
{
	m_isLocked = newLocked;

	for (RpgActiveControlObject *o : m_controlObjectList) {
		if (o)
			emit o->isLockedChanged();
	}
}



/**
 * @brief RpgActiveIface::setIsActive
 * @param newActive
 */

void RpgActiveIface::setIsActive(const bool &newActive)
{
	m_isActive = newActive;

	for (RpgActiveControlObject *o : m_controlObjectList) {
		if (o)
			emit o->isActiveChanged();
	}
}


/**
 * @brief RpgActiveIface::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgActiveIface::onShapeContactBegin(cpShape *self, cpShape *other)
{
	Q_UNUSED(self);

	if (!m_contactedFixtures.contains(other)) {
		m_contactedFixtures.append(other);
	}
}



/**
 * @brief RpgActiveIface::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgActiveIface::onShapeContactEnd(cpShape *self, cpShape *other)
{
	Q_UNUSED(self);

	m_contactedFixtures.removeAll(other);
}




/**
 * @brief RpgActiveIface::updateGlow
 */

void RpgActiveIface::updateGlow(const bool &glow)
{
	if (m_visualItem)
		m_visualItem->setGlowEnabled(glow);
}



