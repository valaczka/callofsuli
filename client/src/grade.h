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

#include "qjsonarray.h"
#include <selectableobject.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"

class Grade;
using GradeList = qolm::QOlm<Grade>;
Q_DECLARE_METATYPE(GradeList*)


class GradingConfig;
using GradingConfigList = qolm::QOlm<GradingConfig>;
Q_DECLARE_METATYPE(GradingConfigList*)


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




/**
 * @brief The GradingConfig class
 */

class GradingConfig : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(QVariantList list READ list NOTIFY listChanged FINAL)

public:
	explicit GradingConfig(QObject *parent = nullptr);

	void fromJson(const QJsonArray &list, GradeList *gradeList);
	QJsonArray toJson() const;

	Q_INVOKABLE void fill(GradeList *list);

	Q_INVOKABLE Grade *grade(const qreal &num) const;

	Q_INVOKABLE void gradeSet(Grade *grade, const qreal &num, const bool &set);
	Q_INVOKABLE void gradeRemove(Grade *grade);

	Q_INVOKABLE void save() const;

	QVariantList list() const;

signals:
	void listChanged();

private:
	QHash<Grade*, QPair<qreal, bool>> m_gradeMap;
};

#endif // GRADE_H
