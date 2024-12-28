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

private:
	TiledObjectBodyPrivate(TiledObjectBody *body, b2::World *world);
	~TiledObjectBodyPrivate() = default;

	void createBody(const b2::Body::Params &params);
	void replaceWorld(b2::World *world, const QPointF &position = {}, const b2Rot &rotation = b2MakeRot(0.));
	void updateScene();
	void updateFilter();
	void setSensorPolygon(const float &length, const float &range);


	TiledObjectBody *const q;

	b2::World *m_world = nullptr;
	QPointer<TiledScene> m_scene;
	QMetaObject::Connection m_sceneConnection;
	b2::BodyRef m_bodyRef;
	b2AABB m_bodyAABB;

	std::vector<b2::ShapeRef> m_bodyShapes;

	b2::ShapeRef m_sensorPolygon;
	b2::ShapeRef m_virtualCircle;
	b2::ShapeRef m_targetCircle;

	TiledObjectBody::FixtureCategories m_categories = TiledObjectBody::FixtureInvalid;
	TiledObjectBody::FixtureCategories m_collidesWith = TiledObjectBody::FixtureInvalid;


	QVector2D m_currentSpeed;
	QVector2D m_lastPosition;

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
