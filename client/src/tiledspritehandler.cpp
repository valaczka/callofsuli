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
#include "qrandom.h"
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

	if (m_handlerMaster && m_handlerMaster->syncHandlers()) {
		setCurrentSprite(m_handlerMaster->m_currentSprite);
		m_currentDirection = m_handlerMaster->m_currentDirection;
		m_currentFrame = m_handlerMaster->m_currentFrame;
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


	// Ordering layers

	static const QHash<TiledObject::Direction, QVector<Filter>> filterOrder = {
		{ TiledObject::Invalid, { FilterNone } },
		{ TiledObject::NorthEast, { FilterDefault, FilterOther, FilterShield } },
		{ TiledObject::East, { FilterDefault, FilterShield, FilterOther } },
		{ TiledObject::SouthEast, { FilterDefault, FilterShield, FilterOther } },
		{ TiledObject::South, { FilterDefault, FilterShield, FilterOther } },
		{ TiledObject::SouthWest, { FilterDefault, FilterOther, FilterShield } },
		{ TiledObject::West, { FilterDefault, FilterOther, FilterShield } },
		{ TiledObject::NorthWest, { FilterDefault, FilterOther, FilterShield } },
		{ TiledObject::North, { FilterDefault, FilterOther, FilterShield } },
	};

	const auto &filter = filterOrder.value(m_currentDirection);

	for (const Filter &f : filter) {
		createNodes(node, f, list);
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

bool TiledSpriteHandler::addSprite(const TextureSprite &sprite, const QString &layer, const QString &source)
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

bool TiledSpriteHandler::addSprite(const TextureSprite &sprite, const QString &layer,
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

bool TiledSpriteHandler::createSpriteItem(const TextureSprite &sprite,
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

	if (!m_layers.contains(layer))
		m_layers.append(layer);

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

	if (m_startFrameSeed < 0. || m_startFrameSeed >= 1.) {
		LOG_CERROR("scene")	<< "Invalid frame seed" << m_startFrameSeed;
		m_currentFrame = 0;
	} else {
		m_currentFrame = std::floor((qreal) ptr.value()->data.frames.size() * m_startFrameSeed);
	}

	m_isReverse = false;

	m_timer.start(ptr.value()->data.duration, Qt::PreciseTimer, this);
	update();

	if (m_handlerSlave && m_syncHandlers)
		m_handlerSlave->update();
}



/**
 * @brief TiledSpriteHandler::createNodes
 */

void TiledSpriteHandler::createNodes(QSGNode *node, const Filter &filter,
									 const QList<QVector<TiledSpriteHandler::Sprite>::const_iterator> &iteratorList)
{
	Q_ASSERT(node);

	for (const QString &layer : m_visibleLayers) {
		// Filter layers

		switch (filter) {
			case FilterDefault:
				if (layer != QStringLiteral("default"))
					continue;
				break;

			case FilterShield:
				if (layer != QStringLiteral("shield"))
					continue;
				break;

			case FilterOther:
				if (layer == QStringLiteral("default") ||
						layer == QStringLiteral("shield"))
					continue;
				break;

			case FilterNone:
				break;
		}

		for (const auto &it : iteratorList) {
			if (it->layer != layer)
				continue;

			if (!it->texture) {
				LOG_CERROR("scene") << "Invalid texture" << it->data.name << it->direction << it->layer;
				continue;
			}

			if (m_currentFrame < 0 || m_currentFrame >= it->data.frames.size()) {
				LOG_CERROR("scene") << "Invalid frame" << m_currentFrame << it->data.name << it->direction << it->layer;
				continue;
			}

			const TextureSpriteFrame &frame = it->data.frames.at(m_currentFrame);


			QSGSimpleTextureNode *imgNode = new QSGSimpleTextureNode();
			imgNode->setOwnsTexture(false);
			imgNode->setFlag(QSGNode::OwnedByParent);
			node->appendChildNode(imgNode);

			switch (m_opacityMask) {
				case MaskTop: {
					QRectF r = boundingRect();
					const qreal h = r.height()*0.5;

					r.setX(r.x() + frame.spriteSourceSize.x);
					r.setY(r.y() + frame.spriteSourceSize.y);
					r.setWidth(frame.spriteSourceSize.w);
					r.setHeight(h - frame.spriteSourceSize.y);

					imgNode->setRect(r);
					imgNode->setSourceRect(
								frame.frame.x,
								frame.frame.y,
								frame.frame.w,
								std::min((qreal)frame.frame.h, h - frame.spriteSourceSize.y)
								);
					break;
				}
				case MaskBottom: {
					QRectF r = boundingRect();
					const qreal h = r.height()*0.5;

					r.setX(r.x() + frame.spriteSourceSize.x);
					r.setY(r.y() + h);
					r.setWidth(frame.spriteSourceSize.w);
					r.setHeight(h);

					imgNode->setRect(r);
					imgNode->setSourceRect(
								frame.frame.x,
								frame.frame.y + h - frame.spriteSourceSize.y,
								frame.frame.w,
								frame.frame.h - (h - frame.spriteSourceSize.y)
								);


					break;
				}
				case MaskFull: {
					QRectF r = boundingRect();
					r.setX(r.x() + frame.spriteSourceSize.x);
					r.setY(r.y() + frame.spriteSourceSize.y);
					r.setWidth(frame.spriteSourceSize.w);
					r.setHeight(frame.spriteSourceSize.h);

					imgNode->setRect(r);
					imgNode->setSourceRect(
								frame.frame.x,
								frame.frame.y,
								frame.frame.w,
								frame.frame.h
								);
					break;
				}
			}


			if (it->texture != imgNode->texture())
				imgNode->setTexture(it->texture);


			imgNode->markDirty(QSGNode::DirtyGeometry);
		}
	}
}

TiledSpriteHandler::OpacityMask TiledSpriteHandler::opacityMask() const
{
	return m_opacityMask;
}

void TiledSpriteHandler::setOpacityMask(const OpacityMask &newOpacityMask)
{
	if (m_opacityMask == newOpacityMask)
		return;
	m_opacityMask = newOpacityMask;
	emit opacityMaskChanged();
}



/**
 * @brief TiledSpriteHandler::handlerSlave
 * @return
 */

TiledSpriteHandler *TiledSpriteHandler::handlerSlave() const
{
	return m_handlerSlave;
}

void TiledSpriteHandler::setHandlerSlave(TiledSpriteHandler *newHandlerSlave)
{
	if (newHandlerSlave == this) {
		LOG_CERROR("scene") << "Self handler master" << this;
		return;
	}

	if (m_handlerSlave == newHandlerSlave)
		return;
	m_handlerSlave = newHandlerSlave;
	emit handlerSlaveChanged();
}


bool TiledSpriteHandler::syncHandlers() const
{
	return m_syncHandlers;
}

void TiledSpriteHandler::setSyncHandlers(bool newSyncHandlers)
{
	if (m_syncHandlers == newSyncHandlers)
		return;
	m_syncHandlers = newSyncHandlers;
	emit syncHandlersChanged();
}


/**
 * @brief TiledSpriteHandler::handlerMaster
 * @return
 */

TiledSpriteHandler *TiledSpriteHandler::handlerMaster() const
{
	return m_handlerMaster;
}

void TiledSpriteHandler::setHandlerMaster(TiledSpriteHandler *newHandlerMaster)
{
	if (newHandlerMaster == this) {
		LOG_CERROR("scene") << "Self handler master" << this;
		return;
	}

	if (m_handlerMaster == newHandlerMaster)
		return;
	m_handlerMaster = newHandlerMaster;
	emit handlerMasterChanged();
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
	setOpacityMask(MaskFull);
	m_visibleLayers = QStringList{ QStringLiteral("default") };
	m_startFrameSeed = 0.;

	if (m_handlerMaster && m_handlerMaster->syncHandlers()) {
		m_handlerMaster->setSyncHandlers(false);
		m_handlerMaster->clear();
	}

	if (m_handlerSlave && m_syncHandlers)
		m_handlerSlave->clear();

	setSyncHandlers(false);
	update();
}


/**
 * @brief TiledSpriteHandler::setStartFrameSeed
 * @param percent
 */

void TiledSpriteHandler::setStartFrameSeed(const qreal &percent)
{
	if (percent < 0. || percent >= 1.) {
		m_startFrameSeed = QRandomGenerator::global()->bounded(1.);
	} else {
		m_startFrameSeed = percent;
	}
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
	if (m_handlerMaster && m_handlerMaster->syncHandlers()) {
		m_timer.stop();
		return;
	}

	if (m_baseObject && m_baseObject->game() && m_baseObject->game()->paused())
		return;

	const auto &ptr = findFirst(m_currentSprite, m_currentDirection);

	if (!ptr) {
		LOG_CERROR("scene") << "Sprite not found:" << m_currentSprite << m_currentDirection;
		m_timer.stop();
		return;
	}


	for (int dir = 0; dir<2; ++dir) {
		int nextFrame = m_isReverse ? m_currentFrame-1 : m_currentFrame+1;

		if (!m_isReverse && nextFrame >= ptr.value()->data.frames.size()) {
			if (!m_jumpToSprite.data.name.isEmpty()) {
				changeSprite(m_jumpToSprite.data.name, m_jumpToSprite.direction);
				m_jumpToSprite = Sprite{};
				return;
			}

			if (ptr.value()->data.loops < 0) {
				m_isReverse = true;
				continue;
			} else if (ptr.value()->data.loops == 0) {
				nextFrame = 0;
			} else {
				if (m_clearAtEnd) {
					clear();
					update();

					if (m_handlerSlave && m_syncHandlers) {
						m_handlerSlave->clear();
						m_handlerSlave->update();
					}

					return;
				}

				return;
			}
		} else if (m_isReverse && nextFrame < 0) {
			m_isReverse = false;
			continue;
		}

		m_currentFrame = nextFrame;
		break;
	}

	update();

	if (m_handlerSlave && m_syncHandlers)
		m_handlerSlave->update();
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


