/*
 * ---- Call of Suli ----
 *
 * ModuleWriter.cpp
 *
 * Created on: 2021. 11. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleWriter
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

#include "modulewriter.h"
#include "qqml.h"
#include "storageseed.h"
#include "writerengine.h"
#include <QRandomGenerator>
#include "../binding/modulebinding.h"
#include "../block/moduleblock.h"
#include "../images/moduleimages.h"
#include "../text/moduletext.h"

const QRegularExpression ModuleWriter::m_expressionWord("(?<!\\\\)%((?:[^%\\\\]|\\\\.)+)%");
const QString ModuleWriter::m_punctation = QStringLiteral(",.-:;?!– ");
const QString ModuleWriter::m_placeholder = QStringLiteral("...");


ModuleWriter::ModuleWriter(QObject *parent) : QObject(parent)
{

}



/**
 * @brief ModuleWriter::testResult
 * @param data
 * @param answer
 * @param success
 * @return
 */

QString ModuleWriter::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const
{
	QString html;


	html += QStringLiteral("<p>");

	if (success)
		html += QStringLiteral("<span class=\"answer\">");
	else
		html += QStringLiteral("<span class=\"answerFail\">");

	html += answer.value(QStringLiteral("text")).toString();

	html += QStringLiteral("</span>");

	if (const QString &a = data.value(QStringLiteral("answer")).toString(); !success && !a.isEmpty()) {
		html += QStringLiteral(" <span class=\"answerCorrect\">")
				+ a + QStringLiteral("</span>");
	}

	html += QStringLiteral("</p>");

	return html;
}


/**
 * @brief ModuleWriter::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleWriter::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (!storage) {
		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = data.value(QStringLiteral("correct")).toString();
		m[QStringLiteral("image")] = QString();

		return m;
	} else if (storage->name() == QStringLiteral("binding")) {
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
		m[QStringLiteral("image")] = QString();

		return m;
	} else if (storage->name() == QStringLiteral("images")) {
		QStringList answers;

		QString image = QString();

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
	} else if (storage->name() == QStringLiteral("sequence")) {
		const QStringList &list = storageData.value(QStringLiteral("items")).toStringList();

		QVariantMap m;
		m[QStringLiteral("title")] = list.join(QStringLiteral(" "));
		m[QStringLiteral("details")] = tr("Kiegészítendő: %1, további szavak: %2")
									   .arg(data.value(QStringLiteral("words")).toInt())
									   .arg(data.value(QStringLiteral("pad")).toInt());
		m[QStringLiteral("image")] = QString();

		return m;
	} else if (storage->name() == QStringLiteral("text")) {
		QStringList list = storageData.value(QStringLiteral("items")).toStringList();

		QVariantMap m;
		m[QStringLiteral("title")] = list.isEmpty() ? QString() : list.at(0);
		if (!list.isEmpty())
			list.removeFirst();
		m[QStringLiteral("details")] = list.isEmpty() ? QString() : list.join(QStringLiteral("\n"));
		m[QStringLiteral("image")] = QString();

		return m;
	} else if (storage->name() == QStringLiteral("mergebinding") || storage->name() == QStringLiteral("mergeblock")) {
		const QStringList usedSections = data.value(QStringLiteral("sections")).toStringList();
		const QVariantList &sections = storageData.value(QStringLiteral("sections")).toList();

		QStringList answers;

		for (int i=0; i<sections.size(); ++i) {
			QVariantMap m = sections.at(i).toMap();
			const QString &key = m.value(QStringLiteral("key")).toString();

			if (!usedSections.contains(key))
				continue;

			answers.append(m.value(QStringLiteral("name")).toString());
		}

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = answers.join(QStringLiteral(", "));
		m[QStringLiteral("image")] = QString();

		return m;
	}

	return QVariantMap({{QStringLiteral("title"), QString()},
						{QStringLiteral("details"), QString()},
						{QStringLiteral("image"), QString()}
					   });
}


/**
 * @brief ModuleWriter::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleWriter::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
									   QVariantMap */*commonDataPtr*/, StorageSeed *seed) const
{
	if (!storage) {
		QVariantList list;
		QVariantMap m;

		m[QStringLiteral("question")] = data.value(QStringLiteral("question")).toString();

		QString correct = data.value(QStringLiteral("correct")).toString();

		if (correct.isEmpty())
			correct = QStringLiteral(" ");

		m[QStringLiteral("answer")] = data.value(QStringLiteral("correct")).toString();

		list.append(m);

		return list;
	}

	if (storage->name() == QStringLiteral("binding"))
		return generateBinding(data, storageData, seed);

	if (storage->name() == QStringLiteral("block")) {
		const ModuleMergeblock::BlockUnion blocks = ModuleMergeblock::getUnion(storageData.value(QStringLiteral("blocks")).toList());
		return generateBlockContains(data, blocks, seed);
	}

	if (storage->name() == QStringLiteral("mergeblock")) {
		const ModuleMergeblock::BlockUnion blocks = ModuleMergeblock::getUnion(
														storageData.value(QStringLiteral("sections")).toList(),
														data.value(QStringLiteral("sections")).toStringList()
														);
		return generateBlockContains(data, blocks, seed);
	}



	if (storage->name() == QStringLiteral("images"))
		return generateImages(data, storageData, seed);

	if (storage->name() == QStringLiteral("sequence"))
		return generateSequence(data, storageData);

	if (storage->name() == QStringLiteral("text"))
		return generateText(data, storageData, seed);

	if (storage->name() == QStringLiteral("mergebinding"))
		return generateMergeBinding(data, storageData, seed);


	return QVariantList();
}




