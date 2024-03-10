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
#include "application.h"
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


QSGNode *TiledSpriteHandler::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
	if (!node) {
		node = new QSGNode;
	}

	bool isActive = m_baseObject && m_baseObject->inVisibleArea();

	if (m_spriteList.isEmpty()) {
		LOG_CERROR("scene") << "Empty sprite list";
		isActive = false;
	}

	auto it = m_spriteList.find(m_currentId);

	if (it == m_spriteList.end()) {
		LOG_CERROR("scene") << "Sprite id not found:" << m_currentId;
		isActive = false;
	} else if (!it->texture()) {
		LOG_CERROR("scene") << "Sprite texture not found:" << m_currentId;
		isActive = false;
	}

	if (!isActive) {
		if (node->childCount() > 0) {
			node->removeAllChildNodes();
		}

		return node;
	}


	QSGSimpleTextureNode *imgNode = nullptr;

	if (node->childCount() > 0) {
		imgNode = dynamic_cast<QSGSimpleTextureNode*>(node->firstChild());
	} else {
		imgNode = new QSGSimpleTextureNode();
		imgNode->setOwnsTexture(false);
		imgNode->setFlag(QSGNode::OwnedByParent);
		node->appendChildNode(imgNode);
	}

	if (!imgNode) {
		LOG_CERROR("scene") << "Invalid texture node";
		return node;
	}

	imgNode->setRect(boundingRect());

	bool textureModified = false;

	if (it->texture() != imgNode->texture()) {
		imgNode->setTexture(it->texture());
		textureModified = true;
	}

	QRectF rect(it->data.x,
				it->data.y,
				it->data.width,
				it->data.height);

	rect.translate(m_currentFrame * it->data.width, 0);

	imgNode->setSourceRect(rect);

	imgNode->markDirty(textureModified ? QSGNode::DirtyMaterial|QSGNode::DirtyGeometry : QSGNode::DirtyGeometry);

	return node;
}


/**
 * @brief TiledSpriteHandler::addSprite
 * @param sprite
 */

bool TiledSpriteHandler::addSprite(const TiledObjectSprite &sprite, const QString &source)
{
	if (find(sprite.name) != -1) {
		LOG_CERROR("scene") << "Sprite already loaded:" << sprite.name << source;
		return false;
	}

	if (source.isEmpty()) {
		LOG_CERROR("scene") << "Empty sprite source:" << sprite.name;
		return false;
	}

	if (!createSpriteItem(sprite, source))
		return false;

	if (!m_spriteNames.contains(sprite.name))
		m_spriteNames.append(sprite.name);

	return true;
}



/**
 * @brief TiledSpriteHandler::addSprite
 * @param sprite
 * @param alteration
 * @param source
 * @return
 */

bool TiledSpriteHandler::addSprite(const TiledObjectSprite &sprite, const QString &alteration, const QString &source)
{
	if (find(sprite.name, alteration) != -1) {
		LOG_CERROR("scene") << "Sprite already loaded:" << sprite.name << source << alteration;
		return false;
	}

	if (source.isEmpty()) {
		LOG_CERROR("scene") << "Empty sprite source:" << sprite.name;
		return false;
	}

	if (!createSpriteItem(sprite, source, alteration))
		return false;

	if (!m_spriteNames.contains(sprite.name))
		m_spriteNames.append(sprite.name);

	return true;
}


/**
 * @brief TiledSpriteHandler::addSprite
 * @param sprite
 * @param direction
 * @param source
 * @return
 */

bool TiledSpriteHandler::addSprite(const TiledObjectSprite &sprite, const TiledObject::Direction &direction, const QString &source)
{
	if (find(sprite.name, QStringLiteral(""), direction) != -1) {
		LOG_CERROR("scene") << "Sprite already loaded:" << sprite.name << source << direction;
		return false;
	}

	if (source.isEmpty()) {
		LOG_CERROR("scene") << "Empty sprite source:" << sprite.name;
		return false;
	}

	if (!createSpriteItem(sprite, source, QStringLiteral(""), direction))
		return false;

	if (!m_spriteNames.contains(sprite.name))
		m_spriteNames.append(sprite.name);

	return true;
}


/**
 * @brief TiledSpriteHandler::addSprite
 * @param sprite
 * @param alteration
 * @param direction
 * @param source
 * @return
 */

