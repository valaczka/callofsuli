/*
 * ---- Call of Suli ----
 *
 * rpgcontrolcontainer.cpp
 *
 * Created on: 2025. 05. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlContainer
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

#include "rpgcontrolcontainer.h"
#include "rpggame.h"
#include "rpgquestion.h"
#include <libtiled/objectgroup.h>


RpgControlContainer::RpgControlContainer(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
	: RpgActiveControl<RpgGameData::ControlContainer,
	  RpgGameData::ControlContainerBaseData,
	  RpgGameData::ControlContainer::State>(RpgConfig::ControlContainer)
{
	Q_ASSERT(group);
	Q_ASSERT(scene);

	m_currentState = RpgGameData::ControlContainer::ContainerClose;

	m_stateHash = QHash<QString, RpgGameData::ControlContainer::State>{
		{ QStringLiteral("close"), RpgGameData::ControlContainer::ContainerClose },
		{ QStringLiteral("open"), RpgGameData::ControlContainer::ContainerOpen },
	};

	setGame(game);
	loadFromGroupLayer(game, scene, group, renderer);

	m_baseData.t = RpgConfig::ControlContainer;
	m_baseData.o = -1;
	m_baseData.id = group->id();
	m_baseData.s = scene->sceneId();
	m_baseData.inv.add(RpgGameData::PickableBaseData::PickableHp);
	//m_baseData.lck;

	if (!m_controlObjectList.isEmpty()) {
		const cpVect &pos = m_controlObjectList.first()->bodyPosition();
		m_baseData.x = pos.x;
		m_baseData.y = pos.y;
	}

	setIsActive(true);
	setQuestionLock(true);

	if (group->hasProperty(QStringLiteral("lock"))) {
		setKeyLock(group->propertyAsString(QStringLiteral("lock")));
		setIsLocked(true);
	} else {
		setIsLocked(false);
	}
}





/**
 * @brief RpgControlContainer::updateFromSnapshot
 * @param snapshot
 */

void RpgControlContainer::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::ControlContainer> &snapshot)
{
	if (snapshot.s1.f < 0 && snapshot.last.f < 0) {
		LOG_CERROR("scene") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.last.f << snapshot.current;
		return;
	}

	RpgControlContainer::updateFromSnapshot(snapshot.last);

	if (!m_game) {
		LOG_CERROR("game") << "Missing RpgGame";
		return;
	}

	loadQuestion(snapshot, m_game->controlledPlayer(), m_game->rpgQuestion());

}



/**
 * @brief RpgControlContainer::updateFromSnapshot
 * @param snap
 */

void RpgControlContainer::updateFromSnapshot(const RpgGameData::ControlContainer &snap)
{
	setCurrentState(snap.st);
	setIsActive(snap.a);
	setIsLocked(snap.lck);
	_updateGlow();
}





/**
 * @brief RpgControlContainer::loadFromLayer
 * @param game
 * @param scene
 * @param layer
 * @param renderer
 * @return
 */

bool RpgControlContainer::loadFromLayer(RpgGame *game, TiledScene *scene, Tiled::Layer *layer, Tiled::MapRenderer *renderer)
{
	if (Tiled::ObjectGroup *objgroup = layer->asObjectGroup()) {
		for (Tiled::MapObject *object : std::as_const(objgroup->objects())) {
			if (object->shape() == Tiled::MapObject::Point) {

				// Külön kell számolni az eltolást, nem lesz jó a rendereren keresztül

				QPointF pos = renderer ? renderer->pixelToScreenCoords(object->position()) : object->position();
				pos += m_basePosition;

				RpgActiveControlObject *o = game->createObject<RpgActiveControlObject>(-1, scene, object->id(),
																					   this,
																					   pos, 35.,
																					   game, nullptr, CP_BODY_TYPE_STATIC);

				o->setSensor(true);

				controlObjectAdd(o);
			}
		}

		onActivated();

		return true;
	}

	return false;
}


/**
 * @brief RpgControlContainer::serializeThis
 * @return
 */

RpgGameData::ControlContainer RpgControlContainer::serializeThis() const
{
	RpgGameData::ControlContainer c;

	c.sc = m_baseData.s;
	c.st = m_currentState;
	c.lck = m_isLocked;
	c.a = m_isActive;

	return c;
}




/**
 * @brief RpgControlContainer::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgControlContainer::onShapeContactBegin(cpShape *self, cpShape *other)
{
	RpgActiveIface::onShapeContactBegin(self, other);

	_updateGlow();
}



/**
 * @brief RpgControlContainer::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgControlContainer::onShapeContactEnd(cpShape *self, cpShape *other)
{
	RpgActiveIface::onShapeContactEnd(self, other);
	_updateGlow();
}





/**
 * @brief RpgControlContainer::_updateGlow
 */

void RpgControlContainer::_updateGlow()
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



