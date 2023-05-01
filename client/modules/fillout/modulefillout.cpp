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
 * @brief ModuleFillout::testResult
 * @param data
 * @param answer
 * @param success
 * @return
 */

QString ModuleFillout::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &) const
{
	const QStringList &options = data.value(QStringLiteral("options")).toStringList();

	QString html = QStringLiteral("<p class=\"options\">");
	html += options.join(QStringLiteral(" • "));
	html += QStringLiteral("</p>");

	const QVariantList &qList = data.value(QStringLiteral("list")).toList();
	const QVariantList &aList = answer.value(QStringLiteral("list")).toList();

	html += QStringLiteral("<p>");

	int aidx = 0;

	for (int i=0; i<qList.size(); ++i) {
		const QVariantMap &m = qList.at(i).toMap();
		if (m.contains(QStringLiteral("q"))) {
			QString a;
			bool success  = false;

			if (aidx < aList.size()) {
				const QVariantMap &m = aList.at(aidx).toMap();
				success = m.value(QStringLiteral("success"), false).toBool();
				a = m.value(QStringLiteral("answer")).toString();
				aidx++;
			}

			if (success)
				html += QStringLiteral("<span class=\"answer\">");
			else
				html += QStringLiteral("<span class=\"answerFail\">");

			if (a.isEmpty())
				a = QStringLiteral("&nbsp;").repeated(10);

			html += a + QStringLiteral("</span> ");
		} else {
			html += m.value(QStringLiteral("w")).toString() + QStringLiteral(" ");
		}
	}

	html += QStringLiteral("</p>");

	return html;
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
	ret["question"] = data.value("question").toString();

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
