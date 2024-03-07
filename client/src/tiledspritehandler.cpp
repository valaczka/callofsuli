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

TiledSpriteHandler::TiledSpriteHandler(QQuickItem *parent)
	: QQuickItem(parent)
{

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
		LOG_CWARNING("scene") << "Sprite not found" << name << alteration << direction;
		return false;
	}

	if (mode == JumpImmediate) {
		m_jumpToId = -1;
		changeSprite(id);
	} else {
		if (m_currentId >= 0 && m_currentId < m_spriteList.size()) {
			m_spriteList.value(m_currentId).item->setProperty("waitForEnd", true);
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
	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledSprite.qml"), this);

	if (!component.isReady()) {
		LOG_CERROR("scene") << "Create sprite item error:" << component.errorString();
		return false;
	}

	QVariantMap prop;
	prop[QStringLiteral("frameCount")] = sprite.count;
	prop[QStringLiteral("frameDuration")] = sprite.duration;
	prop[QStringLiteral("frameHeight")] = sprite.height;
	prop[QStringLiteral("frameWidth")] = sprite.width;
	prop[QStringLiteral("frameX")] = sprite.x;
	prop[QStringLiteral("frameY")] = sprite.y;
	prop[QStringLiteral("baseLoops")] = sprite.loops;

	if (source.startsWith(QStringLiteral(":/")))
		prop[QStringLiteral("source")] = QUrl(QStringLiteral("qrc")+source);
	else
		prop[QStringLiteral("source")] = QUrl(source);


	// waitForEnd

	QQuickItem *item = qobject_cast<QQuickItem*>(component.createWithInitialProperties(prop));

	if (!item) {
		LOG_CERROR("scene") << "TiledSprite create error" << component.errorString();
		return false;
	}

	const QMetaObject *mo = item->metaObject();
	const QMetaObject *_this = this->metaObject();

	const int signal = mo->indexOfSignal("finished()");
	const int method = _this->indexOfMethod("spriteFinished()");

	connect(item, mo->method(signal), this, _this->method(method));

	Sprite s;
	s.alteration = alteration;
	s.direction = direction;
	s.item = item;
	s.baseName = sprite.name;

	item->setParentItem(this);
	m_spriteList.insert(++m_lastId, s);

	//LOG_CTRACE("scene") << "Sprite created:" << m_lastId << s.baseName << s.alteration << s.direction << s.item;

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
		return s.baseName == baseName && s.alteration == alteration && s.direction == direction;
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

	QString name;
	int current = -1;

	for (auto it = m_spriteList.begin(); it != m_spriteList.end(); ++it) {
		if (it.key() == id) {
			name = it->baseName;
			current = id;
			if (it->item)
				it->item->setVisible(true);
		} else {
			if (it->item)
				it->item->setVisible(false);
		}
	}

	m_currentId = current;
	setCurrentSprite(name);
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

	LOG_CTRACE("scene") << this << "CURRENT" << m_currentSprite;
}


/**
 * @brief TiledSpriteHandler::spriteFinished
 */

void TiledSpriteHandler::spriteFinished()
{
	LOG_CWARNING("scene") << "Sprite finished, jump to:" << m_jumpToId;

	if (m_currentId >= 0 && m_currentId < m_spriteList.size()) {
		m_spriteList.value(m_currentId).item->setProperty("waitForEnd", false);
	}

	if (m_jumpToId != -1)
		changeSprite(m_jumpToId);

	m_jumpToId = -1;
}


