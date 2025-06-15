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
#include <QRandomGenerator>
#include <random>


#define STD_DEV_MAX_ATTEMPTS			10000
#define TARGET_STD_DEV					1.5

#define PICKABLE_HP_VALUE				10
#define PICKABLE_SHORTBOW_VALUE			10
#define PICKABLE_LONGBOW_VALUE			10
#define PICKABLE_LONGSWORD_VALUE		10
#define PICKABLE_DAGGER_VALUE			1
#define PICKABLE_SHIELD_VALUE			2


const QHash<RpgConfig::ControlType, int> RpgConfig::m_controlDamageValue = {
	{ RpgConfig::ControlContainer, 15 },
	{ RpgConfig::ControlCollection, 5 },
};



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
		if (sip.s1.f > -1 && sip.s1.f > ptr.lastFullSnap && ptr.lastFullSnap > -1) {
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

	addFullSnapshot(&s.controls.lights, m_controls.lights, tick, findLast);
	addFullSnapshot(&s.controls.containers, m_controls.containers, tick, findLast);
	addFullSnapshot(&s.controls.collections, m_controls.collections, tick, findLast);
	addFullSnapshot(&s.controls.pickables, m_controls.pickables, tick, findLast);
	addFullSnapshot(&s.controls.gates, m_controls.gates, tick, findLast);
	addFullSnapshot(&s.controls.teleports, m_controls.teleports, tick, findLast);


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

	s.controls.lights = convertToSnapshotList(m_controls.lights);
	s.controls.containers = convertToSnapshotList(m_controls.containers);
	s.controls.collections = convertToSnapshotList(m_controls.collections);
	s.controls.pickables = convertToSnapshotList(m_controls.pickables);
	s.controls.gates = convertToSnapshotList(m_controls.gates);
	s.controls.teleports = convertToSnapshotList(m_controls.teleports);

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

	for (auto &ptr : m_players)
		zapSnapshots(ptr.list, tick);

	for (auto &ptr : m_enemies)
		zapSnapshots(ptr.list, tick);

	for (auto &ptr : m_bullets)
		zapSnapshots(ptr.list, tick);


	for (auto &ptr : m_controls.lights)
		zapSnapshots(ptr.list, tick);

	for (auto &ptr : m_controls.containers)
		zapSnapshots(ptr.list, tick);

	for (auto &ptr : m_controls.collections)
		zapSnapshots(ptr.list, tick);

	for (auto &ptr : m_controls.pickables)
		zapSnapshots(ptr.list, tick);

	for (auto &ptr : m_controls.gates)
		zapSnapshots(ptr.list, tick);

	for (auto &ptr : m_controls.teleports)
		zapSnapshots(ptr.list, tick);
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


	if (const QCborArray &a = toCborArray(controls.lights, QStringLiteral("cd"), QStringLiteral("c"), nullptr); !a.isEmpty())
		map.insert(QStringLiteral("cl"), a);

	if (const QCborArray &a = toCborArray(controls.containers, QStringLiteral("cd"), QStringLiteral("c"), nullptr); !a.isEmpty())
		map.insert(QStringLiteral("cc"), a);

	if (const QCborArray &a = toCborArray(controls.collections, QStringLiteral("cd"), QStringLiteral("c"), nullptr); !a.isEmpty())
		map.insert(QStringLiteral("cs"), a);

	if (const QCborArray &a = toCborArray(controls.pickables, QStringLiteral("cd"), QStringLiteral("c"), nullptr); !a.isEmpty())
		map.insert(QStringLiteral("cp"), a);

	if (const QCborArray &a = toCborArray(controls.gates, QStringLiteral("cd"), QStringLiteral("c"), nullptr); !a.isEmpty())
		map.insert(QStringLiteral("cg"), a);

	if (const QCborArray &a = toCborArray(controls.teleports, QStringLiteral("cd"), QStringLiteral("c"), nullptr); !a.isEmpty())
		map.insert(QStringLiteral("ct"), a);

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

	r += fromCborArray(controls.lights, map.value(QStringLiteral("cl")).toArray(), QStringLiteral("cd"), QStringLiteral("c"), nullptr);
	r += fromCborArray(controls.containers, map.value(QStringLiteral("cc")).toArray(), QStringLiteral("cd"), QStringLiteral("c"), nullptr);
	r += fromCborArray(controls.collections, map.value(QStringLiteral("cs")).toArray(), QStringLiteral("cd"), QStringLiteral("c"), nullptr);
	r += fromCborArray(controls.pickables, map.value(QStringLiteral("cp")).toArray(), QStringLiteral("cd"), QStringLiteral("c"), nullptr);
	r += fromCborArray(controls.gates, map.value(QStringLiteral("cg")).toArray(), QStringLiteral("cd"), QStringLiteral("c"), nullptr);
	r += fromCborArray(controls.teleports, map.value(QStringLiteral("ct")).toArray(), QStringLiteral("cd"), QStringLiteral("c"), nullptr);

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




/**
 * @brief Player::controlFailed
 * @param dstBase
 * @param dst
 * @param control
 */

void Player::controlFailed(Player &dst, const RpgConfig::ControlType &control)
{
	float damage = RpgConfig::controlDamageValue().value(control, 0);

	if (damage <= 0.)
		return;

	dst.hp = std::max(0, (int) (dst.hp-damage));
}





/**
 * @brief Player::pick
 * @param dst
 * @param type
 * @return
 */

bool Player::pick(Player &dst, const PickableBaseData::PickableType &type, const QString &name)
{
	switch (type) {
		case PickableBaseData::PickableHp:
			dst.hp += PICKABLE_HP_VALUE;
			break;

		case PickableBaseData::PickableShortbow:
			dst.arm.add(Weapon::WeaponShortbow, PICKABLE_SHORTBOW_VALUE);
			break;

		case PickableBaseData::PickableLongbow:
			dst.arm.add(Weapon::WeaponShortbow, PICKABLE_LONGBOW_VALUE);
			break;

		case PickableBaseData::PickableLongsword:
			dst.arm.add(Weapon::WeaponShortbow, PICKABLE_LONGSWORD_VALUE);
			break;

		case PickableBaseData::PickableDagger:
			dst.arm.add(Weapon::WeaponShortbow, PICKABLE_DAGGER_VALUE);
			break;

		case PickableBaseData::PickableShield:
			dst.arm.add(Weapon::WeaponShortbow, PICKABLE_SHIELD_VALUE);
			break;



		case PickableBaseData::PickableKey:
			dst.inv.add(type, 1, name);
			break;

		case PickableBaseData::PickableTime:
		case PickableBaseData::PickableMp:
		case PickableBaseData::PickableCoin:
		case PickableBaseData::PickableInvalid:
			return false;
	}

	return true;
}



/**
 * @brief Player::useTeleport
 * @param dst
 * @param base
 * @return
 */

bool Player::useTeleport(Player &dst, const ControlTeleportBaseData &base)
{
	if (base.dst.isValid()) {
		qWarning() << "Missing implementation";
		return false;
	}

	dst.pck = base;

	return true;
}



/**
 * @brief Collection::allocate
 * @param num
 * @return
 */

QHash<int, QList<int> > Collection::allocate(const int &num, int *dst)
{
	if (dst)
		*dst = 0;

	int max = 0;

	for (CollectionGroup &g : groups) {
		max += g.pos.size();

		for (CollectionPlace &p : g.pos) {
			p.done = false;
		}
	}

	if (num == 0)
		return {};

	if (max == 0) {
		qWarning() << "Empty CollectionGroup";
		return {};
	}

	const float ratio = (float) num / (float) max;

	QHash<int, QList<int> > ret;

	int generated = 0;

	std::random_device rd;
	std::mt19937 randomEngine(rd());

	for (CollectionGroup &g : groups) {
		std::vector<int> idx(g.pos.size());
		std::iota(idx.begin(), idx.end(), 0);
		std::shuffle(idx.begin(), idx.end(), randomEngine);

		QList<int> list;
		list.reserve(g.pos.size());

		const int lmax = std::ceil(g.pos.size() * ratio);

		auto it = idx.cbegin();

		while (list.size() < g.pos.size()-2 &&
			   list.size() < lmax &&
			   generated < num) {
			list.append(*it);
			++it;
			++generated;
		}

		ret.insert(g.id, list);
	}

	qInfo() << "GENREATED" << generated << "required" << num;
	qDebug() << ret;

	if (dst)
		*dst = generated;

	return ret;
}


/**
 * @brief Collection::getFree
 * @return
 */

QList<CollectionPlace> Collection::getFree(const int &gid) const
{
	const auto &it = find(gid);

	if (it == groups.cend())
		return {};

	QList<CollectionPlace> ret;
	ret.reserve(it->pos.size());

	for (const CollectionPlace &p : it->pos) {
		if (!p.done)
			ret.append(p);
	}

	return ret;
}





/**
 * @brief Collection::find
 * @param id
 * @return
 */

QList<CollectionGroup>::iterator Collection::find(const int &id)
{
	return std::find_if(groups.begin(),
						groups.end(),
						[&id](const CollectionGroup &g){
		return g.id == id;
	});
}



/**
 * @brief Collection::find
 * @param id
 * @return
 */

QList<CollectionGroup>::const_iterator Collection::find(const int &id) const
{
	return std::find_if(groups.cbegin(),
						groups.cend(),
						[&id](const CollectionGroup &g){
		return g.id == id;
	});
}


/**
 * @brief ControlActive::unlock
 * @param ownData
 * @param inventory
 * @return
 */

bool ControlActive::unlock(const ControlActiveBaseData &ownData, const Inventory &inventory, const bool &toLocked)
{
	if (ownData.lck.isEmpty()) {
		lck = toLocked;
		return true;
	}

	if (inventory.contains(PickableBaseData::PickableKey, ownData.lck)) {
		lck = toLocked;
		return true;
	}

	return false;
}


/**
 * @brief ControlGate::unlock
 * @param ownData
 * @param inventory
 * @param toLocked
 * @return
 */

bool ControlGate::unlock(const ControlGateBaseData &ownData, const Inventory &inventory, const bool &toLocked)
{
	if (st == GateDamaged)
		return false;

	return ControlActive::unlock(ownData, inventory, toLocked);
}





/**
 * @brief Randomizer::find
 * @param id
 * @return
 */

QList<RandomizerGroup>::const_iterator Randomizer::find(const int &id) const
{
	return std::find_if(groups.cbegin(),
						groups.cend(),
						[&id](const RandomizerGroup &g){
		return g.gid == id;
	});
}




/**
 * @brief Randomizer::randomize
 */

void Randomizer::randomize()
{
	for (RandomizerGroup &g : groups) {
		g.randomize();
	}
}





/**
 * @brief Randomizer::find
 * @param id
 * @return
 */

QList<RandomizerGroup>::iterator Randomizer::find(const int &id)
{
	return std::find_if(groups.begin(),
						groups.end(),
						[&id](const RandomizerGroup &g){
		return g.gid == id;
	});
}



/**
 * @brief RandomizerGroup::randomize
 */

void RandomizerGroup::randomize()
{
	if (current >= 0)
		return;

	if (idList.empty()) {
		current = -1;
		return;
	}

	current = idList.at(QRandomGenerator::global()->bounded(idList.size()));
}





/**
 * @brief PlayerBaseData::assign
 * @param dst
 * @param num
 */

void PlayerBaseData::assign(const QList<PlayerBaseData *> &dst, const int &num)
{
	if (dst.empty() || num <= dst.size())
		return;

	const int size = dst.size();

	static const auto standard_deviation = [](const std::vector<int>& data) -> double {
		double mean = accumulate(data.begin(), data.end(), 0.0) / data.size();
		double sum_sq_diff = 0.0;
		for (int val : data) {
			sum_sq_diff += (val - mean) * (val - mean);
		}
		return sqrt(sum_sq_diff / data.size());
	};

	std::random_device rd;
	std::mt19937 gen(rd());

	std::vector<int> dist;			// destination
	const int remainder = num % size;


	try {
		std::vector<int> tmp;

		for (int attempt = 0; attempt < STD_DEV_MAX_ATTEMPTS; ++attempt) {
			std::vector<int> base(size, num / size);

			// Véletlenszerűen szétosztjuk a maradékot

			std::vector<int> indices(size);
			std::iota(indices.begin(), indices.end(), 0);
			std::shuffle(indices.begin(), indices.end(), gen);
			for (int i = 0; i < remainder; ++i) {
				base[indices[i]] += 1;
			}

			// Másolat, amin a véletlen szórásnövelés történik

			dist = base;

			if (size * 2 >= num) {
				qWarning() << "Not enough target" << num << "vs." << size;
				throw 1;
			}

			tmp = base;

			// Véletlenszerű átcsoportosítások

			std::uniform_int_distribution<> dis(0, size - 1);
			for (int i = 0; i < 10 * size; ++i) {
				int from = dis(gen);
				int to = dis(gen);
				if (from != to && dist[from] > 0) {
					dist[from] -= 1;
					dist[to] += 1;
				}
			}

			double std = standard_deviation(dist);
			if (abs(std - TARGET_STD_DEV) < 0.1) {
				throw 1;
			}
		}

		qWarning() << "Standard deviation calculation failed";

		dist = tmp;

		// dist ok

	} catch (int e) {
		// dist ok
	}

	for (int i=0; i<(int) dist.size() && i<dst.size(); ++i) {
		dst.at(i)->rq = dist.at(i);
	}
}




}
