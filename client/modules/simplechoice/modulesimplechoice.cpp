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
#include "question.h"

ModuleSimplechoice::ModuleSimplechoice(QObject *parent) : QObject(parent)
{

}

QString ModuleSimplechoice::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const
{
	const QStringList &options = data.value(QStringLiteral("options")).toStringList();

	QString html;

	if (data.value(QStringLiteral("monospace")).toBool())
		html += Question::monspaceTagStart();

	html += QStringLiteral("<p class=\"options\">");
	html += options.join(QStringLiteral(" • "));
	html += QStringLiteral("</p>");

	html += QStringLiteral("<p>");

	if (answer.contains(QStringLiteral("index"))) {
		const int &idx = answer.value(QStringLiteral("index"), -1).toInt();

		if (idx >=0 && idx < options.size()) {
			if (success)
				html += QStringLiteral("<span class=\"answer\">");
			else
				html += QStringLiteral("<span class=\"answerFail\">");

			html += options.at(idx) + QStringLiteral("</span>");

		}
	}

	if (const int cIdx = data.value(QStringLiteral("answer")).toInt(); !success && cIdx >=0 && cIdx < options.size()) {
		html += QStringLiteral(" <span class=\"answerCorrect\">")
				+ options.at(cIdx) + QStringLiteral("</span>");
	}

	html + QStringLiteral("</p>");

	if (data.value(QStringLiteral("monospace")).toBool())
		html += Question::monspaceTagEnd();

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
		QStringList answers = data.value(QStringLiteral("answers")).toStringList();
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = data.value(QStringLiteral("correct")).toString()+QStringLiteral("<br>(")+answers.join(QStringLiteral(", "))+QStringLiteral(")");
		m[QStringLiteral("image")] = QStringLiteral("");

		return m;
	} else if (storage->name() == QStringLiteral("binding") || storage->name() == QStringLiteral("numbers")) {
		QStringList answers;

		bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");

		foreach (QVariant v, storageData.value(QStringLiteral("bindings")).toList()) {
			QVariantMap m = v.toMap();
			QString left = m.value(QStringLiteral("first")).toString();
			QString right = m.value(QStringLiteral("second")).toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			answers.append(QStringLiteral("%1 — %2").arg(isBindToRight ? right : left, isBindToRight ? left : right));
		}

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = QStringLiteral("");

		return m;
	} else if (storage->name() == QStringLiteral("images")) {
		QStringList answers;

		QString image = QStringLiteral("");

		const QString &mode = data.value(QStringLiteral("mode")).toString();

		foreach (QVariant v, storageData.value(QStringLiteral("images")).toList()) {
			QVariantMap m = v.toMap();
			const int &imgId = m.value(QStringLiteral("image"), -1).toInt();
			const QString &text = m.value(QStringLiteral("text")).toString();

			if (imgId != -1 && image.isEmpty())
				image = QStringLiteral("image://mapimage/%1").arg(imgId);

			if (!text.isEmpty()) {
				const QString &answersPattern = data.value(QStringLiteral("answers")).toString();
				if (mode == QStringLiteral("image") && answersPattern.contains(QStringLiteral("%1")))
					answers.append(answersPattern.arg(text));
				else
					answers.append(text);
			}
		}

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = image;

		return m;
	} else if (storage->name() == QStringLiteral("block")) {
		QStringList answers;

		foreach (const QVariant &v, storageData.value(QStringLiteral("blocks")).toList()) {
			const QVariantMap &m = v.toMap();
			const QString &left = m.value(QStringLiteral("first")).toString();
			const QString &right = m.value(QStringLiteral("second")).toStringList().join(QStringLiteral(", "));

			answers.append(QStringLiteral("%1 [%2]").arg(left, right));
		}

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = QStringLiteral("");

		return m;
	}

	return QVariantMap({{QStringLiteral("title"), QStringLiteral("")},
						{QStringLiteral("details"), QStringLiteral("")},
						{QStringLiteral("image"), QStringLiteral("")}
					   });
}




