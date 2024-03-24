/*
 * ---- Call of Suli ----
 *
 * grade.cpp
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

#include "grade.h"
#include "Logger.h"
#include "qjsonobject.h"
#include "clientcache.h"

Grade::Grade(QObject *parent)
	: SelectableObject{parent}
{

}


/**
 * @brief Grade::loadFromJson
 * @param object
 * @param allField
 */

void Grade::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setGradeid(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("shortname")) || allField)
		setShortname(object.value(QStringLiteral("shortname")).toString());

	if (object.contains(QStringLiteral("longname")) || allField)
		setLongname(object.value(QStringLiteral("longname")).toString());

	if (object.contains(QStringLiteral("value")) || allField)
		setValue(object.value(QStringLiteral("value")).toInt());
}



/**
 * @brief Grade::gradeid
 * @return
 */

int Grade::gradeid() const
{
	return m_gradeid;
}

void Grade::setGradeid(int newGradeid)
{
	if (m_gradeid == newGradeid)
		return;
	m_gradeid = newGradeid;
	emit gradeidChanged();
}

const QString &Grade::shortname() const
{
	return m_shortname;
}

void Grade::setShortname(const QString &newShortname)
{
	if (m_shortname == newShortname)
		return;
	m_shortname = newShortname;
	emit shortnameChanged();
}

const QString &Grade::longname() const
{
	return m_longname;
}

void Grade::setLongname(const QString &newLongname)
{
	if (m_longname == newLongname)
		return;
	m_longname = newLongname;
	emit longnameChanged();
}

int Grade::value() const
{
	return m_value;
}

void Grade::setValue(int newValue)
{
	if (m_value == newValue)
		return;
	m_value = newValue;
	emit valueChanged();
}




/**
 * @brief GradingConfig::GradingConfig
 * @param parent
 */

GradingConfig::GradingConfig(QObject *parent)
	: SelectableObject(parent)
{

}


/**
 * @brief GradingConfig::fromJson
 * @param list
 * @return
 */

void GradingConfig::fromJson(const QJsonArray &list, GradeList *gradeList)
{
	if (!gradeList)
		return;

	for (const QJsonValue &v : std::as_const(list)) {
		const QJsonObject &o = v.toObject();

		const int &id = o.value(QStringLiteral("id")).toInt();

		Grade *g = OlmLoader::find<Grade>(gradeList, "gradeid", id);

		if (!g) {
			LOG_CWARNING("client") << "GradeID not found" << id;
			continue;
		}

		gradeSet(g, o.value(QStringLiteral("value")).toDouble(), o.value(QStringLiteral("set")).toBool());
	}
}


/**
 * @brief GradingConfig::fill
 * @param list
 */

void GradingConfig::fill(GradeList *list)
{
	m_gradeMap.clear();

	if (!list)
		return;

	for (Grade *g : std::as_const(*list)) {
		m_gradeMap.insert(g, qMakePair(0, false));
	}

	emit listChanged();
}


/**
 * @brief GradingConfig::grade
 * @param num
 * @return
 */

Grade *GradingConfig::grade(const qreal &num) const
{
	Grade *g = nullptr;
	qreal n = -1;

#if QT_VERSION >= 0x060400
	for (auto [grade, value] : m_gradeMap.asKeyValueRange()) {
#else
	for (auto it=m_gradeMap.constBegin(); it != m_gradeMap.constEnd(); ++it) {
		auto grade = it.key();
		auto value = it.value();
#endif
		if (!value.second)
			continue;

		if (!g && num >= value.first) {
			g = grade;
			n = value.first;
		} else if (g && num >= value.first && value.first > n) {
			g = grade;
			n = value.first;
		} else if (g && num >= value.first && value.first == n && grade->value() > g->value()) {
			g = grade;
			n = value.first;
		}
	}

	return g;
}




/**
 * @brief GradingConfig::gradeAdd
 * @param grade
 * @param num
 */

void GradingConfig::gradeSet(Grade *grade, const qreal &num, const bool &set)
{
	if (!grade)
		return;

	m_gradeMap.insert(grade, qMakePair(num, set));

	emit listChanged();
}



/**
 * @brief GradingConfig::gradeRemove
 * @param grade
 */

void GradingConfig::gradeRemove(Grade *grade)
{
	m_gradeMap.remove(grade);
	emit listChanged();
}




/**
 * @brief GradingConfig::list
 * @return
 */

QVariantList GradingConfig::list() const
{
	QVariantList list;

	QMap<int, Grade*> tmp;
	auto keyList = m_gradeMap.keys();

	for (Grade *g : keyList)
		tmp.insert(g->value(), g);

	for (Grade *g : tmp) {
		const auto &value = m_gradeMap.value(g);
		list.append(QVariantMap{
						{ QStringLiteral("grade"), QVariant::fromValue(g) },
						{ QStringLiteral("value"), value.first },
						{ QStringLiteral("set"), value.second },
					});
	}


	return list;
}




/**
 * @brief GradingConfig::toJson
 * @return
 */

QJsonArray GradingConfig::toJson() const
{
	QJsonArray list;

#if QT_VERSION >= 0x060400
	for (auto [grade, value] : m_gradeMap.asKeyValueRange()) {
#else
	for (auto it=m_gradeMap.constBegin(); it != m_gradeMap.constEnd(); ++it) {
		auto grade = it.key();
		auto value = it.value();
#endif

		if (!grade)
			continue;

		list.append(QJsonObject{
						{ QStringLiteral("id"), grade->gradeid() },
						{ QStringLiteral("value"), value.first },
						{ QStringLiteral("set"), value.second },
					});
	}

	return list;
}

