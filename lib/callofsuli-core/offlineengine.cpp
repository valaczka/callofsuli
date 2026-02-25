/*
 * ---- Call of Suli ----
 *
 * offlineengine.cpp
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

#include "offlineengine.h"

#include <Logger.h>



/**
 * @brief OfflineEngine::computeHashChain
 * @param anchor
 * @param step
 * @return
 */

QByteArray OfflineEngine::computeHashChain(const QByteArray &anchor, const int &step)
{
	if (step < 1)
		return anchor;

	std::array<unsigned char, crypto_generichash_BYTES> hash;

	for (int i=0; i<step; ++i) {
		if (i==0) {
			if (crypto_generichash(hash.data(), hash.size(),
								   reinterpret_cast<const unsigned char*>(anchor.constData()),
								   anchor.size(),
								   nullptr, 0) != 0) {
				LOG_CERROR("client") << "Hash chain compute error";
				return {};
			}
		} else {
			std::array<unsigned char, crypto_generichash_BYTES> from = hash;

			if (crypto_generichash(hash.data(), hash.size(),
								   from.data(), from.size(),
								   nullptr, 0) != 0) {
				LOG_CERROR("client") << "Hash chain compute error";
				return {};
			}
		}
	}

	QByteArray last(reinterpret_cast<const char*>(hash.data()), hash.size());

	return last;
}




/**
 * @brief OfflineEngine::computeMapHash
 * @param data
 * @return
 */

QByteArray OfflineEngine::computeMapHash(const QByteArray &data)
{
	QByteArray hash(crypto_generichash_BYTES, Qt::Uninitialized);

	if (crypto_generichash(reinterpret_cast<unsigned char*>(hash.data()), hash.size(),
						   reinterpret_cast<const unsigned char*>(data.constData()), data.size(),
						   nullptr, 0) != 0) {
		LOG_CERROR("client") << "Hash compute error";
		return {};
	}

	return hash;
}




/**
 * @brief PermitFull::setSignedPermit
 * @param data
 */

void PermitFull::setSignedPermit(const QByteArray &data)
{
	permit = QString::fromLatin1(data.toBase64());
}


/**
 * @brief PermitFull::getPermit
 * @return
 */

PermitContent PermitFull::getPermit() const
{
	PermitContent p;

	const QCborMap m = QCborValue::fromCbor(QByteArray::fromBase64(permit.toLatin1())).toMap();

	if (const QCborMap d = QCborValue::fromCbor(m.value(QStringLiteral("content")).toByteArray()).toMap(); !d.isEmpty()) {
		p.fromCbor(d);
	} else {
		LOG_CWARNING("client") << "Invalid permit content";
	}

	return p;

}




/**
 * @brief PermitContent::toCborMap
 * @param alwaysUseFloats
 * @return
 */

QCborMap PermitContent::toCborMap(const bool &alwaysUseFloats) const
{
	QCborMap m = QSerializer::toCborMap(alwaysUseFloats);

	QCborArray a;

	for (const PermitMap &m : maps)
		a.append(m.toCborMap());

	m.insert(QStringLiteral("maps"), a);

	return m;
}



/**
 * @brief PermitContent::fromCbor
 * @param val
 */

void PermitContent::fromCbor(const QCborValue &val)
{
	QSerializer::fromCbor(val);

	const QCborMap m = val.toMap();

	if (m.contains(QStringLiteral("maps"))) {
		maps.clear();
		const QCborArray a = m.value(QStringLiteral("maps")).toArray();
		maps.reserve(a.size());

		for (const QCborValue &v : a) {
			PermitMap map;
			map.fromCbor(v);
			maps.push_back(std::move(map));
		}
	}
}


/**
 * @brief PermitContent::getClientTime
 * @param clock
 * @return
 */

qint64 PermitContent::getClientTime(const qint64 &clock) const
{
	return serverTime + (clock - clientClock)/1000;
}



/**
 * @brief ReceiptList::toCborMap
 * @param alwaysUseFloats
 * @return
 */

QCborMap ReceiptList::toCborMap(const bool &alwaysUseFloats) const
{
	QCborMap m = QSerializer::toCborMap(alwaysUseFloats);

	QCborArray a;

	for (const QByteArray &r : receipts)
		a.append(r);

	m.insert(QStringLiteral("receipts"), a);

	return m;
}



/**
 * @brief ReceiptList::fromCbor
 * @param val
 */

void ReceiptList::fromCbor(const QCborValue &val)
{
	QSerializer::fromCbor(val);

	const QCborMap m = val.toMap();

	if (m.contains(QStringLiteral("receipts"))) {
		receipts.clear();
		const QCborArray a = m.value(QStringLiteral("receipts")).toArray();
		receipts.reserve(a.size());

		for (const QCborValue &v : a) {
			receipts.emplace_back(v.toByteArray());
		}
	}
}
