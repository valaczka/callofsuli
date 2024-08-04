/*
 * ---- Call of Suli ----
 *
 * rpguserwallet.cpp
 *
 * Created on: 2024. 07. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgUserWallet
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

#include "rpguserwallet.h"
#include "Logger.h"
#include "application.h"
#include "client.h"
#include "rpgarmory.h"
#include "rpgpickableobject.h"
#include "rpggame.h"
#include "utils_.h"

RpgUserWallet::RpgUserWallet(QObject *parent)
	: QObject{parent}
{

}


/**
 * @brief RpgUserWallet::getJson
 * @return
 */

QJsonObject RpgUserWallet::getJson() const
{
	RpgWallet w;
	w.type = m_market.type;
	w.name = m_market.name;
	w.amount = 1;
	return w.toJson();
}


/**
 * @brief RpgUserWallet::market
 * @return
 */

RpgMarket RpgUserWallet::market() const
{
	return m_market;
}

void RpgUserWallet::setMarket(const RpgMarket &newMarket)
{
	if (m_market == newMarket)
		return;
	m_market = newMarket;
	emit marketChanged();
	emit marketTypeChanged();
}

int RpgUserWallet::amount() const
{
	return m_amount;
}

void RpgUserWallet::setAmount(int newAmount)
{
	if (m_amount == newAmount)
		return;
	m_amount = newAmount;
	emit amountChanged();
}

QDateTime RpgUserWallet::expiry() const
{
	return m_expiry;
}

void RpgUserWallet::setExpiry(const QDateTime &newExpiry)
{
	if (m_expiry == newExpiry)
		return;
	m_expiry = newExpiry;
	emit expiryChanged();
}

RpgUserWallet *RpgUserWallet::bullet() const
{
	return m_bullet;
}

void RpgUserWallet::setBullet(RpgUserWallet *newBullet)
{
	if (m_bullet == newBullet)
		return;
	m_bullet = newBullet;
	emit bulletChanged();
}


/**
 * @brief RpgUserWallet::available
 * @return
 */

bool RpgUserWallet::available() const
{
	return (m_amount > 0 && (m_expiry.isNull() || m_expiry > QDateTime::currentDateTime())) ||
			(m_market.cost == 0 && m_market.rank == 0);
}



/**
 * @brief RpgUserWallet::marketType
 * @return
 */

RpgMarket::Type RpgUserWallet::marketType() const
{
	switch (m_market.type) {
		case RpgMarket::Time:
		case RpgMarket::Hp:
		case RpgMarket::Mp:
		case RpgMarket::Pickable:
		case RpgMarket::Other:
			return RpgMarket::Other;

		default:
			return m_market.type;
	}

	return m_market.type;
}


/**
 * @brief RpgUserWallet::readableName
 * @return
 */

QString RpgUserWallet::readableName() const
{
	return m_readableName;
}

void RpgUserWallet::setReadableName(const QString &newReadableName)
{
	if (m_readableName == newReadableName)
		return;
	m_readableName = newReadableName;
	emit readableNameChanged();
}

QString RpgUserWallet::image() const
{
	return m_image;
}

void RpgUserWallet::setImage(const QString &newImage)
{
	if (m_image == newImage)
		return;
	m_image = newImage;
	emit imageChanged();
}

QString RpgUserWallet::sortName() const
{
	return m_sortName;
}

void RpgUserWallet::setSortName(const QString &newSortName)
{
	if (m_sortName == newSortName)
		return;
	m_sortName = newSortName;
	emit sortNameChanged();
}

Rank RpgUserWallet::rank() const
{
	return m_rank;
}

void RpgUserWallet::setRank(const Rank &newRank)
{
	if (m_rank == newRank)
		return;
	m_rank = newRank;
	emit rankChanged();
}


/**
 * @brief RpgUserWallet::buyable
 * @return
 */

bool RpgUserWallet::buyable() const
{
	const bool a = available();

	if (m_market.type == RpgMarket::Weapon && a)
		return false;

	if (m_market.type == RpgMarket::Map && a)
		return false;

	if (m_market.type == RpgMarket::Skin && a)
		return false;


	if (m_market.rollover != RpgMarket::None && m_amount >= m_market.num)
		return false;

	Server *s = Application::instance()->client()->server();

	if (!s)
		return false;

	return m_market.rank <= 0 || s->user()->rank().id() >= m_market.rank;
}


/**
 * @brief RpgUserWallet::extendedInfo
 * @return
 */

