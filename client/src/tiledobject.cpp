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

#include "tiledobject.h"
#include "application.h"
#include "Logger.h"
#include "box2dfixture.h"
#include "tiledscene.h"
#include <libtiled/maprenderer.h>





/**
 * @brief TiledObject::scene
 * @return
 */

TiledObjectBase::TiledObjectBase(QQuickItem *parent)
	: QQuickItem(parent)
	, m_body(new Box2DBody)
{
	LOG_CTRACE("scene") << "TiledObjectBase created" << this;

	m_body->setBodyType(Box2DBody::Static);
	m_body->setTarget(this);
	m_body->setActive(true);
	m_body->setSleepingAllowed(true);
}


/**
 * @brief TiledObjectBase::~TiledObjectBase
 */

TiledObjectBase::~TiledObjectBase()
{
	LOG_CTRACE("scene") << "TiledObject destroyed" << this;
}



/**
 * @brief TiledObjectBase::scene
 * @return
 */

TiledScene *TiledObjectBase::scene() const
{
	return m_scene;
}

void TiledObjectBase::setScene(TiledScene *newScene)
{
	if (m_scene == newScene)
		return;
	m_scene = newScene;
	emit sceneChanged();

	if (m_scene) {
		setParentItem(m_scene);
		if (m_scene->world())
			m_body->setWorld(m_scene->world());
		onSceneConnected();
	}
}


/**
 * @brief TiledObject::body
 * @return
 */

Box2DBody *TiledObjectBase::body() const
{
	return m_body.get();
}



/**
 * @brief TiledObject::defaultFixture
 * @return
 */
Box2DFixture* TiledObjectBase::defaultFixture() const
{
	return m_defaultFixture.get();
}




/**
 * @brief TiledObject::TiledObject
 * @param parent
 */

TiledObject::TiledObject(QQuickItem *parent)
	: TiledObjectBase(parent)
{

}




/**
 * @brief TiledObject::createFromMapObject
 * @param object
 * @return
 */

TiledObject *TiledObject::createFromMapObject(const Tiled::MapObject *object, Tiled::MapRenderer *renderer, QQuickItem *parent)
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
 * @brief TiledObject::createFromPolygon
 * @param polygon
 * @return
 */

TiledObject *TiledObject::createFromPolygon(const QPolygonF &polygon, Tiled::MapRenderer *renderer, QQuickItem *parent)
{
	if (!renderer) {
		LOG_CERROR("scene") << "Missing Tiled::MapRenderer";
		return nullptr;
	}

	QPolygonF screenPolygon = renderer->pixelToScreenCoords(polygon);

	const QRectF &boundingRect = screenPolygon.boundingRect();

	TiledObject *r = new TiledObject(parent);
	r->setX(boundingRect.x());
	r->setY(boundingRect.y());
	r->setWidth(boundingRect.width());
	r->setHeight(boundingRect.height());

	screenPolygon.translate(-boundingRect.topLeft());

	QVariantList vert;

	for (const QPointF &f : screenPolygon)
		vert.append(f);

	Box2DPolygon *p = new Box2DPolygon();
	p->setVertices(vert);
	r->m_defaultFixture.reset(p);
	r->body()->addFixture(r->m_defaultFixture.get());
	r->bodyComplete();

	r->m_screenPolygon = screenPolygon;

	return r;
}


/**
 * @brief TiledObject::toPolygonF
 * @param object
 * @param renderer
 * @return
 */

QPolygonF TiledObject::toPolygonF(const Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	if (!object) {
		LOG_CERROR("scene") << "Empty Tiled::MapObject";
		return {};
	}

	if (!renderer) {
		LOG_CERROR("scene") << "Missing Tiled::MapRenderer";
		return {};
	}

	const QPolygonF &polygon = object->polygon().translated(object->position());
	return renderer->pixelToScreenCoords(polygon);
}




/**
 * @brief TiledObject::jumpToSprite
 * @param sprite
 */

void TiledObject::jumpToSprite(const QString &sprite) const
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
 * @brief TiledObject::loadSprite
 * @param sprite
 * @return
 */

bool TiledObject::appendSprite(const QString &source, const TiledObjectSprite &sprite)
{
	Q_ASSERT(m_visualItem);

	if (m_availableSprites.contains(getSpriteName(sprite.name))) {
		LOG_CERROR("scene") << "Sprite already loaded:" << source << sprite.name;
		return false;
	}

	QVariantMap data = sprite.toJson().toVariantMap();

	data[QStringLiteral("source")] = QUrl::fromLocalFile(source);

	QMetaObject::invokeMethod(m_visualItem, "appendSprite", Qt::DirectConnection,
							  Q_ARG(QVariant, data));

	m_availableSprites.append(sprite.name);

	return true;
}



