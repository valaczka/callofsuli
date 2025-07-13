/*
 * ---- Call of Suli ----
 *
 * rpguserwallet.h
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

#ifndef RPGUSERWALLET_H
#define RPGUSERWALLET_H

#include "rank.h"
#include "rpgconfig.h"
#include "rpggame.h"
#include "rpgworldlanddata.h"
#include <QObject>

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class Client;
class RpgUserWalletList;


/**
 * @brief The RpgUserWallet class
 */

class RpgUserWallet : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RpgMarket market READ market WRITE setMarket NOTIFY marketChanged FINAL)
	Q_PROPERTY(RpgMarket::Type marketType READ marketType NOTIFY marketTypeChanged FINAL)
	Q_PROPERTY(int amount READ amount WRITE setAmount NOTIFY amountChanged FINAL)
	Q_PROPERTY(QDateTime expiry READ expiry WRITE setExpiry NOTIFY expiryChanged FINAL)
	Q_PROPERTY(bool available READ available NOTIFY availableChanged FINAL)
	Q_PROPERTY(bool buyable READ buyable NOTIFY buyableChanged FINAL)
	Q_PROPERTY(QString readableName READ readableName WRITE setReadableName NOTIFY readableNameChanged FINAL)
	Q_PROPERTY(QString baseReadableName READ baseReadableName WRITE setBaseReadableName NOTIFY baseReadableNameChanged FINAL)
	Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged FINAL)
	Q_PROPERTY(QString sortName READ sortName WRITE setSortName NOTIFY sortNameChanged FINAL)
	Q_PROPERTY(Rank rank READ rank WRITE setRank NOTIFY rankChanged FINAL)
	Q_PROPERTY(QList<RpgMarketExtendedInfo> extendedInfo READ extendedInfo WRITE setExtendedInfo NOTIFY extendedInfoChanged FINAL)

public:
	explicit RpgUserWallet(QObject *parent = nullptr);
	virtual ~RpgUserWallet() {}

	Q_INVOKABLE QJsonObject getJson() const;
	RpgWallet toWallet() const;

	Q_INVOKABLE RpgUserWallet *getBelongsTo() const;

	RpgMarket market() const;
	void setMarket(const RpgMarket &newMarket);

	int amount() const;
	void setAmount(int newAmount);

	QDateTime expiry() const;
	void setExpiry(const QDateTime &newExpiry);

	bool available() const;

	RpgMarket::Type marketType() const;

	QString readableName() const;
	void setReadableName(const QString &newReadableName);

	QString image() const;
	void setImage(const QString &newImage);

	QString sortName() const;
	void setSortName(const QString &newSortName);

	Rank rank() const;
	void setRank(const Rank &newRank);

	bool buyable() const;

	QList<RpgMarketExtendedInfo> extendedInfo() const;
	void setExtendedInfo(const QList<RpgMarketExtendedInfo> &newExtendedInfo);

	QString baseReadableName() const;
	void setBaseReadableName(const QString &newBaseReadableName);

signals:
	void marketChanged();
	void amountChanged();
	void expiryChanged();
	void availableChanged();
	void marketTypeChanged();
	void readableNameChanged();
	void imageChanged();
	void sortNameChanged();
	void rankChanged();
	void buyableChanged();
	void extendedInfoChanged();
	void baseReadableNameChanged();

private:
	static QList<RpgMarketExtendedInfo> getExtendedInfo(const RpgGameDefinition &def);
	static QList<RpgMarketExtendedInfo> getExtendedInfo(const RpgMarket &market);
	static QList<RpgMarketExtendedInfo> getExtendedInfo(const RpgPlayerCharacterConfig &player);

	bool hasCharacter(const QString &character, RpgUserWalletList *list = nullptr) const;
	bool isBelongsSolved() const;

	RpgMarket m_market;
	int m_amount = 0;
	QDateTime m_expiry;
	QString m_readableName;
	QString m_baseReadableName;
	QString m_image;
	QString m_sortName;
	Rank m_rank;
	RpgUserWalletList *m_walletList = nullptr;
	QList<RpgMarketExtendedInfo> m_extendedInfo;

	friend class RpgUserWalletList;
};




