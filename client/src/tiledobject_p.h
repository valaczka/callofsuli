/*
 * ---- Call of Suli ----
 *
 * tiledobject_p.h
 *
 * Created on: 2024. 12. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef TILEDOBJECT_P_H
#define TILEDOBJECT_P_H

#include "tiledobject.h"

class TiledObjectBodyPrivate {

public:
	static bool isEqual(const float &v1, const float &v2) {
		return qAbs(v1 - v2) * 10000.f > QtPrivate::min(qAbs(v1), qAbs(v2));
	};

	static bool isEqual(const cpVect &v1, const cpVect &v2) {
		return isEqual(v1.x, v2.x) && isEqual(v1.y, v2.y);
	};

	static float normalizeFromRadian(const float &radian);
	static float normalizeToRadian(const float &normal);

private:
	TiledObjectBodyPrivate(TiledObjectBody *body);
	~TiledObjectBodyPrivate();

	void createBody(const cpBodyType &type, const cpFloat &mass, const cpFloat &moment);
	void deleteBody();

	void setSensorPolygon(const float &length, const float &range);
	void addVirtualCircle(const float &length);
	void removeVirtualCircle();
	void addTargetCircle(const float &length);

	void setVelocity(const cpVect &speed);


	void drawShape(TiledDebugDraw *draw, cpShape *shape, const QColor &color,
				   const qreal &lineWidth = 1., const bool filled = true, const bool outlined = true) const;


	TiledObjectBody *const q;

	cpBody *m_bodyRef = nullptr;

	bool m_aboutToDestruction = false;

	std::vector<cpShape*> m_bodyShapes;

	cpShape *m_sensorPolygon = nullptr;
	cpShape *m_virtualCircle = nullptr;
	cpShape *m_targetCircle = nullptr;

	float m_sensorLength = 0.;
	float m_targetLength = 0.;

	float m_currentSpeedSq = 0.;

	struct RotateAnimation {
		bool running = false;
		float destRadian = 0;
		bool clockwise = true;

		qreal speed = 0.2;				// 0.2 radian in 1/60 sec
	};

	RotateAnimation m_rotateAnimation;

	friend class TiledObjectBody;
};

#endif // TILEDOBJECT_P_H
