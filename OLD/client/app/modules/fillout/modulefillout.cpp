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
#include <QRandomGenerator>

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
 * @brief ModuleFillout::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleFillout::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	Q_UNUSED(storage)
	Q_UNUSED(storageData)

	QVariantList list;

	list.append(generateOne(data));

	return list;
}



/**
 * @brief ModuleFillout::generate
 * @param data
 * @param storage
 * @param storageData
 * @param answer
 * @return
 */

QVariantMap ModuleFillout::generateOne(const QVariantMap &data) const
{
	QString text = data.value("text").toString();

	if (text.isEmpty())
		return QVariantMap();


	struct ItemStruct {
		QString text;
		bool isQuestion;

		ItemStruct(const QString &t, const bool &q) : text(t), isQuestion(q) {}
	};

	QVector<ItemStruct> items;

	QRegularExpressionMatchIterator i = m_expressionWord.globalMatch(text);

	int _cptrd = 0;


	while (i.hasNext())
	{
		QRegularExpressionMatch match = i.next();

		int _s = match.capturedStart();
		if (_s > _cptrd) {
			foreach (QString s, text.mid(_cptrd, _s-_cptrd).split(QRegExp("\\s+"), Qt::SkipEmptyParts))
				items.append(ItemStruct(s.replace("\\%", "%"), false));
		}
		_cptrd = match.capturedStart()+match.capturedLength();

		QString q = text.mid(match.capturedStart(1), match.capturedLength(1)).replace("\\%", "%");

		items.append(ItemStruct(q, true));
	}


	foreach (QString s, text.mid(_cptrd).split(QRegExp("\\s+"), Qt::SkipEmptyParts))
		items.append(ItemStruct(s.replace("\\%", "%"), false));


	int maxQuestion = qMax(data.value("count", -1).toInt(), 1);

	QVector<int> questionIndexList;

	for (int i=0; i<items.size(); ++i) {
		if (items.at(i).isQuestion)
			questionIndexList.append(i);
	}

	QVector<int> usedIndexList;
	QStringList options;

	while (questionIndexList.size() && usedIndexList.size() < maxQuestion) {
		int idx = questionIndexList.takeAt(QRandomGenerator::global()->bounded(questionIndexList.size()));
		usedIndexList.append(idx);
		options.append(items.at(idx).text);
	}


	int maxOptions = qMax(data.value("optionsCount", -1).toInt(), options.size()+1);

	QStringList oList = data.value("options").toStringList();

	while (oList.size() && options.size() < maxOptions) {
		options.append(oList.takeAt(QRandomGenerator::global()->bounded(oList.size())));
	}


	QVariantMap answer;

	QVariantList words;

	for (int i=0; i<items.size(); ++i) {
		if (usedIndexList.contains(i)) {
			QString id = QString("%1").arg(i);

			answer.insert(id, items.at(i).text);

			words.append(QVariantMap({{"q", id}}));
		} else {
			words.append(QVariantMap({{"w", items.at(i).text}}));
		}
	}


	QStringList optList;

	while (options.size())
		optList.append(options.takeAt(QRandomGenerator::global()->bounded(options.size())));



	QVariantMap ret;
	ret["list"] = words;
	ret["options"] = optList;
	ret["answer"] = answer;

	return ret;
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
