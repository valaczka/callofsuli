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


class TiledReturnPathMotor : public AbstractTiledMotor
{
public:
	TiledReturnPathMotor();
	virtual ~TiledReturnPathMotor() {}

	QPointF currentPosition() const override;

	void moveBody(TiledObjectBody *body, const float32 &angle, const qreal &radius);
	void finish(TiledObjectBody *body);
	bool stepBack(const qreal &radius);

	QPolygonF path() const;

	bool isReturning() const;
	void setIsReturning(bool newIsReturning);


private:
	TiledPathMotor m_pathMotor;
	QPolygonF m_path;
	float32 m_lastAngle = 0;
	bool m_isReturning = false;
};

#endif // TILEDRETURNPATHMOTOR_H
