/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupstate.h
 *
 * Created on: 2024. 10. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupState
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

#ifndef RPGCONTROLGROUPSTATE_H
#define RPGCONTROLGROUPSTATE_H

#include "rpgcontrolgroup.h"
#include <libtiled/grouplayer.h>
#include <libtiled/imagelayer.h>
#include "tiledvisualitem.h"
#include "rpgplayer.h"
#include "rpggame.h"



/*
 * template <class T>
 *
 * class T {
 * 	int id = -1;
 * 	QUrl image;
 * 	QPointF relativePosition;
 * }
 *
*/



class RpgControlGroupStateIface
{
protected:
	virtual void onFixtureBeginContact(cpShape *) = 0;
	virtual void onFixtureEndContact(cpShape *) = 0;

	friend class RpgControlGroupStateBody;
};




/**
 * @brief The RpgControlGroupStateBody class
 */

class RpgControlGroupStateBody : public TiledObjectBody
{
public:
	explicit RpgControlGroupStateBody(const QPointF &center, const qreal &radius,
									  TiledGame *game,
									  Tiled::MapRenderer *renderer,
									  const cpBodyType &type = CP_BODY_TYPE_STATIC)
		: TiledObjectBody(center, radius, game, renderer, type)
	{
	}

	virtual void onShapeContactBegin(cpShape *, cpShape *shape) override {
		if (m_control)
			m_control->onFixtureBeginContact(shape);
	}

	virtual void onShapeContactEnd(cpShape *, cpShape *shape) override {
		if (m_control)
			m_control->onFixtureEndContact(shape);
	}

	RpgControlGroupStateIface *m_control = nullptr;

};



/**
 * @brief The RpgControlGroupState class
 */

template <class T>
class RpgControlGroupState : public RpgControlGroup, public RpgControlGroupStateIface
{
public:
	RpgControlGroupState(const RpgControlGroup::Type &type, RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group);
	virtual ~RpgControlGroupState() {}


	TiledVisualItem *visualItem() const { return m_visualItem; }
	void setVisualItem(TiledVisualItem *newVisualItem);

	TiledVisualItem *createVisualItem(Tiled::GroupLayer *layer);

	void addTiledObjectBody(RpgControlGroupStateBody *object);

	const QVector<RpgControlGroupStateBody*> &tiledObjects() const;

	int currentState() const { return m_currentState; }

	QPointF basePosition() const { return m_basePosition; }
	void setBasePosition(QPointF newBasePosition);

	void removePlayerFixture(RpgPlayer *player);

protected:
	const QVector<T> &states() const { return m_states; }
	QVector<T>::const_iterator cstate(const int &id = -1) const;
	QVector<T>::iterator state(const int &id = -1);

	bool stateAdd(const T &state);

	void refreshVisualItem() const;

	virtual void onFixtureBeginContact(cpShape *) override;
	virtual void onFixtureEndContact(cpShape *) override;
	cpShapeFilter getShapeParams(const TiledObjectBody::FixtureCategories &category = TiledObjectBody::FixtureTrigger) const;
	void onControlledPlayerChanged();
	void updateGlow();

	QPointF m_basePosition;
	QVector<T> m_states;
	QPointer<TiledVisualItem> m_visualItem;
	QVector<RpgControlGroupStateBody*> m_tiledObjects;
	int m_currentState = -1;

	QVector<QPointer<TiledQuick::TileLayerItem>> m_tileLayers;
	std::vector<cpShape *> m_contactedFixtures;
};



/**
 * @brief RpgControlGroupState::RpgControlGroupState
 * @param type
 * @param game
 * @param scene
 * @param group
 */

template<class T>
RpgControlGroupState<T>::RpgControlGroupState(const Type &type, RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group)
	: RpgControlGroup(type, game, scene)
	, RpgControlGroupStateIface()
{
	Q_ASSERT(game);
	Q_ASSERT(scene);
	Q_ASSERT(group);

	setBasePosition(group->position()+group->totalOffset());

	QObject::connect(game, &RpgGame::controlledPlayerChanged, game, [this](){
		this->onControlledPlayerChanged();
	});
}



/**
 * @brief RpgControlGroupState::setVisualItem
 * @param newVisualItem
 */

template<class T>
inline void RpgControlGroupState<T>::setVisualItem(TiledVisualItem *newVisualItem)
{
	if (m_visualItem == newVisualItem)
		return;
	m_visualItem = newVisualItem;

	refreshVisualItem();
}



/**
 * @brief RpgControlGroupState::createVisualItem
 * @param layer
 * @return
 */

template<class T>
inline TiledVisualItem *RpgControlGroupState<T>::createVisualItem(Tiled::GroupLayer *layer)
{
	if (!m_scene)
		return nullptr;

	TiledVisualItem *item = m_scene->addVisualItem();

	Q_ASSERT(item);

	item->setName(layer->name());
	item->setGlowColor(QStringLiteral("#FFF59D"));

	if (layer && layer->hasProperty(QStringLiteral("z"))) {
		item->setZ(layer->property(QStringLiteral("z")).toInt());
	} else {
		item->setZ(0);
	}

	setVisualItem(item);

	return item;
}


