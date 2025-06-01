/*
 * ---- Call of Suli ----
 *
 * rpgpickable.cpp
 *
 * Created on: 2025. 06. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgPickable
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

#include "rpgpickable.h"
#include "rpggame.h"




/**
 * @brief RpgPickable::RpgPickable
 * @param game
 * @param scene
 * @param base
 * @param pos
 */

RpgPickable::RpgPickable(RpgGame *game, TiledScene *scene, const RpgGameData::PickableBaseData &base)
	: RpgActiveControl<RpgGameData::Pickable,
	  RpgGameData::PickableBaseData,
	  RpgActiveIface::DefaultEnum>(RpgConfig::ControlPickable)
	, RpgGameData::LifeCycle()
	, m_baseData(base)
{
	Q_ASSERT(scene);

	setGame(game);

	setIsActive(true);
	setQuestionLock(false);
	setIsLocked(false);

	m_visualItem = createVisualItem(scene, nullptr);
	m_visualItem->setSource(QUrl::fromLocalFile(":/rpg/key/pickable.png"));
	m_visualItem->setVisible(true);

	LOG_CDEBUG("scene") << "Add visual item" << this << m_visualItem->name() << base.o << base.s << base.id;

	QPointF pos;

	if (base.p.size() > 1) {
		pos.setX(base.p.at(0));
		pos.setY(base.p.at(1));
	}

	RpgPickableControlObject *o = game->createObject<RpgPickableControlObject>(m_baseData.o,
																				   scene,
																				   m_baseData.id,
																				   this,
																				   pos, 25.,
																				   game);

	if (!o)
		return;

	o->setSensor(true);
	o->setVisualItem(m_visualItem);

	controlObjectAdd(o);

	onActivated();
}



/**
 * @brief RpgPickable::objectId
 * @return
 */

TiledObjectBody::ObjectId RpgPickable::objectId() const
{
	return TiledObjectBody::ObjectId{
		.ownerId = m_baseData.o,
				.sceneId = m_baseData.s,
				.id = m_baseData.id
	};
}


/**
 * @brief RpgPickable::updateFromSnapshot
 * @param snapshot
 */

void RpgPickable::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Pickable> &snapshot)
{
	if (snapshot.s1.f < 0 && snapshot.last.f < 0) {
		LOG_CERROR("scene") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.last.f << snapshot.current;
		return;
	}

	updateFromSnapshot(snapshot.last);
}


/**
 * @brief RpgPickable::updateFromSnapshot
 * @param snap
 */

void RpgPickable::updateFromSnapshot(const RpgGameData::Pickable &snap)
{
	setIsActive(snap.a);
	setIsLocked(snap.lck);
	_updateGlow();
}


/**
 * @brief RpgPickable::use
 * @param player
 */

void RpgPickable::use(RpgPlayer *player)
{

}



/**
 * @brief RpgPickable::serializeThis
 * @return
 */

RpgGameData::Pickable RpgPickable::serializeThis() const
{
	return {};
}


/**
 * @brief RpgPickable::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgPickable::onShapeContactBegin(cpShape *self, cpShape *other)
{
	RpgActiveIface::onShapeContactBegin(self, other);
	_updateGlow();
}




/**
 * @brief RpgPickable::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgPickable::onShapeContactEnd(cpShape *self, cpShape *other)
{
	RpgActiveIface::onShapeContactEnd(self, other);
	_updateGlow();
}



/**
 * @brief RpgPickable::_updateGlow
 */

void RpgPickable::_updateGlow()
{
	if (!m_isActive)
		return updateGlow(false);

	bool glow = false;

	for (cpShape *s : m_contactedFixtures) {
		if (TiledObjectBody::fromShapeRef(s) == m_game->controlledPlayer()) {
			glow = true;
			break;
		}
	}

	updateGlow(glow);
}




/**
 * @brief RpgPickableControlObject::RpgPickableControlObject
 * @param control
 * @param center
 * @param radius
 * @param game
 */

RpgPickableControlObject::RpgPickableControlObject(RpgPickable *control, const QPointF &center, const qreal &radius, TiledGame *game)
	: RpgActiveControlObject(control, center, radius, game, nullptr, CP_BODY_TYPE_STATIC)
	, m_control(control)
{

}



/**
 * @brief RpgPickableControlObject::synchronize
 */

void RpgPickableControlObject::synchronize()
{
	if (!m_visualItem)
		return;

	const QPointF &pos = bodyPositionF();

	QPointF offset(m_visualItem->width()/2, m_visualItem->height()/2);

	m_visualItem->setPosition(pos-offset);
}
