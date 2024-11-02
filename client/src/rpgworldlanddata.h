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

#include "rpgconfig.h"
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


class RpgUserWorld;
class RpgUserWalletList;
class RpgWorldLandDataPrivate;


#ifndef OPAQUE_PTR_RpgUserWorld
#define OPAQUE_PTR_RpgUserWorld
  Q_DECLARE_OPAQUE_POINTER(RpgUserWorld*)
#endif


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

	void setMapBinding(const RpgWorldMapBinding &binding, RpgUserWalletList *walletList = nullptr);
	void setLandGeometry(const RpgWorldLandGeometry &geometry);
	void updateWallet(RpgUserWalletList *walletList);

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

	QString bindedMap() const;

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
	RpgWorldLandDataPrivate *d;
	RpgUserWorld *const m_world;
	QString m_landId;
	LandState m_landState = LandInvalid;
	RpgWorldMapBinding m_mapBinding;
	RpgWorldLandGeometry m_geometry;
	QUrl m_imageSource;
	QUrl m_borderSource;
};

#endif // RPGWORLDLANDDATA_H
