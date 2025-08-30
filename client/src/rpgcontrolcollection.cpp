/*
 * ---- Call of Suli ----
 *
 * rpgcontrolcollection.cpp
 *
 * Created on: 2025. 05. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlCollection
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

#include "rpgcontrolcollection.h"
#include "rpggame.h"

#define ITEM_MAX_SPEED				350


static const auto fnStopCollection = [](TiledObjectBody *body){
	const cpVect vel = cpBodyGetVelocity(body->body());

	if (vel.x != 0. || vel.y != 0.)
		body->stop();
};



/**
 * @brief RpgControlCollection::RpgControlCollection
 * @param game
 * @param scene
 * @param base
 * @param renderer
 */

RpgControlCollection::RpgControlCollection(RpgGame *game, TiledScene *scene,
										   const RpgGameData::ControlCollectionBaseData &base, const QPointF &pos)
	: RpgActiveControl<RpgGameData::ControlCollection,
	  RpgGameData::ControlCollectionBaseData,
	  RpgActiveIface::DefaultEnum>(RpgConfig::ControlCollection)
{
	Q_ASSERT(scene);

	m_baseData = base;

	setGame(game);

	setIsActive(true);
	setQuestionLock(true);
	setIsLocked(false);


	m_helperText.first = QObject::tr("Collect the item");

	m_visualItem = createVisualItem(scene, nullptr);

	const RpgCollectionData &data = m_game->getCollectionImageData(base.img);

	if (data.url.isEmpty())
		LOG_CERROR("scene") << "Missing collection image" << base;
	else
		m_visualItem->setSource(data.url);

	m_visualItem->setVisible(true);

	m_visualItem->setDisplayName(data.displayName);

	if (!data.helperText.isEmpty())
		m_helperText.first = data.helperText;

	int size = data.size > 0 ? data.size : 20;

	RpgControlCollectionObject *o = game->createObject<RpgControlCollectionObject>(m_baseData.o,
																				   scene,
																				   m_baseData.id,
																				   this,
																				   pos, size,
																				   game);

	o->setSensor(true);
	o->setVisualItem(m_visualItem);
	o->setSubZ(0.2);

	controlObjectAdd(o);

	onActivated();

}



/**
 * @brief RpgControlCollection::updateFromSnapshot
 * @param snapshot
 */

void RpgControlCollection::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::ControlCollection> &snapshot)
{
	if (snapshot.s1.f < 0 && snapshot.last.f < 0) {
		LOG_CERROR("scene") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.last.f << snapshot.current;
		return;
	}

	RpgControlCollection::updateFromSnapshot(snapshot.last);

	if (!m_game) {
		LOG_CERROR("game") << "Missing RpgGame";
		return;
	}

	loadQuestion(snapshot, m_game->controlledPlayer(), m_game->rpgQuestion());

	if (m_controlObjectList.size() != 1) {
		LOG_CERROR("game") << "Invalid control object" << this;
		return;
	}


	RpgActiveControlObject *control = m_controlObjectList.first();

	const RpgGameData::ControlCollection &from = snapshot.s1.f >= 0 ? snapshot.s1 : snapshot.last;
	const RpgGameData::ControlCollection &to = snapshot.s2.f >= 0 ? snapshot.s2 : snapshot.last;

	if (from.f < 0 || to.f < 0 || from.f > to.f) {
		return fnStopCollection(control);
	}

	if (to.p.size() < 2) {
		return fnStopCollection(control);
	}

	const cpVect pos1 = control->bodyPosition();
	const cpVect pos2 = cpv(to.p.at(0), to.p.at(1));

	if (pos1 == pos2)
		return fnStopCollection(control);

	// Amíg mozog, addig nem lehet aktív

	setIsActive(false);
	_updateGlow();

	if (to.f <= snapshot.current) {
		control->moveToPoint(pos2, 1, ITEM_MAX_SPEED);
		return;
	} else {
		const int frames = to.f-snapshot.current;
		control->moveToPoint(pos2, frames, ITEM_MAX_SPEED);
		return;
	}
}



/**
 * @brief RpgControlCollection::updateFromSnapshot
 * @param snap
 */

void RpgControlCollection::updateFromSnapshot(const RpgGameData::ControlCollection &snap)
{
	setIsActive(snap.a);
	setIsLocked(snap.lck);

	if (m_visualItem)
		m_visualItem->setVisible(!snap.own.isValid());

	_updateGlow();
}




/**
 * @brief RpgControlCollection::serializeThis
 * @return
 */

RpgGameData::ControlCollection RpgControlCollection::serializeThis() const
{
	return {};
}




/**
 * @brief RpgControlCollection::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgControlCollection::onShapeContactBegin(cpShape *self, cpShape *other)
{
	RpgActiveIface::onShapeContactBegin(self, other);
	_updateGlow();
}



/**
 * @brief RpgControlCollection::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgControlCollection::onShapeContactEnd(cpShape *self, cpShape *other)
{
	RpgActiveIface::onShapeContactEnd(self, other);
	_updateGlow();
}






/**
 * @brief RpgControlCollection::_updateGlow
 */

void RpgControlCollection::_updateGlow()
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
 * @brief RpgControlCollection::idx
 * @return
 */

int RpgControlCollection::idx() const
{
	return m_idx;
}

void RpgControlCollection::setIdx(int newIdx)
{
	m_idx = newIdx;
}


/**
 * @brief RpgControlCollection::moveTo
 * @param dest
 */

void RpgControlCollection::moveTo(const qint64 &startAt, const cpVect &dest)
{
	RpgControlCollectionObject *control = qobject_cast<RpgControlCollectionObject*>(m_controlObjectList.first());

	if (!control)
		return;

	setIsActive(false);
	_updateGlow();
	control->setDest(startAt, dest);
}





/**
 * @brief RpgControlCollectionObject::synchronize
 */

RpgControlCollectionObject::RpgControlCollectionObject(RpgControlCollection *control,
													   const QPointF &center, const qreal &radius,
													   TiledGame *game)
	: RpgActiveControlObject(control, center, radius, game, nullptr, CP_BODY_TYPE_KINEMATIC)
	, m_control(control)
{

}



/**
 * @brief RpgControlCollectionObject::setDest
 * @param dest
 */

void RpgControlCollectionObject::setDest(const qint64 &startAt, const cpVect &dest)
{
	m_startAt = startAt;
	m_dest = dest;
}




/**
 * @brief RpgControlCollectionObject::synchronize
 */


void RpgControlCollectionObject::synchronize()
{
	if (!m_visualItem)
		return;

	const QPointF &pos = bodyPositionF();

	QPointF offset(m_visualItem->width()/2, m_visualItem->height()/2);
	//offset += m_bodyOffset;

	m_visualItem->setPosition(pos-offset);
}





/**
 * @brief RpgControlCollectionObject::worldStep
 */

void RpgControlCollectionObject::worldStep()
{
	if (m_dest.has_value()) {
		if (m_game->tickTimer()->currentTick() < m_startAt) {
			if (currentSpeedSq())
			fnStopCollection(this);
			m_control->setIsActive(false);
			return;
		}

		if (m_dest.value() == bodyPosition()) {
			fnStopCollection(this);
			m_control->setIsActive(true);
			m_dest.reset();

			return;
		}

		moveToPoint(m_dest.value(), 1, ITEM_MAX_SPEED);
		return;
	}

	TiledObjectBody::worldStep();
}
