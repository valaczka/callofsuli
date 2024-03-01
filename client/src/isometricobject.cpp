/*
 * ---- Call of Suli ----
 *
 * isogameobject.cpp
 *
 * Created on: 2024. 02. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricObject
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

#include "isometricobject.h"
#include "Logger.h"
#include "box2dfixture.h"
#include "tiledscene.h"
#include <QtMath>

IsometricObject::IsometricObject(QQuickItem *parent)
	: TiledObject(parent)
{
	m_body->setFixedRotation(true);

	connect(this, &IsometricObject::xChanged, this, &IsometricObject::onXYChanged);
	connect(this, &IsometricObject::yChanged, this, &IsometricObject::onXYChanged);
}



/**
 * @brief IsometricObject::angleFromDirection
 * @param directions
 * @param direction
 * @return
 */

qreal IsometricObject::angleFromDirection(const Direction &direction)
{
	switch (direction) {
		case West: return M_PI; break;
		case NorthWest: return M_PI - atan(0.5); break;
		case North: return M_PI_2; break;
		case NorthEast: return atan(0.5); break;
		case East: return 0; break;
		case SouthEast: return -atan(0.5); break;
		case South: return -M_PI_2; break;
		case SouthWest: return -M_PI + atan(0.5); break;
		case Invalid: return 0; break;
	}

	return 0;
}




/**
 * @brief IsometricObject::directionFromAngle
 * @param angle
 * @param directions
 * @return
 */

IsometricObject::Direction IsometricObject::directionFromAngle(const Directions &directions, const qreal &angle)
{
	static const std::map<qreal, Direction, std::greater<qreal>> map8 = {
		{ M_PI * 7/8	, West },
		{ M_PI * 5/8	, NorthWest },
		{ M_PI * 3/8	, North },
		{ M_PI * 1/8	, NorthEast },
		{ M_PI * -1/8	, East },
		{ M_PI * -3/8	, SouthEast },
		{ M_PI * -5/8	, South },
		{ M_PI * -7/8	, SouthWest },
		{ M_PI * -1		, West },
	};

	static const std::map<qreal, Direction, std::greater<qreal>> map4 = {
		{ M_PI * 3/4	, West },
		{ M_PI * 1/4	, North },
		{ M_PI * -1/4	, East },
		{ M_PI * -3/4	, South },
		{ M_PI * -1		, West },
	};


	const std::map<qreal, Direction, std::greater<qreal>> *ptr = nullptr;

	switch (directions) {
		case Direction_4:
			ptr = &map4;
			break;
		case Direction_8:
		case Direction_Infinite:
			ptr = &map8;
			break;
		default:
			break;
	}

	if (!ptr)
		return Invalid;

	for (const auto &[a, dir] : *ptr) {
		if (angle >= a)
			return dir;
	}

	return Invalid;
}


/**
 * @brief IsometricObject::nearestDirectionFromAngle
 * @param directions
 * @param angle
 * @return
 */

IsometricObject::Direction IsometricObject::nearestDirectionFromAngle(const Directions &directions, const qreal &angle)
{
	static const std::map<qreal, Direction, std::greater<qreal>> map8 = {
		{ M_PI			, West },
		{ M_PI * 3/4	, NorthWest },
		{ M_PI_2		, North },
		{ M_PI_4		, NorthEast },
		{ 0				, East },
		{ -M_PI_4		, SouthEast },
		{ -M_PI_2		, South },
		{ M_PI * -3/4	, SouthWest },
		{ -M_PI			, West },
	};

	static const std::map<qreal, Direction, std::greater<qreal>> map4 = {
		{ M_PI		, West },
		{ M_PI_2	, North },
		{ 0			, East },
		{ -M_PI_2	, South },
		{ -M_PI		, West },
	};

	qreal diff = -1;
	Direction ret = Invalid;

	const std::map<qreal, Direction, std::greater<qreal>> *ptr = nullptr;

	switch (directions) {
		case Direction_4:
			ptr = &map4;
			break;
		case Direction_8:
		case Direction_Infinite:
			ptr = &map8;
			break;
		default:
			break;
	}

	if (!ptr)
		return Invalid;

	for (const auto &[a, dir] : *ptr) {
		const qreal d = std::abs(a-angle);
		if (diff == -1 || d < diff) {
			diff = d;
			ret = dir;
		}
	}

	return ret;
}



IsometricObject::Direction IsometricObject::currentDirection() const
{
	return m_currentDirection;
}

void IsometricObject::setCurrentDirection(Direction newCurrentDirection)
{
	if (m_currentDirection == newCurrentDirection)
		return;
	m_currentDirection = newCurrentDirection;
	emit currentDirectionChanged();
}






/**
 * @brief IsometricObject::availableDirections
 * @return
 */

IsometricObject::Directions IsometricObject::availableDirections() const
{
	return m_availableDirections;
}

void IsometricObject::setAvailableDirections(Directions newAvailableDirections)
{
	if (m_availableDirections == newAvailableDirections)
		return;
	m_availableDirections = newAvailableDirections;
	emit availableDirectionsChanged();
}




/**
 * @brief IsometricObject::appendSprite
 * @param sprite
 * @param source
 * @return
 */

