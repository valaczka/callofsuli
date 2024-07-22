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
	Q_PROPERTY(RpgUserWallet *bullet READ bullet WRITE setBullet NOTIFY bulletChanged FINAL)
	Q_PROPERTY(bool available READ available NOTIFY availableChanged FINAL)
	Q_PROPERTY(bool buyable READ buyable NOTIFY buyableChanged FINAL)
	Q_PROPERTY(QString readableName READ readableName WRITE setReadableName NOTIFY readableNameChanged FINAL)
	Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged FINAL)
	Q_PROPERTY(QString sortName READ sortName WRITE setSortName NOTIFY sortNameChanged FINAL)
	Q_PROPERTY(Rank rank READ rank WRITE setRank NOTIFY rankChanged FINAL)
	Q_PROPERTY(QList<RpgMarketExtendedInfo> extendedInfo READ extendedInfo WRITE setExtendedInfo NOTIFY extendedInfoChanged FINAL)

public:
	explicit RpgUserWallet(QObject *parent = nullptr);
	virtual ~RpgUserWallet() {}

	Q_INVOKABLE QJsonObject getJson() const;

	RpgMarket market() const;
	void setMarket(const RpgMarket &newMarket);

	int amount() const;
	void setAmount(int newAmount);

	QDateTime expiry() const;
	void setExpiry(const QDateTime &newExpiry);

	RpgUserWallet *bullet() const;
	void setBullet(RpgUserWallet *newBullet);

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

signals:
	void marketChanged();
	void amountChanged();
	void expiryChanged();
	void bulletChanged();
	void availableChanged();
	void marketTypeChanged();
	void readableNameChanged();
	void imageChanged();
	void sortNameChanged();
	void rankChanged();
	void buyableChanged();
	void extendedInfoChanged();

private:
	static QList<RpgMarketExtendedInfo> getExtendedInfo(const RpgGameDefinition &def);
	static QList<RpgMarketExtendedInfo> getExtendedInfo(const RpgMarket &market);
	static QList<RpgMarketExtendedInfo> getExtendedInfo(const RpgPlayerCharacterConfig &player);

	RpgMarket m_market;
	int m_amount = 0;
	QDateTime m_expiry;
	RpgUserWallet *m_bullet = nullptr;
	QString m_readableName;
	QString m_image;
	QString m_sortName;
	Rank m_rank;
	RpgUserWalletList *m_walletList = nullptr;
	QList<RpgMarketExtendedInfo> m_extendedInfo;

	friend class RpgUserWalletList;
};




/**
 * @brief The RpgUserWalletList class
 */

class RpgUserWalletList : public qolm::QOlm<RpgUserWallet>
{
	Q_OBJECT

	Q_PROPERTY(int currency READ currency WRITE setCurrency NOTIFY currencyChanged FINAL)

public:
	RpgUserWalletList(QObject *parent = nullptr);
	virtual ~RpgUserWalletList();

	Q_INVOKABLE void reload();
	void reloadMarket();
	void reloadWallet();

	int currency() const;
	void setCurrency(int newCurrency);

signals:
	void reloaded();

	void currencyChanged();

private:
	void loadMarket(const QJsonObject &json);
	void loadWallet(const QJsonObject &json);
	void updateMarket(const RpgMarket &market);
	void updateMarket(const RpgWallet &wallet);
	void removeMissing(const QVector<RpgMarket> &list);
	void updateBullets();

	int m_currency = 0;
};


#endif // RPGUSERWALLET_H