QList<RpgMarketExtendedInfo> RpgUserWallet::extendedInfo() const
{
	return m_extendedInfo;
}


void RpgUserWallet::setExtendedInfo(const QList<RpgMarketExtendedInfo> &newExtendedInfo)
{
	if (m_extendedInfo == newExtendedInfo)
		return;
	m_extendedInfo = newExtendedInfo;
	emit extendedInfoChanged();
}


/**
 * @brief RpgUserWallet::getExtendedInfo
 * @param def
 * @return
 */

QList<RpgMarketExtendedInfo> RpgUserWallet::getExtendedInfo(const RpgGameDefinition &def)
{
	QList<RpgMarketExtendedInfo> list;

	for (const RpgQuest &q : def.quests) {
		if (q.type == RpgQuest::SuddenDeath) {
			list.append(RpgMarketExtendedInfo{
							QStringLiteral("qrc:/Qaterial/Icons/skull-scan-outline.svg"),
							tr("Sudden death quest:"),
							QString::number(q.currency),
							QStringLiteral("qrc:/rpg/coin/coin.png"),
						});
		}
	}


	return list;
}


/**
 * @brief RpgUserWallet::getExtendedInfo
 * @param market
 * @return
 */

QList<RpgMarketExtendedInfo> RpgUserWallet::getExtendedInfo(const RpgMarket &market)
{
	QList<RpgMarketExtendedInfo> list;

	if (market.type == RpgMarket::Map) {
		if (market.info.isEmpty())
			return list;

		if (const int v = market.info.value("duration").toInt(); v > 0)
			list.append(RpgMarketExtendedInfo{
							QStringLiteral("qrc:/Qaterial/Icons/timer-sand.svg"),
							tr("Időtartam:"),
							Utils::formatMSecs(v*1000),
						});

		if (const int v = market.info.value("enemyCount").toInt(); v > 0)
			list.append(RpgMarketExtendedInfo{
							QStringLiteral("qrc:/Qaterial/Icons/target-account.svg"),
							tr("Ellenfél:"),
							QString::number(v)
						});

		if (const int v = market.info.value("mpCount").toInt(); v > 0)
			list.append(RpgMarketExtendedInfo{
							QStringLiteral("qrc:/Qaterial/Icons/shimmer.svg"),
							tr("Elhelyezett MP:"),
							QString::number(v)
						});

		if (const int v = market.info.value("currencyCount").toInt(); v > 0)
			list.append(RpgMarketExtendedInfo{
							QStringLiteral("qrc:/Qaterial/Icons/cash-usd-outline.svg"),
							tr("Elhelyezett pénz:"),
							QString::number(v),
							QStringLiteral("qrc:/rpg/coin/coin.png"),
						});

		if (market.info.value("hasMarket").toBool())
			list.append(RpgMarketExtendedInfo{
							QStringLiteral("qrc:/Qaterial/Icons/bank.svg"),
							tr("Bank")
						});

	} else if (market.type == RpgMarket::Bullet) {
		TiledWeapon::WeaponType w = TiledWeapon::WeaponInvalid;

		switch (RpgPickableObject::typeFromString(market.name)) {
			case RpgPickableObject::PickableArrow:
				w = TiledWeapon::WeaponShortbow;
				break;

			case RpgPickableObject::PickableFireball:
				w = TiledWeapon::WeaponLongbow;
				break;

			case RpgPickableObject::PickableShield:
			case RpgPickableObject::PickableKey:
			case RpgPickableObject::PickableHp:
			case RpgPickableObject::PickableMp:
			case RpgPickableObject::PickableCoin:
			case RpgPickableObject::PickableShortbow:
			case RpgPickableObject::PickableLongbow:
			case RpgPickableObject::PickableLongsword:
			case RpgPickableObject::PickableDagger:
			case RpgPickableObject::PickableTime:
			case RpgPickableObject::PickableLightning:
			case RpgPickableObject::PickableInvalid:
				break;
		}

		if (w != TiledWeapon::WeaponInvalid)
			list.append(RpgMarketExtendedInfo{
							QStringLiteral("qrc:/Qaterial/Icons/sword-cross.svg"),
							tr("Fegyver:"),
							TiledWeapon::weaponNameEn(w),
							QStringLiteral("qrc:/rpg/")+RpgArmory::weaponHash().value(w)+ QStringLiteral("/market.jpg"),
							60
						});

	}

	return list;
}


/**
 * @brief RpgUserWallet::getExtendedInfo
 * @param player
 * @return
 */

