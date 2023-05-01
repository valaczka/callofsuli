/*
 * ---- Call of Suli ----
 *
 * modulesimplechoice.cpp
 *
 * Created on: 2021. 08. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleSimplechoice
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

#include <QRandomGenerator>
#include "modulesimplechoice.h"

ModuleSimplechoice::ModuleSimplechoice(QObject *parent) : QObject(parent)
{

}

QString ModuleSimplechoice::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const
{
	const QStringList &options = data.value(QStringLiteral("options")).toStringList();

	QString html = QStringLiteral("<p class=\"options\">");
	html += options.join(QStringLiteral(" • "));
	html += QStringLiteral("</p>");

	if (answer.contains(QStringLiteral("index"))) {
		const int &idx = answer.value(QStringLiteral("index"), -1).toInt();

		if (idx >=0 && idx < options.size()) {
			if (success)
				html += QStringLiteral("<p class=\"answer\">");
			else
				html += QStringLiteral("<p class=\"answerFail\">");

			html += options.at(idx) + QStringLiteral("</p>");
		}
	}

	return html;
}


/**
 * @brief ModuleSimplechoice::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleSimplechoice::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (!storage) {
		QVariantMap m;
		QStringList answers = data.value("answers").toStringList();
		m["title"] = data.value("question").toString();
		m["details"] = data.value("correct").toString()+"<br>("+answers.join(", ")+")";
		m["image"] = "";

		return m;
	} else if (storage->name() == "binding" || storage->name() == "numbers") {
		QStringList answers;

		bool isBindToRight = data.value("mode").toString() == "right";

		foreach (QVariant v, storageData.value("bindings").toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value("first").toString();
			QString right = m.value("second").toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			answers.append(QString("%1 — %2").arg(isBindToRight ? right : left).arg(isBindToRight ? left : right));
		}

		QVariantMap m;
		m["title"] = data.value("question").toString();
		m["details"] = answers.join(", ");
		m["image"] = "";

		return m;
	} else if (storage->name() == "images") {
		QStringList answers;

		QString image = "";

		const QString &mode = data.value("mode").toString();

		foreach (QVariant v, storageData.value("images").toList()) {
			QVariantMap m = v.toMap();
			const int &imgId = m.value("image", -1).toInt();
			const QString &text = m.value("text").toString();

			if (imgId != -1 && image.isEmpty())
				image = QString("image://mapimage/%1").arg(imgId);

			if (!text.isEmpty()) {
				const QString &answersPattern = data.value("answers").toString();
				if (mode == "image" && answersPattern.contains("%1"))
					answers.append(answersPattern.arg(text));
				else
					answers.append(text);
			}
		}

		QVariantMap m;
		m["title"] = data.value("question").toString();
		m["details"] = answers.join(", ");
		m["image"] = image;

		return m;
	}

	return QVariantMap({{"title", ""},
						{"details", ""},
						{"image", ""}
					   });
}




/**
 * @brief ModuleSimplechoice::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (!storage) {
		QVariantList list;
		QVariantMap m;

		m["question"] = data.value("question").toString();

		QString correct = data.value("correct").toString();

		if (correct.isEmpty())
			correct = " ";

		QStringList alist = data.value("answers").toStringList();

		m.insert(generateOne(correct, alist));

		list.append(m);

		return list;
	}

	if (storage->name() == "binding" || storage->name() == "numbers")
		return generateBinding(data, storageData);

	if (storage->name() == "images")
		return generateImages(data, storageData);


	return QVariantList();
}




/**
 * @brief ModuleSimplechoice::generateBinding
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateBinding(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	bool isBindToRight = data.value("mode").toString() == "right";
	QString question = data.value("question").toString();

	foreach (QVariant v, storageData.value("bindings").toList()) {
		QVariantMap m = v.toMap();
		QString left = m.value("first").toString();
		QString right = m.value("second").toString();

		if (left.isEmpty() || right.isEmpty())
			continue;

		QVariantMap retMap;

		if (question.isEmpty())
			retMap["question"] = (isBindToRight ? right : left);
		else if (question.contains("%1"))
			retMap["question"] = question.arg(isBindToRight ? right : left);
		else
			retMap["question"] = question;

		QStringList alist;

		foreach (QVariant v, storageData.value("bindings").toList()) {
			QVariantMap mm = v.toMap();
			QString f1 = mm.value("first").toString();
			QString f2 = mm.value("second").toString();

			if ((isBindToRight && right == f2) || (!isBindToRight && left == f1))
				continue;

			alist.append(isBindToRight ? f1 : f2);
		}

		retMap.insert(generateOne(isBindToRight ? left : right, alist));

		ret.append(retMap);
	}

	return ret;
}




/**
 * @brief ModuleSimplechoice::generateImages
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateImages(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	const QString &mode = data.value("mode").toString();
	const QString &question = data.value("question").toString();
	const QString &answersPattern = data.value("answers").toString();

	foreach (QVariant v, storageData.value("images").toList()) {
		QVariantMap m = v.toMap();
		const int &imgId = m.value("image", -1).toInt();
		const QString &text = m.value("text").toString();

		if (imgId == -1 || text.isEmpty())
			continue;

		QVariantMap retMap;

		if (mode == "text" && question.contains("%1"))
			retMap["question"] = question.arg(text);
		else
			retMap["question"] = question;

		if (mode == "image")
			retMap["image"] = QString("image://mapimage/%1").arg(imgId);
		else
			retMap["imageAnswers"] = true;

		QStringList alist;

		foreach (QVariant v, storageData.value("images").toList()) {
			QVariantMap mm = v.toMap();
			const int &f1 = mm.value("image", -1).toInt();
			const QString &f2 = mm.value("text").toString();

			if ((mode == "image" && text == f2) || (mode == "text" && (imgId == f1 || f1 == -1)))
				continue;

			if (mode == "image" && answersPattern.contains("%1"))
				alist.append(answersPattern.arg(f2));
			else if (mode == "image")
				alist.append(f2);
			else
				alist.append(QString("image://mapimage/%1").arg(f1));
		}

		if (mode == "image" && answersPattern.contains("%1"))
			retMap.insert(generateOne(answersPattern.arg(text), alist));
		else if (mode == "image")
			retMap.insert(generateOne(text, alist));
		else
			retMap.insert(generateOne(QString("image://mapimage/%1").arg(imgId), alist));

		ret.append(retMap);
	}

	return ret;
}





/**
 * @brief ModuleSimplechoice::generateOne
 * @param correctAnswer
 * @param options
 * @return
 */