/**
 * @brief ModuleSimplechoice::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
											 QVariantMap *commonDataPtr) const
{
	Q_UNUSED(commonDataPtr);

	if (!storage) {
		QVariantList list;
		QVariantMap m;

		m[QStringLiteral("question")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

		QString correct = data.value(QStringLiteral("correct")).toString();

		if (correct.isEmpty())
			correct = QStringLiteral(" ");

		QStringList alist = data.value(QStringLiteral("answers")).toStringList();

		m.insert(generateOne(correct, alist, data.value(QStringLiteral("maxOptions")).toInt()));

		list.append(m);

		return list;
	}

	if (storage->name() == QStringLiteral("binding") || storage->name() == QStringLiteral("numbers"))
		return generateBinding(data, storageData);

	if (storage->name() == QStringLiteral("images"))
		return generateImages(data, storageData);

	if (storage->name() == QStringLiteral("block"))
		return generateBlock(data, storageData);


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

	bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");
	QString question = data.value(QStringLiteral("question")).toString();
	const int maxOptions = data.value(QStringLiteral("maxOptions")).toInt();

	foreach (QVariant v, storageData.value(QStringLiteral("bindings")).toList()) {
		QVariantMap m = v.toMap();
		QString left = m.value(QStringLiteral("first")).toString();
		QString right = m.value(QStringLiteral("second")).toString();

		if (left.isEmpty() || right.isEmpty())
			continue;

		QVariantMap retMap;

		if (question.isEmpty())
			retMap[QStringLiteral("question")] = (isBindToRight ? right : left);
		else if (question.contains(QStringLiteral("%1")))
			retMap[QStringLiteral("question")] = question.arg(isBindToRight ? right : left);
		else
			retMap[QStringLiteral("question")] = question;

		retMap[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

		QStringList alist;

		foreach (QVariant v, storageData.value(QStringLiteral("bindings")).toList()) {
			QVariantMap mm = v.toMap();
			QString f1 = mm.value(QStringLiteral("first")).toString();
			QString f2 = mm.value(QStringLiteral("second")).toString();

			if ((isBindToRight && right == f2) || (!isBindToRight && left == f1))
				continue;

			alist.append(isBindToRight ? f1 : f2);
		}

		retMap.insert(generateOne(isBindToRight ? left : right, alist, maxOptions));

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

	const QString &mode = data.value(QStringLiteral("mode")).toString();
	const QString &question = data.value(QStringLiteral("question")).toString();
	const QString &answersPattern = data.value(QStringLiteral("answers")).toString();

	foreach (const QVariant &v, storageData.value(QStringLiteral("images")).toList()) {
		const QVariantMap &m = v.toMap();
		const int &imgId = m.value(QStringLiteral("second"), -1).toInt();
		const QString &text = m.value(QStringLiteral("first")).toString();

		if (imgId == -1 || text.isEmpty())
			continue;

		QVariantMap retMap;

		if (mode == QStringLiteral("text") && question.contains(QStringLiteral("%1")))
			retMap[QStringLiteral("question")] = question.arg(text);
		else
			retMap[QStringLiteral("question")] = question;

		if (mode == QStringLiteral("image"))
			retMap[QStringLiteral("image")] = QStringLiteral("image://mapimage/%1").arg(imgId);
		else
			retMap[QStringLiteral("imageAnswers")] = true;

		retMap[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

		QStringList alist;

		foreach (const QVariant &v, storageData.value(QStringLiteral("images")).toList()) {
			const QVariantMap &mm = v.toMap();
			const int &f1 = mm.value(QStringLiteral("second"), -1).toInt();
			const QString &f2 = mm.value(QStringLiteral("first")).toString();

			if ((mode == QStringLiteral("image") && text == f2) || (mode == QStringLiteral("text") && (imgId == f1 || f1 == -1)))
				continue;

			if (mode == QStringLiteral("image") && answersPattern.contains(QStringLiteral("%1")))
				alist.append(answersPattern.arg(f2));
			else if (mode == QStringLiteral("image"))
				alist.append(f2);
			else
				alist.append(QStringLiteral("image://mapimage/%1").arg(f1));
		}

		if (mode == QStringLiteral("image") && answersPattern.contains(QStringLiteral("%1")))
			retMap.insert(generateOne(answersPattern.arg(text), alist, 4));
		else if (mode == QStringLiteral("image"))
			retMap.insert(generateOne(text, alist, 4));
		else
			retMap.insert(generateOne(QStringLiteral("image://mapimage/%1").arg(imgId), alist, 4));

		ret.append(retMap);
	}

	return ret;
}




/**
 * @brief ModuleSimplechoice::generateBlock
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateBlock(const QVariantMap &data, const QVariantMap &storageData) const
{
	const QString &mode = data.value(QStringLiteral("mode")).toString();

	if (mode == QStringLiteral("simple"))
		return generateBlockSimple(data, storageData);
	else
		return generateBlockContains(data, storageData);

}


/**
 * @brief ModuleSimplechoice::generateBlockContains
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateBlockContains(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	const QString &question = data.value(QStringLiteral("question")).toString();

	QVector<QString> bNames;

	foreach (const QVariant &v, storageData.value(QStringLiteral("blocks")).toList()) {
		const QVariantMap &m = v.toMap();
		const QString &left = m.value(QStringLiteral("first")).toString();
		bNames.append(left);
	}

	const QVariantList &list = storageData.value(QStringLiteral("blocks")).toList();

	for (int idx = 0; idx < list.size(); ++idx) {
		const QVariantMap &m = list.at(idx).toMap();
		const QString &left = m.value(QStringLiteral("first")).toString().simplified();
		const QStringList &right = m.value(QStringLiteral("second")).toStringList();

		if (left.isEmpty() || right.isEmpty())
			continue;

		QVariantMap retMap;

		foreach (QString s, right) {
			s = s.simplified();
			if (s.isEmpty())
				continue;

			if (question.isEmpty())
				retMap[QStringLiteral("question")] = s;
			else if (question.contains(QStringLiteral("%1")))
				retMap[QStringLiteral("question")] = question.arg(s);
			else
				retMap[QStringLiteral("question")] = question;

			retMap[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

			QStringList alist;

			for (int i=0; i<bNames.size(); ++i) {
				if (idx == i)
					continue;

				const QString &opt = bNames.at(i).simplified();

				if (opt.isEmpty())
					continue;

				alist.append(opt);
			}

			retMap.insert(generateOne(left, alist, data.value(QStringLiteral("maxOptions")).toInt()));

			ret.append(retMap);
		}

	}

	return ret;
}






/**
 * @brief ModuleSimplechoice::generateBlockSimple
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateBlockSimple(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	const QString &question = data.value(QStringLiteral("question")).toString();
	const int maxOptions = data.value(QStringLiteral("maxOptions")).toInt();


	const QVariantList &list = storageData.value(QStringLiteral("blocks")).toList();

	for (auto it = list.constBegin(); it != list.constEnd(); ++it) {
		const QVariantMap &m = it->toMap();
		const QString &left = m.value(QStringLiteral("first")).toString().simplified();

		const QStringList &right = m.value(QStringLiteral("second")).toStringList();

		if (left.isEmpty() || right.isEmpty())
			continue;

		QVariantMap retMap;

		if (question.isEmpty())
			retMap[QStringLiteral("question")] = left;
		else if (question.contains(QStringLiteral("%1")))
			retMap[QStringLiteral("question")] = question.arg(left);
		else
			retMap[QStringLiteral("question")] = question;

		retMap[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

		const QString &correct = right.at(QRandomGenerator::global()->bounded(right.size()));

		QStringList alist;

		for (auto it2 = list.constBegin(); it2 != list.constEnd(); ++it2) {
			if (it2 == it)
				continue;

			const QVariantMap &m = it2->toMap();
			const QStringList &right = m.value(QStringLiteral("second")).toStringList();

			if (right.isEmpty())
				continue;

			alist.append(right.at(QRandomGenerator::global()->bounded(right.size())));
		}

		retMap.insert(generateOne(correct, alist, maxOptions));

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

QVariantMap ModuleSimplechoice::generateOne(const QString &correctAnswer, QStringList optionsList, const int &maxOptions) const
{
	QVector<QPair<QString, bool>> opts;
	opts.append(qMakePair(correctAnswer, true));

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(optionsList.begin(), optionsList.end(), g);

	for (const QString &o : optionsList) {
		if (opts.size() >= (maxOptions > 1 ? maxOptions : 4))
			break;

		if (!o.isEmpty())
			opts.append(qMakePair(o, false));
	}


	int correctIdx = -1;

	QStringList optList;

	std::shuffle(opts.begin(), opts.end(), g);

	for (const auto &p : opts) {
		optList.append(p.first);

		if (p.second)
			correctIdx = optList.size()-1;
	}

	QVariantMap m;
	m[QStringLiteral("answer")] = correctIdx;
	m[QStringLiteral("options")] = optList;

	return m;
}


/**
 * @brief ModuleSimplechoice::preview
 * @param generatedList
 * @return
 */

