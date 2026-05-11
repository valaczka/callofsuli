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
#include "utils_.h"
#include "server.h"

RpgUserWallet::RpgUserWallet(QObject *parent)
	: QObject{parent}
{

}





/**
 * @brief RpgUserWallet::getBelongsTo
 * @return
 */

RpgUserWallet *RpgUserWallet::getBelongsTo() const
{
	/*if (m_market.belongs.isEmpty())
		return nullptr;

	const auto it = std::find_if(m_walletList->constBegin(),
								 m_walletList->constEnd(),
								 [this](RpgUserWallet *w) {
					return w->market().type == RpgMarket::Skin && w->market().name == m_market.belongs;
});

	if (it != m_walletList->constEnd())
		return *it;
	else*/
		return nullptr;
}






/**
 * @brief RpgUserWalletList::getWeaponImage
 * @param name
 * @param subType
 * @return
 */

QString RpgUserWalletList::getWeaponImage(const QString &name, const int &subType)
{
	QString img;

	if (subType > 0) {
		if (QString s = QStringLiteral(":/rpg/")+name+QStringLiteral("/market%1.jpg").arg(subType); QFile::exists(s))
			img = QStringLiteral("qrc")+s;
	}

	if (img.isEmpty()) {
		if (QString s = QStringLiteral(":/rpg/")+name+QStringLiteral("/market.jpg"); QFile::exists(s))
			img = QStringLiteral("qrc")+s;
	}

	return img;
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


/**
 * @brief RpgUserWallet::available
 * @return
 */

bool RpgUserWallet::available() const
{
	return true;

	/*return ((m_amount > 0 || (m_market.type == RpgMarket::Weapon && m_amount < 0))
			&& (m_expiry.isNull() || m_expiry > QDateTime::currentDateTime())) ||
			(m_market.cost == 0 && m_market.rank == 0);*/
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

	setBaseReadableName(newReadableName);
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

	return a;

	/*if (m_market.type == RpgMarket::Weapon && m_market.cost == 0 && m_market.rank == 0)
		return false;

	if (m_market.type == RpgMarket::Map && a)
		return false;

	if (m_market.type == RpgMarket::Weapon && a)
		return false;

	if (m_market.type == RpgMarket::Skin && a)
		return false;


	if (m_market.rollover != RpgMarket::None && m_amount >= m_market.num)
		return false;

	Server *s = Application::instance()->client()->server();

	if (!s)
		return false;

	if ((!m_market.belongs.isEmpty() || m_market.type == RpgMarket::Weapon) && !hasCharacter(m_market.belongs))
		return false;

	if (!isBelongsSolved())
		return false;

	return m_market.rank <= 0 || s->user()->rank().id() >= m_market.rank;*/
}



/**
 * @brief RpgUserWallet::getExtendedInfo
 * @param def
 * @return
 */
/*
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
*/


/**
 * @brief RpgUserWallet::getExtendedInfo
 * @param player
 * @return
 */

/*
QList<RpgMarketExtendedInfo> RpgUserWallet::getExtendedInfo(const RpgPlayerCharacterConfig &player)
{
	QList<RpgMarketExtendedInfo> list;

	const auto t = RpgArmory::weaponHash().key(player.weapon, RpgGameData::Weapon::WeaponInvalid);

	if (t != RpgGameData::Weapon::WeaponInvalid)
		list.append(RpgMarketExtendedInfo{
						QStringLiteral("qrc:/Qaterial/Icons/sword-cross.svg"),
						tr("Fegyver:"),
						RpgWeapon::weaponName(t),
					});

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

*/

/**
 * @brief RpgUserWallet::hasCharacter
 * @param character
 * @param list
 * @return
 */

bool RpgUserWallet::hasCharacter(const QString &character, RpgUserWalletList *list) const
{
	if (character.isEmpty())
		return false;

	if (!list)
		list = m_walletList;

	if (!list) {
		LOG_CERROR("game") << "Wallet list missing";
		return false;
	}

		return false;
}


/**
 * @brief RpgUserWallet::isBelongsSolved
 * @return
 */

bool RpgUserWallet::isBelongsSolved() const
{
	/*if (m_market.belongs.isEmpty() && m_market.type == RpgMarket::Weapon)
		return false;

	if (m_market.belongs.isEmpty() && m_market.type != RpgMarket::Weapon)
		return true;

	for (RpgUserWallet *w : *m_walletList) {
		const RpgMarket &m = w->market();

		if (m.type != RpgMarket::Weapon || m.belongs != m_market.belongs)
			continue;

		if (m.belongsValue < m_market.belongsValue && !w->available())
			return false;
	}*/

	return true;
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
 * @brief RpgUserWalletList::loadWorld
 */

void RpgUserWalletList::loadWorld()
{
	if (m_world)
		return;

	LOG_CDEBUG("game") << "Load world...";

	const auto &ptr = Utils::fileToJsonObject(":/world/world01_Hungary/data.json");

	if (ptr) {
		RpgWorld world;
		world.fromJson(ptr.value());

		m_world.reset(new RpgUserWorld(world));
		m_world->reloadLands("qrc:/world/world01_Hungary");
		m_world->updateWallet(this);
	} else {
		return unloadWorld();
	}

	emit worldChanged();
}


/**
 * @brief RpgUserWalletList::unloadWorld
 */

void RpgUserWalletList::unloadWorld()
{
	LOG_CDEBUG("game") << "Unload world...";
	m_world.reset();
	emit worldChanged();
}


/**
 * @brief RpgUserWalletList::loadMarket
 * @param json
 */

void RpgUserWalletList::loadMarket(const QJsonObject &json)
{
	/*QMutexLocker locker(&m_mutex);

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
	reloadWallet();*/
}



/**
 * @brief RpgUserWalletList::loadWallet
 * @param json
 */

void RpgUserWalletList::loadWallet(const QJsonObject &json)
{
	/*QMutexLocker locker(&m_mutex);

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


		/// *
		if (w->marketType() == RpgMarket::Weapon) {
			const auto t = RpgArmory::weaponHash().key(market.name, RpgGameData::Weapon::WeaponInvalid);

			if (t != RpgGameData::Weapon::WeaponInvalid) {
				if (RpgUserWallet *b = w->getBelongsTo()) {
					QString n;

					if (const QString &str = market.info.value(QStringLiteral("readableName")).toString(); !str.isEmpty())
						n = str;
					else
						n = RpgWeapon::weaponNameEn(t);

					w->setReadableName(n + '\n' + b->readableName());
					w->setBaseReadableName(b->readableName());
					w->setSortName(b->readableName() + QStringLiteral("%1").arg(t, 2, u'0'));
				}
			}
		} else if (w->marketType() == RpgMarket::Skin) {
			const auto nameSkin = RpgGame::characters().find(market.name);

			if (RpgUserWallet *b = w->getBelongsTo()) {
				const auto baseSkin = RpgGame::characters().find(b->market().name);

				if (nameSkin != RpgGame::characters().constEnd() &&
						baseSkin != RpgGame::characters().constEnd()) {
					w->setReadableName(nameSkin->name + QStringLiteral(" of ") + baseSkin->name);
					w->setBaseReadableName(baseSkin->name);
				}

			}

			if (nameSkin != RpgGame::characters().constEnd()) {
				const auto &arm = getArmory(market.name);
				w->setSubImage(getWeaponImage(arm.cw, arm.s));
			}
		}
		*/
/*
		const bool found = std::find_if(mList.cbegin(), mList.cend(), [&market](const RpgWallet &m){
			return m.isEqual(market);
		}) != mList.cend();

		if (!found) {
			w->setAmount(0);
			emit w->availableChanged();
		}
	}

	if (m_world)
		m_world->updateWallet(this);

	emit reloaded();*/
}





/**
 * @brief RpgUserWalletList::updateAllWalletBuyable
 */

void RpgUserWalletList::updateAllWalletBuyable()
{
	for (RpgUserWallet *w : *this) {
		emit w->buyableChanged();
	}
}





/**
 * @brief RpgUserWalletList::world
 * @return
 */

RpgUserWorld *RpgUserWalletList::world() const
{
	return m_world.get();
}


/**
 * @brief RpgUserWalletList::worldGetSelectedWallet
 * @return
 */

RpgUserWallet *RpgUserWalletList::worldGetSelectedWallet() const
{
	if (!m_world || !m_world->selectedLand())
		return nullptr;

	/*const auto it = std::find_if(this->constBegin(), this->constEnd(),
								 [land = m_world->selectedLand()](RpgUserWallet *w){
		return (w->market().type == RpgMarket::Map && w->market().name == land->bindedMap());
	});

	if (it != this->constEnd())
		return *it;*/

	return nullptr;
}



int RpgUserWalletList::currency() const
{
	QMutexLocker locker(&m_mutex);

	return m_currency;
}

void RpgUserWalletList::setCurrency(int newCurrency)
{
	QMutexLocker locker(&m_mutex);

	if (m_currency == newCurrency)
		return;
	m_currency = newCurrency;
	emit currencyChanged();
}



/**
 * @brief RpgUserWorld::RpgUserWorld
 * @param parent
 */

RpgUserWorld::RpgUserWorld(const RpgWorld &worldData, QObject *parent)
	: QObject(parent)
	, RpgWorld(worldData)
	, m_landList(std::make_unique<RpgWorldLandDataList>())
{
	setWorldSize(QSize(worldData.orig.width, worldData.orig.height));
}



/**
 * @brief RpgUserWorld::~RpgUserWorld
 */

RpgUserWorld::~RpgUserWorld()
{
	m_landList->clear();
	setSelectedLand(nullptr);

	if (m_cachedMapItem) {
		m_cachedMapItem->setProperty("world", QVariant::fromValue(nullptr));
		m_cachedMapItem->deleteLater();
		m_cachedMapItem = nullptr;
	}
}


/**
 * @brief RpgUserWorld::reloadLands
 */

void RpgUserWorld::reloadLands(const QString &path)
{
	LOG_CDEBUG("game") << "Reload land list:" << qPrintable(orig.description);
	m_landList->clear();

	setBasePath(path);

	for (const auto &[id, geom] : lands.asKeyValueRange()) {
		RpgWorldLandData *land = new RpgWorldLandData(this);
		land->setLandId(id);
		land->setLandGeometry(geom);
		land->setMapBinding(binding.value(id));

		m_landList->append(land);
	}

	LOG_CDEBUG("game") << "Loaded" << m_landList->size() << "lands";

	emit imageBackgroundChanged();
	emit imageOverChanged();

}



/**
 * @brief RpgUserWorld::updateWallet
 * @param wallet
 */

void RpgUserWorld::updateWallet(RpgUserWalletList *wallet)
{
	LOG_CTRACE("game") << "Update wallet";

	for (RpgWorldLandData *land : *m_landList) {
		land->updateWallet(wallet);
	}

	// Open adjacent lands

	LOG_CTRACE("game") << "Open adjacent lands";

	for (RpgWorldLandData *land : *m_landList) {
		if (land->landState() != RpgWorldLandData::LandAchieved)
			continue;

		const QJsonArray array = orig.adjacency.value(land->landId());
		for (const QJsonValue &v : array) {
			const QString &id = v.toString();

			const auto it = std::find_if(m_landList->constBegin(), m_landList->constEnd(), [&id](RpgWorldLandData *d){
							return d->landId() == id;
		});

			if (it != m_landList->constEnd() && (*it)->landState() == RpgWorldLandData::LandLocked)
				(*it)->setLandState(RpgWorldLandData::LandSelectable);
		}
	}
}


/**
 * @brief RpgUserWorld::selectFromWallet
 * @param wallet
 */

void RpgUserWorld::selectFromWallet(RpgUserWallet *wallet)
{
	/*if (!wallet || !wallet->available() || wallet->market().type != RpgMarket::Map)
		return setSelectedLand(nullptr);

	const auto it = std::find_if(m_landList->constBegin(), m_landList->constEnd(), [wallet](RpgWorldLandData *d){
					return d->landId() == wallet->market().name &&
					(d->landState() == RpgWorldLandData::LandSelectable ||
					 d->landState() == RpgWorldLandData::LandAchieved);
});

	if (it != m_landList->constEnd())
		return setSelectedLand(*it);
*/
	setSelectedLand(nullptr);
}


/**
 * @brief RpgUserWorld::select
 * @param map
 */

void RpgUserWorld::select(const QString &map, const bool &forced)
{

	const auto it = std::find_if(m_landList->constBegin(), m_landList->constEnd(),
								 [&map, forced](RpgWorldLandData *d){
		return d->bindedMap() == map &&
				(d->landState() == RpgWorldLandData::LandSelectable ||
				 d->landState() == RpgWorldLandData::LandAchieved ||
				 forced);
	});

	if (it != m_landList->constEnd())
		return setSelectedLand(*it);

	setSelectedLand(nullptr);
}


/**
 * @brief RpgUserWorld::selectLand
 * @param land
 */

void RpgUserWorld::selectLand(RpgWorldLandData *land)
{
	if (land && (land->landState() == RpgWorldLandData::LandSelectable ||
				 land->landState() == RpgWorldLandData::LandAchieved))
		setSelectedLand(land);
	else
		setSelectedLand(nullptr);
}




QSize RpgUserWorld::worldSize() const
{
	return m_worldSize;
}

void RpgUserWorld::setWorldSize(const QSize &newWorldSize)
{
	if (m_worldSize == newWorldSize)
		return;
	m_worldSize = newWorldSize;
	emit worldSizeChanged();
}

QString RpgUserWorld::basePath() const
{
	return m_basePath;
}

void RpgUserWorld::setBasePath(const QString &newBasePath)
{
	if (m_basePath == newBasePath)
		return;
	m_basePath = newBasePath;
	emit basePathChanged();
}


/**
 * @brief RpgUserWorld::landList
 * @return
 */

RpgWorldLandDataList *RpgUserWorld::landList() const
{
	return m_landList.get();
}


/**
 * @brief RpgUserWorld::selectedLand
 * @return
 */

RpgWorldLandData *RpgUserWorld::selectedLand() const
{
	return m_selectedLand;
}

void RpgUserWorld::setSelectedLand(RpgWorldLandData *newSelectedLand)
{
	if (m_selectedLand == newSelectedLand)
		return;
	m_selectedLand = newSelectedLand;
	emit selectedLandChanged();
}



/**
 * @brief RpgUserWorld::imageBackground
 * @return
 */

QUrl RpgUserWorld::imageBackground() const
{
	return orig.background.isEmpty() ? QUrl() : m_basePath+QStringLiteral("/")+orig.background;
}


/**
 * @brief RpgUserWorld::imageOver
 * @return
 */

QUrl RpgUserWorld::imageOver() const
{
	return orig.over.isEmpty() ? QUrl() : m_basePath+QStringLiteral("/")+orig.over;
}



/**
 * @brief RpgUserWorld::getCachedMapItem
 * @return
 */

QQuickItem *RpgUserWorld::getCachedMapItem()
{
	if (!m_cachedMapItem)
	{
		LOG_CDEBUG("client") << "Create RpgUserWorld cached map";

		QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/RpgUserWorldMap.qml"), this);

		m_cachedMapItem = qobject_cast<QQuickItem*>(component.create());

		if (!m_cachedMapItem) {
			LOG_CERROR("client") << "RpgUserWorld cached map create error" << component.errorString();
			return nullptr;
		}

		m_cachedMapItem->setParent(this);
		m_cachedMapItem->setProperty("world", QVariant::fromValue(this));
	}

	return m_cachedMapItem;
}


QString RpgUserWallet::baseReadableName() const
{
	return m_baseReadableName;
}

void RpgUserWallet::setBaseReadableName(const QString &newBaseReadableName)
{
	if (m_baseReadableName == newBaseReadableName)
		return;
	m_baseReadableName = newBaseReadableName;
	emit baseReadableNameChanged();
}


/**
 * @brief RpgUserWallet::subImage
 * @return
 */

QString RpgUserWallet::subImage() const
{
	return m_subImage;
}

void RpgUserWallet::setSubImage(const QString &newSubImage)
{
	if (m_subImage == newSubImage)
		return;
	m_subImage = newSubImage;
	emit subImageChanged();
}
