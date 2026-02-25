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
#include "../block/moduleblock.h"
#include "../images/moduleimages.h"
#include "../binding/modulebinding.h"
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
		m[QStringLiteral("image")] = QString();

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
	} else if (storage->name() == QStringLiteral("sequence")) {
		QStringList answers = storageData.value(QStringLiteral("items")).toStringList();;

		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("questionMin")).toString()+
									 QStringLiteral(" | ")+
									 data.value(QStringLiteral("questionMax")).toString();
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
 * @brief ModuleSimplechoice::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
											 QVariantMap *commonDataPtr, StorageSeed *seed) const
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
		return generateBinding(data, storageData, seed);

	if (storage->name() == QStringLiteral("images"))
		return generateImages(data, storageData, seed);

	if (storage->name() == QStringLiteral("block"))
		return generateBlock(data, storageData, seed);

	if (storage->name() == QStringLiteral("mergeblock"))
		return generateMergeBlock(data, storageData, seed);

	if (storage->name() == QStringLiteral("mergebinding"))
		return generateMergeBinding(data, storageData, seed);

	if (storage->name() == QStringLiteral("sequence"))
		return generateSequence(data, storageData, seed);


	return QVariantList();
}




/**
 * @brief ModuleSimplechoice::generateBinding
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");
	QString question = data.value(QStringLiteral("question")).toString();
	const int maxOptions = data.value(QStringLiteral("maxOptions")).toInt();

	const QVariantList &list = storageData.value(QStringLiteral("bindings")).toList();

	SeedDuplexHelper helper(seed, isBindToRight ? SEED_BINDING_RIGHT : SEED_BINDING_LEFT,
							isBindToRight ? SEED_BINDING_LEFT: SEED_BINDING_RIGHT);

	for (int i = 0; i<list.size(); ++i) {
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

		retMap[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

		QStringList alist;

		for (const QVariant &v : list) {
			QVariantMap mm = v.toMap();
			QString f1 = mm.value(QStringLiteral("first")).toString();
			QString f2 = mm.value(QStringLiteral("second")).toString();

			if ((isBindToRight && right == f2) || (!isBindToRight && left == f1))
				continue;

			alist.append(isBindToRight ? f1 : f2);
		}

		retMap.insert(generateOne(isBindToRight ? left : right, alist, maxOptions));

		helper.append(retMap, i+1, i+1);
	}

	return helper.getVariantList(true);
}




/**
 * @brief ModuleSimplechoice::generateMergeBinding
 * @param data
 * @param storageData
 * @param seed
 * @return
 */

QVariantList ModuleSimplechoice::generateMergeBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");
	QString question = data.value(QStringLiteral("question")).toString();
	const int maxOptions = data.value(QStringLiteral("maxOptions")).toInt();

	const QStringList usedSections = data.value(QStringLiteral("sections")).toStringList();
	const QVariantList &sections = storageData.value(QStringLiteral("sections")).toList();

	SeedDuplexHelper helper(seed, isBindToRight ? SEED_BINDING_RIGHT : SEED_BINDING_LEFT,
							isBindToRight ? SEED_BINDING_LEFT: SEED_BINDING_RIGHT);

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

		retMap[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

		QStringList alist;

		for (const Data &v : list) {
			if (d.idx == v.idx)
				continue;

			alist.append(isBindToRight ? v.left : v.right);
		}

		retMap.insert(generateOne(isBindToRight ? d.left : d.right, alist, maxOptions));

		helper.append(retMap, d.idx, d.idx);
	}

	return helper.getVariantList(true);
}




/**
 * @brief ModuleSimplechoice::generateImages
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateImages(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	const QString &mode = data.value(QStringLiteral("mode")).toString();
	const QString &question = data.value(QStringLiteral("question")).toString();
	const QString &answersPattern = data.value(QStringLiteral("answers")).toString();

	SeedDuplexHelper helper(seed, mode == QStringLiteral("text") ? SEED_IMAGES_TEXT : SEED_IMAGES_IMAGE,
							mode == QStringLiteral("text") ? SEED_IMAGES_IMAGE : SEED_IMAGES_TEXT);

	const QVariantList &list = storageData.value(QStringLiteral("images")).toList();

	for (int i=0; i<list.size(); ++i) {
		const QVariantMap &m = list.at(i).toMap();
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

		for (const QVariant &v : list) {
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

		helper.append(retMap, i+1, i+1);
	}

	return helper.getVariantList(true);
}




/**
 * @brief ModuleSimplechoice::generateBlock
 * @param data
 * @param storageData
 * @return
 */

