/*
 * ---- Call of Suli ----
 *
 * isometricentity.h
 *
 * Created on: 2024. 03. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricEntity
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

#ifndef ISOMETRICENTITY_H
#define ISOMETRICENTITY_H

#include "isometricobject.h"
#include <QQmlEngine>
#include "tiledscene.h"

class TiledGame;

/**
 * @brief The IsometricEntity class
 */

class IsometricEntityIface
{
public:
	IsometricEntityIface()
	{}

	static std::optional<QPointF> checkEntityVisibility(TiledObjectBody *body, TiledObjectBase *entity,
														const TiledObjectBody::FixtureCategory &category,
														float32 *transparentGroundPtr);

	static float32 checkGroundDistance(TiledObjectBody *body, const QPointF &targetPoint, float32 *lengthPtr = nullptr);

	template <typename T>
	static T getVisibleEntity(TiledObjectBody *body, const QList<T> &entities,
							  const TiledObjectBody::FixtureCategory &category,
							  float32 *transparentGroundPtr,
							  QPointF *visiblePointPtr = nullptr);

	TiledObject::Direction movingDirection() const;
	void setMovingDirection(const TiledObject::Direction &newMovingDirection);

	qreal maximumSpeed() const;
	void setMaximumSpeed(qreal newMaximumSpeed);

	static QPointF maximizeSpeed(const QPointF &point, const qreal &maximumSpeed);
	static QPointF &maximizeSpeed(QPointF &point, const qreal &maximumSpeed);
	QPointF maximizeSpeed(const QPointF &point) const {
		return maximizeSpeed(point, m_maximumSpeed);
	}
	QPointF &maximizeSpeed(QPointF &point) const {
		return maximizeSpeed(point, m_maximumSpeed);
	}

	int hp() const;
	void setHp(int newHp);

	int maxHp() const;
	void setMaxHp(int newMaxHp);

	bool isAlive() const { return m_hp > 0; }
	virtual bool isDiscoverable() const { return true; }

	static qreal toMovingSpeed(const qreal &speed) { return speed * 0.53334f; }

	virtual void updateSprite() = 0;

public:
	virtual void hurt() = 0;
	virtual void healed() = 0;
	virtual void hpChanged() = 0;
	virtual void maxHpChanged() = 0;


	qreal movingSpeed() const;
	void setMovingSpeed(qreal newMovingSpeed);

protected:
	void entityIfaceWorldStep(const qreal &factor, const QPointF &position, const TiledObject::Directions &availableDirections);

	virtual void onAlive() = 0;
	virtual void onDead() = 0;

	TiledObject::Direction m_movingDirection = TiledObject::Invalid;
	qreal m_movingSpeed = 0.;
	qreal m_maximumSpeed = 10.;
	int m_hp = 1;
	int m_maxHp = 1;
	QStringList m_moveDisabledSpriteList;		// At these sprites move disabled

private:
	QPointF m_lastPosition;
};




/**
 * @brief IsometricEntityIface::getVisibleEntity
 * @param body
 * @param entities
 * @param visiblePointPtr
 * @return
 */

template<typename T>
T IsometricEntityIface::getVisibleEntity(TiledObjectBody *body, const QList<T> &entities,
										 const TiledObjectBody::FixtureCategory &category,
										 float32 *transparentGroundPtr,
										 QPointF *visiblePointPtr)
{
	Q_ASSERT(body);

	if (body->baseObject() && body->baseObject()->scene()->isGroundContainsPoint(body->bodyPosition())) {
		return nullptr;
	}


	QMap<qreal, QPair<T, QPointF>> list;

	float32 transparentGroundDist = -1.0;

	for (const T &p : std::as_const(entities)) {
		if (!p)
			continue;

		if (!p->isDiscoverable())
			continue;

		float32 dist = -1.0;

		if (const auto &ptr = checkEntityVisibility(body, p, category, &dist); ptr && p->hp() > 0) {
			const qreal &dist = QVector2D(p->body()->bodyPosition() - body->bodyPosition()).length();
			list.insert(dist, qMakePair(p, ptr.value()));
		}

		if (dist >= 0. && (transparentGroundDist == -1.0 || dist < transparentGroundDist))
			transparentGroundDist = dist;
	}

	if (transparentGroundPtr)
		*transparentGroundPtr = transparentGroundDist;


	if (list.isEmpty())
		return nullptr;
	else {
		const auto &ptr = list.first();
		if (visiblePointPtr)
			*visiblePointPtr = ptr.second;
		return ptr.first;
	}
}



/**
 * @brief The IsometricCircleEntity class
 */

class IsometricCircleEntity : public IsometricObjectCircle, public IsometricEntityIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged FINAL)
	Q_PROPERTY(int maxHp READ maxHp WRITE setMaxHp NOTIFY maxHpChanged FINAL)

public:
	explicit IsometricCircleEntity(QQuickItem *parent = nullptr);

	void emplace(const QPointF &pos) {
		m_body->emplace(pos);
		updateSprite();
	}

signals:
	void hurt() override final;
	void healed() override final;
	void hpChanged() override final;
	void maxHpChanged() override final;

protected:
	virtual void entityWorldStep(const qreal &factor) { Q_UNUSED(factor); }

	void worldStep(const qreal &factor) override final {
		entityIfaceWorldStep(factor, position(), m_availableDirections);
		entityWorldStep(factor);
	};

};

#endif // ISOMETRICENTITY_H
