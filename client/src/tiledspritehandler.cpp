/*
 * ---- Call of Suli ----
 *
 * tiledspritehandler.cpp
 *
 * Created on: 2024. 03. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledSpriteHandler
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

#include "tiledspritehandler.h"
#include "tiledscene.h"
#include "tiledgame.h"
#include <QSGSimpleTextureNode>



/**
 * @brief TiledSpriteHandler::TiledSpriteHandler
 * @param parent
 */

TiledSpriteHandler::TiledSpriteHandler(QQuickItem *parent)
	: QQuickItem(parent)
{
	setFlag(ItemHasContents);
	//setZ(position.y() * parent->map().mMap->tileHeight());
}


/**
 * @brief TiledSpriteHandler::~TiledSpriteHandler
 */

TiledSpriteHandler::~TiledSpriteHandler()
{
	m_timer.stop();
	LOG_CTRACE("scene") << "TiledSpriteHandler destroyed" << this;
}


QSGNode *TiledSpriteHandler::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
	if (!node) {
		node = new QSGNode;
	}

	bool isActive = m_baseObject && m_baseObject->inVisibleArea();

	if (m_spriteList.isEmpty()) {
		///LOG_CERROR("scene") << "Empty sprite list";
		isActive = false;
	}

	const auto &list = find(m_currentSprite, m_currentDirection);

	if (list.isEmpty()) {
		///LOG_CERROR("scene") << "Sprite not found:" << m_currentSprite << m_currentDirection;
		isActive = false;
	}

	if (node->childCount() > 0)
		node->removeAllChildNodes();

	if (!isActive)
		return node;


	for (const QString &layer : std::as_const(m_visibleLayers)) {
		for (const auto &it : std::as_const(list)) {
			if (it->layer != layer)
				continue;

			if (!it->texture) {
				LOG_CERROR("scene") << "Invalid texture" << it->data.name << it->direction << it->layer;
				continue;
			}

			QSGSimpleTextureNode *imgNode = new QSGSimpleTextureNode();
			imgNode->setOwnsTexture(false);
			imgNode->setFlag(QSGNode::OwnedByParent);
			node->appendChildNode(imgNode);

			imgNode->setRect(boundingRect());

			if (it->texture != imgNode->texture())
				imgNode->setTexture(it->texture);

			QRectF rect(it->data.x,
						it->data.y,
						it->data.width,
						it->data.height);

			rect.translate(m_currentFrame * it->data.width, 0);

			imgNode->setSourceRect(rect);

			imgNode->markDirty(QSGNode::DirtyGeometry);
		}
	}

	return node;
}




/**
 * @brief TiledSpriteHandler::addSprite
 * @param sprite
 * @param layer
 * @param source
 * @return
 */

bool TiledSpriteHandler::addSprite(const TiledObjectSprite &sprite, const QString &layer, const QString &source)
{
	if (exists(sprite.name, layer)) {
		LOG_CERROR("scene") << "Sprite already loaded:" << sprite.name << source << layer;
		return false;
	}

	if (source.isEmpty()) {
		LOG_CERROR("scene") << "Empty sprite source:" << sprite.name;
		return false;
	}

	if (!createSpriteItem(sprite, source, layer))
		return false;

	if (!m_spriteNames.contains(sprite.name))
		m_spriteNames.append(sprite.name);

	return true;
}



/**
 * @brief TiledSpriteHandler::addSprite
 * @param sprite
 * @param layer
 * @param direction
 * @param source
 * @return
 */

bool TiledSpriteHandler::addSprite(const TiledObjectSprite &sprite, const QString &layer,
								   const TiledObject::Direction &direction, const QString &source)
{
	if (exists(sprite.name, layer, direction)) {
		LOG_CERROR("scene") << "Sprite already loaded:" << sprite.name << source << layer << direction;
		return false;
	}

	if (source.isEmpty()) {
		LOG_CERROR("scene") << "Empty sprite source:" << sprite.name;
		return false;
	}

	if (!createSpriteItem(sprite, source, layer, direction))
		return false;

	if (!m_spriteNames.contains(sprite.name))
		m_spriteNames.append(sprite.name);

	return true;
}