/**
 * @brief TiledObject::appendSpriteList
 * @param source
 * @param spriteList
 * @return
 */

bool TiledObject::appendSpriteList(const QString &source, const TiledObjectSpriteList &spriteList)
{
	bool r = true;

	const auto it = std::find_if(spriteList.sprites.constBegin(),
								 spriteList.sprites.constEnd(),
								 [](const TiledObjectSprite &s) {
		return s.name == QStringLiteral("default");
	});

	if (it != spriteList.sprites.constEnd())
		r &= appendSprite(source, *it);


	for (const TiledObjectSprite &s : spriteList.sprites) {
		if (s.name == QStringLiteral("default"))
			continue;

		r &= appendSprite(source, s);
	}

	return r;
}



/**
 * @brief TiledObject::appendSprite
 * @param path
 * @param sprite
 * @return
 */

bool TiledObject::appendSprite(const TiledMapObjectAlterableSprite &sprite, const QString &path)
{
	Q_ASSERT(m_visualItem);

	for (const TiledObjectSprite &s : sprite.sprites) {
		for (auto it = sprite.alterations.constBegin(); it != sprite.alterations.constEnd(); ++it) {
			const QString &alteration = it.key();
			const QString &spriteName = getSpriteName(s.name, alteration);

			if (m_availableSprites.contains(spriteName)) {
				LOG_CERROR("scene") << "Sprite already loaded:" << s.name << alteration << path << it.value();
				return false;
			}

			TiledObjectSprite s2 = s;
			s2.name = spriteName;
			QJsonObject to2;
			for (auto it = s.to.constBegin(); it != s.to.constEnd(); ++it) {
				to2.insert(getSpriteName(it.key(), alteration), it.value());
			}
			s2.to = to2;

			QVariantMap data = s2.toJson().toVariantMap();

			data[QStringLiteral("source")] = QUrl::fromLocalFile(path+it.value());

			QMetaObject::invokeMethod(m_visualItem, "appendSprite", Qt::DirectConnection,
									  Q_ARG(QVariant, data));

			LOG_CTRACE("scene") << "Append sprite" << data;

			m_availableSprites.append(spriteName);
		}
	}

	m_availableAlterations.append(sprite.alterations.keys());

	return true;
}


/**
 * @brief TiledObject::appendSpriteList
 * @param path
 * @param spriteList
 * @return
 */

bool TiledObject::appendSpriteList(const TiledObjectAlterableSpriteList &spriteList, const QString &path)
{
	Q_ASSERT(m_visualItem);

	for (const auto &s : spriteList.list) {
		if (!appendSprite(s, path)) {
			LOG_CERROR("scene") << "Load sprite error:" << path;
			return false;
		}
	}

	return true;
}


/**
 * @brief TiledObject::getSpriteName
 * @param sprite
 * @param alteration
 * @return
 */

QString TiledObject::getSpriteName(const QString &sprite, const QString &alteration)
{
	if (alteration.isEmpty())
		return sprite;
	else
		return alteration+QStringLiteral("-").append(sprite);
}


/**
 * @brief TiledObject::createVisual
 */

void TiledObject::createVisual()
{
	Q_ASSERT(!m_visualItem);

	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledObjectVisual.qml"), this);

	LOG_CDEBUG("scene") << "Create sprite item:" << component.isReady();

	m_visualItem = qobject_cast<QQuickItem*>(component.create());

	if (!m_visualItem) {
		LOG_CERROR("scene") << "TiledObject createVisual error" << component.errorString();
		return;
	}

	m_spriteSequence = qvariant_cast<QQuickItem*>(m_visualItem->property("spriteSequence"));

	m_visualItem->setParentItem(this);
	m_visualItem->setProperty("baseObject", QVariant::fromValue(this));

	if (!m_spriteSequence) {
		LOG_CERROR("scene") << "TiledObject SpriteSequence error";
		return;
	}

	const QMetaObject *mo = m_spriteSequence->metaObject();

	auto p = mo->property(mo->indexOfProperty("currentSprite"));
	auto t = this->metaObject()->indexOfMethod("onCurrentSpriteChanged(QString)");

	if (p.hasNotifySignal()) {
		connect(m_spriteSequence, p.notifySignal(), this, this->metaObject()->method(t));
	}
}

QStringList TiledObject::availableAlterations() const
{
	return m_availableAlterations;
}

QStringList TiledObject::availableSprites() const
{
	return m_availableSprites;
}

QPolygonF TiledObject::screenPolygon() const
{
	return m_screenPolygon.isEmpty() ? m_screenPolygon : m_screenPolygon.translated(position());
}


/**
 * @brief TiledObject::onCurrentSpriteChanged
 * @param sprite
 */

void TiledObject::onCurrentSpriteChanged(QString sprite)
{
	//LOG_CTRACE("scene") << "Current sprite" << sprite;
}