bool TiledSpriteHandler::addSprite(const TiledObjectSprite &sprite, const QString &alteration,
								   const TiledObject::Direction &direction, const QString &source)
{
	if (find(sprite.name, alteration, direction) != -1) {
		LOG_CERROR("scene") << "Sprite already loaded:" << sprite.name << source << alteration << direction;
		return false;
	}

	if (source.isEmpty()) {
		LOG_CERROR("scene") << "Empty sprite source:" << sprite.name;
		return false;
	}

	if (!createSpriteItem(sprite, source, alteration, direction))
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
									  const QString &alteration,
									  const TiledObject::Direction &direction,
									  const JumpMode &mode)
{
	const int id = find(name, alteration, direction);

	if (id == -1) {
#ifndef QT_NO_DEBUG
		LOG_CWARNING("scene") << "Sprite not found" << name << alteration << direction;
#endif
		return false;
	}

	if (mode == JumpImmediate) {
		m_jumpToId = -1;
		changeSprite(id);
	} else {
		if (m_currentId >= 0 && m_currentId < m_spriteList.size()) {
			m_jumpToId = id;
		} else {
			LOG_CERROR("scene") << "Current sprite not found" << m_currentId;
			m_jumpToId = -1;
			changeSprite(id);
		}
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
										  const QString &alteration,
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
	s.alteration = alteration;
	s.direction = direction;
	s.data = sprite;
	s.shr_texture = m_baseObject->scene()->game()->getTexture(path);

	if (!s.shr_texture.get()) {
		LOG_CERROR("scene") << "Invalid texture";
		return false;
	}

	m_spriteList.insert(++m_lastId, s);

	////LOG_CTRACE("scene") << "Sprite created:" << m_lastId << s.data.name << s.alteration << s.direction;

	return true;
}



/**
 * @brief TiledSpriteHandler::find
 * @param baseName
 * @param alteration
 * @param direction
 * @return
 */

int TiledSpriteHandler::find(const QString &baseName,
							 const QString &alteration,
							 const TiledObject::Direction &direction) const
{
	auto it = std::find_if(m_spriteList.constBegin(), m_spriteList.constEnd(),
						   [alteration, baseName, direction](const Sprite &s) {
		return s.data.name == baseName && s.alteration == alteration && s.direction == direction;
	});

	if (it == m_spriteList.constEnd())
		return -1;

	return it.key();
}


/**
 * @brief TiledSpriteHandler::findSprite
 * @param baseName
 * @param alteration
 * @param direction
 * @return
 */

std::optional<TiledSpriteHandler::Sprite> TiledSpriteHandler::findSprite(const QString &baseName, const QString &alteration, const TiledObject::Direction &direction) const
{
	const int id = find(baseName, alteration, direction);
	if (id == -1)
		return std::nullopt;
	else
		return m_spriteList.value(id);
}


/**
 * @brief TiledSpriteHandler::changeSprite
 * @param id
 */

void TiledSpriteHandler::changeSprite(const int &id)
{
	if (m_currentId == id)
		return;

	auto it = m_spriteList.find(id);

	if (it == m_spriteList.end()) {
		LOG_CERROR("scene") << "Sprite id not found:" << id;
		m_timer.stop();
		return;
	}

	m_currentId = id;
	setCurrentSprite(it->data.name);

	m_currentFrame = 0;
	m_timer.start(it->data.duration, Qt::PreciseTimer, this);
	update();
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
	m_baseObject = newBaseObject;
	emit baseObjectChanged();
}



/**
 * @brief TiledSpriteHandler::timerEvent
 * @param event
 */

void TiledSpriteHandler::timerEvent(QTimerEvent *)
{
	auto it = m_spriteList.find(m_currentId);

	if (it == m_spriteList.end()) {
		LOG_CERROR("scene") << "Sprite id not found:" << m_currentId;
		return;
	}


	// Todo: loop counting (?)

	int nextFrame = m_currentFrame+1;

	if (nextFrame >= it->data.count) {
		if (m_jumpToId != -1) {
			changeSprite(m_jumpToId);
			m_jumpToId = -1;
			return;
		}

		if (it->data.loops <= 0)
			nextFrame = 0;
		else
			return m_timer.stop();
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


