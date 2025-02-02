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
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class ClassObject;
using ClassList = qolm::QOlm<ClassObject>;
Q_DECLARE_METATYPE(ClassList*)

/**
 * @brief The ClassObject class
 */

class ClassObject : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(int classid READ classid WRITE setClassid NOTIFY classidChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(int dailyLimit READ dailyLimit WRITE setDailyLimit NOTIFY dailyLimitChanged FINAL)

public:
	explicit ClassObject(QObject *parent = nullptr);
	virtual ~ClassObject() {}

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	const QString &name() const;
	void setName(const QString &newName);

	int classid() const;
	void setClassid(int newClassid);

	int dailyLimit() const;
	void setDailyLimit(int newDailyLimit);

signals:
	void nameChanged();
	void classidChanged();
	void dailyLimitChanged();

private:
	int m_classid = -1;
	QString m_name;
	int m_dailyLimit;
};

#endif // CLASSOBJECT_H
