/*
 * ---- Call of Suli ----
 *
 * modulefillout.cpp
 *
 * Created on: 2021. 11. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleFillout
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

#include "modulefillout.h"
#include "fillouthighlighter.h"
#include <QQmlEngine>

const QRegularExpression ModuleFillout::m_expressionWord("(?<!\\\\)%((?:[^%\\\\]|\\\\.)+)%");


ModuleFillout::ModuleFillout(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModuleFillout::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleFillout::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QStringList answers = data.value("answers").toStringList();

	QVariantMap m;
	m["title"] = data.value("text").toString();
	m["details"] = answers.join(", ");
	m["image"] = "";

	return m;
}


/**
 * @brief ModuleFillout::generate
 * @param data
 * @param storage
 * @param storageData
 * @param answer
 * @return
 */

QVariantMap ModuleFillout::generate(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData, QVariantMap *answer) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	return QVariantMap();
}


/**
 * @brief ModuleFillout::registerQmlTypes
 */

void ModuleFillout::registerQmlTypes() const
{
	qmlRegisterType<FilloutHighlighter>("COS.Client", 1, 0, "FilloutHighlighter");
}


/**
 * @brief ModuleFillout::expressionWord
 * @return
 */

const QRegularExpression &ModuleFillout::expressionWord()
{
	return m_expressionWord;
}
