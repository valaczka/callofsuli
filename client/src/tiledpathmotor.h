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
#include "qline.h"
#include "qobjectdefs.h"
#include "qpolygon.h"
#include <QSerializer>




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

	QPolygonF linesToPolygon() const;

	QPolygonF polygon() const;
	void setPolygon(const QPolygonF &newPolygon);

	Direction direction() const;
	void setDirection(Direction newDirection);

	qreal currentDistance() const;
	qreal fullDistance() const;

	cpVect getShortestPoint(const cpVect &pos, float *dstDistance = nullptr, int *dstSegment = nullptr, float *dstFactor = nullptr);
	std::optional<cpVect> getLastSegmentPoint();

	void updateBody(TiledObject *object, const float &speed, AbstractGame::TickTimer *timer = nullptr) override;
	cpVect basePoint() override;


	bool atBegin(TiledObjectBody *body = nullptr) const;
	bool atEnd(TiledObjectBody *body = nullptr) const;
	bool isClosed() const { return m_polygon.isClosed(); }

	WaitTimerState waitTimerState(AbstractGame::TickTimer *timer) const;

	static int getShortestSegment(const QPolygonF &polygon, const cpVect &pos);
	static bool clearFromSegment(QPolygonF *polygon, const int &segment);
	static bool clearToSegment(QPolygonF *polygon, const int &segment);

	bool clearFromSegment(const int &segment);
	bool clearToSegment(const int &segment);

	qint64 waitAtEnd() const;
	void setWaitAtEnd(qint64 newWaitAtEnd);

	qint64 waitAtBegin() const;
	void setWaitAtBegin(qint64 newWaitAtBegin);

	int lastSegment() const;
	float lastSegmentFactor() const;

private:
	struct Line {
		QLineF line;
		qreal angle = 0;
	};

	void loadLines();
	qreal angleFromLine(const Line &line) const;


	QPolygonF m_polygon;
	Direction m_direction = Forward;
	qint64 m_waitAtEnd = 0;
	qint64 m_waitAtBegin = 0;

	qint64 m_waitTimerEnd = 0;
	QList<Line> m_lines;

	int m_lastSegment = -1;
	float m_lastSegmentFactor = 0.;

	friend class TiledReturnPathMotor;
};

#endif // TILEDPATHMOTOR_H
