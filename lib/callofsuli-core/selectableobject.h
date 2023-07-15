/*
 * ---- Call of Suli ----
 *
 * selectableobject.h
 *
 * Created on: 2023. 01. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * SelectableObject
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

#ifndef SELECTABLEOBJECT_H
#define SELECTABLEOBJECT_H

#include <QObject>
#include <QMap>
#include <QVariant>

class SelectableObject : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectedChanged)

public:
	explicit SelectableObject(QObject *parent = nullptr);

	bool selected() const;
	void setSelected(bool newSelected);

	Q_INVOKABLE QVariantMap toVariantMap() const;
	QJsonObject toJsonObject() const;

	typedef std::function<QVariantMap(const QVariant &)> MapConvertFunc;
	typedef std::function<QJsonObject(const QVariant &)> JsonConvertFunc;

signals:
	void selectedChanged();

protected:
	QMap<QString, MapConvertFunc> m_mapConvertFuncs;
	QMap<QString, JsonConvertFunc> m_jsonConvertFuncs;

private:
	bool m_selected = false;
};

#endif // SELECTABLEOBJECT_H