/**
 * @brief TiledSpriteHandler::jumpToSprite
 * @param name
 * @param alteration
 * @param direction
 * @param mode
 * @return
 */

bool TiledSpriteHandler::jumpToSprite(const QString &name,
									  const TiledObject::Direction &direction,
									  const JumpMode &mode)
{
	const auto &ptr = findFirst(name, direction);

	if (!ptr) {
#ifndef QT_NO_DEBUG
		LOG_CWARNING("scene") << "Sprite not found" << name << direction;
#endif
		return false;
	}

	if (mode == JumpImmediate) {
		m_jumpToSprite = Sprite{};
		changeSprite(name, direction);
	} else {
		m_jumpToSprite = *(ptr.value());
	}

	return true;
}


/**
 * @brief TiledSpriteHandler::spriteNames
 * @return
 */

const QStringList &TiledSpriteHandler::spriteNames() const
{
	return m_spriteNames;
}



/**
 * @brief TiledSpriteHandler::createSpriteItem
 * @param sprite
 * @return
 */

bool TiledSpriteHandler::createSpriteItem(const TiledObjectSprite &sprite,
										  const QString &source,
										  const QString &layer,
										  const TiledObject::Direction &direction)
{
	if (!m_baseObject || !m_baseObject->scene() || !m_baseObject->scene()->game()) {
		LOG_CERROR("scene") << "Missing game";
		return false;
	}

	QString path = source;
	if (path.startsWith(QStringLiteral("qrc:/")))
		path.replace(QStringLiteral("qrc:/"), QStringLiteral(":/"));

	Sprite s;
	s.layer = layer;
	s.direction = direction;
	s.data = sprite;
	s.texture = m_baseObject->scene()->game()->getTexture(path, window());

	if (!s.texture) {
		LOG_CERROR("scene") << "Invalid texture";
		return false;
	}

	///LOG_CTRACE("scene") << "Sprite created:" << s.data.name << s.layer << s.direction;

	m_spriteList.append(s);

	return true;
}


/**
 * @brief TiledSpriteHandler::find
 * @param baseName
 * @param direction
 * @return
 */


QList<QVector<TiledSpriteHandler::Sprite>::const_iterator> TiledSpriteHandler::find(const QString &baseName,
																					const TiledObject::Direction &direction) const
{
	QList<QVector<Sprite>::const_iterator> list;

	for (auto it = m_spriteList.constBegin(); it != m_spriteList.constEnd(); ++it) {
		if (it->data.name == baseName && it->direction == direction)
			list.append(it);
	}

	return list;
}


/**
 * @brief TiledSpriteHandler::findFirst
 * @param baseName
 * @param direction
 * @return
 */

std::optional<QVector<TiledSpriteHandler::Sprite>::const_iterator>
TiledSpriteHandler::findFirst(const QString &baseName, const TiledObject::Direction &direction) const
{
	for (auto it = m_spriteList.constBegin(); it != m_spriteList.constEnd(); ++it) {
		if (it->data.name == baseName && it->direction == direction)
			return it;
	}

	return std::nullopt;
}


/**
 * @brief TiledSpriteHandler::exists
 * @param baseName
 * @param direction
 * @return
 */

bool TiledSpriteHandler::exists(const QString &baseName, const TiledObject::Direction &direction) const
{
	for (const Sprite &s : std::as_const(m_spriteList)) {
		if (s.data.name == baseName && s.direction == direction)
			return true;
	}

	return false;
}


/**
 * @brief TiledSpriteHandler::exists
 * @param baseName
 * @param alteration
 * @param direction
 * @return
 */

bool TiledSpriteHandler::exists(const QString &baseName, const QString &layer, const TiledObject::Direction &direction) const
{
	for (const Sprite &s : std::as_const(m_spriteList)) {
		if (s.data.name == baseName && s.direction == direction && s.layer == layer)
			return true;
	}

	return false;
}





/**
 * @brief TiledSpriteHandler::changeSprite
 * @param id
 */

