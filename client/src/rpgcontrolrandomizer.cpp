/*
 * ---- Call of Suli ----
 *
 * rpgcontrolrandomizer.cpp
 *
 * Created on: 2025. 06. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlRandomizer
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

#include "rpgcontrolrandomizer.h"
#include "rpggame.h"
#include <libtiled/objectgroup.h>


RpgControlRandomizer::RpgControlRandomizer(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
	: RpgControlBase(RpgConfig::ControlRandomizer)
{
	Q_ASSERT(group);
	Q_ASSERT(scene);

	setGame(game);

	m_baseData.o = -1;
	m_baseData.id = group->id();
	m_baseData.s = scene->sceneId();
	m_baseData.t = RpgConfig::ControlRandomizer;

	setName(group->name());
	addGroupLayer(scene, group, renderer);
}




/**
 * @brief RpgControlRandomizer::baseData
 * @return
 */

const RpgGameData::ControlBaseData &RpgControlRandomizer::baseData() const
{
	return m_baseData;
}



/**
 * @brief RpgControlRandomizer::addGroupLayer
 * @param scene
 * @param group
 * @param renderer
 */

void RpgControlRandomizer::addGroupLayer(TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(group);

	RpgControlRandomizerContent content(group->id());

	for (Tiled::Layer *layer : std::as_const(*group)) {
		if (Tiled::ImageLayer *tl = layer->asImageLayer()) {
			TiledVisualItem *item = scene->addVisualItem(tl);
			item->setVisible(false);
			content.layerAdd(item);
		} else if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			TiledQuick::TileLayerItem *item = scene->addTileLayer(tl, renderer);
			item->setVisible(false);
			content.layerAdd(item);
		} else if (Tiled::ObjectGroup *objgroup = layer->asObjectGroup()) {
			for (Tiled::MapObject *object : std::as_const(objgroup->objects())) {
				const QString &clName = object->className();

				if (clName == QStringLiteral("dynamicZ")) {
					m_game->loadDynamicZ(scene, object, renderer);
				} else if (clName == QStringLiteral("ground")) {
					TiledObjectBody *body = m_game->loadGround(scene, object, renderer);
					body->filterSet(TiledObjectBody::FixtureInvalid);
					content.objectAdd(body);
				}
			}
		}
	}

	if (!content.isEmpty())
		m_content.push_back(content);
}




/**
 * @brief RpgControlRandomizer::name
 * @return
 */


QString RpgControlRandomizer::name() const
{
	return m_name;
}

void RpgControlRandomizer::setName(const QString &newName)
{
	m_name = newName;
}



/**
 * @brief RpgControlRandomizer::find
 * @param list
 * @param group
 * @return
 */

RpgControlRandomizer *RpgControlRandomizer::find(const std::vector<std::unique_ptr<RpgControlBase> > &list,
												 Tiled::GroupLayer *group, const int &sceneId)
{
	if (!group || list.empty())
		return nullptr;

	const QString &name = group->name();

	for (const auto &ptr : list) {
		if (RpgControlRandomizer *c = dynamic_cast<RpgControlRandomizer*>(ptr.get())) {
			if (c->name() == name && c->baseData().s == sceneId)
				return c;
		}
	}

	return nullptr;
}


/**
 * @brief RpgControlRandomizer::toRandomizerGroup
 * @return
 */

RpgGameData::RandomizerGroup RpgControlRandomizer::toRandomizerGroup() const
{
	RpgGameData::RandomizerGroup group(m_baseData.s, m_baseData.id);

	group.current = m_activeId;

	for (const RpgControlRandomizerContent &c : m_content)
		group.idList.append(c.id());

	return group;
}




/**
 * @brief RpgControlRandomizer::fromRandomizerGroup
 * @param group
 */

bool RpgControlRandomizer::fromRandomizerGroup(const RpgGameData::RandomizerGroup &group)
{
	if (group.gid != m_baseData.id || group.scene != m_baseData.s) {
		LOG_CERROR("game") << "Invalid randomizer group" << group.scene << group.gid;
		return false;
	}

	return setActiveId(group.current);
}




/**
 * @brief RpgControlRandomizer::activeId
 * @return
 */

int RpgControlRandomizer::activeId() const
{
	return m_activeId;
}

bool RpgControlRandomizer::setActiveId(int newActiveId)
{
	if (m_activeId == newActiveId)
		return false;

	m_activeId = newActiveId;

	for (RpgControlRandomizerContent &c : m_content)
		c.setActive(c.id() == m_activeId);

	return true;
}




/**
 * @brief RpgControlRandomizerContent::RpgControlRandomizerContent
 * @param id
 */

RpgControlRandomizerContent::RpgControlRandomizerContent(const int &id)
	: m_id(id)
{

}


/**
 * @brief RpgControlRandomizerContent::objectAdd
 * @param object
 */

void RpgControlRandomizerContent::objectAdd(TiledObjectBody *object)
{
	if (!object || m_objects.contains(object))
		return;

	m_objects.append(object);
}


/**
 * @brief RpgControlRandomizerContent::layerAdd
 * @param item
 */

void RpgControlRandomizerContent::layerAdd(QQuickItem *item)
{
	if (!item || m_layers.contains(item))
		return;

	m_layers.append(item);
}


/**
 * @brief RpgControlRandomizerContent::setActive
 * @param active
 */

void RpgControlRandomizerContent::setActive(const bool &active)
{
	for (TiledObjectBody *b : m_objects) {
		b->filterSet(active ?
						 TiledObjectBody::FixtureGround :
						 TiledObjectBody::FixtureInvalid
						 );
	}


	for (QQuickItem *item : m_layers) {
		if (item)
			item->setVisible(active);
	}

}

int RpgControlRandomizerContent::id() const
{
	return m_id;
}
