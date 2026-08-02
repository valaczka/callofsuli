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
#include "Logger.h"
#include "application.h"
#include "rpggame.h"


RpgWorldLandData::RpgWorldLandData(RpgUserWorld *world, QObject *parent)
	: QObject{parent}
	, m_world(world)
{
	Q_ASSERT(world);
}



/**
 * @brief RpgWorldLandData::setMapBinding
 * @param binding
 */

void RpgWorldLandData::setMapBinding(const RpgWorldMapBinding &binding)
{
	m_mapBinding = binding;

	resetLands(QStringList());
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
 * @brief RpgWorldLandData::updateLands
 * @param terrainList
 */

void RpgWorldLandData::resetLands(const QStringList &achieved)
{
	if (!m_mapBinding.map.isEmpty() && RpgGame::terrains().contains(m_mapBinding.map)) {
		if (achieved.contains(m_mapBinding.map))
			setLandState(LandAchieved);
		else if (m_mapBinding.free)
			setLandState(LandSelectable);
		else
			setLandState(LandLocked);
		emit nameChanged();
		emit backgroundSourceChanged();

		LOG_CTRACE("game") << "Update land" << m_landId << m_mapBinding.map << m_landState;
		return;
	}

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
 * @brief RpgWorldLandData::name
 * @return
 */

QString RpgWorldLandData::name() const
{
	return RpgGame::terrains().value(m_mapBinding.map).name;
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
	return RpgGame::terrains().contains(m_mapBinding.map) ?
				QStringLiteral("qrc:/map/%1/thumbnail.png").arg(m_mapBinding.map) :
				QString{};
}

QString RpgWorldLandData::bindedMap() const
{
	return m_mapBinding.map;
}

qreal RpgWorldLandData::rotate() const
{
	return m_geometry.rotate;
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
 * @brief RpgUserWorld::resetLands
 * @param achieved
 */

void RpgUserWorld::resetLands(const QStringList &achieved)
{
	LOG_CDEBUG("game") << "Reset Lands" << achieved;

	for (RpgWorldLandData *land : *m_landList) {
		land->resetLands(achieved);
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
