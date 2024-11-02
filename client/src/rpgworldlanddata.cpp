/*
 * ---- Call of Suli ----
 *
 * rpgworldlanddata.cpp
 *
 * Created on: 2024. 10. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgWorldLandData
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

#include "rpgworldlanddata.h"
#include "rpgworldlanddata_p.h"
#include "rpguserwallet.h"

RpgWorldLandData::RpgWorldLandData(RpgUserWorld *world, QObject *parent)
	: QObject{parent}
	, d(new RpgWorldLandDataPrivate(this))
	, m_world(world)
{
	Q_ASSERT(world);
}



/**
 * @brief RpgWorldLandData::setMapBinding
 * @param binding
 */

void RpgWorldLandData::setMapBinding(const RpgWorldMapBinding &binding, RpgUserWalletList *walletList)
{
	m_mapBinding = binding;

	updateWallet(walletList);
}


/**
 * @brief RpgWorldLandData::setLandGeometry
 * @param geometry
 */

void RpgWorldLandData::setLandGeometry(const RpgWorldLandGeometry &geometry)
{
	m_geometry = geometry;
	emit posXChanged();
	emit posYChanged();
	emit textXChanged();
	emit textYChanged();
	emit rotateChanged();
}



/**
 * @brief RpgWorldLandData::updateWallet
 * @param walletList
 */

void RpgWorldLandData::updateWallet(RpgUserWalletList *walletList)
{
	if (walletList && !m_mapBinding.map.isEmpty()) {
		const auto it = std::find_if(walletList->cbegin(), walletList->cend(), [this](RpgUserWallet *w){
						return w->market().type == RpgMarket::Map && w->market().name == m_mapBinding.map;
	});

		if (it != walletList->cend()) {
			d->m_walletMap = *it;
			if (m_mapBinding.free || (*it)->available())
				setLandState(LandAchieved);
			else
				setLandState(LandLocked);
			emit nameChanged();
			emit backgroundSourceChanged();

			LOG_CTRACE("game") << "Update land" << m_landId << m_mapBinding.map << m_landState;
			return;
		}
	}

	d->m_walletMap = nullptr;
	setLandState(LandUnused);
	emit nameChanged();
	emit backgroundSourceChanged();

	LOG_CTRACE("game") << "Clear land" << m_landId << m_mapBinding.map << m_landState;
}




/**
 * @brief RpgWorldLandData::landState
 * @return
 */

RpgWorldLandData::LandState RpgWorldLandData::landState() const
{
	return m_landState;
}

void RpgWorldLandData::setLandState(LandState newLandState)
{
	if (m_landState == newLandState)
		return;
	m_landState = newLandState;
	emit landStateChanged();
}


/**
 * @brief RpgWorldLandData::posX
 * @return
 */

qreal RpgWorldLandData::posX() const
{
	return m_geometry.x;
}

qreal RpgWorldLandData::posY() const
{
	return m_geometry.y;
}

qreal RpgWorldLandData::textX() const
{
	return m_geometry.textX;
}

qreal RpgWorldLandData::textY() const
{
	return m_geometry.textY;
}



/**
 * @brief RpgWorldLandDataPrivate::RpgWorldLandDataPrivate
 * @param q
 */

RpgWorldLandDataPrivate::RpgWorldLandDataPrivate(RpgWorldLandData *data)
	: q(data)
{

}


/**
 * @brief RpgWorldLandData::name
 * @return
 */

QString RpgWorldLandData::name() const
{
	return d->m_walletMap ? d->m_walletMap->readableName() : QString{};
}

QString RpgWorldLandData::landId() const
{
	return m_landId;
}

void RpgWorldLandData::setLandId(const QString &newLandId)
{
	if (m_landId == newLandId)
		return;
	m_landId = newLandId;
	emit landIdChanged();

	setImageSource(m_world->basePath()+QStringLiteral("/land-%1.svg").arg(m_landId));
	setBorderSource(m_world->basePath()+QStringLiteral("/land-%1-border.svg").arg(m_landId));
}

QUrl RpgWorldLandData::imageSource() const
{
	return m_imageSource;
}

void RpgWorldLandData::setImageSource(const QUrl &newImageSource)
{
	if (m_imageSource == newImageSource)
		return;
	m_imageSource = newImageSource;
	emit imageSourceChanged();
}

QUrl RpgWorldLandData::borderSource() const
{
	return m_borderSource;
}

void RpgWorldLandData::setBorderSource(const QUrl &newBorderSource)
{
	if (m_borderSource == newBorderSource)
		return;
	m_borderSource = newBorderSource;
	emit borderSourceChanged();
}


RpgUserWorld *RpgWorldLandData::world() const
{
	return m_world;
}

QString RpgWorldLandData::backgroundSource() const
{
	return d->m_walletMap ? d->m_walletMap->image() : QString{};
}

QString RpgWorldLandData::bindedMap() const
{
	return m_mapBinding.map;
}

qreal RpgWorldLandData::rotate() const
{
	return m_geometry.rotate;
}
