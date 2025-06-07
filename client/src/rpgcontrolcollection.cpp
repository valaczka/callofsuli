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

	m_visualItem = createVisualItem(scene, nullptr);
	m_visualItem->setSource(QUrl::fromLocalFile(":/rpg/time/pickable.png"));
	m_visualItem->setVisible(true);

	LOG_CDEBUG("scene") << "Add visual item" << this << m_visualItem->name();


	RpgControlCollectionObject *o = game->createObject<RpgControlCollectionObject>(m_baseData.o,
																				   scene,
																				   m_baseData.id,
																				   this,
																				   pos, 15.,
																				   game);

	o->setSensor(true);
	o->setVisualItem(m_visualItem);

	onActivated();

	controlObjectAdd(o);
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


	static const auto fnStop = [](TiledObjectBody *body){
		const cpVect vel = cpBodyGetVelocity(body->body());

		if (vel.x != 0. || vel.y != 0.)
			body->stop();
	};

	RpgActiveControlObject *control = m_controlObjectList.first();

	const RpgGameData::ControlCollection &from = snapshot.s1.f >= 0 ? snapshot.s1 : snapshot.last;
	const RpgGameData::ControlCollection &to = snapshot.s2.f >= 0 ? snapshot.s2 : snapshot.last;

	if (from.f < 0 || to.f < 0 || from.f > to.f) {
		LOG_CERROR("game") << "ERRROR" << from.f << to.f;
		return fnStop(control);
	}

	if (to.p.size() < 2) {
		LOG_CERROR("game") << "ERRROR TO P" << to.p;
		return fnStop(control);
	}

	const cpVect pos1 = control->bodyPosition();
	const cpVect pos2 = cpv(to.p.at(0), to.p.at(1));

	if (pos1 == pos2)
		return fnStop(control);


	if (to.f <= snapshot.current) {
		control->moveToPoint(pos2, 1, 250);
		return;
	} else {
		const int frames = to.f-snapshot.current;
		control->moveToPoint(pos2, frames, 250);
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
