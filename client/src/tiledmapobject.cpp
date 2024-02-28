/*
 * ---- Call of Suli ----
 *
 * tiledmapobject.cpp
 *
 * Created on: 2024. 02. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledMapObject
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

#include "tiledmapobject.h"
#include "application.h"
#include "Logger.h"
#include "box2dfixture.h"
#include <libtiled/maprenderer.h>



/**
 * @brief TiledMapObject::TiledMapObject
 * @param parent
 */

TiledMapObject::TiledMapObject(QQuickItem *parent)
	: QQuickItem(parent)
	, m_body(new Box2DBody)
{
	LOG_CTRACE("scene") << "TiledMapObject created" << this;

	m_body->setBodyType(Box2DBody::Static);
	m_body->setTarget(this);
	m_body->setActive(true);
	m_body->setSleepingAllowed(true);
}


/**
 * @brief TiledMapObject::~TiledMapObject
 */

TiledMapObject::~TiledMapObject()
{
	LOG_CTRACE("scene") << "TiledMapObject destroyed" << this;
}



/**
 * @brief TiledMapObject::createFromMapObject
 * @param object
 * @return
 */

TiledMapObject *TiledMapObject::createFromMapObject(const Tiled::MapObject *object, Tiled::MapRenderer *renderer, QQuickItem *parent)
{
	if (!object) {
		LOG_CERROR("scene") << "Empty Tiled::MapObject";
		return nullptr;
	}

	if (!renderer) {
		LOG_CERROR("scene") << "Missing Tiled::MapRenderer";
		return nullptr;
	}

	switch (object->shape()) {
		case Tiled::MapObject::Rectangle:
			return createFromPolygon(object->bounds(), renderer, parent);
		case Tiled::MapObject::Polygon:
			return createFromPolygon(object->polygon().translated(object->position()), renderer, parent);
		default:
			LOG_CERROR("scene") << "Invalid Tiled::MapObject shape" << object->shape();
	}

	return nullptr;
}



/**
 * @brief TiledMapObject::createFromPolygon
 * @param polygon
 * @return
 */

TiledMapObject *TiledMapObject::createFromPolygon(const QPolygonF &polygon, Tiled::MapRenderer *renderer, QQuickItem *parent)
{
	if (!renderer) {
		LOG_CERROR("scene") << "Missing Tiled::MapRenderer";
		return nullptr;
	}

	QPolygonF screenPolygon = renderer->pixelToScreenCoords(polygon);

	const QRectF &boundingRect = screenPolygon.boundingRect();

	TiledMapObject *r = new TiledMapObject(parent);
	r->setX(boundingRect.x());
	r->setY(boundingRect.y());
	r->setWidth(boundingRect.width());
	r->setHeight(boundingRect.height());

	screenPolygon.translate(-boundingRect.topLeft());

	QVariantList vert;

	for (const QPointF &f : screenPolygon)
		vert.append(f);

	Box2DPolygon *p = new Box2DPolygon(parent);
	p->setVertices(vert);
	r->m_defaultFixture.reset(p);
	r->body()->addFixture(r->m_defaultFixture.get());
	r->bodyComplete();

	r->m_screenPolygon = screenPolygon;

	return r;
}


/**
 * @brief TiledMapObject::jumpToSprite
 * @param sprite
 */

void TiledMapObject::jumpToSprite(const QString &sprite) const
{
	if (!m_spriteSequence) {
		LOG_CERROR("scene") << "Missing spriteSequence";
		return;
	}

	if (m_spriteSequence->property("currentSprite").toString() == sprite)
		return;

	QMetaObject::invokeMethod(m_spriteSequence, "jumpTo", Qt::DirectConnection,
							  Q_ARG(QString, sprite)
							  );
}


/**
 * @brief TiledMapObject::scene
 * @return
 */

TiledScene *TiledMapObject::scene() const
{
	return m_scene;
}

void TiledMapObject::setScene(TiledScene *newScene)
{
	if (m_scene == newScene)
		return;
	m_scene = newScene;
	emit sceneChanged();

	if (m_scene) {
		if (m_scene->world())
			m_body->setWorld(m_scene->world());
		onSceneConnected();
	}
}


/**
 * @brief TiledMapObject::body
 * @return
 */

Box2DBody*TiledMapObject::body() const
{
	return m_body.get();
}


/**
 * @brief TiledMapObject::loadSprite
 * @param sprite
 * @return
 */

bool TiledMapObject::appendSprite(const QString &path, const TiledMapObjectSprite &sprite)
{
	Q_ASSERT(m_visualItem);

	if (m_availableSprites.contains(sprite.name)) {
		LOG_CERROR("scene") << "Sprite already loaded:" << path << sprite.name;
		return false;
	}

	QVariantMap data = sprite.toJson().toVariantMap();

	data[QStringLiteral("source")] = QUrl::fromLocalFile(path);

	QMetaObject::invokeMethod(m_visualItem, "appendSprite", Qt::DirectConnection,
							  Q_ARG(QVariant, data));

	m_availableSprites.append(sprite.name);

	return true;
}



/**
 * @brief TiledMapObject::appendSpriteList
 * @param path
 * @param spriteList
 * @return
 */

bool TiledMapObject::appendSpriteList(const QString &path, const TiledMapObjectSpriteList &spriteList)
{
	bool r = true;

	const auto it = std::find_if(spriteList.sprites.constBegin(),
								 spriteList.sprites.constEnd(),
								 [](const TiledMapObjectSprite &s) {
		return s.name == QStringLiteral("idle");
	});

	if (it != spriteList.sprites.constEnd())
		r &= appendSprite(path, *it);


	for (const TiledMapObjectSprite &s : spriteList.sprites) {
		if (s.name == QStringLiteral("idle"))
			continue;

		r &= appendSprite(path, s);
	}

	return r;
}


/**
 * @brief TiledMapObject::createVisual
 */

void TiledMapObject::createVisual()
{
	Q_ASSERT(!m_visualItem);

	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledMapObjectVisual.qml"), this);

	LOG_CDEBUG("scene") << "Create sprite item:" << component.isReady();

	m_visualItem = qobject_cast<QQuickItem*>(component.create());

	if (!m_visualItem) {
		LOG_CERROR("scene") << "TiledMapObject createVisual error" << component.errorString();
		return;
	}

	m_spriteSequence = qvariant_cast<QQuickItem*>(m_visualItem->property("spriteSequence"));

	m_visualItem->setParentItem(this);
	m_visualItem->setProperty("baseObject", QVariant::fromValue(this));

	if (!m_spriteSequence) {
		LOG_CERROR("scene") << "TiledMapObject SpriteSequence error";
		return;
	}

	const QMetaObject *mo = m_spriteSequence->metaObject();

	auto p = mo->property(mo->indexOfProperty("currentSprite"));
	auto t = this->metaObject()->indexOfMethod("onCurrentSpriteChanged(QString)");

	if (p.hasNotifySignal()) {
		connect(m_spriteSequence, p.notifySignal(), this, this->metaObject()->method(t));
	}
}

QPolygonF TiledMapObject::screenPolygon() const
{
	return m_screenPolygon.isEmpty() ? m_screenPolygon : m_screenPolygon.translated(position());
}


/**
 * @brief TiledMapObject::defaultFixture
 * @return
 */
Box2DFixture* TiledMapObject::defaultFixture() const
{
	return m_defaultFixture.get();
}


/**
 * @brief TiledMapObject::onCurrentSpriteChanged
 * @param sprite
 */

void TiledMapObject::onCurrentSpriteChanged(QString sprite)
{
	//LOG_CTRACE("scene") << "Current sprite" << sprite;
}