/**
 * @brief ModuleWriter::preview
 * @return
 */

QVariantMap ModuleWriter::preview(const QVariantList &generatedList, const QVariantMap &commonData) const
{
	Q_UNUSED(commonData);

	QVariantMap m;
	QString s;

	foreach (QVariant v, generatedList) {
		QVariantMap m = v.toMap();

		const QString &image = m.value(QStringLiteral("image")).toString();

		s.append((image.isEmpty() ? QString() : tr("[KÉP] "))
				 +QStringLiteral("**")+m.value(QStringLiteral("question")).toString()+QStringLiteral("**\n"));

		const QString &answer = m.value(QStringLiteral("answer")).toString();

		s.append(QStringLiteral("- ")+answer+QStringLiteral("\n"));

		s.append(QStringLiteral("---\n"));
	}

	m[QStringLiteral("text")] = s;

	return m;
}




/**
 * @brief ModuleWriter::registerQmlTypes
 */

void ModuleWriter::registerQmlTypes() const
{
	qmlRegisterType<WriterEngine>("CallOfSuli", 1, 0, "WriterEngine");
}


/**
 * @brief ModuleWriter::generateBinding
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleWriter::generateBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");
	QString question = data.value(QStringLiteral("question")).toString();

	SeedDuplexHelper helper(seed, isBindToRight ? SEED_BINDING_RIGHT : SEED_BINDING_LEFT,
							isBindToRight ? SEED_BINDING_LEFT: SEED_BINDING_RIGHT);

	const QVariantList &list = storageData.value(QStringLiteral("bindings")).toList();

	for (int i=0; i<list.size(); ++i) {
		QVariantMap m = list.at(i).toMap();
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

		retMap[QStringLiteral("answer")] = isBindToRight ? left : right;

		helper.append(retMap, i+1, i+1);
	}

	return helper.getVariantList(true);
}


/**
 * @brief ModuleWriter::generateImages
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleWriter::generateImages(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	const QString &question = data.value(QStringLiteral("question")).toString();

	SeedHelper helper(seed, SEED_IMAGES_IMAGE);

	const QVariantList &list = storageData.value(QStringLiteral("images")).toList();

	for (int i=0; i<list.size(); ++i) {
		const QVariantMap &m = list.at(i).toMap();
		const int &imgId = m.value(QStringLiteral("second"), -1).toInt();
		const QString &text = m.value(QStringLiteral("first")).toString();

		if (imgId == -1 || text.isEmpty())
			continue;

		QVariantMap retMap;

		retMap[QStringLiteral("question")] = question;
		retMap[QStringLiteral("image")] = QStringLiteral("image://mapimage/%1").arg(imgId);
		retMap[QStringLiteral("answer")] = text;

		helper.append(retMap, i+1);
	}

	return helper.getVariantList(true);
}



/**
 * @brief ModuleWriter::generateSequence
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleWriter::generateSequence(const QVariantMap &data, const QVariantMap &storageData) const
{
	QVariantList ret;

	const QStringList &list = storageData.value(QStringLiteral("items")).toStringList();

	int words = qMax(1, data.value(QStringLiteral("words")).toInt());
	const int &pad = data.value(QStringLiteral("pad")).toInt();

	int padLeft = -1, padRight = -1, start = 0;

	if (list.size() > words)
		start = QRandomGenerator::global()->bounded(list.size()-words);


	if (pad > 0) {
		if (start > 0)
			padLeft = QRandomGenerator::global()->bounded(qMin(pad+1, start));
		else
			padLeft = 0;

		padRight = pad-padLeft;

		padLeft += -qMin(0, list.size()-(start+words+padRight));
	}

	QString question;
	QStringList answerList;

	for (int i=0; i<list.size(); ++i) {
		if ((padLeft > -1 && i < start-padLeft) ||
				(padRight > -1 && i >= start+words+padRight))
			continue;

		QString w = list.at(i).simplified();

		if (i>=start && i<start+words) {
			QString wPunct, wPrep;

			while (!w.isEmpty() && m_punctation.contains(w.at(0))) {
				wPrep.append(w.at(0));
				w.remove(0, 1);
			}

			while (!w.isEmpty() && m_punctation.contains(w.right(1))) {
				wPunct.prepend(w.right(1));
				w.chop(1);
			}

			if (!question.endsWith(m_placeholder)) {
				if (!question.isEmpty())
					question.append(QStringLiteral(" "));
				question.append(wPrep+m_placeholder);
			}

			if (!wPunct.isEmpty())
				question.append(wPunct);

			answerList.append(w);

		} else {
			if (!question.isEmpty())
				question.append(QStringLiteral(" "));
			question.append(w);
		}
	}


	QVariantMap retMap;

	retMap[QStringLiteral("question")] = question;
	retMap[QStringLiteral("answer")] = answerList.join(QStringLiteral(" "));

	ret.append(retMap);
	return ret;
}




/**
 * @brief ModuleWriter::generateText
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleWriter::generateText(const QVariantMap &/*data*/, const QVariantMap &storageData, StorageSeed *seed) const
{
	SeedHelper helper(seed, SEED_TEXT);

	const QStringList &list = storageData.value(QStringLiteral("items")).toStringList();

	for (int i=0; i<list.size(); ++i) {
		const QString &text = list.at(i);

		if (text.isEmpty())
			continue;


		struct ItemStruct {
			QString text;
			bool isQuestion;

			ItemStruct(const QString &t, const bool &q) : text(t), isQuestion(q) {}
		};

		QVector<ItemStruct> items;

		QRegularExpressionMatchIterator it = m_expressionWord.globalMatch(text);

		int _cptrd = 0;

		static const QRegularExpression regExp("\\s+");

		while (it.hasNext())
		{
			QRegularExpressionMatch match = it.next();

			int _s = match.capturedStart();
			if (_s > _cptrd) {
				foreach (QString s, text.mid(_cptrd, _s-_cptrd).split(regExp, Qt::SkipEmptyParts))
					items.append(ItemStruct(s.replace(QStringLiteral("\\%"), QStringLiteral("%")), false));
			}
			_cptrd = match.capturedStart()+match.capturedLength();

			QString q = text.mid(match.capturedStart(1), match.capturedLength(1)).replace(QStringLiteral("\\%"), QStringLiteral("%"));

			items.append(ItemStruct(q, true));
		}


		foreach (QString s, text.mid(_cptrd).split(regExp, Qt::SkipEmptyParts))
			items.append(ItemStruct(s.replace(QStringLiteral("\\%"), QStringLiteral("%")), false));


		QVector<int> questionIndexList;

		for (int i=0; i<items.size(); ++i) {
			if (items.at(i).isQuestion)
				questionIndexList.append(i);
		}

		foreach (int questionIndex, questionIndexList) {
			QString question;

			for (int i=0; i<items.size(); ++i) {
				const QString &w = items.at(i).text;

				if (!question.isEmpty() && !w.isEmpty() && !m_punctation.contains(w.at(0)))
					question.append(QStringLiteral(" "));

				if (i == questionIndex)
					question.append(m_placeholder);
				else
					question.append(w);

			}


			QVariantMap retMap;

			retMap[QStringLiteral("question")] = question;
			retMap[QStringLiteral("answer")] = items.at(questionIndex).text;

			helper.append(retMap, i+1);
		}
	}

	return helper.getVariantList(true);
}



