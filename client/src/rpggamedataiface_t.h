/*
 * ---- Call of Suli ----
 *
 * rpggamedataiface_t.h
 *
 * Created on: 2025. 04. 06.
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

#ifndef RPGGAMEDATAIFACE_T_H
#define RPGGAMEDATAIFACE_T_H


#include "rpggamedataiface.h"

/**
 *
 */

template<typename T, typename T2, typename T3, typename T4>
template<typename E>
inline QVector2D RpgGameDataInterface<T, T2, T3, T4>::entityMove(IsometricEntity *entity,
																 const RpgGameData::SnapshotInterpolation<T> &snapshot,
																 const E &idle,
																 const E &moving,
																 const qreal &speed)
{
	Q_ASSERT(entity);


	static const auto fnStop = [](IsometricEntity *entity, const float &angle) {
		Q_ASSERT(entity);
		const b2Vec2 &vel = entity->body().GetLinearVelocity();

		if (vel.x != 0. || vel.y != 0.)
			entity->stop();

		entity->setCurrentAngle(angle);
	};

	static const auto fnEmplace = [](IsometricEntity *entity, const QVector2D &dst, const float &angle) {
		Q_ASSERT(entity);
		const b2Vec2 &pos = entity->body().GetPosition();

		if (pos.x == dst.x() && pos.y == dst.y()) {
			entity->stop();
			entity->setCurrentAngle(angle);
			return;
		}

		entity->emplace(dst);
		entity->setCurrentAngle(angle);
	};


	QVector2D currentSpeed;

	if (snapshot.s1.f < 0) {
		LOG_CERROR("game") << "Invalid snap" << snapshot.s1.f << snapshot.current << snapshot.s2.f;
		fnStop(entity, snapshot.s1.a);
		return currentSpeed;
	}

	if (snapshot.s2.f >= 0 && snapshot.s2.f <= snapshot.current) {
		return currentSpeed;
	}

	if (snapshot.s1.f < (snapshot.current - RPG_UDP_DELTA_MSEC*2) && snapshot.s2.f < 0) {
		entity->stop();
		return currentSpeed;
	}


	// Get final point
	// Calculate distance and angle to final point

	QVector2D final;
	float dist = 0.;

	if (snapshot.s2.f > 0 && snapshot.s2.p.size() > 1) {
		final.setX(snapshot.s2.p.at(0));
		final.setY(snapshot.s2.p.at(1));
		dist = entity->distanceToPoint(final) * 1000. / (float) (snapshot.s2.f-snapshot.current);
	} else if (snapshot.s1.p.size() > 1) {
		if (snapshot.s1.st == moving && snapshot.s1.cv.size() > 1 && snapshot.current - snapshot.s1.f < 1000) {
			final.setX(snapshot.s1.p.at(0) + snapshot.s1.cv.at(0));
			final.setY(snapshot.s1.p.at(1) + snapshot.s1.cv.at(1));
			dist = entity->distanceToPoint(final) / (float) (snapshot.s1.f + 1000 - snapshot.current);
		} else if (snapshot.s1.p.size() > 1) {
			final.setX(snapshot.s1.p.at(0));
			final.setY(snapshot.s1.p.at(1));
			dist = entity->distanceToPoint(final);
		} else {
			LOG_CERROR("game") << "Invalid snapshot data" << entity;

			entity->stop();
			return currentSpeed;
		}
	} else {
		LOG_CERROR("game") << "Invalid snapshot data" << entity;

		entity->stop();
		return currentSpeed;
	}


	if (snapshot.s1.st == idle && snapshot.s2.st == idle && dist < 2 * speed / 60.) {
		fnEmplace(entity, final, snapshot.s2.a);
		return currentSpeed;
	}


	if (snapshot.s2.f > 0 && snapshot.s2.st != moving && dist < 2 * speed / 60.) {
		fnEmplace(entity, final, snapshot.s2.a);
		return currentSpeed;
	}

	const b2Vec2 &vel = entity->body().GetLinearVelocity();
	if (snapshot.s1.st == idle && vel.x == 0. && vel.y == 0. && dist < speed / 180.) {
		LOG_CINFO("game") << "STAY";
		return currentSpeed;
	}

	const float angle = entity->angleToPoint(final);

	entity->setCurrentAngle(angle);
	entity->setSpeedFromAngle(angle, dist);

	if (snapshot.s1.cv.size() > 1) {
		currentSpeed.setX(snapshot.s1.cv.at(0));
		currentSpeed.setY(snapshot.s1.cv.at(1));
	}

	return currentSpeed;
}


#endif // RPGGAMEDATAIFACE_T_H