QVariantList ModuleSimplechoice::generateBlock(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	const QString &mode = data.value(QStringLiteral("mode")).toString();
	const ModuleMergeblock::BlockUnion blocks = ModuleMergeblock::getUnion(storageData.value(QStringLiteral("blocks")).toList());

	if (mode == QStringLiteral("simple"))
		return generateBlockSimple(data, blocks, seed);
	else if (mode == QStringLiteral("contains"))
		return generateBlockContains(data, blocks, seed);
	else if (mode == QStringLiteral("quiz"))
		return generateBlockQuiz(data, blocks, seed);
	else if (mode == QStringLiteral("exclude"))
		return generateBlockExclude(data, blocks, seed);

	return {};
}




/**
 * @brief ModuleSimplechoice::generateMergeBlock
 * @param data
 * @param storageData
 * @param seed
 * @return
 */

QVariantList ModuleSimplechoice::generateMergeBlock(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	const QString &mode = data.value(QStringLiteral("mode")).toString();

	const ModuleMergeblock::BlockUnion blocks = ModuleMergeblock::getUnion(
													storageData.value(QStringLiteral("sections")).toList(),
													data.value(QStringLiteral("sections")).toStringList()
													);

	if (mode == QStringLiteral("simple"))
		return generateBlockSimple(data, blocks, seed);
	else if (mode == QStringLiteral("contains"))
		return generateBlockContains(data, blocks, seed);
	else if (mode == QStringLiteral("quiz"))
		return generateBlockQuiz(data, blocks, seed);
	else if (mode == QStringLiteral("exclude"))
		return generateBlockExclude(data, blocks, seed);

	return {};
}



/**
 * @brief ModuleSimplechoice::generateBlockContains
 * @param data
 * @param blocks
 * @return
 */

