/*
 * ---- Call of Suli ----
 *
 * tiledreturnpathmotor.h
 *
 * Created on: 2024. 03. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledReturnPathMotor
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

#ifndef TILEDRETURNPATHMOTOR_H
#define TILEDRETURNPATHMOTOR_H

#include "abstracttiledmotor.h"
#include "tiledobject.h"
#include "tiledpathmotor.h"




/**
 * @brief The TiledReturnPathMotor class
 */

class TiledReturnPathMotor : public AbstractTiledMotor
{
public:
	TiledReturnPathMotor(const QPointF &basePoint);
	virtual ~TiledReturnPathMotor() {}

	void updateBody(TiledObject *object, const float &distance, AbstractGame::TickTimer *timer = nullptr) override;
	QPointF basePoint() override;

	void moveBody(TiledObjectBody *body, const float32 &angle, const qreal &radius);
	void finish(TiledObjectBody *body, AbstractGame::TickTimer *timer);

	QPolygonF path() const;

	bool isReturning() const;
	void setIsReturning(bool newIsReturning);
	bool hasReturned() const;

	qint64 waitMsec() const;
	void setWaitMsec(qint64 newWaitMsec);

	bool isReturnReady(AbstractGame::TickTimer *timer) const;

	const std::optional<QPointF> &lastSeenPoint() const;
	void setLastSeenPoint(const QPointF &newLastSeenPoint);
	void clearLastSeenPoint();

private:
	void addPoint(const QPointF &point, const float &angle);

	QPointF m_basePoint;
	std::unique_ptr<TiledPathMotor> m_pathMotor;
	bool m_isReturning = false;
	bool m_hasReturned = false;
	std::optional<QPointF> m_lastSeenPoint;
	qint64 m_waitMsec = 2500;
	qint64 m_waitEnd = 0;

	QPolygonF m_path;
	float m_lastAngle = 0.;
};

#endif // TILEDRETURNPATHMOTOR_H