/**
 * @brief ModuleWriter::generateBlockContains
 * @param data
 * @param blocks
 * @param seed
 * @return
 */

QVariantList ModuleWriter::generateBlockContains(const QVariantMap &data, const ModuleMergeblock::BlockUnion &blocks, StorageSeed *seed) const
{
	SeedDuplexHelper helper(seed, SEED_BLOCK_RIGHT, SEED_BLOCK_LEFT);

	const QString &question = data.value(QStringLiteral("question")).toString();

	for (const auto &[left, list] : blocks.asKeyValueRange()) {
		for (const auto &d : list) {
			const QStringList &right = d.content;
			if (left.isEmpty() || right.isEmpty())
				continue;

			QVariantMap retMap;

			for (int i=0; i<right.size(); ++i) {
				const QString &s = right.at(i).simplified();
				if (s.isEmpty())
					continue;

				if (question.isEmpty())
					retMap[QStringLiteral("question")] = s;
				else if (question.contains(QStringLiteral("%1")))
					retMap[QStringLiteral("question")] = question.arg(s);
				else
					retMap[QStringLiteral("question")] = question;

				retMap[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

				retMap[QStringLiteral("answer")] = left;

				// Seed main: 2
				// Seed sub: (block index+1) * 1000 + (answer index + 1)

				const int sub = d.blockidx + i+1;

				helper.append(retMap, sub, d.blockidx);
			}

		}
	}

	return helper.getVariantList(true);
}




/**
 * @brief ModuleWriter::generateMergeBinding
 * @param data
 * @param storageData
 * @param seed
 * @return
 */

QVariantList ModuleWriter::generateMergeBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");
	QString question = data.value(QStringLiteral("question")).toString();

	SeedDuplexHelper helper(seed, isBindToRight ? SEED_BINDING_RIGHT : SEED_BINDING_LEFT,
							isBindToRight ? SEED_BINDING_LEFT: SEED_BINDING_RIGHT);



	const QStringList usedSections = data.value(QStringLiteral("sections")).toStringList();
	const QVariantList &sections = storageData.value(QStringLiteral("sections")).toList();

	struct Data {
		QString left;
		QString right;
		int idx = 0;
	};

	QList<Data> list;

	for (int i=0; i<sections.size(); ++i) {
		QVariantMap m = sections.at(i).toMap();
		const QString &key = m.value(QStringLiteral("key")).toString();

		if (!usedSections.contains(key))
			continue;

		const QVariantList &l = m.value(QStringLiteral("bindings")).toList();

		for (int j = 0; j<l.size(); ++j) {
			QVariantMap m = l.at(j).toMap();
			QString left = m.value(QStringLiteral("first")).toString();
			QString right = m.value(QStringLiteral("second")).toString();

			if (left.isEmpty() || right.isEmpty())
				continue;

			list.append(Data{.left = left, .right = right, .idx = (i+1)*1000+j+1});
		}
	}

	for (const Data &d : list) {
		QVariantMap retMap;

		if (question.isEmpty())
			retMap[QStringLiteral("question")] = (isBindToRight ? d.right : d.left);
		else if (question.contains(QStringLiteral("%1")))
			retMap[QStringLiteral("question")] = question.arg(isBindToRight ? d.right : d.left);
		else
			retMap[QStringLiteral("question")] = question;

		retMap[QStringLiteral("answer")] = isBindToRight ? d.left : d.right;

		helper.append(retMap, d.idx, d.idx);
	}

	return helper.getVariantList(true);
}

