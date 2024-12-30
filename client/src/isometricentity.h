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

	template <typename T, typename = std::enable_if<std::is_base_of<IsometricEntity, T>::value>::type>
	static TiledReportedFixtureMap getVisibleEntities(const TiledObjectBody *body, const QList<T*> &entities, QVector2D *transparentGnd);

	template <typename T, typename = std::enable_if<std::is_base_of<IsometricEntity, T>::value>::type>
	TiledReportedFixtureMap getVisibleEntities(const QList<T*> &entities, QVector2D *transparentGnd) const {
		return getVisibleEntities(this, entities, transparentGnd);
	}

	virtual void synchronize() override;
	virtual void updateSprite() = 0;

signals:
	void hurt();
	void healed();
	void hpChanged();
	void maxHpChanged();


protected:
	[[deprecated]] static std::optional<QPointF> checkEntityVisibility(TiledObjectBody *body, TiledObject *entity,
																	   const TiledObjectBody::FixtureCategory &category,
																	   float *transparentGroundPtr);

	static float checkGroundDistance(TiledObjectBody *body, const QPointF &targetPoint, float *lengthPtr = nullptr);



	virtual void onAlive() = 0;
	virtual void onDead() = 0;


protected:
	int m_hp = 1;
	int m_maxHp = 1;
	QStringList m_moveDisabledSpriteList;		// At these sprites move disabled
};



/**
 * @brief IsometricEntity::getVisibleEntities
 * @param body
 * @param entities
 * @param transparentGnd
 * @return
 */

template<typename T, typename T2>
TiledReportedFixtureMap IsometricEntity::getVisibleEntities(const TiledObjectBody *body, const QList<T *> &entities, QVector2D *transparentGnd)
{
	TiledReportedFixtureMap ret;

	for (IsometricEntity *p : entities) {
		if (!p || !p->isDiscoverable() || p->bodyShapes().empty())
			continue;

		const TiledReportedFixtureMap &map = body->rayCast(p->bodyPosition(),
														   FixtureCategories::fromInt(p->bodyShapes().front().GetFilter().categoryBits));

		QVector2D pos(p->bodyPosition());

		for (auto it=map.cbegin(); it != map.cend(); ++it) {
			b2::ShapeRef r = it->shape;

			if (r.IsSensor())
				continue;

			if (r.GetFilter().categoryBits & FixtureGround) {
				if (it->body && it->body->opaque()) {
					break;
				} else if (transparentGnd) {
					if (transparentGnd->isNull() || pos.distanceToPoint(it->point) < pos.distanceToPoint(*transparentGnd))
						*transparentGnd = it->point;
				}
			}

			if (it->body == p)
				ret.insert(it.key(), it.value());
		}
	}

	return ret;
}





#endif // ISOMETRICENTITY_H
