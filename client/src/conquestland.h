/*
 * ---- Call of Suli ----
 *
 * conqueststate.h
 *
 * Created on: 2024. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestLand
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

#ifndef CONQUESTLAND_H
#define CONQUESTLAND_H

#include "conquestlanddata.h"
#include <QQuickItem>

class ConquestLand : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(ConquestLandData *landData READ landData WRITE setLandData NOTIFY landDataChanged FINAL)
	Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged FINAL)

	QML_ELEMENT

public:
	ConquestLand();

	ConquestLandData *landData() const;
	void setLandData(ConquestLandData *newLandData);

	QColor baseColor() const;
	void setBaseColor(const QColor &newBaseColor);

signals:
	void landDataChanged();
	void baseColorChanged();

private:
	QPointer<ConquestLandData> m_landData;
	QColor m_baseColor;

};

#endif // CONQUESTLAND_H