QList<RpgMarketExtendedInfo> RpgUserWallet::getExtendedInfo(const RpgPlayerCharacterConfig &player)
{
	QList<RpgMarketExtendedInfo> list;

	if (player.cast != RpgPlayerCharacterConfig::CastInvalid) {
		if (player.mpMax > 0)
			list.append(RpgMarketExtendedInfo{
							QStringLiteral("qrc:/Qaterial/Icons/shimmer.svg"),
							tr("Max. MP:"),
							QString::number(player.mpMax)
						});

		QString name;
		QString image;

		switch (player.cast) {
			case RpgPlayerCharacterConfig::CastInvisible:
				name = tr("Invisibility");
				image = QStringLiteral("qrc:/rpg/castIcon/invisibility.png");
				break;

			case RpgPlayerCharacterConfig::CastFireball:
				name = tr("Fireball");
				image = QStringLiteral("qrc:/rpg/castIcon/fireball.png");
				break;

			case RpgPlayerCharacterConfig::CastFireballTriple:
				name = tr("Triple fireball");
				image = QStringLiteral("qrc:/rpg/castIcon/fireball-3.png");
				break;

			case RpgPlayerCharacterConfig::CastLightning:
				name = tr("Lightning");
				image = QStringLiteral("qrc:/rpg/castIcon/lightning.png");
				break;

			case RpgPlayerCharacterConfig::CastFireFog:
				name = tr("Fire fog");
				image = QStringLiteral("qrc:/rpg/castIcon/firefog.png");
				break;

			case RpgPlayerCharacterConfig::CastArrowQuintuple:
				name = tr("5x arrow");
				image = QStringLiteral("qrc:/rpg/castIcon/arrow-5.png");
				break;

			case RpgPlayerCharacterConfig::CastProtect:
				name = tr("Protect");
				image = QStringLiteral("qrc:/rpg/castIcon/protect.png");
				break;

			case RpgPlayerCharacterConfig::CastInvalid:
				break;
		}

		if (!name.isEmpty())
			list.append(RpgMarketExtendedInfo{
							{},
							tr("Super power:"),
							name,
							image,
							60
						});
	}

	return list;
}




/**
 * @brief RpgUserWalletList::RpgUserWalletList
 * @param parent
 * @param exposedRoles
 * @param displayRole
 */

RpgUserWalletList::RpgUserWalletList(QObject *parent)
	: qolm::QOlm<RpgUserWallet>(parent)
{
}


/**
 * @brief RpgUserWalletList::~RpgUserWalletList
 */

RpgUserWalletList::~RpgUserWalletList()
{
}



/**
 * @brief RpgUserWalletList::reload
 */

void RpgUserWalletList::reload()
{
	LOG_CDEBUG("client") << "Reload user wallet";

	reloadMarket();
}


/**
 * @brief RpgUserWalletList::reloadMarket
 */

void RpgUserWalletList::reloadMarket()
{
	Application::instance()->client()->send(HttpConnection::ApiGeneral, QStringLiteral("market"))
			->done(this, &RpgUserWalletList::loadMarket)
			->fail(this, [](const QString &err){
		LOG_CERROR("game") << "Load market error" << qPrintable(err);
	});
}



/**
 * @brief RpgUserWalletList::reloadWallet
 */

void RpgUserWalletList::reloadWallet()
{
	Application::instance()->client()->send(HttpConnection::ApiUser, QStringLiteral("wallet"))
			->done(this, &RpgUserWalletList::loadWallet)
			->fail(this, [](const QString &err){
		LOG_CERROR("game") << "Load wallet error" << qPrintable(err);
	});
}


/**
 * @brief RpgUserWalletList::loadMarket
 * @param json
 */

void RpgUserWalletList::loadMarket(const QJsonObject &json)
{
	const QJsonArray &list = json.value(QStringLiteral("list")).toArray();

	QVector<RpgMarket> mList;

	for (const QJsonValue &v : list) {
		RpgMarket market;
		market.fromJson(v);

		if (market.type == RpgMarket::Invalid) {
			LOG_CERROR("game") << "Load market error: invalid type";
			continue;
		}

		updateMarket(market);

		mList.append(market);
	}

	removeMissing(mList);
	updateBullets();

	reloadWallet();
}



/**
 * @brief RpgUserWalletList::loadWallet
 * @param json
 */

