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
#include "conquestconfig.h"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"


class ConquestLandData;
using ConquestLandDataList = qolm::QOlm<ConquestLandData>;
Q_DECLARE_METATYPE(ConquestLandDataList*)


class ConquestGame;

#ifndef OPAQUE_PTR_ConquestGame
#define OPAQUE_PTR_ConquestGame
  Q_DECLARE_OPAQUE_POINTER(ConquestGame*)
#endif



/**
 * @brief The ConquestLandData class
 */

class ConquestLandData : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString landId READ landId WRITE setLandId NOTIFY landIdChanged FINAL)
	Q_PROPERTY(qreal baseX READ baseX WRITE setBaseX NOTIFY baseXChanged FINAL)
	Q_PROPERTY(qreal baseY READ baseY WRITE setBaseY NOTIFY baseYChanged FINAL)
	Q_PROPERTY(qreal overX READ overX WRITE setOverX NOTIFY overXChanged FINAL)
	Q_PROPERTY(qreal overY READ overY WRITE setOverY NOTIFY overYChanged FINAL)
	Q_PROPERTY(QUrl imgMap READ imgMap WRITE setImgMap NOTIFY imgMapChanged FINAL)
	Q_PROPERTY(QUrl imgBorder READ imgBorder WRITE setImgBorder NOTIFY imgBorderChanged FINAL)
	Q_PROPERTY(int proprietor READ proprietor WRITE setProprietor NOTIFY proprietorChanged FINAL)
	Q_PROPERTY(ConquestGame *game READ game WRITE setGame NOTIFY gameChanged FINAL)
	Q_PROPERTY(int xp READ xp WRITE setXp NOTIFY xpChanged FINAL)
	Q_PROPERTY(int fortress READ fortress WRITE setFortress NOTIFY fortressChanged FINAL)

public:
	explicit ConquestLandData(QObject *parent = nullptr);

	void loadFromConfig(const ConquestWorldData &data);

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

	int proprietor() const;
	void setProprietor(int newProprietor);

	ConquestGame *game() const;
	void setGame(ConquestGame *newGame);

	int xp() const;
	void setXp(int newXp);

	qreal overX() const;
	void setOverX(qreal newOverX);

	qreal overY() const;
	void setOverY(qreal newOverY);

	int fortress() const;
	void setFortress(int newFortress);

signals:
	void landIdChanged();
	void baseXChanged(qreal x);
	void baseYChanged(qreal y);
	void imgMapChanged();
	void imgBorderChanged();
	void proprietorChanged();
	void gameChanged();
	void xpChanged();
	void overXChanged();
	void overYChanged();
	void fortressChanged();

private:
	QString m_landId;
	qreal m_baseX = 0;
	qreal m_baseY = 0;
	qreal m_overX = 0;
	qreal m_overY = 0;
	QUrl m_imgMap;
	QUrl m_imgBorder;
	int m_proprietor = -1;
	ConquestGame *m_game = nullptr;
	int m_xp = 0;
	int m_fortress = -1;
};

#endif // CONQUESTLANDDATA_H
