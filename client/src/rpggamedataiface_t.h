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
 * @brief RpgGameDataInterface::fnStop
 * @param entity
 * @param angle
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline bool RpgGameDataInterface<T, T2, T3, T4>::fnStop(IsometricEntity *entity, const float &angle)
{
	Q_ASSERT(entity);
	const cpVect vel = cpBodyGetVelocity(entity->body());

	if (vel.x != 0. || vel.y != 0.)
		entity->stop();

	entity->setCurrentAngleForced(angle);

	return !cpveql(vel, cpvzero);
}


/**
 * @brief RpgGameDataInterface::fnEmplace
 * @param entity
 * @param dst
 * @param angle
 * @return
 */


template<typename T, typename T2, typename T3, typename T4>
inline bool RpgGameDataInterface<T, T2, T3, T4>::fnEmplace(IsometricEntity *entity, const cpVect &dst, const float &angle)
{
	Q_ASSERT(entity);
	if (dst == entity->bodyPosition()) {
		entity->stop();
		entity->setCurrentAngleForced(angle);
		return false;
	}

	entity->emplace(dst);
	entity->setCurrentAngleForced(angle);

	return true;
}


/**
 * @brief RpgGameDataInterface::fnMove
 * @param entity
 * @param final
 * @param inFrame
 * @param maxSpeed
 * @param cv
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline cpVect RpgGameDataInterface<T, T2, T3, T4>::fnMove(IsometricEntity *entity, const cpVect &final,
														  const int &inFrame, const float &maxSpeed, const QList<float> &cv)
{
	Q_ASSERT(entity);

	if (!entity->moveToPoint(final, inFrame, maxSpeed))
		entity->stop();

	if (cv.size() > 1)
		return cpv(cv.at(0), cv.at(1));
	else
		return cpBodyGetVelocity(entity->body());
}





/**
 *
 */

template<typename T, typename T2, typename T3, typename T4>
template<typename E>
inline cpVect RpgGameDataInterface<T, T2, T3, T4>::entityMove(IsometricEntity *entity,
															  const RpgGameData::SnapshotInterpolation<T> &snapshot,
															  const E &idle,
															  const E &moving,
															  const qreal &speed,
															  const qreal &maxSpeed,
															  QString *msg)
{
	Q_ASSERT(entity);

	const T &from = snapshot.s1.f >= 0 ? snapshot.s1 : snapshot.last;
	const T &to = snapshot.s2.f >= 0 ? snapshot.s2 : snapshot.last;

	if (from.f < 0 || to.f < 0 || from.f > to.f) {
		if (msg) *msg = QStringLiteral("Invalid snap");
		fnStop(entity, 0.);
		return cpvzero;
	}

	std::optional<cpVect> toFinal;
	std::optional<cpVect> toCv;

	if (!to.p.isEmpty())
		toFinal = cpv(to.p.at(0), to.p.at(1));

	if (!to.cv.isEmpty())
		toCv = cpv(to.cv.at(0), to.cv.at(1));

	if (to.f < (snapshot.current - 2*RPG_UDP_DELTA_TICK)) {
		if (fnStop(entity, to.a)) {
			if (msg) *msg = QStringLiteral("*** OVER ***");
			LOG_CERROR("game") << "Snapshot over" << to.f << "->" << snapshot.current;
		}

		return cpvzero;
	}


	// Extrapolation

	if (to.st == idle && from.st == idle && toFinal) {
		if (entity->bodyPosition() == toFinal.value()) {
			if (fnStop(entity, to.a)) {
				if (msg) *msg = QStringLiteral("________stop_______");
			} else {
				if (msg) *msg = QStringLiteral("________bypass_______");
			}

			return cpvzero;
		} else if (entity->distanceToPointSq(toFinal.value()) < (speed/120.)) {
			fnEmplace(entity, toFinal.value(), to.a);
			if (msg) *msg = QStringLiteral("________emplace_______");
			return cpvzero;
		} else {
			return fnMove(entity, toFinal.value(), 1, maxSpeed, QList<float>{});
		}
	}

	if (to.st == moving) {
		// Extrapolation on moving (+1 frame)

		if (!toFinal || !toCv) {
			fnStop(entity, to.a);
			if (msg) *msg = QStringLiteral("*** MISSING POINT ***");
			return cpvzero;
		}

		if (to.f <= snapshot.current) {
			static const float factor = 1.0;//(to.f <= (snapshot.current - RPG_UDP_DELTA_TICK) ? 0.99 : 1.0);

			const int frames = snapshot.current-to.f+1;
			const cpVect pos = cpvadd(toFinal.value(),
									  cpvmult(toCv.value(), factor*frames/60.));

			if (msg) *msg = QStringLiteral("@[%1,%2] x%3").arg(pos.x).arg(pos.y).arg(factor);

			return fnMove(entity, pos, frames, maxSpeed, from.cv);
		} else {
			const int frames = to.f-snapshot.current;
			const cpVect pos =
					frames > 1 ?
						cpvlerp(entity->bodyPosition(),
								cpvadd(toFinal.value(), cpvmult(toCv.value(), frames/60.)),
								1./frames) :
						cpvadd(toFinal.value(), cpvmult(toCv.value(), 1/60.))
						;


			if (msg) *msg = QStringLiteral("+%1[%2,%3]").arg(frames).arg(pos.x).arg(pos.y);

			return fnMove(entity, pos, frames, maxSpeed, from.cv);
		}

	} else {
		// Extrapolation on idle (or other)

		if (!toFinal) {
			fnStop(entity, to.a);
			if (msg) *msg = QStringLiteral("*** STOP ***");
			return cpvzero;
		}

		if (to.f <= snapshot.current) {
			if (msg) *msg = QStringLiteral("![%1,%2]").arg(toFinal->x).arg(toFinal->y);

			if (fnEmplace(entity, toFinal.value(), to.a) && msg)
				msg->append("#");

			return cpvzero;
		} else {
			const int frames = to.f-snapshot.current;
			const cpVect pos = cpvlerp(entity->bodyPosition(), toFinal.value(), 1./frames);

			if (msg) *msg = QStringLiteral("![%1,%2]").arg(pos.x).arg(pos.y);

			return fnMove(entity, pos, frames, maxSpeed, from.cv);
		}
	}

	return cpvzero;
}




#endif // RPGGAMEDATAIFACE_T_H
