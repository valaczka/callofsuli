/*
 * ---- Call of Suli ----
 *
 * offlineengine.h
 *
 * Created on: 2026. 02. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OfflineEngine
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

#ifndef OFFLINEENGINE_H
#define OFFLINEENGINE_H

#include <QSerializer>
#include <sodium.h>
#include <QCborMap>
#include <QCborArray>

#include "gamemap.h"



/**
 * @brief The Receipt class
 */

class Receipt : public QSerializer
{
	Q_GADGET

public:
	Receipt()
		: QSerializer()
		, level(0)
		, clock(0)
		, mode(GameMap::Invalid)
		, duration(0)
		, success(false)
		, xp(0)
		, currency(0)
	{}

	QS_SERIALIZABLE

	QS_BYTEARRAY(chainHash)
	QS_BYTEARRAY(prevHash)

	QS_BYTEARRAY(map)
	QS_BYTEARRAY(mission)
	QS_FIELD(quint32, level)
	QS_FIELD(qint64, clock)
	QS_FIELD(GameMap::GameMode, mode)
	QS_FIELD(quint32, duration)
	QS_FIELD(bool, success)
	QS_FIELD(quint32, xp)
	QS_FIELD(quint32, currency)
	QS_FIELD(QJsonArray, stat)
	QS_FIELD(QJsonObject, extended)
};





/**
 * @brief The ReceiptList class
 */

class ReceiptList : public QSerializer
{
	Q_GADGET

	QS_BYTEARRAY(permit)

	QS_SERIALIZABLE_DERIVED

public:
	std::vector<QByteArray> receipts;

	virtual QCborMap toCborMap(const bool &alwaysUseFloats = false) const override;
	virtual void fromCbor(const QCborValue &val) override;

};



/**
 * @brief The PermitMap class
 */

class PermitMap : public QSerializer
{
	Q_GADGET

public:
	PermitMap()
		: QSerializer()
	{}

	QS_SERIALIZABLE

	QS_FIELD(QString, map)
	QS_FIELD(QString, mission)

	QS_BYTEARRAY(hash)				// map hash

	QS_FIELD(QJsonObject, solver)	// mission => solver.toJsonArray()

};




/**
 * @brief The PermitContent class
 */

class PermitContent : public QSerializer
{
	Q_GADGET

public:
	PermitContent()
		: QSerializer()
		, id(0)
		, campaign(-1)
		, modes(GameMap::Invalid)
		, hashStep(0)
		, serverTime(0)
		, clientClock(0)
		, expire(0)
	{}

	QS_SERIALIZABLE_DERIVED

	QS_FIELD(quint64, id)
	QS_FIELD(QString, username)
	QS_BYTEARRAY(deviceid)

	QS_FIELD(qint32, campaign)
	QS_FIELD(GameMap::GameModes, modes)

	QS_BYTEARRAY(hashAnchor)
	QS_FIELD(quint32, hashStep)

	QS_FIELD(qint64, serverTime)				// server epoch time (secs)
	QS_FIELD(qint64, clientClock)				// client boot time (msecs)
	QS_FIELD(qint64, expire)					// expire epoch time (secs)


public:
	std::vector<PermitMap> maps;

	virtual QCborMap toCborMap(const bool &alwaysUseFloats = false) const override;
	virtual void fromCbor(const QCborValue &val) override;

	qint64 getClientTime(const qint64 &clock) const;
	qint64 getClientTime(const Receipt &receipt) const { return getClientTime(receipt.clock); }
};




/**
 * @brief The PermitExtraUser class
 */

class PermitExtraUser : public QSerializer
{
	Q_GADGET

public:
	PermitExtraUser()
		: QSerializer()
	{}


	QS_SERIALIZABLE

	QS_FIELD(QString, familyName)
	QS_FIELD(QString, givenName)
	QS_FIELD(QString, nickName)
};




/**
 * @brief The PermitExtraCampaign class
 */


class PermitExtraCampaign : public QSerializer
{
	Q_GADGET

public:
	PermitExtraCampaign()
		: QSerializer()
	{}


	QS_SERIALIZABLE

	QS_FIELD(QString, description)
	QS_FIELD(QJsonArray, task)
};




/**
 * @brief The PermitExtraMap class
 */

class PermitExtraMap : public QSerializer
{
	Q_GADGET

public:
	PermitExtraMap()
		: QSerializer()
	{}


	QS_SERIALIZABLE

	QS_FIELD(QString, name)
	QS_FIELD(QJsonObject, cache)
};




/**
 * @brief The PermitFull class
 */


class PermitFull : public QSerializer
{
	Q_GADGET

public:
	PermitFull()
		: QSerializer()
	{}

	void setSignedPermit(const QByteArray &data);
	PermitContent getPermit() const;

	QS_FIELD(QString, permit)
	QS_OBJECT(PermitExtraUser, user)
	QS_OBJECT(PermitExtraCampaign, campaign)
	QS_QT_DICT_OBJECTS(QMap, QString, PermitExtraMap, map)

	QS_SERIALIZABLE
};






/**
 * @brief The PermitResponse class
 */

class PermitResponse : public QSerializer
{
	Q_GADGET

public:
	PermitResponse()
		: QSerializer()
		, id(0)
		, campaign(-1)
		, hashStep(0)
	{}

	QS_SERIALIZABLE

	QS_FIELD(quint64, id)
	QS_FIELD(qint32, campaign)
	QS_FIELD(quint32, hashStep)
};




/**
 * @brief The OfflineEngine class
 */

class OfflineEngine
{
public:
	OfflineEngine() {}

	static QByteArray computeHashChain(const QByteArray &anchor, const int &step);
	static QByteArray computeMapHash(const QByteArray &data);

private:

};










#endif // OFFLINEENGINE_H