void TiledSpriteHandler::changeSprite(const QString &name, const TiledObject::Direction &direction)
{

	if (m_currentSprite == name && m_currentDirection == direction)
		return;

	const auto &ptr = findFirst(name, direction);

	if (!ptr) {
		LOG_CERROR("scene") << "Sprite not found:" << name << direction;
		m_timer.stop();
		return;
	}

	m_currentDirection = direction;
	setCurrentSprite(name);
	m_currentFrame = 0;

	m_timer.start(ptr.value()->data.duration, Qt::PreciseTimer, this);
	update();
}



/**
 * @brief TiledSpriteHandler::clear
 */

void TiledSpriteHandler::clear()
{
	m_timer.stop();
	m_spriteList.clear();
	m_spriteNames.clear();
	m_currentSprite.clear();
	m_currentDirection = TiledObject::Invalid;
	m_layers.clear();
	m_visibleLayers = { QStringLiteral("default") };
}



/**
 * @brief TiledSpriteHandler::clearAtEnd
 * @return
 */

bool TiledSpriteHandler::clearAtEnd() const
{
	return m_clearAtEnd;
}

void TiledSpriteHandler::setClearAtEnd(bool newClearAtEnd)
{
	if (m_clearAtEnd == newClearAtEnd)
		return;
	m_clearAtEnd = newClearAtEnd;
	emit clearAtEndChanged();
}



/**
 * @brief TiledSpriteHandler::visibleLayers
 * @return
 */

const QStringList &TiledSpriteHandler::visibleLayers() const
{
	return m_visibleLayers;
}

void TiledSpriteHandler::setVisibleLayers(const QStringList &newVisibleLayers)
{
	m_visibleLayers = newVisibleLayers;
}


/**
 * @brief TiledSpriteHandler::layers
 * @return
 */

const QStringList &TiledSpriteHandler::layers() const
{
	return m_layers;
}



/**
 * @brief TiledSpriteHandler::baseObject
 * @return
 */

TiledObject *TiledSpriteHandler::baseObject() const
{
	return m_baseObject;
}

void TiledSpriteHandler::setBaseObject(TiledObject *newBaseObject)
{
	if (m_baseObject == newBaseObject)
		return;

	if (m_baseObject)
		disconnect(m_baseObject, &TiledObject::inVisibleAreaChanged, this, &TiledSpriteHandler::update);

	m_baseObject = newBaseObject;
	emit baseObjectChanged();

	if (m_baseObject)
		connect(m_baseObject, &TiledObject::inVisibleAreaChanged, this, &TiledSpriteHandler::update);
}



/**
 * @brief TiledSpriteHandler::timerEvent
 * @param event
 */

void TiledSpriteHandler::timerEvent(QTimerEvent *)
{
	const auto &ptr = findFirst(m_currentSprite, m_currentDirection);

	if (!ptr) {
		LOG_CERROR("scene") << "Sprite not found:" << m_currentSprite << m_currentDirection;
		m_timer.stop();
		return;
	}


	// Todo: loop counting (?)

	int nextFrame = m_currentFrame+1;

	if (nextFrame >= ptr.value()->data.count) {
		if (!m_jumpToSprite.data.name.isEmpty()) {
			changeSprite(m_jumpToSprite.data.name, m_jumpToSprite.direction);
			m_jumpToSprite = Sprite{};
			return;
		}

		if (ptr.value()->data.loops <= 0)
			nextFrame = 0;
		else {
			if (m_clearAtEnd) {
				clear();
				return update();
			}

			return m_timer.stop();
		}
	}

	m_currentFrame = nextFrame;
	update();
}


/**
 * @brief TiledSpriteHandler::currentSprite
 * @return
 */

const QString &TiledSpriteHandler::currentSprite() const
{
	return m_currentSprite;
}

void TiledSpriteHandler::setCurrentSprite(const QString &newCurrentSprite)
{
	if (m_currentSprite == newCurrentSprite)
		return;
	m_currentSprite = newCurrentSprite;
	emit currentSpriteChanged();
}


