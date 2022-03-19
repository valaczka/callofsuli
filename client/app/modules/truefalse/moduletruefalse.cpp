/*
 * ---- Call of Suli ----
 *
 * moduletruefalse.cpp
 *
 * Created on: 2021. 08. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleTruefalse
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

#include "moduletruefalse.h"

ModuleTruefalse::ModuleTruefalse(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModuleTruefalse::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleTruefalse::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QVariantMap m;
	m["title"] = data.value("question").toString();
	m["details"] = data.value("correct").toBool() ? QObject::tr("igaz") : QObject::tr("hamis");
	m["image"] = "";

	return m;
}



/**
 * @brief ModuleTruefalse::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleTruefalse::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storageData)

	if (!storage) {
		QVariantList list;
		QVariantMap m = data;

		m.insert("answer", data.value("correct").toBool());

		list.append(m);

		return list;
	}

	return QVariantList();
}



