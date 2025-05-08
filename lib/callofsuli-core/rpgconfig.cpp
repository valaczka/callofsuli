/*
 * ---- Call of Suli ----
 *
 * rpgconfig.cpp
 *
 * Created on: 2025. 04. 02.
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


#include "rpgconfig.h"
#include <chipmunk/chipmunk.h>

namespace RpgGameData {

const QHash<Weapon::WeaponType, int> Weapon::m_damageValue = {
	{ Weapon::WeaponHand, 1 },
	{ Weapon::WeaponDagger, 3 },
	{ Weapon::WeaponLongsword, 10 },
	{ Weapon::WeaponHammer, 20 },
	{ Weapon::WeaponMace, 22 },
	{ Weapon::WeaponAxe, 25 },
	{ Weapon::WeaponBroadsword, 30 },
	{ Weapon::WeaponShortbow, 10 },
	{ Weapon::WeaponLongbow, 25 },
	{ Weapon::WeaponGreatHand, 50 },
	{ Weapon::WeaponLightningWeapon, 40 },
	{ Weapon::WeaponFireFogWeapon, 60 },
};


const QHash<Weapon::WeaponType, int> Weapon::m_protectValue = {
	{ Weapon::WeaponShield, 10 },
};









/**
 * @brief SnapshotStorage::saveLastTick
 * @param dst
 * @param sip
 */

template<typename T, typename T2, typename T3, typename T4>
void SnapshotStorage::saveLastTick(SnapshotData<T, T2> *dst, const SnapshotInterpolation<T> &sip)
{
	Q_ASSERT(dst);

	qint64 last = sip.s2.f > -1 ?
					  std::max(sip.s2.f, sip.last.f) :
					  sip.last.f
					  ;

	if (last > dst->lastFullSnap)
		dst->lastFullSnap = last;
}





/**
 * @brief SnapshotStorage::addFullSnapshot
 * @param dst
 * @param src
 * @param findLast
 */

template<typename T, typename T2, typename T3, typename T4>
void SnapshotStorage::addFullSnapshot(SnapshotInterpolationList<T2, T> *dst, SnapshotList<T, T2> &src,
									  const qint64 &currentTick, const bool &findLast)
{
	Q_ASSERT(dst);

	for (auto &ptr : src) {
		SnapshotInterpolation<T> sip = getSnapshotInterpolation(ptr.list, currentTick, findLast ? ptr.lastFullSnap : -1);
		if (sip.s1.f > -1 && sip.s1.f > ptr.lastFullSnap) {
			qWarning() << "Snapshot skipped" << ptr.data.o << ptr.data.s << ptr.data.id << "tick" << sip.s1.f << "vs" << ptr.lastFullSnap
					   << "====" << sip.current;
		}
		saveLastTick(&ptr, sip);
		dst->emplace_back(ptr.data, sip);
	}
}




/**
 * @brief ArmoredEntity::attack
 * @param src
 * @param weapon
 * @return
 */

void ArmoredEntity::attacked(const ArmoredEntityBaseData &dstBase, ArmoredEntity &dst,
							 const Weapon::WeaponType &weapon, const ArmoredEntityBaseData &other)
{
	float damage = Weapon::damageValue().value(weapon, 0) * other.df;

	if (damage <= 0.)
		return;

	for (auto it = dst.arm.wl.begin(); damage > 0. && it != dst.arm.wl.end(); ++it) {
		if (it->b == 0)
			continue;

		const float protect = Weapon::protectValue().value(it->t, 0) * dstBase.pf;
		if (protect <= 0.)
			continue;

		// Ha -1 töltény van, akkor a teljes támadást ki tudjuk védeni

		if (it->b == -1) {
			damage = 0.;
			break;
		} else {
			float sumProtect = std::min(protect * it->b, damage);
			damage -= sumProtect;
			it->b = std::max(0, it->b - (int) std::ceil(sumProtect / protect));
		}
	}

	if (damage <= 0.)
		return;

	dst.hp = std::max(0, (int) (dst.hp-damage));
}