void RpgUserWalletList::loadWallet(const QJsonObject &json)
{
	const QJsonArray &list = json.value(QStringLiteral("list")).toArray();

	if (json.contains(QStringLiteral("currency")))
		setCurrency(json.value(QStringLiteral("currency")).toInt());

	QVector<RpgWallet> mList;

	for (const QJsonValue &v : list) {
		RpgWallet wallet;
		wallet.fromJson(v);

		if (wallet.type == RpgMarket::Invalid) {
			LOG_CERROR("game") << "Load wallet error: invalid type";
			continue;
		}

		updateMarket(wallet);

		mList.append(wallet);
	}

	for (RpgUserWallet *w : *this) {
		const RpgMarket &market = w->market();
		const bool found = std::find_if(mList.cbegin(), mList.cend(), [&market](const RpgWallet &m){
			return market.type == m.type && market.name == m.name;
		}) != mList.cend();

		if (!found) {
			w->setAmount(0);
			emit w->availableChanged();
		}
	}

	emit reloaded();
}




/**
 * @brief RpgUserWalletList::updateMarket
 * @param market
 */

void RpgUserWalletList::updateMarket(const RpgMarket &market)
{
	const auto it = std::find_if(begin(), end(), [&market](RpgUserWallet *w) {
					return (w->market().type == market.type && w->market().name == market.name);
});

	RpgUserWallet *ptr = nullptr;

	if (it == end()) {
		std::unique_ptr<RpgUserWallet> w(new RpgUserWallet);
		w->setMarket(market);
		w->m_walletList = this;

		QList<RpgMarketExtendedInfo> info;

		if (market.type == RpgMarket::Map) {
			const auto t = RpgGame::terrains().find(market.name);

			if (t == RpgGame::terrains().constEnd()) {
				LOG_CTRACE("game") << "Terrain not found:" << market.name;
				return;
			}

			w->setReadableName(t->name);
			w->setSortName(market.name);

			info.append(RpgUserWallet::getExtendedInfo(*t));

		} else if (market.type == RpgMarket::Skin) {
			const auto t = RpgGame::characters().find(market.name);

			if (t == RpgGame::characters().constEnd()) {
				LOG_CTRACE("game") << "Character not found:" << market.name;
				return;
			}

			w->setReadableName(t->name);
			w->setSortName(market.name);
			w->setImage(t->image);

			info.append(RpgUserWallet::getExtendedInfo(*t));

		} else if (market.type == RpgMarket::Weapon) {
			const auto t = RpgArmory::weaponHash().key(market.name, TiledWeapon::WeaponInvalid);

			if (t == TiledWeapon::WeaponInvalid) {
				LOG_CTRACE("game") << "Weapon not found:" << market.name;
				return;
			}

			w->setReadableName(TiledWeapon::weaponNameEn(t));
			w->setSortName(QStringLiteral("%1").arg(t, 2, u'0'));

		} else if (market.type == RpgMarket::Bullet) {
			const auto t = RpgPickableObject::typeFromString(market.name);

			if (t == RpgPickableObject::PickableInvalid) {
				LOG_CTRACE("game") << "Weapon not found:" << market.name;
				return;
			}

			w->setReadableName(RpgPickableObject::pickableNameEn(t));
			w->setSortName(QStringLiteral("%1").arg(t, 2, u'0'));
		} else if (market.type == RpgMarket::Hp) {
			w->setReadableName(tr("HP"));
		} else if (market.type == RpgMarket::Time) {
			w->setReadableName(tr("Second"));
		} else if (market.type == RpgMarket::Xp) {
			w->setReadableName(tr("XP"));
		} else if (market.type == RpgMarket::Mp) {
			w->setReadableName(tr("MP"));
		}

		info.append(RpgUserWallet::getExtendedInfo(market));

		w->setExtendedInfo(info);

		ptr = w.release();
		this->append(ptr);
	} else {
		ptr = *it;
		ptr->setMarket(market);
	}

	if (!ptr)
		return;

	ptr->setRank(Application::instance()->client()->server()->rank(market.rank));
	emit ptr->buyableChanged();

	if (market.type == RpgMarket::Map) {
		for (const QString &s : QStringList{
			 QStringLiteral(":/map/")+market.name+QStringLiteral("/thumbnail.png"),
			 QStringLiteral(":/map/")+market.name+QStringLiteral("/thumbnail.jpg"),
	}
			 ) {
			if (QFile::exists(s)) {
				ptr->setImage(QStringLiteral("qrc")+s);
				break;
			}
		}
	} else if (market.type == RpgMarket::Weapon || market.type == RpgMarket::Bullet) {
		if (QString s = QStringLiteral(":/rpg/")+market.name+QStringLiteral("/market.jpg"); QFile::exists(s)) {
			ptr->setImage(QStringLiteral("qrc")+s);
		}
	} else if (market.type == RpgMarket::Xp || market.type == RpgMarket::Hp ||
			   market.type == RpgMarket::Time || market.type == RpgMarket::Mp) {
		for (const QString &s : QStringList{
			 QStringLiteral(":/rpg/")+market.name+QStringLiteral("/market.png"),
			 QStringLiteral(":/rpg/")+market.name+QStringLiteral("/market.jpg"),
	}
			 ) {
			if (QFile::exists(s)) {
				ptr->setImage(QStringLiteral("qrc")+s);
				break;
			}
		}
	}
}


