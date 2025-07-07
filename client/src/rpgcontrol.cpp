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
#include "rpggame.h"
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

	checkVisibleForContactedPlayer();
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

	checkVisibleForContactedPlayer();
}


/**
 * @brief RpgActiveControlObject::connectScene
 */

void RpgActiveControlObject::connectScene()
{

	TiledScene *sc = scene();

	if (!sc)
		return;

	m_visibleAreaMargins.setTop(50);
	m_visibleAreaMargins.setBottom(50);
	m_visibleAreaMargins.setLeft(50);
	m_visibleAreaMargins.setRight(50);

	connect(sc, &TiledScene::onScreenAreaChanged, this, &RpgActiveControlObject::updateInVisibleArea);

	LOG_CINFO("game") << "______________ CONNECT SCENE" << this << sc;
}



/**
 * @brief RpgActiveControlObject::updateInVisibleArea
 */

void RpgActiveControlObject::updateInVisibleArea()
{
	TiledScene *sc = scene();

	if (!sc)
		return;

	const bool inArea = sc->onScreenArea().marginsRemoved(m_visibleAreaMargins).contains(bodyAABB());

	setInVisibleArea(inArea);
}



/**
 * @brief RpgActiveControlObject::checkVisibleForContactedPlayer
 */

void RpgActiveControlObject::checkVisibleForContactedPlayer() const
{
	bool ch = m_inVisibleArea && hasContactedPlayer();

	if (RpgGame *g = qobject_cast<RpgGame*>(m_game); g && m_iface && ch)
		g->controlAppeared(m_iface);
}


/**
 * @brief RpgActiveControlObject::hasContactedPlayer
 * @return
 */

bool RpgActiveControlObject::hasContactedPlayer() const
{
	if (!m_iface)
		return false;

	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!g)
		return false;

	for (cpShape *sh : m_iface->m_contactedFixtures) {
		RpgPlayer *pl = dynamic_cast<RpgPlayer*>(TiledObjectBody::fromShapeRef(sh));

		if (pl && pl == g->controlledPlayer())
			return true;
	}

	return false;
}



/**
 * @brief RpgActiveControlObject::inVisibleArea
 * @return
 */

bool RpgActiveControlObject::inVisibleArea() const
{
	return m_inVisibleArea;
}

void RpgActiveControlObject::setInVisibleArea(bool newInVisibleArea)
{
	if (m_inVisibleArea == newInVisibleArea)
		return;
	m_inVisibleArea = newInVisibleArea;
	emit inVisibleAreaChanged();

	checkVisibleForContactedPlayer();
}



/**
 * @brief RpgActiveIface::createVisualItem
 * @param layer
 * @return
 */

RpgActiveIface::~RpgActiveIface()
{
	for (RpgActiveControlObject *o : m_controlObjectList) {
		if (!o)
			continue;

		o->m_iface = nullptr;

		if (o->m_game)
			o->m_game->removeObject(o);
	}

	m_controlObjectList.clear();

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

	if (layer)
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

	object->connectScene();

	return object;
}





/**
 * @brief RpgActiveIface::overlayAdd
 * @param item
 */

void RpgActiveIface::overlayAdd(QQuickItem *item)
{
	if (!item)
		return;

	if (!m_overlays.contains(item)) {
		m_overlays.append(item);
		updateOverlays();
	}
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
	if (m_isActive == newActive)
		return;

	m_isActive = newActive;

	for (RpgActiveControlObject *o : m_controlObjectList) {
		if (o)
			emit o->isActiveChanged();
	}

	if (m_isActive)
		onActivated();
	else
		onDeactivated();

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
		updateOverlays();
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

	updateOverlays();
}



/**
 * @brief RpgActiveIface::onActivated
 */


void RpgActiveIface::onActivated()
{
	for (RpgActiveControlObject *o : m_controlObjectList)
		o->filterSet(TiledObjectBody::FixtureControl,

					 TiledObjectBody::FixturePlayerBody |
					 TiledObjectBody::FixtureSensor |
					 TiledObjectBody::FixtureVirtualCircle);

	updateOverlays();
}



/**
 * @brief RpgActiveIface::onDeactivated
 */

void RpgActiveIface::onDeactivated()
{
	for (RpgActiveControlObject *o : m_controlObjectList)
		o->filterSet(TiledObjectBody::FixtureInvalid, TiledObjectBody::FixtureInvalid);

	updateOverlays();
}




/**
 * @brief RpgActiveIface::updateGlow
 */

void RpgActiveIface::updateGlow(const bool &glow)
{
	if (m_visualItem)
		m_visualItem->setGlowEnabled(glow);
}





/**
 * @brief RpgActiveIface::updateOverlays
 */

void RpgActiveIface::updateOverlays()
{
	if (m_overlays.isEmpty())
		return;

	bool hasPlayer = false;

	for (cpShape *sh : m_contactedFixtures) {
		if (cpShapeGetFilter(sh).categories & TiledObjectBody::FixtureVirtualCircle) {
			hasPlayer = true;
			break;
		}
	}

	for (QQuickItem *it : m_overlays) {
		if (it)
			it->setVisible(hasPlayer);
	}
}