/**
 * @brief SnapshotStorage::getFullSnapshot
 * @param tick
 * @return
 */

FullSnapshot SnapshotStorage::getFullSnapshot(const qint64 &tick, const bool &findLast)
{
	FullSnapshot s;

	addFullSnapshot(&s.players, m_players, tick, findLast);
	addFullSnapshot(&s.enemies, m_enemies, tick, findLast);
	addFullSnapshot(&s.bullets, m_bullets, tick, findLast);

	return s;
}







/**
 * @brief SnapshotStorage::getCurrentSnapshot
 * @return
 */

CurrentSnapshot SnapshotStorage::getCurrentSnapshot()
{
	CurrentSnapshot s;

	s.players = convertToSnapshotList(m_players);
	s.enemies = convertToSnapshotList(m_enemies);
	s.bullets = convertToSnapshotList(m_bullets);

	return s;
}



/**
 * @brief SnapshotStorage::zapSnapshots
 * @param tick
 */

void SnapshotStorage::zapSnapshots(const qint64 &tick)
{
	if (tick <= 0)
		return;

	for (auto &ptr : m_players) {
		zapSnapshots(ptr.list, tick);
	}

	for (auto &ptr : m_enemies) {
		zapSnapshots(ptr.list, tick);
	}

	for (auto &ptr : m_bullets) {
		zapSnapshots(ptr.list, tick);
	}
}




/**
 * @brief CurrentSnapshot::toCbor
 * @return
 */

QCborMap CurrentSnapshot::toCbor() const
{
	QCborMap map;

	if (const QCborArray &a = toCborArray(players, QStringLiteral("pd"), QStringLiteral("p"), nullptr); !a.isEmpty())
		map.insert(QStringLiteral("pp"), a);

	if (const QCborArray &a = toCborArray(enemies, QStringLiteral("ed"), QStringLiteral("e"), nullptr); !a.isEmpty())
		map.insert(QStringLiteral("ee"), a);

	if (const QCborArray &a = toCborArray(bullets, QStringLiteral("bd"), QStringLiteral("b"), nullptr); !a.isEmpty())
		map.insert(QStringLiteral("bb"), a);

	return map;
}





/**
 * @brief CurrentSnapshot::fromCbor
 * @param map
 * @return
 */

int CurrentSnapshot::fromCbor(const QCborMap &map)
{
	int r = 0;

	r += fromCborArray(players, map.value(QStringLiteral("pp")).toArray(), QStringLiteral("pd"), QStringLiteral("p"), nullptr);
	r += fromCborArray(enemies, map.value(QStringLiteral("ee")).toArray(), QStringLiteral("ed"), QStringLiteral("e"), nullptr);
	r += fromCborArray(bullets, map.value(QStringLiteral("bb")).toArray(), QStringLiteral("bd"), QStringLiteral("b"), nullptr);

	return r;
}



/**
 * @brief Entity::canInterpolateFrom
 * @param other
 * @return
 */

bool Entity::canInterpolateFrom(const Entity &other) const {
	if (!Body::canInterpolateFrom(other) ||
			hp != other.hp ||
			mhp != other.mhp ||
			a != other.a)
		return false;

	if (other.cv.size() < 2)
		return p == other.p;

	const cpVect vel = cpv(other.cv.at(0), other.cv.at(1));

	if (vel == cpvzero)
		return p == other.p;

	if (other.p.size() < 2 || p.size() < 2)
		return false;

	const cpVect curr = cpv(p.at(0), p.at(1));

	return curr == cpvadd(cpv(other.p.at(0), other.p.at(1)),
						  cpvmult(vel, (f-other.f)/60.));

}



/**
 * @brief LifeCycle::destroy
 * @param tick
 */

void LifeCycle::destroy(const qint64 &tick)
{
	setStage(StageDestroy);

	if (m_destroyTick == -1)
		m_destroyTick = tick;
}





}