bool IsometricObject::appendSprite(const QString &source, const IsometricObjectSprite &sprite)
{
	Q_ASSERT(m_visualItem);

	for (int i=0; i<sprite.directions.size(); ++i) {
		const int n = sprite.directions.at(i);
		Direction direction = QVariant::fromValue(n).value<Direction>();

		if (direction == Invalid) {
			LOG_CERROR("scene") << "Sprite invalid direction:" << n << source;
			return false;
		}

		TiledObjectSprite s2 = sprite;

		if (sprite.startColumn >= 0) {
			s2.frameX = sprite.startColumn + i*sprite.frameWidth;
		} else if (sprite.startRow >= 0) {
			s2.frameY = sprite.startRow + i*sprite.frameHeight;
		}

		const QString &spriteName = getSpriteName(sprite.name, direction);

		if (m_availableSprites.contains(spriteName)) {
			LOG_CERROR("scene") << "Sprite already loaded:" << sprite.name << direction << source;
			return false;
		}

		s2.name = spriteName;
		QJsonObject to2;
		for (auto it = sprite.to.constBegin(); it != sprite.to.constEnd(); ++it) {
			to2.insert(getSpriteName(it.key(), direction), it.value());
		}
		s2.to = to2;

		QVariantMap data = s2.toJson().toVariantMap();

		data[QStringLiteral("source")] = QUrl::fromLocalFile(source);

		QMetaObject::invokeMethod(m_visualItem, "appendSprite", Qt::DirectConnection,
								  Q_ARG(QVariant, data));

		LOG_CTRACE("scene") << "Append sprite" << data;

		m_availableSprites.append(spriteName);
	}

	return true;
}




/**
 * @brief IsometricObject::appendSpriteList
 * @param source
 * @param spriteList
 * @return
 */

bool IsometricObject::appendSpriteList(const QString &source, const IsometricObjectSpriteList &spriteList)
{
	Q_ASSERT(m_visualItem);

	for (const auto &s : spriteList.sprites) {
		if (!appendSprite(source, s)) {
			LOG_CERROR("scene") << "Load sprite error:" << source;
			return false;
		}
	}

	return true;
}



/**
 * @brief IsometricObject::appendSprite
 * @param sprite
 * @param path
 * @return
 */

bool IsometricObject::appendSprite(const IsometricObjectAlterableSprite &sprite, const QString &path)
{
	Q_ASSERT(m_visualItem);

	for (const IsometricObjectSprite &s : sprite.sprites) {
		for (auto it = sprite.alterations.constBegin(); it != sprite.alterations.constEnd(); ++it) {
			const QString &alteration = it.key();

			for (int i=0; i<s.directions.size(); ++i) {
				const int n = s.directions.at(i);
				Direction direction = QVariant::fromValue(n).value<Direction>();

				if (direction == Invalid) {
					LOG_CERROR("scene") << "Sprite invalid direction:" << n << path;
					return false;
				}

				const QString &spriteName = getSpriteName(s.name, direction, alteration);

				if (m_availableSprites.contains(spriteName)) {
					LOG_CERROR("scene") << "Sprite already loaded:" << s.name << alteration << path << it.value();
					return false;
				}

				TiledObjectSprite s2 = s;

				if (s.startColumn >= 0) {
					s2.frameX = s.startColumn + i*s.frameWidth;
				} else if (s.startRow >= 0) {
					s2.frameY = s.startRow + i*s.frameHeight;
				}

				s2.name = spriteName;
				QJsonObject to2;
				for (auto it = s.to.constBegin(); it != s.to.constEnd(); ++it) {
					to2.insert(getSpriteName(it.key(), direction, alteration), it.value());
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
	}

	m_availableAlterations.append(sprite.alterations.keys());

	return true;
}



/**
 * @brief IsometricObject::appendSpriteList
 * @param sprite
 * @param path
 * @return
 */

bool IsometricObject::appendSpriteList(const IsometricObjectAlterableSpriteList &sprite, const QString &path)
{
	Q_ASSERT(m_visualItem);

	for (const auto &s : sprite.list) {
		if (!appendSprite(s, path)) {
			LOG_CERROR("scene") << "Load sprite error:" << path;
			return false;
		}
	}

	return true;
}


/**
 * @brief IsometricObject::getSpriteName
 * @param sprite
 * @param direction
 * @param alteration
 * @return
 */

QString IsometricObject::getSpriteName(const QString &sprite, const Direction &direction, const QString &alteration)
{
	QString s = TiledObject::getSpriteName(sprite, alteration);

	if (direction != Invalid)
		s.append(QStringLiteral("-")).append(QString::number(direction));

	return s;
}



/**
 * @brief IsometricObject::useDynamicZ
 * @return
 */

bool IsometricObject::useDynamicZ() const
{
	return m_useDynamicZ;
}

void IsometricObject::setUseDynamicZ(bool newUseDynamicZ)
{
	if (m_useDynamicZ == newUseDynamicZ)
		return;
	m_useDynamicZ = newUseDynamicZ;
	emit useDynamicZChanged();
}



/**
 * @brief IsometricObject::onXYChanged
 */

void IsometricObject::onXYChanged()
{
	if (!m_scene || !m_useDynamicZ)
		return;

	QPointF p = position() + QPointF(width()/2, height()/2);
	setZ(m_scene->getDynamicZ(p, m_defaultZ));
}


/**
 * @brief IsometricObject::defaultZ
 * @return
 */

qreal IsometricObject::defaultZ() const
{
	return m_defaultZ;
}

void IsometricObject::setDefaultZ(qreal newDefaultZ)
{
	if (qFuzzyCompare(m_defaultZ, newDefaultZ))
		return;
	m_defaultZ = newDefaultZ;
	emit defaultZChanged();
}
