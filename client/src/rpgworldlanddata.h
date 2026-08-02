/*
 * ---- Call of Suli ----
 *
 * rpgworldlanddata.h
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

#ifndef RPGWORLDLANDDATA_H
#define RPGWORLDLANDDATA_H

#include "qquickitem.h"
#include "rpglogic.h"
#include <QObject>

#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"


class RpgWorldLandData;
using RpgWorldLandDataList = qolm::QOlm<RpgWorldLandData>;
Q_DECLARE_METATYPE(RpgWorldLandDataList*)





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

	Q_INVOKABLE void resetLands(const QStringList &achieved);
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
 * @brief The RpgWorldLandData class
 */

class RpgWorldLandData : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString landId READ landId WRITE setLandId NOTIFY landIdChanged FINAL)
	Q_PROPERTY(LandState landState READ landState WRITE setLandState NOTIFY landStateChanged FINAL)
	Q_PROPERTY(qreal posX READ posX NOTIFY posXChanged FINAL)
	Q_PROPERTY(qreal posY READ posY NOTIFY posYChanged FINAL)
	Q_PROPERTY(qreal textX READ textX NOTIFY textXChanged FINAL)
	Q_PROPERTY(qreal textY READ textY NOTIFY textYChanged FINAL)
	Q_PROPERTY(qreal rotate READ rotate NOTIFY rotateChanged FINAL)
	Q_PROPERTY(QString name READ name NOTIFY nameChanged FINAL)
	Q_PROPERTY(QUrl imageSource READ imageSource WRITE setImageSource NOTIFY imageSourceChanged FINAL)
	Q_PROPERTY(QUrl borderSource READ borderSource WRITE setBorderSource NOTIFY borderSourceChanged FINAL)
	Q_PROPERTY(QString backgroundSource READ backgroundSource NOTIFY backgroundSourceChanged FINAL)
	Q_PROPERTY(RpgUserWorld *world READ world CONSTANT FINAL)

public:
	explicit RpgWorldLandData(RpgUserWorld *world, QObject *parent = nullptr);

	enum LandState {
		LandInvalid = 0,
		LandUnused,
		LandLocked,
		LandSelectable,
		LandAchieved
	};

	Q_ENUM(LandState)

	void setMapBinding(const RpgWorldMapBinding &binding);
	void setLandGeometry(const RpgWorldLandGeometry &geometry);
	void resetLands(const QStringList &achieved);

	LandState landState() const;
	void setLandState(LandState newLandState);

	qreal posX() const;
	qreal posY() const;

	qreal textX() const;
	qreal textY() const;

	QString name() const;

	QString landId() const;
	void setLandId(const QString &newLandId);

	QUrl imageSource() const;
	void setImageSource(const QUrl &newImageSource);

	QUrl borderSource() const;
	void setBorderSource(const QUrl &newBorderSource);

	RpgUserWorld *world() const;

	QString backgroundSource() const;

	Q_INVOKABLE QString bindedMap() const;

	qreal rotate() const;

signals:
	void landStateChanged();
	void posXChanged();
	void posYChanged();
	void textXChanged();
	void textYChanged();
	void nameChanged();
	void landIdChanged();
	void imageSourceChanged();
	void borderSourceChanged();
	void backgroundSourceChanged();
	void rotateChanged();

private:
	RpgUserWorld *const m_world;
	QString m_landId;
	LandState m_landState = LandInvalid;
	RpgWorldMapBinding m_mapBinding;
	RpgWorldLandGeometry m_geometry;
	QUrl m_imageSource;
	QUrl m_borderSource;
};











#endif // RPGWORLDLANDDATA_H
