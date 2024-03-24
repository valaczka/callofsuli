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
#include "../writer/modulewriter.h"



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
	const QVariantMap &correctMap = data.value(QStringLiteral("answer")).toMap();

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

			if (const QString &c = correctMap.value(m.value(QStringLiteral("q")).toString()).toString(); !success && !c.isEmpty()) {
				html += QStringLiteral("<span class=\"answerCorrect\">")
						+ c + QStringLiteral("</span> ");
			}
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
	if (!storage) {
		const QStringList &answers = data.value(QStringLiteral("answers")).toStringList();

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("text")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = QStringLiteral("");

		return m;
	} else if (storage->name() == QStringLiteral("text")) {
		QStringList list = storageData.value(QStringLiteral("items")).toStringList();

		QVariantMap m;
		m[QStringLiteral("title")] = list.isEmpty() ? QStringLiteral("") : list.at(0);
		if (!list.isEmpty())
			list.removeFirst();
		m[QStringLiteral("details")] = list.isEmpty() ? QStringLiteral("") : list.join(QStringLiteral("\n"));
		m[QStringLiteral("image")] = QStringLiteral("");

		return m;
	}

	return QVariantMap({{QStringLiteral("title"), QStringLiteral("")},
						{QStringLiteral("details"), QStringLiteral("")},
						{QStringLiteral("image"), QStringLiteral("")}
					   });
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
	if (!storage)
		return QVariantList{ generateOne(data)};

	if (storage->name() == QStringLiteral("text"))
		return generateText(data, storageData);

	if (storage->name() == QStringLiteral("sequence"))
		return generateSequence(data, storageData);

	return QVariantList();
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
	QString text = data.value(QStringLiteral("text")).toString();

	if (text.isEmpty())
		return QVariantMap();


	struct ItemStruct {
		QString text;
		bool isQuestion;

		ItemStruct(const QString &t, const bool &q) : text(t), isQuestion(q) {}
	};

	QVector<ItemStruct> items;

	QRegularExpressionMatchIterator i = ModuleWriter::m_expressionWord.globalMatch(text);

	int _cptrd = 0;

	int qCount = 0;

	static const QRegularExpression regExp("\\s+");

	while (i.hasNext())
	{
		QRegularExpressionMatch match = i.next();

		int _s = match.capturedStart();
		if (_s > _cptrd) {
			foreach (QString s, text.mid(_cptrd, _s-_cptrd).split(regExp, Qt::SkipEmptyParts))
				items.append(ItemStruct(s.replace(QStringLiteral("\\%"), QStringLiteral("%")), false));
		}
		_cptrd = match.capturedStart()+match.capturedLength();

		QString q = text.mid(match.capturedStart(1), match.capturedLength(1)).replace(QStringLiteral("\\%"), QStringLiteral("%"));

		items.append(ItemStruct(q, true));
		++qCount;
	}


	foreach (QString s, text.mid(_cptrd).split(regExp, Qt::SkipEmptyParts))
		items.append(ItemStruct(s.replace(QStringLiteral("\\%"), QStringLiteral("%")), false));


	int maxQuestion = 0;

	if (const int &c = data.value(QStringLiteral("count"), -1).toInt(); c > 0)
		maxQuestion = qMin(c, qCount);
	else
		maxQuestion = qCount;

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
		const QString &txt = items.at(idx).text;
		if (!txt.isEmpty())
			options.append(txt);
	}


	int maxOptions = 0;

	if (const int &c = data.value(QStringLiteral("optionsCount"), -1).toInt(); c > 0)
		maxOptions = qMin(c, options.size()+1);
	else
		maxOptions = 0;

	QStringList oList = data.value(QStringLiteral("options")).toStringList();

	while (oList.size() && options.size() < maxOptions) {
		options.append(oList.takeAt(QRandomGenerator::global()->bounded(oList.size())));
	}


	QVariantMap answer;

	QVariantList words;

	for (int i=0; i<items.size(); ++i) {
		if (usedIndexList.contains(i)) {
			QString id = QStringLiteral("%1").arg(i);

			answer.insert(id, items.at(i).text);

			words.append(QVariantMap({{QStringLiteral("q"), id}}));
		} else {
			words.append(QVariantMap({{QStringLiteral("w"), items.at(i).text}}));
		}
	}


	QStringList optList;

	while (options.size())
		optList.append(options.takeAt(QRandomGenerator::global()->bounded(options.size())));



	QVariantMap ret;
	ret[QStringLiteral("list")] = words;
	ret[QStringLiteral("options")] = optList;
	ret[QStringLiteral("answer")] = answer;
	ret[QStringLiteral("question")] = data.value(QStringLiteral("question")).toString();

	return ret;
}





