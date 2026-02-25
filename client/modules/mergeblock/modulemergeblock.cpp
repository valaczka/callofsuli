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

#include "modulemergeblock.h"

ModuleMergeblock::ModuleMergeblock(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModulePair::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleMergeblock::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QStringList list;
	QStringList blockNames;

	QVariantList l = data.value(QStringLiteral("sections")).toList();
	for (const QVariant &v : l) {
		QVariantMap m = v.toMap();
		list.append(m.value(QStringLiteral("name")).toString());

		QVariantList l = m.value(QStringLiteral("blocks")).toList();
		for (const QVariant &v : l) {
			QVariantMap m = v.toMap();
			const QString &n = m.value(QStringLiteral("first")).toString();
			if (n.isEmpty())
				continue;
			///list.append(QStringLiteral("%1 [%2]").arg(n, m.value(QStringLiteral("second")).toStringList().join(QStringLiteral(", "))));
			blockNames.append(n);
		}
	}

	const QString &name = data.value(QStringLiteral("name")).toString();

	QVariantMap m;
	m[QStringLiteral("title")] = name.isEmpty() ? blockNames.join(QStringLiteral(", ")) : name;
	m[QStringLiteral("details")] = list.join(QStringLiteral("\n"));
	m[QStringLiteral("image")] = "";

	return m;
}



/**
 * @brief ModuleMergeblock::getUnion
 * @param blocks
 * @return
 */

ModuleMergeblock::BlockUnion ModuleMergeblock::getUnion(const QVariantList &blocks)
{
	BlockUnion ret;

	for (int j=0; j<blocks.size(); ++j) {
		const QVariantMap &m = blocks.at(j).toMap();

		const QString &left = m.value(QStringLiteral("first")).toString().simplified();
		QStringList right = m.value(QStringLiteral("second")).toStringList();

		if (left.isEmpty() || right.isEmpty())
			continue;

		Data data {.blockidx = (j+1)*1000, .content = right};

		ret[left].push_back(std::move(data));
	}

	return ret;
}


/**
 * @brief ModuleMergeblock::getUnion
 * @param sections
 * @param usedSections
 * @return
 */

ModuleMergeblock::BlockUnion ModuleMergeblock::getUnion(const QVariantList &sections, const QStringList &usedSections)
{
	BlockUnion ret;

	for (int i=0; i<sections.size(); ++i) {
		QVariantMap m = sections.at(i).toMap();
		const QString &key = m.value(QStringLiteral("key")).toString();

		if (!usedSections.contains(key))
			continue;

		const QVariantList &l = m.value(QStringLiteral("blocks")).toList();

		for (int j=0; j<l.size(); ++j) {
			const QVariantMap &m = l.at(j).toMap();

			const QString &left = m.value(QStringLiteral("first")).toString().simplified();
			QStringList right = m.value(QStringLiteral("second")).toStringList();

			if (left.isEmpty() || right.isEmpty())
				continue;

			Data data {.blockidx = (i+1)*100000 + (j+1)*1000, .content = right};

			ret[left].push_back(std::move(data));
		}
	}

	return ret;
}
