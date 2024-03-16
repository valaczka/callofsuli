/*
 * ---- Call of Suli ----
 *
 * tiledpathmotor.h
 *
 * Created on: 2024. 03. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledPathMotor
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

#ifndef TILEDPATHMOTOR_H
#define TILEDPATHMOTOR_H

#include "abstracttiledmotor.h"
#include "qelapsedtimer.h"
#include "qline.h"
#include "qobjectdefs.h"
#include "qpolygon.h"
#include <QSerializer>





/**
 * @brief The TiledPathMotorSerializer class
 */

class TiledPathMotorSerializer : public QSerializer
{
	Q_GADGET

public:
	TiledPathMotorSerializer()
		: distance(0.)
		, forward(true)
	{}

	QS_SERIALIZABLE
	QS_FIELD(qreal, distance)
	QS_FIELD(bool, forward)
};




/**
 * @brief The TiledPathMotor class
 */

class TiledPathMotor : public AbstractTiledMotor
{
	Q_GADGET

public:
	enum Direction {
		Forward = 0,
		Backward
	};

	Q_ENUM(Direction);


	enum WaitTimerState {
		Invalid = 0,
		Running,
		Overdue
	};

	Q_ENUM(WaitTimerState);

	TiledPathMotor(const QPolygonF &polygon, const Direction &direction = Forward);
	TiledPathMotor() : TiledPathMotor(QPolygonF()) {}


	TiledPathMotorSerializer toSerializer() const;
	void fromSerializer(const TiledPathMotorSerializer &data);

	QPolygonF linesToPolygon() const;

	QPolygonF polygon() const;
	void setPolygon(const QPolygonF &newPolygon);

	Direction direction() const;
	void setDirection(Direction newDirection);

	QPointF currentPosition() const override;
	qreal currentDistance() const;
	qreal currentAngle() const;
	qreal currentAngleRadian() const;
	qreal fullDistance() const;

	void updateBody(TiledObject *object, const qreal &maximumSpeed = 0.) override;

	bool toBegin();
	bool toEnd();
	bool toDistance(const qreal &distance);
	void toPercent(const qreal &percent) { toDistance(m_fullDistance*percent); }
	bool step(const qreal &distance) override;
	bool step(const qreal &distance, const Direction &direction);

	bool atBegin() const { return m_currentDistance <= 0.00001; }
	bool atEnd() const { return m_currentDistance >= m_fullDistance; }
	bool isClosed() const { return m_polygon.isClosed(); }

	void waitTimerStart(const qint64 &msec);
	void waitTimerStop();
	WaitTimerState waitTimerState() const;

	int currentSegment() const;
	bool clearFromSegment(const int &segment);

	qint64 waitAtEnd() const;
	void setWaitAtEnd(qint64 newWaitAtEnd);

	qint64 waitAtBegin() const;
	void setWaitAtBegin(qint64 newWaitAtBegin);


private:
	struct Line {
		QLineF line;
		qreal length = 0;
		qreal angle = 0;
		qreal speed = 1.0;
	};

	void loadLines();
	qreal angleFromLine(const Line &line) const;


	QPolygonF m_polygon;
	Direction m_direction = Forward;
	qreal m_currentDistance = 0.0;
	qreal m_fullDistance = 0.0;
	qreal m_currentAngle = 0.0;
	QPointF m_currentPosition;
	int m_currentSegment = -1;
	qint64 m_waitAtEnd = 0;
	qint64 m_waitAtBegin = 0;

	QElapsedTimer m_waitTimer;
	qint64 m_waitForMsec = 0;
	QList<Line> m_lines;

	friend class TiledReturnPathMotor;
};

#endif // TILEDPATHMOTOR_H
