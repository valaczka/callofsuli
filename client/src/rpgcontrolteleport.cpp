/*
 * ---- Call of Suli ----
 *
 * rpgcontrolteleport.cpp
 *
 * Created on: 2025. 06. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlTeleport
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

#include "rpgcontrolteleport.h"
#include "rpggame.h"
#include <libtiled/objectgroup.h>


RpgControlTeleport::RpgControlTeleport(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, const bool &isHideout,
									   Tiled::MapRenderer *renderer)
	: RpgActiveControl<RpgGameData::ControlTeleport,
	  RpgGameData::ControlTeleportBaseData,
	  RpgControlTeleportState>(RpgConfig::ControlTeleport)
{
	Q_ASSERT(group);
	Q_ASSERT(scene);

	m_currentState = Inactive;

	m_stateHash = QHash<QString, RpgControlTeleportState>{
		{ QStringLiteral("inactive"), Inactive },
		{ QStringLiteral("active"), Active },
		{ QStringLiteral("operating"), Operating },
	};

	if (isHideout) {
		m_helperText.first = QObject::tr("Hide yourself");
		m_helperText.second = QObject::tr("Hideout locked, find the key");
	} else
		m_helperText.first = QObject::tr("Use the teleport");

	setGame(game);
	loadFromGroupLayer(game, scene, group, renderer);

	m_baseData.t = RpgConfig::ControlTeleport;
	m_baseData.o = -1;
	m_baseData.id = group->id();
	m_baseData.s = scene->sceneId();
	m_baseData.hd = isHideout;
	//m_baseData.lck;
	//m_baseData.dst = ?

	setQuestionLock(false);

	if (group->hasProperty(QStringLiteral("lock"))) {
		setKeyLock(group->propertyAsString(QStringLiteral("lock")));
		setIsLocked(true);
	} else {
		setIsLocked(false);
	}

	if (isHideout) {
		setIsActive(true);
		onActivated();
	} else {
		setIsActive(false);
		onDeactivated();
	}
}



/**
 * @brief RpgControlTeleport::updateFromSnapshot
 * @param snapshot
 */

void RpgControlTeleport::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::ControlTeleport> &snapshot)
{
	if (snapshot.s1.f < 0 && snapshot.last.f < 0) {
		LOG_CERROR("scene") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.last.f << snapshot.current;
		return;
	}

	RpgControlTeleport::updateFromSnapshot(snapshot.last);

}



/**
 * @brief RpgControlTeleport::updateFromSnapshot
 * @param snap
 */

void RpgControlTeleport::updateFromSnapshot(const RpgGameData::ControlTeleport &snap)
{
	setIsActive(snap.a);
	setIsLocked(snap.lck);
	if (m_isActive && snap.op)
		setCurrentState(Operating);
	else
		setCurrentState(m_isActive ? Active : Inactive);

	_updateGlow();
}





/**
 * @brief RpgControlTeleport::loadFromLayer
 * @param game
 * @param scene
 * @param layer
 * @param renderer
 * @return
 */

bool RpgControlTeleport::loadFromLayer(RpgGame *game, TiledScene *scene, Tiled::Layer *layer, Tiled::MapRenderer *renderer)
{
	struct ExitPoint {
		float x = 0;
		float y = 0;
		float a = 0;
	};


	if (Tiled::ObjectGroup *objgroup = layer->asObjectGroup()) {
		std::optional<ExitPoint> exitPoint;

		for (Tiled::MapObject *object : std::as_const(objgroup->objects())) {
			const QString &clName = object->className();

			if (clName == QStringLiteral("dynamicZ")) {
				m_game->loadDynamicZ(scene, object, renderer);
			} else if (clName == QStringLiteral("trigger")) {
				if (!object->name().isEmpty())
					m_game->loadDynamicZ(scene, object, renderer);

				RpgActiveControlObject *o = game->createObject<RpgActiveControlObject>(-1, scene, object->id(),
																					   this,
																					   object,
																					   game, renderer, CP_BODY_TYPE_STATIC);

				o->setSensor(true);

				controlObjectAdd(o);

				const QPointF &pos = o->bodyAABB().center();

				if (!exitPoint.has_value()) {
					ExitPoint p;
					p.x = pos.x();
					p.y = pos.y();
					exitPoint = p;
				}
			} else if (clName == QStringLiteral("exit")) {
				QPointF pos = renderer ? renderer->pixelToScreenCoords(object->position()) : object->position();
				pos += m_basePosition;

				ExitPoint p;
				p.x = pos.x();
				p.y = pos.y();

				if (object->hasProperty(QStringLiteral("angle"))) {
					p.a = TiledObject::toRadian(object->property(QStringLiteral("angle")).toInt());
				}

				exitPoint = p;
			}
		}

		if (exitPoint.has_value()) {
			m_baseData.x = exitPoint->x;
			m_baseData.y = exitPoint->y;
			m_baseData.a = exitPoint->a;
		}
	} else {
		return false;
	}

	return true;
}



/**
 * @brief RpgControlTeleport::serializeThis
 * @return
 */

RpgGameData::ControlTeleport RpgControlTeleport::serializeThis() const
{
	RpgGameData::ControlTeleport c;

	c.sc = m_baseData.s;
	//c.op = false;
	c.lck = m_isLocked;
	c.a = m_isActive;

	return c;
}


/**
 * @brief RpgControlTeleport::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgControlTeleport::onShapeContactBegin(cpShape *self, cpShape *other)
{
	RpgActiveIface::onShapeContactBegin(self, other);
	_updateGlow();
}


/**
 * @brief RpgControlTeleport::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgControlTeleport::onShapeContactEnd(cpShape *self, cpShape *other)
{
	RpgActiveIface::onShapeContactEnd(self, other);
	_updateGlow();
}





void RpgControlTeleport::_updateGlow()
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