/**
 * @brief ModuleFillout::generateText
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleFillout::generateText(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	foreach (const QString &text, storageData.value(QStringLiteral("items")).toStringList()) {
		if (text.isEmpty())
			continue;

		QVariantMap d;
		d[QStringLiteral("text")] = text;
		d[QStringLiteral("question")] = data.value(QStringLiteral("question")).toString();

		ret.append(generateOne(d));
	}

	return ret;
}




/**
 * @brief ModuleFillout::generateSequence
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleFillout::generateSequence(const QVariantMap &data, const QVariantMap &storageData) const
{
	const QStringList &list = storageData.value(QStringLiteral("items")).toStringList();
	const int &words = data.value(QStringLiteral("words")).toInt();
	const int &maxQuestion = data.value(QStringLiteral("count"), -1).toInt();

	int start = 0;
	int end = list.size();

	if (words > 0 && list.size() > words) {
		start = QRandomGenerator::global()->bounded(list.size()-words);
		end = start+words;
	}

	QVector<int> indices;

	for (int i=start; i<end; ++i)
		indices.append(i);


	while (indices.size() && maxQuestion > 0 && indices.size() > maxQuestion)
		indices.removeAt(QRandomGenerator::global()->bounded(indices.size()));


	QVariantMap answer;
	QVariantList wordList;

	for (int i=start; i<end; ++i) {
		if (indices.contains(i)) {
			QString w = list.at(i).simplified();

			QString wPunct, wPrep;

			while (!w.isEmpty() && ModuleWriter::m_punctation.contains(w.at(0))) {
				wPrep.append(w.at(0));
				w.remove(0, 1);
			}

			while (!w.isEmpty() && ModuleWriter::m_punctation.contains(w.right(1))) {
				wPunct.prepend(w.right(1));
				w.chop(1);
			}

			if (!wPrep.isEmpty())
				wordList.append(QVariantMap({{QStringLiteral("w"), wPrep}}));

			const QString &id = QStringLiteral("%1").arg(i);

			answer.insert(id, w);

			wordList.append(QVariantMap({{QStringLiteral("q"), id}}));

			if (!wPunct.isEmpty())
				wordList.append(QVariantMap({{QStringLiteral("w"), wPunct}}));

		} else {
			wordList.append(QVariantMap({{QStringLiteral("w"), list.at(i).simplified()}}));
		}
	}


	QStringList optList;

	for (const QVariant &v : std::as_const(answer))
		optList.append(v.toString());


	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(optList.begin(), optList.end(), g);


	QVariantMap ret;
	ret[QStringLiteral("list")] = wordList;
	ret[QStringLiteral("options")] = optList;
	ret[QStringLiteral("answer")] = answer;
	ret[QStringLiteral("question")] = data.value(QStringLiteral("question")).toString();

	return QVariantList{ret};

}


/**
 * @brief ModuleFillout::registerQmlTypes
 */

void ModuleFillout::registerQmlTypes() const
{
	qmlRegisterType<FilloutHighlighter>("CallOfSuli", 1, 0, "FilloutHighlighter");
}
