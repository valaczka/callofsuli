/*
 * ---- Call of Suli ----
 *
 * rank.cpp
 *
 * Created on: 2023. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Rank
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

#include "rank.h"
#include <math.h>

Rank::Rank()
{

}

int Rank::id() const
{
	return m_id;
}

int Rank::level() const
{
	return m_level;
}

int Rank::sublevel() const
{
	return m_sublevel;
}

int Rank::xp() const
{
	return m_xp;
}

const QString &Rank::name() const
{
	return m_name;
}


/**
 * @brief Rank::fromJson
 * @param object
 * @return
 */

Rank Rank::fromJson(const QJsonObject &object)
{
	Rank r;

	r.m_id = object.value(QStringLiteral("id")).toInt(-1);

	if (r.m_id == -1)
		return r;

	r.m_level = object.value(QStringLiteral("level")).toInt(-1);
	r.m_sublevel = object.value(QStringLiteral("sublevel")).toInt(-1);
	r.m_xp = object.value(QStringLiteral("xp")).toInt(0);
	r.m_name = object.value(QStringLiteral("name")).toString();

	return r;
}


/**
 * @brief Rank::toJson
 * @return
 */

QJsonObject Rank::toJson() const
{
	QJsonObject o;
	o[QStringLiteral("id")] = m_id;
	o[QStringLiteral("level")] = m_level;
	o[QStringLiteral("sublevel")] = m_sublevel;
	o[QStringLiteral("xp")] = m_xp;
	o[QStringLiteral("name")] = m_name;
	return o;
}



/**
 * @brief RankList::fromJson
 * @param array
 * @return
 */

RankList RankList::fromJson(const QJsonArray &array)
{
	RankList list;

	list.reserve(array.size());

	foreach (const QJsonValue &v, array) {
		const Rank &r = Rank::fromJson(v.toObject());
		if (r.isValid())
			list.append(r);
	}

	list.squeeze();

	return list;
}



/**
 * @brief RankList::toJson
 * @return
 */

QJsonArray RankList::toJson() const
{
	QJsonArray list;
	for (const Rank &r : *this)
		list.append(r.toJson());
	return list;
}


/**
 * @brief RankList::defaultRankList
 * @return
 */

RankList RankList::defaultRankList(const int &sublevels, const int &base_xp, const qreal &rank_factor_step, qreal rank_factor)
{
	QStringList ranks;

	ranks << QObject::tr("közkatona")
		  << QObject::tr("őrvezető")
		  << QObject::tr("tizedes")
		  << QObject::tr("szakaszvezető")
		  << QObject::tr("őrmester")
		  << QObject::tr("törzsőrmester")
		  << QObject::tr("főtörzsőrmester")
		  << QObject::tr("zászlós")
		  << QObject::tr("törzszászlós")
		  << QObject::tr("főtörzszászlós")
		  << QObject::tr("alhadnagy")
		  << QObject::tr("hadnagy")
		  << QObject::tr("főhadnagy")
		  << QObject::tr("százados")
		  << QObject::tr("őrnagy")
		  << QObject::tr("alezredes")
		  << QObject::tr("ezredes")
		  << QObject::tr("dandártábornok")
		  << QObject::tr("vezérőrnagy")
		  << QObject::tr("altábornagy");

	RankList list;
	list.reserve(ranks.size()*sublevels+1);

	int multiply = 0;

	int id = 1;

	for (int n=0; n<ranks.size(); n++) {
		const QString &rank = ranks.at(n);
		for (int i=1; i<=sublevels; i++) {
			Rank r;
			r.m_id = id++;
			r.m_level = n;
			r.m_sublevel = i;
			r.m_name = rank;
			r.m_xp = (int) round(base_xp*rank_factor*multiply);
			list.append(r);

			multiply += 1+rank_factor_step;
		}
		rank_factor += rank_factor_step;
	}


	// Reserved rank

	Rank r;
	r.m_id = id++;
	r.m_level = 100;
	r.m_sublevel = -1;
	r.m_name = QObject::tr("vezérezredes");
	r.m_xp = -1;
	list.append(r);

	list.squeeze();

	return list;
}
