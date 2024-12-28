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

class TiledGame;

/**
 * @brief The IsometricEntity class
 */

class IsometricEntity : public IsometricObject
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged FINAL)
	Q_PROPERTY(int maxHp READ maxHp WRITE setMaxHp NOTIFY maxHpChanged FINAL)

public:
	explicit IsometricEntity(TiledScene *scene);

	int hp() const;
	void setHp(int newHp);

	int maxHp() const;
	void setMaxHp(int newMaxHp);

	bool isAlive() const { return m_hp > 0; }
	virtual bool isDiscoverable() const { return true; }

	/////static qreal toMovingSpeed(const qreal &speed) { return speed * 0.53334f; }

	virtual void synchronize() override;
	virtual void updateSprite() = 0;

signals:
	void hurt();
	void healed();
	void hpChanged();
	void maxHpChanged();


protected:
	static std::optional<QPointF> checkEntityVisibility(TiledObjectBody *body, TiledObject *entity,
														const TiledObjectBody::FixtureCategory &category,
														float *transparentGroundPtr);

	static float checkGroundDistance(TiledObjectBody *body, const QPointF &targetPoint, float *lengthPtr = nullptr);

	template <typename T>
	static T getVisibleEntity(TiledObject *body, const QList<T> &entities,
							  const TiledObjectBody::FixtureCategory &category,
							  float *transparentGroundPtr,
							  QPointF *visiblePointPtr = nullptr);



	virtual void onAlive() = 0;
	virtual void onDead() = 0;


protected:
	int m_hp = 1;
	int m_maxHp = 1;
	QStringList m_moveDisabledSpriteList;		// At these sprites move disabled
};




/**
 * @brief IsometricEntityIface::getVisibleEntity
 * @param body
 * @param entities
 * @param visiblePointPtr
 * @return
 */

template<typename T>
T IsometricEntity::getVisibleEntity(TiledObject *body, const QList<T> &entities,
										 const TiledObjectBody::FixtureCategory &category,
										 float *transparentGroundPtr,
										 QPointF *visiblePointPtr)
{
	Q_ASSERT(body);

	return nullptr;

	/*if (body && body->scene()->isGroundContainsPoint(body->bodyPosition())) {
		return nullptr;
	}


	QMap<qreal, QPair<T, QPointF>> list;

	float transparentGroundDist = -1.0;

	for (const T &p : std::as_const(entities)) {
		if (!p)
			continue;

		if (!p->isDiscoverable())
			continue;

		float dist = -1.0;

		if (const auto &ptr = checkEntityVisibility(body, p, category, &dist); ptr && p->hp() > 0) {
			const qreal &dist = QVector2D(p->bodyPosition() - body->bodyPosition()).length();
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
	}*/
}




#endif // ISOMETRICENTITY_H
