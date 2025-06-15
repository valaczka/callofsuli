/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgate.cpp
 *
 * Created on: 2025. 06. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGate
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

#include "rpgcontrolgate.h"
#include "rpggame.h"
#include <libtiled/objectgroup.h>



/**
 * @brief RpgControlGate::RpgControlGate
 * @param game
 * @param scene
 * @param group
 * @param renderer
 */

RpgControlGate::RpgControlGate(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
	: RpgActiveControl<RpgGameData::ControlGate,
	  RpgGameData::ControlGateBaseData,
	  RpgGameData::ControlGate::State>(RpgConfig::ControlGate)
	, m_scene(scene)
{
	Q_ASSERT(group);
	Q_ASSERT(scene);

	m_currentState = RpgGameData::ControlGate::GateClose;

	m_stateHash = QHash<QString, RpgGameData::ControlGate::State>{
		{ QStringLiteral("close"), RpgGameData::ControlGate::GateClose },
		{ QStringLiteral("open"), RpgGameData::ControlGate::GateOpen },
		{ QStringLiteral("damaged"), RpgGameData::ControlGate::GateDamaged },
	};

	setGame(game);
	loadFromGroupLayer(game, scene, group, renderer);

	m_baseData.t = RpgConfig::ControlGate;
	m_baseData.o = -1;
	m_baseData.id = group->id();
	m_baseData.s = scene->sceneId();
	//m_baseData.lck;

	setIsActive(true);
	setQuestionLock(false);

	if (group->hasProperty(QStringLiteral("lock"))) {
		setKeyLock(group->propertyAsString(QStringLiteral("lock")));
		setIsLocked(true);
	} else {
		setIsLocked(false);
	}

	onCurrentStateChanged();
	onActivated();
}




/**
 * @brief RpgControlGate::updateFromSnapshot
 * @param snapshot
 */

void RpgControlGate::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::ControlGate> &snapshot)
{
	if (snapshot.s1.f < 0 && snapshot.last.f < 0) {
		LOG_CERROR("scene") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.last.f << snapshot.current;
		return;
	}

	RpgControlGate::updateFromSnapshot(snapshot.last);
}



/**
 * @brief RpgControlGate::updateFromSnapshot
 * @param snap
 */

void RpgControlGate::updateFromSnapshot(const RpgGameData::ControlGate &snap)
{
	setCurrentState(snap.st);
	setIsActive(snap.a);
	setIsLocked(snap.lck);
	_updateGlow();
}





/**
 * @brief RpgControlGate::loadFromLayer
 * @param game
 * @param scene
 * @param layer
 * @param renderer
 * @return
 */

bool RpgControlGate::loadFromLayer(RpgGame *game, TiledScene *scene, Tiled::Layer *layer, Tiled::MapRenderer *renderer)
{
	if (Tiled::ObjectGroup *objgroup = layer->asObjectGroup()) {
		for (Tiled::MapObject *object : std::as_const(objgroup->objects())) {
			const QString &clName = object->className();

			if (clName == QStringLiteral("dynamicZ")) {
				m_game->loadDynamicZ(scene, object, renderer);
			} else if (clName == QStringLiteral("ground")) {
				m_game->loadGround(scene, object, renderer);
			} else if (m_stateHash.contains(clName)) {
				const auto &st = m_stateHash.value(clName);

				TiledObjectBody *body = m_game->loadGround(scene, object, renderer);

				m_groundList[st].append(body);

			} else if (clName == QStringLiteral("trigger")) {
				RpgActiveControlObject *o = game->createObject<RpgActiveControlObject>(-1, scene, object->id(),
																					   this,
																					   object,
																					   game, renderer, CP_BODY_TYPE_STATIC);

				o->setSensor(true);

				controlObjectAdd(o);
			}
		}
	} else {
		return false;
	}

	return true;
}



/**
 * @brief RpgControlGate::serializeThis
 * @return
 */

RpgGameData::ControlGate RpgControlGate::serializeThis() const
{
	RpgGameData::ControlGate c;

	c.sc = m_baseData.s;
	c.st = m_currentState;
	c.lck = m_isLocked;
	c.a = m_isActive;

	return c;
}



/**
 * @brief RpgControlGate::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgControlGate::onShapeContactBegin(cpShape *self, cpShape *other)
{
	RpgActiveIface::onShapeContactBegin(self, other);
	_updateGlow();
}



/**
 * @brief RpgControlGate::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgControlGate::onShapeContactEnd(cpShape *self, cpShape *other)
{
	RpgActiveIface::onShapeContactEnd(self, other);
	_updateGlow();
}



/**
 * @brief RpgControlGate::onCurrentStateChanged
 */

void RpgControlGate::onCurrentStateChanged()
{
	for (const auto &[st, list] : m_groundList.asKeyValueRange()) {
		for (TiledObjectBody *o : list) {
			o->filterSet(m_currentState == st && st == RpgGameData::ControlGate::GateClose ?
							 TiledObjectBody::FixtureGround :
							 TiledObjectBody::FixtureInvalid
							 );
		}
	}

	m_game->reloadTcodMap(m_scene);
}



/**
 * @brief RpgControlGate::_updateGlow
 */

void RpgControlGate::_updateGlow()
{
	if (m_currentState == RpgGameData::ControlGate::GateDamaged)
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
