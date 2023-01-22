/*
 * ---- Call of Suli ----
 *
 * classobject.h
 *
 * Created on: 2023. 01. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ClassObject
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

#ifndef CLASSOBJECT_H
#define CLASSOBJECT_H

#include <selectableobject.h>
#include <QObject>
#include "QOlm/QOlm.hpp"

class ClassObject;
using ClassList = qolm::QOlm<ClassObject>;

/**
 * @brief The ClassObject class
 */

class ClassObject : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
	explicit ClassObject(QObject *parent = nullptr);
	virtual ~ClassObject() {}

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	int id() const;
	void setId(int newId);

	const QString &name() const;
	void setName(const QString &newName);

signals:
	void idChanged();
	void nameChanged();

private:
	int m_id = -1;
	QString m_name;
};

#endif // CLASSOBJECT_H
