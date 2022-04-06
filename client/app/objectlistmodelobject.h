/*
 * ---- Call of Suli ----
 *
 * objectlistmodelobject.h
 *
 * Created on: 2021. 12. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ObjectListModelObject
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

#ifndef OBJECTLISTMODELOBJECT_H
#define OBJECTLISTMODELOBJECT_H

#include <QObject>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaObject>
#include <QMetaProperty>


class ObjectListModelObject : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectedChanged STORED false)

public:
	Q_INVOKABLE explicit ObjectListModelObject(QObject *parent = nullptr);
	virtual ~ObjectListModelObject();

	bool selected() const;
	void setSelected(bool newSelected);

	QJsonObject toJsonObject() const;

	void updateProperties(QObject *other);

	template <typename T>
	static T *fromJsonObject(const QJsonObject &jsonObject, QObject *parent = nullptr) {
		return qobject_cast<T*>(fromJsonObjectPrivate(T::staticMetaObject, jsonObject, parent));
	}

protected:
	static QObject *fromJsonObjectPrivate(const QMetaObject objectType, const QJsonObject &jsonObject, QObject *parent = nullptr);

signals:
	void selectedChanged();

private:
	bool m_selected;
};




#endif // OBJECTLISTMODELOBJECT_H