QVariantList ModuleSimplechoice::generateBlockContains(const QVariantMap &data, const ModuleMergeblock::BlockUnion &blocks, StorageSeed *seed) const
{
	SeedDuplexHelper helper(seed, SEED_BLOCK_RIGHT, SEED_BLOCK_LEFT);

	const QString &question = data.value(QStringLiteral("question")).toString();

	const QStringList bNames = blocks.keys();

	for (const auto &[left, list] : blocks.asKeyValueRange()) {
		for (const auto &d : list) {
			const QStringList &right = d.content;

			if (left.isEmpty() || right.isEmpty())
				continue;

			for (int i=0; i<right.size(); ++i) {
				const QString &s = right.at(i).simplified();

				if (s.isEmpty())
					continue;

				QVariantMap retMap;

				if (question.isEmpty())
					retMap[QStringLiteral("question")] = s;
				else if (question.contains(QStringLiteral("%1")))
					retMap[QStringLiteral("question")] = question.arg(s);
				else
					retMap[QStringLiteral("question")] = question;

				retMap[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

				QStringList alist;

				for (const QString &opt : bNames) {
					if (opt == left)
						continue;

					alist.append(opt);
				}

				retMap.insert(generateOne(left, alist, data.value(QStringLiteral("maxOptions")).toInt()));

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
 * @brief ModuleSimplechoice::generateBlockExclude
 * @param data
 * @param blocks
 * @param seed
 * @return
 */

QVariantList ModuleSimplechoice::generateBlockExclude(const QVariantMap &data, const ModuleMergeblock::BlockUnion &blocks, StorageSeed *seed) const
{
	SeedDuplexHelper helper(seed, SEED_BLOCK_LEFT, SEED_BLOCK_RIGHT);

	const QString &question = data.value(QStringLiteral("question")).toString();
	const int maxOptions = data.value(QStringLiteral("maxOptions")).toInt();


	struct Data {
		QString answer;
		int idx = 0;
	};


	for (auto bit = blocks.cbegin(); bit != blocks.cend(); ++bit) {
		const QString &left = bit.key();

		for (auto it = bit.value().cbegin(); it != bit.value().cend(); ++it) {
			const QStringList &right = it->content;

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


			QList<Data> alist;


			for (auto bit2 = blocks.cbegin(); bit2 != blocks.cend(); ++bit2) {
				for (auto it2 = bit2.value().cbegin(); it2 != bit2.value().cend(); ++it2) {
					if (bit == bit2 && it2 == it)
						continue;

					const QString &wleft = bit2.key();
					const QStringList &wright = it2->content;

					if (wleft.isEmpty() || wright.isEmpty())
						continue;

					const int offset = it2->blockidx+1;

					const QList<int> idxList = helper.getAllSubB(offset, offset+wright.size());

					for (const int &idx : idxList) {
						const int real = idx - offset;

						alist.append(Data{.answer = wright.at(real), .idx = idx});
					}
				}
			}

			if (alist.isEmpty())
				continue;

			const Data &d = alist.at(QRandomGenerator::global()->bounded(alist.size()));

			retMap.insert(generateOne(d.answer, right, maxOptions));
			helper.append(retMap, it->blockidx, d.idx);

		}
	}

	return helper.getVariantList(true);
}






/**
 * @brief ModuleSimplechoice::generateBlockSimple
 * @param data
 * @param blocks
 * @return
 */

QVariantList ModuleSimplechoice::generateBlockSimple(const QVariantMap &data, const ModuleMergeblock::BlockUnion &blocks, StorageSeed *seed) const
{
	SeedDuplexHelper helper(seed, SEED_BLOCK_LEFT, SEED_BLOCK_RIGHT);

	const QString &question = data.value(QStringLiteral("question")).toString();
	const int maxOptions = data.value(QStringLiteral("maxOptions")).toInt();


	for (auto bit = blocks.cbegin(); bit != blocks.cend(); ++bit) {
		const QString &left = bit.key();

		for (auto it = bit.value().cbegin(); it != bit.value().cend(); ++it) {
			const QStringList &right = it->content;

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

			//const int idx = QRandomGenerator::global()->bounded(right.size());
			const int idx = helper.getSubBOffset(right.size(), it->blockidx+1);
			const QString &correct = right.at(idx);

			QStringList alist;

			for (auto bit2 = blocks.cbegin(); bit2 != blocks.cend(); ++bit2) {
				for (auto it2 = bit2.value().cbegin(); it2 != bit2.value().cend(); ++it2) {
					if (bit == bit2 && it2 == it)
						continue;

					const QStringList &right = it2->content;

					if (right.isEmpty())
						continue;

					alist.append(right.at(QRandomGenerator::global()->bounded(right.size())));
				}
			}

			retMap.insert(generateOne(correct, alist, maxOptions));

			helper.append(retMap, it->blockidx, it->blockidx + idx+1);
		}
	}

	return helper.getVariantList(true);
}



/**
 * @brief ModuleSimplechoice::generateBlockQuiz
 * @param data
 * @param blocks
 * @param seed
 * @return
 */

QVariantList ModuleSimplechoice::generateBlockQuiz(const QVariantMap &data, const ModuleMergeblock::BlockUnion &blocks, StorageSeed *seed) const
{
	SeedDuplexHelper helper(seed, SEED_BLOCK_LEFT, SEED_BLOCK_RIGHT);

	const QString &question = data.value(QStringLiteral("question")).toString();
	const int maxOptions = data.value(QStringLiteral("maxOptions")).toInt();


	for (auto bit = blocks.cbegin(); bit != blocks.cend(); ++bit) {
		const QString &left = bit.key();

		for (auto it = bit.value().cbegin(); it != bit.value().cend(); ++it) {
			QStringList right = it->content;

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

			const QString correct = right.takeFirst();

			retMap.insert(generateOne(correct, right, maxOptions));

			helper.append(retMap, it->blockidx, it->blockidx + 0 + 1);
		}
	}

	return helper.getVariantList(true);
}



/**
 * @brief ModuleSimplechoice::generateSequence
 * @param data
 * @param storageData
 * @param seed
 * @return
 */

QVariantList ModuleSimplechoice::generateSequence(const QVariantMap &data, const QVariantMap &storageData, StorageSeed */*seed*/) const
{
	QString questionMin = data.value(QStringLiteral("questionMin")).toString();
	QString questionMax = data.value(QStringLiteral("questionMax")).toString();
	int maxOptions = data.value(QStringLiteral("maxOptions")).toInt();

	if (maxOptions < 2)
		maxOptions = 2;

	const QStringList &list = storageData.value(QStringLiteral("items")).toStringList();

	if (list.size() < 2)
		return {};

	QVariantList retList;

	std::vector<int> v(list.size());
	std::iota (std::begin(v), std::end(v), 0);

	for (int i=0; i<4; ++i) {
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(v.begin(), v.end(), g);

		std::vector<int> curr;
		curr.reserve(list.size());

		int count = 0;
		for (auto it=v.cbegin(); it != v.cend() && count<maxOptions ; ++it) {
			curr.push_back(*it);
			++count;
		}

		const bool isMax = (i % 2 == 0);

		std::vector<int>::const_iterator itCorr = isMax ? std::max_element(curr.cbegin(), curr.cend()) :
														  std::min_element(curr.cbegin(), curr.cend());

		QVariantMap retMap;


		retMap[QStringLiteral("question")] = isMax ? questionMax : questionMin;
		retMap[QStringLiteral("monospace")] = data.value(QStringLiteral("monospace")).toBool();

		QStringList alist;

		for (auto it = curr.cbegin(); it != curr.cend(); ++it) {
			if (it == itCorr)
				continue;
			alist.append(list.at(*it));
		}

		retMap.insert(generateOne(list.at(*itCorr), alist, maxOptions));

		retList << retMap;
	}

	return retList;
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

		s.append((image.isEmpty() ? QString() : tr("[KÉP] "))
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
