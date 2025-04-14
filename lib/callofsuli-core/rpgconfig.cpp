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

FullSnapshot SnapshotStorage::getFullSnapshot(const qint64 &tick)
{
	QMutexLocker locker(&m_mutex);

	FullSnapshot s;

	for (auto &ptr : m_players) {
		SnapshotInterpolation<RpgGameData::Player> sip = getSnapshotInterpolation(ptr.list, tick, ptr.lastOut);
		s.players.emplace_back(ptr.data, sip);
		if (sip.s2.f >= 0)
			ptr.lastOut = sip.s2.f;
	}

	for (auto &ptr : m_enemies) {
		SnapshotInterpolation<RpgGameData::Enemy> sip = getSnapshotInterpolation(ptr.list, tick, ptr.lastOut);
		s.enemies.emplace_back(ptr.data, sip);
		if (sip.s2.f >= 0)
			ptr.lastOut = sip.s2.f;
	}

	for (auto &ptr : m_bullets) {
		SnapshotInterpolation<RpgGameData::Bullet> sip = getSnapshotInterpolation(ptr.list, tick, ptr.lastOut);
		s.bullets.emplace_back(ptr.data, sip);
		if (sip.s2.f >= 0)
			ptr.lastOut = sip.s2.f;
	}

	return s;
}





/**
 * @brief SnapshotStorage::getCurrentSnapshot
 * @return
 */

CurrentSnapshot SnapshotStorage::getCurrentSnapshot()
{
	QMutexLocker locker(&m_mutex);

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
	QMutexLocker locker(&m_mutex);

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

	if (const QCborArray &a = toCborArray(players, QStringLiteral("pd"), QStringLiteral("p")); !a.isEmpty())
		map.insert(QStringLiteral("pp"), a);

	if (const QCborArray &a = toCborArray(enemies, QStringLiteral("ed"), QStringLiteral("e")); !a.isEmpty())
		map.insert(QStringLiteral("ee"), a);

	if (const QCborArray &a = toCborArray(bullets, QStringLiteral("bd"), QStringLiteral("b")); !a.isEmpty())
		map.insert(QStringLiteral("bb"), a);

	return map;
}



/**
 * @brief CurrentSnapshot::toProtectedCbor
 * @return
 */

QCborMap CurrentSnapshot::toProtectedCbor() const
{
	QCborMap map;

	if (const QCborArray &a = toProtectedCborArray(players, QStringLiteral("pd"), QStringLiteral("p"),
												   &CurrentSnapshot::removePlayerProtectedFields
												   ); !a.isEmpty())
		map.insert(QStringLiteral("pp"), a);

	if (const QCborArray &a = toProtectedCborArray(enemies, QStringLiteral("ed"), QStringLiteral("e"),
												   &CurrentSnapshot::removeEnemyProtectedFields
												   ); !a.isEmpty())
		map.insert(QStringLiteral("ee"), a);

	if (const QCborArray &a = toProtectedCborArray(bullets, QStringLiteral("bd"), QStringLiteral("b"),
												   &CurrentSnapshot::removeBulletProtectedFields
												   ); !a.isEmpty())
		map.insert(QStringLiteral("bb"), a);

	return map;
}


/**
 * @brief CurrentSnapshot::removeEntityProtectedFields
 * @param map
 */

void CurrentSnapshot::removeEntityProtectedFields(QCborMap *map)
{
	Q_ASSERT(map);

	map->remove(QStringLiteral("hp"));
	map->remove(QStringLiteral("mhp"));
}


/**
 * @brief CurrentSnapshot::removeArmoredEntityProtectedFields
 * @param map
 */

void CurrentSnapshot::removeArmoredEntityProtectedFields(QCborMap *map)
{
	Q_ASSERT(map);
	removeEntityProtectedFields(map);

	if (auto it = map->find(QStringLiteral("arm")); it != map->end()) {
		QCborMap arm = it->toMap();
		arm.remove(QStringLiteral("wl"));
		map->insert(QStringLiteral("arm"), arm);
	}
}



/**
 * @brief CurrentSnapshot::removeEnemyProtectedFields
 * @param map
 */

void CurrentSnapshot::removeEnemyProtectedFields(QCborMap *map)
{
	Q_ASSERT(map);
	removeArmoredEntityProtectedFields(map);
}



/**
 * @brief CurrentSnapshot::removePlayerProtectedFields
 * @param map
 */

void CurrentSnapshot::removePlayerProtectedFields(QCborMap *map)
{
	Q_ASSERT(map);
	removeArmoredEntityProtectedFields(map);
}


/**
 * @brief CurrentSnapshot::removeBulletProtectedFields
 * @param map
 */

void CurrentSnapshot::removeBulletProtectedFields(QCborMap *map)
{
	Q_ASSERT(map);
}





}
