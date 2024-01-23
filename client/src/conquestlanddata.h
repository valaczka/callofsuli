/*
 * ---- Call of Suli ----
 *
 * conquestlanddata.h
 *
 * Created on: 2024. 01. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestLandData
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

#ifndef CONQUESTLANDDATA_H
#define CONQUESTLANDDATA_H

#include "qurl.h"
#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"


class ConquestLandData;
using ConquestLandDataList = qolm::QOlm<ConquestLandData>;
Q_DECLARE_METATYPE(ConquestLandDataList*)


class ConquestLandData : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString landId READ landId WRITE setLandId NOTIFY landIdChanged FINAL)
	Q_PROPERTY(qreal baseX READ baseX WRITE setBaseX NOTIFY baseXChanged FINAL)
	Q_PROPERTY(qreal baseY READ baseY WRITE setBaseY NOTIFY baseYChanged FINAL)
	Q_PROPERTY(QUrl imgMap READ imgMap WRITE setImgMap NOTIFY imgMapChanged FINAL)
	Q_PROPERTY(QUrl imgBorder READ imgBorder WRITE setImgBorder NOTIFY imgBorderChanged FINAL)

public:
	explicit ConquestLandData(QObject *parent = nullptr);

	QString landId() const;
	void setLandId(const QString &newLandId);

	qreal baseX() const;
	void setBaseX(qreal newBaseX);

	qreal baseY() const;
	void setBaseY(qreal newBaseY);

	QUrl imgMap() const;
	void setImgMap(const QUrl &newImgMap);

	QUrl imgBorder() const;
	void setImgBorder(const QUrl &newImgBorder);

signals:
	void landIdChanged();
	void baseXChanged(qreal x);
	void baseYChanged(qreal y);
	void imgMapChanged();
	void imgBorderChanged();

private:
	QString m_landId;
	qreal m_baseX = 0;
	qreal m_baseY = 0;
	QUrl m_imgMap;
	QUrl m_imgBorder;
};

#endif // CONQUESTLANDDATA_H