/**
 * @brief RpgUserWalletList::updateMarket
 * @param wallet
 */

void RpgUserWalletList::updateMarket(const RpgWallet &wallet)
{
	const auto it = std::find_if(begin(), end(), [&wallet](RpgUserWallet *w) {
					return (w->market().type == wallet.type && w->market().name == wallet.name);
});

	if (it == end()) {
		LOG_CTRACE("game") << "Load wallet error: missing market item:" << wallet.type << wallet.name;
		return;
	}

	(*it)->setAmount(wallet.amount);
	(*it)->setExpiry(wallet.expiry > 0 ? QDateTime::fromSecsSinceEpoch(wallet.expiry) : QDateTime{});
	emit (*it)->availableChanged();
}


/**
 * @brief RpgUserWalletList::removeMissing
 * @param list
 */

void RpgUserWalletList::removeMissing(const QVector<RpgMarket> &list)
{
	QList<RpgUserWallet*> r;

	for (RpgUserWallet *w : *this) {
		const RpgMarket &market = w->market();
		const bool found = std::find_if(list.cbegin(), list.cend(), [&market](const RpgMarket &m){
			return market.type == m.type && market.name == m.name;
		}) != list.cend();

		if (!found)
			r.append(w);
	}

	for (RpgUserWallet *w : *this) {
		if (r.contains(w->bullet()))
			w->setBullet(nullptr);
	}

	this->remove(r);
}



/**
 * @brief RpgUserWalletList::updateBullets
 */

void RpgUserWalletList::updateBullets()
{
	const auto &hash = RpgArmory::weaponHash();

	for (RpgUserWallet *w : *this) {
		if (w->market().type != RpgMarket::Weapon) {
			w->setBullet(nullptr);
			continue;
		}

		const TiledWeapon::WeaponType weapon = hash.key(w->market().name, TiledWeapon::WeaponInvalid);
		RpgPickableObject::PickableType pickable = RpgPickableObject::PickableInvalid;


		switch (weapon) {
			case TiledWeapon::WeaponShortbow:
				pickable = RpgPickableObject::PickableArrow;
				break;

			case TiledWeapon::WeaponLongbow:
				pickable = RpgPickableObject::PickableFireball;
				break;

			case TiledWeapon::WeaponLongsword:
			case TiledWeapon::WeaponDagger:
			case TiledWeapon::WeaponBroadsword:
			case TiledWeapon::WeaponHand:
			case TiledWeapon::WeaponAxe:
			case TiledWeapon::WeaponMace:
			case TiledWeapon::WeaponHammer:
			case TiledWeapon::WeaponGreatHand:
			case TiledWeapon::WeaponShield:
			case TiledWeapon::WeaponMageStaff:
			case TiledWeapon::WeaponLightningWeapon:
			case TiledWeapon::WeaponFireFogWeapon:
				break;

			case TiledWeapon::WeaponInvalid:
				LOG_CERROR("client") << "Invalid weapon in market" << w->market().name;
				break;
		}


		if (pickable == RpgPickableObject::PickableInvalid)
			w->setBullet(nullptr);
		else {
			for (RpgUserWallet *w2 : *this) {
				if (w2->market().type == RpgMarket::Bullet && RpgPickableObject::typeFromString(w2->market().name) == pickable) {
					w->setBullet(w2);
					break;
				}
			}
		}
	}
}

int RpgUserWalletList::currency() const
{
	return m_currency;
}

void RpgUserWalletList::setCurrency(int newCurrency)
{
	if (m_currency == newCurrency)
		return;
	m_currency = newCurrency;
	emit currencyChanged();
}