QVariantMap ModuleSimplechoice::preview(const QVariantList &generatedList, const QVariantMap &commonData) const
{
	Q_UNUSED(commonData);

	QVariantMap m;
	QString s;

	foreach (QVariant v, generatedList) {
		QVariantMap m = v.toMap();

		const QString &image = m.value(QStringLiteral("image")).toString();
		const bool &imageAnswers = m.value(QStringLiteral("imageAnswers")).toBool();

		s.append((image.isEmpty() ? QStringLiteral("") : tr("[KÉP] "))
				 +QStringLiteral("**")+m.value(QStringLiteral("question")).toString()+QStringLiteral("**\n"));

		int correct = m.value(QStringLiteral("answer"), -1).toInt();
		QStringList l = m.value(QStringLiteral("options")).toStringList();
		for (int i=0; i<l.size(); ++i) {
			if (imageAnswers) {
				if (i==correct)
					s.append(QStringLiteral("- **")+tr("[KÉP]")+QStringLiteral("**\n"));
				else
					s.append(QStringLiteral("- ")+tr("[KÉP]")+QStringLiteral("\n"));
			} else {
				if (i==correct)
					s.append(QStringLiteral("- **")+l.at(i)+QStringLiteral("**\n"));
				else
					s.append(QStringLiteral("- ")+l.at(i)+QStringLiteral("\n"));
			}
		}
		s.append(QStringLiteral("---\n"));
	}

	m[QStringLiteral("text")] = s;

	return m;
}
