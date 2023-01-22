/*
 * ---- Call of Suli ----
 *
 * rank.h
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

#ifndef RANK_H
#define RANK_H

#include "qjsonobject.h"
#include "qjsonarray.h"
#include "qobjectdefs.h"
#include <QString>

class Rank
{
	Q_GADGET

	Q_PROPERTY(int id MEMBER m_id)
	Q_PROPERTY(int level MEMBER m_level)
	Q_PROPERTY(int sublevel MEMBER m_sublevel)
	Q_PROPERTY(int xp MEMBER m_xp)
	Q_PROPERTY(QString name MEMBER m_name)

public:
	Rank();
	Rank(const int &id, const int &level, const int &sublevel, const int &xp, const QString &name)
		: m_id(id)
		, m_level(level)
		, m_sublevel(sublevel)
		, m_xp(xp)
		, m_name(name)
	{}

	bool isValid() const { return m_id != -1; }
	bool isReserved() const { return m_xp < 0; }

	int id() const;
	int level() const;
	int sublevel() const;
	int xp() const;
	const QString &name() const;

	static Rank fromJson(const QJsonObject &object);
	QJsonObject toJson() const;

	friend bool operator==(const Rank &r, const Rank &other) { return r.m_id == other.m_id; }

private:
	int m_id = -1;
	int m_level = -1;
	int m_sublevel = -1;
	int m_xp = 0;
	QString m_name;
	friend class RankList;
};


Q_DECLARE_METATYPE(Rank);


/**
 * @brief The RankList class
 */

typedef QVector<Rank> _RankList;

class RankList : public _RankList
{
public:
	RankList() : _RankList() {}

	static RankList fromJson(const QJsonArray &array);
	QJsonArray toJson() const;

	static RankList defaultRankList(const int &sublevels = 3,
									const int &base_xp = 500,
									const qreal &rank_factor_step = 0.15,
									qreal rank_factor = 1.0);

};


#endif // RANK_H
