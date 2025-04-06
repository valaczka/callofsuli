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
inline void RpgGameDataInterface<T, T2, T3, T4>::entityMove(IsometricEntity *entity,
															const RpgGameData::SnapshotInterpolation<T> &snapshot,
															const E &idle,
															const E &moving,
															const qreal &speed)
{
	Q_ASSERT(entity);

	if (snapshot.s2.f >= 0 && snapshot.s2.f <= snapshot.current) {
		LOG_CDEBUG("game") << "---------------------skip" << snapshot.current << snapshot.s2.f;
	} else {
		if (snapshot.s1.p.size() > 1 && snapshot.s2.p.size() > 1) {
			QVector2D final(snapshot.s2.p.at(0), snapshot.s2.p.at(1));

			if (snapshot.s1.st == idle && snapshot.s2.st == idle) {

				// "Teleport"
				if (const float dist = entity->distanceToPoint(final) * 1000. / (float) (snapshot.s2.f-snapshot.current);
						dist > 30. * speed / 60.) {
					const float angle = entity->angleToPoint(final);
					entity->setCurrentAngle(angle);
					entity->setSpeedFromAngle(angle, dist);
				} else {
					const b2Vec2 &vel = entity->body().GetLinearVelocity();
					if (vel.x != 0. || vel.y != 0.) {
						LOG_CINFO("game") << "FULL STOP ENTITY" << final;
						entity->stop();
						entity->emplace(final);
					}
					entity->setCurrentAngle(snapshot.s2.a);
				}
			} else if (snapshot.s2.st == idle &&
					   entity->distanceToPoint(final) < speed / 60.) {
				//LOG_CINFO("game") << "STOP ENTITY" << final;
				entity->stop();
				entity->emplace(final);
				entity->setCurrentAngle(snapshot.s2.a);
			} else if (snapshot.s2.st == moving) {
				const float dist = entity->distanceToPoint(final) * 1000. / (float) (snapshot.s2.f-snapshot.current);
				const float angle = entity->angleToPoint(final);
				entity->setCurrentAngle(angle);
				entity->setSpeedFromAngle(angle, dist);

				//LOG_CDEBUG("game") << "DIST" << snapshot.current << snapshot.s1.f << snapshot.s1.st << snapshot.s2.f << snapshot.s2.st << dist;
			} else {
				LOG_CDEBUG("game") << "INVALID" << snapshot.current << snapshot.s1.f << snapshot.s1.st << snapshot.s2.f << snapshot.s2.st;
			}

		} else {
			// TODO: add velocity
			///LOG_CERROR("game") << "???";
			//stop();
		}
	}
}


#endif // RPGGAMEDATAIFACE_T_H
