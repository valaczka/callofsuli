/*
 * ---- Call of Suli ----
 *
 * modulepair.cpp
 *
 * Created on: 2021. 11. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModulePair
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

#include "modulemergebinding.h"

ModuleMergebinding::ModuleMergebinding(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModulePair::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleMergebinding::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QStringList list;

	QVariantList l = data.value(QStringLiteral("sections")).toList();
	for (const QVariant &v : l) {
		QVariantMap m = v.toMap();
		list.append(m.value(QStringLiteral("name")).toString());
		//list.append(QStringLiteral("%1 — %2").arg(m.value(QStringLiteral("first")).toString()).arg(m.value(QStringLiteral("second")).toString()));
	}

	const QString &name = data.value(QStringLiteral("name")).toString();

	QVariantMap m;
	m[QStringLiteral("title")] = name.isEmpty() ? list.join(", ") : name;
	m[QStringLiteral("details")] = name.isEmpty() ? "" : list.join(", ");
	m[QStringLiteral("image")] = "";

	return m;
}
