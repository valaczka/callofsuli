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

class IsometricBullet : public IsometricObjectCircle
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(bool impacted READ impacted WRITE setImpacted NOTIFY impactedChanged FINAL)
	Q_PROPERTY(qreal maxDistance READ maxDistance WRITE setMaxDistance NOTIFY maxDistanceChanged FINAL)

public:
	explicit IsometricBullet(QQuickItem *parent = nullptr);
	virtual ~IsometricBullet();

	static IsometricBullet* createBullet(TiledGame *game, TiledScene *scene);

	void shot(const QPointF &from, const Direction &direction);
	void shot(const QPointF &from, const qreal &angle);

	void worldStep() override;

	bool impacted() const;
	void setImpacted(bool newImpacted);

	qreal maxDistance() const;
	void setMaxDistance(qreal newMaxDistance);

signals:
	void impactedChanged();
	void maxDistanceChanged();

protected:
	virtual void load();

protected:
	QPointF m_startPoint;
	Direction m_direction = Invalid;
	qreal m_angle = 0.;
	qreal m_maxDistance = 725.;

private:
	bool m_impacted = false;
};

#endif // ISOMETRICBULLET_H