QVariantMap ModuleSimplechoice::generateOne(const QString &correctAnswer, QStringList optionsList) const
{
	QVector<QPair<QString, bool>> opts;
	opts.append(qMakePair(correctAnswer, true));

	while (optionsList.size() && opts.size() < 4) {
		QString o = optionsList.takeAt(QRandomGenerator::global()->bounded(optionsList.size()));
		if (!o.isEmpty())
			opts.append(qMakePair(o, false));
	}


	int correctIdx = -1;

	QStringList optList;

	while (opts.size()) {
		QPair<QString, bool> p = opts.takeAt(QRandomGenerator::global()->bounded(opts.size()));
		optList.append(p.first);

		if (p.second)
			correctIdx = optList.size()-1;
	}

	QVariantMap m;
	m["answer"] = correctIdx;
	m["options"] = optList;

	return m;
}


/**
 * @brief ModuleSimplechoice::preview
 * @param generatedList
 * @return
 */

QVariantMap ModuleSimplechoice::preview(const QVariantList &generatedList) const
{
	QVariantMap m;
	QString s;

	foreach (QVariant v, generatedList) {
		QVariantMap m = v.toMap();

		const QString &image = m.value("image").toString();
		const bool &imageAnswers = m.value("imageAnswers").toBool();

		s.append((image.isEmpty() ? "" : tr("[KÉP] "))
				 +"**"+m.value("question").toString()+"**\n");

		int correct = m.value("answer", -1).toInt();
		QStringList l = m.value("options").toStringList();
		for (int i=0; i<l.size(); ++i) {
			if (imageAnswers) {
				if (i==correct)
					s.append("- **"+tr("[KÉP]")+"**\n");
				else
					s.append("- "+tr("[KÉP]")+"\n");
			} else {
				if (i==correct)
					s.append("- **"+l.at(i)+"**\n");
				else
					s.append("- "+l.at(i)+"\n");
			}
		}
		s.append("---\n");
	}

	m["text"] = s;

	return m;
}
