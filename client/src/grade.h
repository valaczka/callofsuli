/*
 * ---- Call of Suli ----
 *
 * grade.h
 *
 * Created on: 2023. 04. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Grade
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

#ifndef GRADE_H
#define GRADE_H

#include <selectableobject.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class Grade;
using GradeList = qolm::QOlm<Grade>;
Q_DECLARE_METATYPE(GradeList*)


/**
 * @brief The Grade class
 */

class Grade : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(int gradeid READ gradeid WRITE setGradeid NOTIFY gradeidChanged)
	Q_PROPERTY(QString shortname READ shortname WRITE setShortname NOTIFY shortnameChanged)
	Q_PROPERTY(QString longname READ longname WRITE setLongname NOTIFY longnameChanged)
	Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)

public:
	explicit Grade(QObject *parent = nullptr);
	virtual ~Grade() {}

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	int gradeid() const;
	void setGradeid(int newGradeid);

	const QString &shortname() const;
	void setShortname(const QString &newShortname);

	const QString &longname() const;
	void setLongname(const QString &newLongname);

	int value() const;
	void setValue(int newValue);

signals:
	void gradeidChanged();
	void shortnameChanged();
	void longnameChanged();
	void valueChanged();

private:
	int m_gradeid = 0;
	QString m_shortname;
	QString m_longname;
	int m_value = 0;

};

#endif // GRADE_H