/**
 * @brief RpgControlGroupState::addTiledObject
 * @param object
 */

template<class T>
inline void RpgControlGroupState<T>::addTiledObjectBody(RpgControlGroupStateBody *object)
{
	Q_ASSERT(object);
	m_tiledObjects.append(object);
}



/**
 * @brief RpgControlGroupState::tiledObjects
 * @return
 */

template<class T>
inline const QVector<RpgControlGroupStateBody *> &RpgControlGroupState<T>::tiledObjects() const
{
	return m_tiledObjects;
}




/**
 * @brief RpgControlGroupState::setBasePosition
 * @param newBasePosition
 */

template<class T>
inline void RpgControlGroupState<T>::setBasePosition(QPointF newBasePosition)
{
	if (m_basePosition == newBasePosition)
		return;
	m_basePosition = newBasePosition;
	refreshVisualItem();
}




/**
 * @brief RpgControlGroupState::removePlayerFixture
 * @param player
 */

template<class T>
inline void RpgControlGroupState<T>::removePlayerFixture(RpgPlayer *player)
{
	if (!player)
		return;

	onFixtureEndContact(player->sensorPolygon());
	onFixtureEndContact(player->virtualCircle());
	onFixtureEndContact(player->targetCircle());

	for (cpShape *sh : player->bodyShapes())
		onFixtureEndContact(sh);
}



/**
 * @brief RpgControlGroupState::refreshVisualItem
 */

template<class T>
inline void RpgControlGroupState<T>::refreshVisualItem() const
{
	if (!m_visualItem)
		return;

	const auto &ptr = cstate();

	if (ptr == m_states.cend()) {
		m_visualItem->setSource({});
		m_visualItem->setVisible(false);
	} else {
		m_visualItem->setSource(ptr->image);
		m_visualItem->setVisible(true);
		m_visualItem->setPosition(m_basePosition + ptr->relativePosition);
	}
}


/**
 * @brief RpgControlGroupState::onFixtureBeginContact
 * @param other
 */

template<class T>
inline void RpgControlGroupState<T>::onFixtureBeginContact(cpShape *other)
{
	const auto it = std::find_if(m_contactedFixtures.cbegin(), m_contactedFixtures.cend(), [other](cpShape *s){
					return s == other;
});

	if (it == m_contactedFixtures.cend()) {
		m_contactedFixtures.push_back(other);
		updateGlow();
	}
}



template<class T>
inline void RpgControlGroupState<T>::onFixtureEndContact(cpShape *other)
{
	std::erase_if(m_contactedFixtures, [other](cpShape *s){
		return s == other;
	});
	updateGlow();
}



/**
 * @brief RpgControlGroupState::onControlledPlayerChanged
 */

template<class T>
inline void RpgControlGroupState<T>::onControlledPlayerChanged()
{
	m_contactedFixtures.clear();
	updateGlow();
}



/**
 * @brief RpgControlGroupState::connectFixture
 * @param fixture
 */


template<class T>
inline cpShapeFilter RpgControlGroupState<T>::getShapeParams(const TiledObjectBody::FixtureCategories &category) const
{
	return TiledObjectBody::getFilter(category,
									  TiledObjectBody::FixturePlayerBody |
									  TiledObjectBody::FixtureSensor |
									  TiledObjectBody::FixtureVirtualCircle);
}


/**
 * @brief RpgControlGroupState::updateGlow
 */

template<class T>
inline void RpgControlGroupState<T>::updateGlow()
{
	if (m_visualItem)
		m_visualItem->setGlowEnabled(!m_contactedFixtures.empty());
}


/**
 * @brief RpgControlGroupState::state
 * @param id
 * @return
 */

template<class T>
inline QVector<T>::iterator RpgControlGroupState<T>::state(const int &id)
{
	if (id == -1)
		return std::find_if(m_states.begin(), m_states.end(), [this](const T &s){
			return s.id == m_currentState;
		});
	else
		return std::find_if(m_states.begin(), m_states.end(), [&id](const T &s){
			return s.id == id;
		});
}


/**
 * @brief RpgControlGroupState::cstate
 * @param id
 * @return
 */

template<class T>
inline QVector<T>::const_iterator RpgControlGroupState<T>::cstate(const int &id) const
{
	if (id == -1)
		return std::find_if(m_states.cbegin(), m_states.cend(), [this](const T &s){
			return s.id == m_currentState;
		});
	else
		return std::find_if(m_states.cbegin(), m_states.cend(), [&id](const T &s){
			return s.id == id;
		});
}



/**
 * @brief RpgControlGroupState::stateAdd
 * @param state
 * @return
 */

template<class T>
inline bool RpgControlGroupState<T>::stateAdd(const T &state)
{
	if (std::find_if(m_states.cbegin(), m_states.cend(), [&state](const T &s){
					 return s.id == state.id;
}) != m_states.cend()) {
		LOG_CERROR("scene") << "State already exists" << state.id;
		return false;
	}

	m_states.append(state);

	return true;
}

#endif // RPGCONTROLGROUPSTATE_H
