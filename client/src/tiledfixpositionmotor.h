/*
 * ---- Call of Suli ----
 *
 * tiledfixpositionmotor.h
 *
 * Created on: 2024. 03. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledFixPositionMotor
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

#ifndef TILEDFIXPOSITIONMOTOR_H
#define TILEDFIXPOSITIONMOTOR_H

#include "abstracttiledmotor.h"


/**
 * @brief The TiledReturnPathMotor class
 */

class TiledFixPositionMotor : public AbstractTiledMotor
{
public:
	TiledFixPositionMotor();
	virtual ~TiledFixPositionMotor() {}

	void updateBody(TiledObject *object, const float &distance, AbstractGame::TickTimer *timer = nullptr) override;
	cpVect basePoint() override { return m_point; }

	cpVect point() const;
	void setPoint(const cpVect &newPoint);

	TiledObject::Direction direction() const;
	void setDirection(const TiledObject::Direction &newDirection);

private:
	cpVect m_point;
	TiledObject::Direction m_direction = TiledObject::Invalid;
};
#endif // TILEDFIXPOSITIONMOTOR_H
