/*
 * ---- Call of Suli ----
 *
 * isometricbullet.h
 *
 * Created on: 2024. 03. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricBullet
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

#ifndef ISOMETRICBULLET_H
#define ISOMETRICBULLET_H

#include "isometricobject.h"
#include <QQmlEngine>



/**
 * @brief The IsometricBullet class
 */

class IsometricBullet : public IsometricObject
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(bool impacted READ impacted WRITE setImpacted NOTIFY impactedChanged FINAL)
	Q_PROPERTY(qreal maxDistance READ maxDistance WRITE setMaxDistance NOTIFY maxDistanceChanged FINAL)

public:
	explicit IsometricBullet(TiledScene *scene);
	virtual ~IsometricBullet();

	void initialize();

	virtual void shot(const QPointF &from, const qreal &angle);

	void worldStep() override;

	bool impacted() const;
	void setImpacted(bool newImpacted);

	qreal maxDistance() const;
	void setMaxDistance(qreal newMaxDistance);

	virtual void onShapeContactBegin(b2::ShapeRef self, b2::ShapeRef other) override;

	void disableBullet();

signals:
	void impactedChanged();
	void maxDistanceChanged();

protected:
	virtual void load() = 0;
	virtual void impactEvent(TiledObjectBody *base, b2::ShapeRef shape) = 0;
	virtual void overshootEvent() {}

	virtual void synchronize() override;

protected:
	QVector2D m_startPoint;
	qreal m_speed = 200.;
	qreal m_maxDistance = 700.;

private:
	bool m_impacted = false;
};



#endif // ISOMETRICBULLET_H
