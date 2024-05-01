/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupcontainer.cpp
 *
 * Created on: 2024. 04. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupContainer
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

#include "rpgcontrolgroupcontainer.h"
#include <libtiled/grouplayer.h>
#include <libtiled/objectgroup.h>
#include "rpggame.h"




/**
 * @brief RpgControlGroupContainer::RpgControlGroupContainer
 * @param game
 * @param scene
 * @param group
 * @param renderer
 */

RpgControlGroupContainer::RpgControlGroupContainer(RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group, Tiled::MapRenderer *renderer)
	: RpgControlGroup(ControlGroupContainer, game, scene)
	, m_container(new RpgChestContainer)
{
	Q_ASSERT(game);
	Q_ASSERT(scene);
	Q_ASSERT(group);
	Q_ASSERT(renderer);

	m_container->setType(TiledContainer::ContainerBase);
	m_container->setIsActive(true);
	m_container->setScene(scene);

	if (group->hasProperty(QStringLiteral("pickable"))) {
		QVector<RpgPickableObject::PickableType> pickableList;

		const QStringList &pList = group->property(QStringLiteral("pickable")).toString().split(',', Qt::SkipEmptyParts);
		for (const QString &s : pList) {
			const RpgPickableObject::PickableType &type = RpgPickableObject::typeFromString(s.simplified());

			if (type == RpgPickableObject::PickableInvalid) {
				LOG_CWARNING("scene") << "Invalid pickable type:" << s << group->id() << group->className() << group->name();
				continue;
			}

			pickableList.append(type);
		}

		m_container->setPickableList(pickableList);
	}


	QObject::connect(game, &RpgGame::controlledPlayerChanged, game, [this](){
		onControlledPlayerChanged();
	});

	QObject::connect(m_container.get(), &TiledContainer::isActiveChanged, game, [this](){
		onActiveChanged();
		updateLayers();
	});


	for (Tiled::Layer *layer : std::as_const(*group)) {
		if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			if (QQuickItem *item = scene->addVisualTileLayer(tl, renderer)) {
				m_tileLayers.append(item);

				item->setProperty("glowColor", QStringLiteral("#FFF59D"));
				item->setProperty("glowEnabled", false);

				LOG_CTRACE("game") << "Add tile layer" << tl->name() << "to control group:" << this;
			}
		} else if (Tiled::ObjectGroup *group = layer->asObjectGroup()) {
			for (Tiled::MapObject *object : std::as_const(group->objects())) {
				TiledObjectBase *base = nullptr;

				if (object->className() != QStringLiteral("trigger")) {
					LOG_CWARNING("game") << "RpgControlGroupContainer object skipped:" << object->id() << object->name();
					continue;
				}

				if (object->shape() == Tiled::MapObject::Polygon ||
						object->shape() == Tiled::MapObject::Rectangle) {
					TiledObjectBasePolygon *ptr = nullptr;
					TiledObject::createFromMapObject<TiledObjectBasePolygon>(&ptr, object, renderer, scene);
					connectFixture(ptr->fixture());
					base = ptr;
					m_container->setCenterPoint(ptr->screenPolygon().boundingRect().center());
				} else if (object->shape() == Tiled::MapObject::Point) {
					TiledObjectBaseCircle *ptr = nullptr;
					TiledObject::createFromCircle<TiledObjectBaseCircle>(&ptr, object->position(), 70., renderer, scene);
					connectFixture(ptr->fixture());
					base = ptr;

					m_container->setCenterPoint(renderer ?
													renderer->pixelToScreenCoords(object->position()) :
													object->position()
													);
				}

				if (!base) {
					LOG_CERROR("game") << "Invalid object" << object->id() << object->name() << "in" << group->id() << group->name();
					return;
				}

				base->setParent(m_game);
				base->setScene(scene);
				base->setProperty("tiledContainer", QVariant::fromValue(m_container.get()));
			}

		}
	}
}



/**
 * @brief RpgControlGroupContainer::onFixtureBeginContact
 * @param other
 */

void RpgControlGroupContainer::onFixtureBeginContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!base || !g)
		return;

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureVirtualCircle))) {
		if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(base)) {
			if (player == g->controlledPlayer() && !m_contactedFixtures.contains(other)) {
				m_contactedFixtures.append(QPointer(other));
				updateLayers();
			}
		}
	}
}


/**
 * @brief RpgControlGroupContainer::onFixtureEndContact
 * @param other
 */

void RpgControlGroupContainer::onFixtureEndContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!base || !g)
		return;

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureVirtualCircle))) {
		if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(base)) {
			if (player == g->controlledPlayer()) {
				m_contactedFixtures.removeAll(QPointer(other));
				updateLayers();
			}
		}
	}
}


/**
 * @brief RpgControlGroupContainer::onControlledPlayerChanged
 */

void RpgControlGroupContainer::onControlledPlayerChanged()
{
	m_contactedFixtures.clear();
	updateLayers();
}



/**
 * @brief RpgControlGroupContainer::connectFixture
 * @param fixture
 */

void RpgControlGroupContainer::connectFixture(Box2DFixture *fixture)
{
	fixture->setSensor(true);
	fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer));
	fixture->setCollidesWith(Box2DFixture::All);

	QObject::connect(fixture, &Box2DFixture::beginContact, m_game, [this](Box2DFixture *other){
		this->onFixtureBeginContact(other);
	});
	QObject::connect(fixture, &Box2DFixture::endContact, m_game, [this](Box2DFixture *other){
		this->onFixtureEndContact(other);
	});

	m_containerFixtures.append(fixture);
}



/**
 * @brief RpgControlGroupContainer::updateLayers
 */

void RpgControlGroupContainer::updateLayers()
{
	const bool visible = m_container->isActive();
	const bool glow = !m_contactedFixtures.isEmpty();

	for (QQuickItem *item : std::as_const(m_tileLayers)) {
		item->setVisible(visible);
		item->setProperty("glowEnabled", glow);
	}
}



/**
 * @brief RpgControlGroupContainer::onActiveChanged
 */

void RpgControlGroupContainer::onActiveChanged()
{
	for (Box2DFixture *f : m_containerFixtures) {
		if (!f)
			continue;

		TiledObjectBase *object = TiledObjectBase::getFromFixture(f);
		TiledObjectBody *body = object ? object->body() : nullptr;

		if (m_container->isActive()) {
			f->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer));
			f->setCollidesWith(Box2DFixture::All);
		} else {
			f->setCategories(Box2DFixture::None);
			f->setCollidesWith(Box2DFixture::None);
		}

		if (body)
			body->setActive(m_container->isActive());
	}
}


/**
 * @brief RpgChestContainer::RpgChestContainer
 * @param parent
 */

RpgChestContainer::RpgChestContainer(QObject *parent)
	: TiledContainer(parent)
{

}


/**
 * @brief RpgChestContainer::pickableList
 * @return
 */

QVector<RpgPickableObject::PickableType> RpgChestContainer::pickableList() const
{
	return m_pickableList;
}

void RpgChestContainer::setPickableList(const QVector<RpgPickableObject::PickableType> &newPickableList)
{
	m_pickableList = newPickableList;
}

QPointF RpgChestContainer::centerPoint() const
{
	return m_centerPoint;
}

void RpgChestContainer::setCenterPoint(QPointF newCenterPoint)
{
	m_centerPoint = newCenterPoint;
}