/**
 * @brief The RpgUserWorld class
 */

class RpgUserWorld : public QObject, public RpgWorld
{
	Q_OBJECT

	Q_PROPERTY(QString basePath READ basePath WRITE setBasePath NOTIFY basePathChanged FINAL)
	Q_PROPERTY(QSize worldSize READ worldSize WRITE setWorldSize NOTIFY worldSizeChanged FINAL)
	Q_PROPERTY(RpgWorldLandDataList *landList READ landList CONSTANT FINAL)
	Q_PROPERTY(RpgWorldLandData *selectedLand READ selectedLand WRITE setSelectedLand NOTIFY selectedLandChanged FINAL)
	Q_PROPERTY(QUrl imageBackground READ imageBackground NOTIFY imageBackgroundChanged FINAL)
	Q_PROPERTY(QUrl imageOver READ imageOver NOTIFY imageOverChanged FINAL)

public:
	RpgUserWorld(const RpgWorld &worldData, QObject *parent = nullptr);
	virtual ~RpgUserWorld();

	void reloadLands(const QString &path);
	void updateWallet(RpgUserWalletList *wallet);

	Q_INVOKABLE void selectFromWallet(RpgUserWallet *wallet);
	Q_INVOKABLE void select(const QString &map, const bool &forced = false);
	Q_INVOKABLE void selectLand(RpgWorldLandData *land);

	QSize worldSize() const;
	void setWorldSize(const QSize &newWorldSize);

	QString basePath() const;
	void setBasePath(const QString &newBasePath);

	RpgWorldLandDataList *landList() const;

	RpgWorldLandData *selectedLand() const;
	void setSelectedLand(RpgWorldLandData *newSelectedLand);

	QUrl imageBackground() const;
	QUrl imageOver() const;

	Q_INVOKABLE QQuickItem *getCachedMapItem();

signals:
	void worldSizeChanged();
	void basePathChanged();
	void selectedLandChanged();
	void imageBackgroundChanged();
	void imageOverChanged();

private:
	QString m_basePath;
	QSize m_worldSize;
	std::unique_ptr<RpgWorldLandDataList> m_landList;
	QPointer<RpgWorldLandData> m_selectedLand;
	QQuickItem *m_cachedMapItem = nullptr;
};



/**
 * @brief The RpgUserWalletList class
 */

class RpgUserWalletList : public qolm::QOlm<RpgUserWallet>
{
	Q_OBJECT

	Q_PROPERTY(int currency READ currency WRITE setCurrency NOTIFY currencyChanged FINAL)
	Q_PROPERTY(RpgUserWorld *world READ world NOTIFY worldChanged FINAL)

public:
	RpgUserWalletList(QObject *parent = nullptr);
	virtual ~RpgUserWalletList();

	Q_INVOKABLE void reload();
	void reloadMarket();
	void reloadWallet();

	void loadWorld();
	void unloadWorld();

	int currency() const;
	void setCurrency(int newCurrency);

	RpgUserWorld *world() const;
	Q_INVOKABLE RpgUserWallet *worldGetSelectedWallet() const;

	RpgGameData::Armory getArmory(const QString &character) const;

signals:
	void reloaded();

	void currencyChanged();
	void worldChanged();

private:
	void loadMarket(const QJsonObject &json);
	void loadWallet(const QJsonObject &json);
	void updateMarket(const RpgMarket &market);
	void updateMarket(const RpgWallet &wallet);
	void updateAllWalletBuyable();
	void removeMissing(const QVector<RpgMarket> &list);

	int m_currency = 0;

	mutable QRecursiveMutex m_mutex;

	std::unique_ptr<RpgUserWorld> m_world;

};


#endif // RPGUSERWALLET_H
