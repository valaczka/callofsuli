/*
 * ---- Call of Suli ----
 *
 * moduleDoublechoice.cpp
 *
 * Created on: 2021. 11. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ModuleDoublechoice
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

#include "moduledoublechoice.h"
#include "exampaper.h"
#include "question.h"
#include <QRandomGenerator>

#include "../block/moduleblock.h"
#include "../binding/modulebinding.h"
#include "../mergeblock/modulemergeblock.h"




const QList<quint8> ModuleDoublechoice::m_optionsA = {
	ExamPaper::LetterA,
	ExamPaper::LetterB,
	ExamPaper::LetterC,
	ExamPaper::LetterA | ExamPaper::LetterB,
	ExamPaper::LetterA | ExamPaper::LetterC,
	ExamPaper::LetterB | ExamPaper::LetterC,
	ExamPaper::LetterA | ExamPaper::LetterB | ExamPaper::LetterC,
};



const QList<quint8> ModuleDoublechoice::m_optionsB = {
	ExamPaper::LetterD,
	ExamPaper::LetterE,
	ExamPaper::LetterF,
	ExamPaper::LetterD | ExamPaper::LetterE,
	ExamPaper::LetterD | ExamPaper::LetterF,
	ExamPaper::LetterE | ExamPaper::LetterF,
	ExamPaper::LetterD | ExamPaper::LetterE | ExamPaper::LetterF
};



namespace Doublechoice {

struct Data {
	QString left;
	QString right;
	int idx = 0;
};


class DataList : public QList<Data>
{
public:
	DataList() = default;

	QStringList toAnswerList(const bool &isBindToRight) const;
	void toDuplexHelper(SeedDuplexHelper &helper, const QVariantMap &data, const bool &isBindToRight, const QStringList &aList) const;
};

};



ModuleDoublechoice::ModuleDoublechoice(QObject *parent) : QObject(parent)
{

}


/**
 * @brief ModuleDoublechoice::testResult
 * @param data
 * @param answer
 * @return
 */

QString ModuleDoublechoice::testResult(const QVariantMap &data, const QVariantMap &answer, const bool &success) const
{
	const QStringList &optionsA = data.value(QStringLiteral("optionsA")).toStringList();
	const QStringList &optionsB = data.value(QStringLiteral("optionsB")).toStringList();
	static QString placeholder("%1 | %2");


	QString html;

	if (data.value(QStringLiteral("monospace")).toBool())
		html += Question::monspaceTagStart();

	html += QStringLiteral("<p class=\"options\">");
	html += placeholder.arg(optionsA.join(QStringLiteral(" • "))).arg(optionsB.join(QStringLiteral(" • ")));
	html += QStringLiteral("</p>");

	html += QStringLiteral("<p>");


	if (answer.contains(QStringLiteral("indexA"))) {
		if (success)
			html += QStringLiteral("<span class=\"answer\">");
		else
			html += QStringLiteral("<span class=\"answerFail\">");

		if (const int &idxA = answer.value(QStringLiteral("indexA"), -1).toInt(); idxA >=0 && idxA < optionsA.size()) {
			html += optionsA.at(idxA) + QStringLiteral(" ");
		}

		if (const int &idxB = answer.value(QStringLiteral("indexB"), -1).toInt(); idxB >=0 && idxB < optionsB.size()) {
			html += optionsB.at(idxB);
		}

		html += QStringLiteral("</span>");
	}


	if (!success) {
		html += QStringLiteral(" <span class=\"answerCorrect\">");

		const QVariantMap aMap = data.value(QStringLiteral("answer")).toMap();

		if (const int cIdx = aMap.value(QStringLiteral("first")).toInt(); cIdx >=0 && cIdx < optionsA.size()) {
			html += optionsA.at(cIdx) + QStringLiteral(" ");
		}

		if (const int cIdx = aMap.value(QStringLiteral("second")).toInt(); cIdx >=0 && cIdx < optionsB.size()) {
			html += optionsB.at(cIdx) + QStringLiteral(" ");
		}

		html += QStringLiteral("</span>");
	}



	html += QStringLiteral("</p>");

	if (data.value(QStringLiteral("monospace")).toBool())
		html += Question::monspaceTagEnd();

	return html;
}



/**
 * @brief ModuleDoublechoice::details
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantMap ModuleDoublechoice::details(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData) const
{
	if (!storage) {
		QVariantMap m;
		m[QStringLiteral("title")] = data.value(QStringLiteral("question")).toString();
		m[QStringLiteral("details")] = data.value(QStringLiteral("corrects")).toStringList().join(QStringLiteral(", "))+
									   QStringLiteral("<br>(")+data.value(QStringLiteral("answers")).toStringList().join(QStringLiteral(", "))+
									   QStringLiteral(")");
		m[QStringLiteral("image")] = QString();
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
	}

	return {};
}



/**
 * @brief ModuleDoublechoice::generateAll
 * @param data
 * @param storage
 * @param storageData
 * @return
 */

QVariantList ModuleDoublechoice::generateAll(const QVariantMap &data, ModuleInterface *storage, const QVariantMap &storageData,
											 QVariantMap *commonDataPtr, StorageSeed *seed) const
{
	Q_UNUSED(commonDataPtr);

	if (!storage) {
		QVariantList list;

		for (int i=0; i<5; ++i)
			list.append(generateOne(data));

		return list;
	}

	if (storage->name() == QStringLiteral("binding"))
		return generateBinding(data, storageData, seed);

	if (storage->name() == QStringLiteral("mergebinding"))
		return generateMergeBinding(data, storageData, seed);

	if (storage->name() == QStringLiteral("block"))
		return generateBlock(false, data, storageData, seed);

	if (storage->name() == QStringLiteral("mergeblock"))
		return generateBlock(true, data, storageData, seed);


	return QVariantList();
}




/**
 * @brief ModuleDoublechoice::preview
 * @return
 */

QVariantMap ModuleDoublechoice::preview(const QVariantList &generatedList, const QVariantMap &commonData) const
{
	Q_UNUSED(commonData);

	QVariantMap m;
	QString s;

	for (const QVariant &v : generatedList) {
		QVariantMap m = v.toMap();
		QVariantMap correctMap = m.value(QStringLiteral("answer")).toMap();

		s.append(QStringLiteral("**")+m.value(QStringLiteral("question")).toString()+QStringLiteral("**\n"));

		{
			const int correct = correctMap.value(QStringLiteral("first"), -1).toInt();

			s.append(QStringLiteral("- (1)\n"));
			QStringList l = m.value(QStringLiteral("optionsA")).toStringList();

			for (int i=0; i<l.size(); ++i) {
				if (i==correct)
					s.append(QStringLiteral("  - **")+l.at(i)+QStringLiteral("**\n"));
				else
					s.append(QStringLiteral("  - ")+l.at(i)+QStringLiteral("\n"));
			}
		}

		{
			const int correct = correctMap.value(QStringLiteral("second"), -1).toInt();

			s.append(QStringLiteral("- (2)\n"));
			QStringList l = m.value(QStringLiteral("optionsB")).toStringList();

			for (int i=0; i<l.size(); ++i) {
				if (i==correct)
					s.append(QStringLiteral("  - **")+l.at(i)+QStringLiteral("**\n"));
				else
					s.append(QStringLiteral("  - ")+l.at(i)+QStringLiteral("\n"));
			}
		}

		s.append(QStringLiteral("---\n"));
	}

	m[QStringLiteral("text")] = s;

	return m;
}




/**
 * @brief ModuleDoublechoice::generate
 * @param data
 * @param storage
 * @param storageData
 * @param answer
 * @return
 */

QVariantMap ModuleDoublechoice::generateOne(const QVariantMap &data) const
{
	const QString &question = data.value(QStringLiteral("question")).toString();
	const bool monospace = data.value(QStringLiteral("monospace")).toBool();
	const int maxOptionsA = std::max(2, data.value(QStringLiteral("maxOptionsA")).toInt());
	const int maxOptionsB = std::max(2, data.value(QStringLiteral("maxOptionsB")).toInt());
	const QStringList &aList = data.value(QStringLiteral("answers")).toStringList();
	const QString separator = data.value(QStringLiteral("separator")).toString();
	const QString &correct = data.value(QStringLiteral("correct")).toString();

	QVariantMap m = _generate(DoubleData(aList, correct, separator), maxOptionsA, maxOptionsB);

	m[QStringLiteral("question")] = question;
	m[QStringLiteral("monospace")] = monospace;

	return m;
}



/**
 * @brief ModuleDoublechoice::generateBinding
 * @param data
 * @param storageData
 * @param seed
 * @return
 */

QVariantList ModuleDoublechoice::generateBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");

	const QVariantList &bindings = storageData.value(QStringLiteral("bindings")).toList();

	SeedDuplexHelper helper(seed, isBindToRight ? SEED_BINDING_RIGHT : SEED_BINDING_LEFT,
							isBindToRight ? SEED_BINDING_LEFT: SEED_BINDING_RIGHT);


	Doublechoice::DataList list;

	for (int i=0; const QVariant &v : bindings) {
		const QVariantMap m = v.toMap();

		list.append(Doublechoice::Data{.left = m.value(QStringLiteral("first")).toString().simplified(),
									   .right = m.value(QStringLiteral("second")).toString().simplified(),
									   .idx = (i+1)
					});

		++i;
	}

	const QStringList aList = list.toAnswerList(isBindToRight);
	list.toDuplexHelper(helper, data, isBindToRight, aList);

	return helper.getVariantList(true);

}




/**
 * @brief ModuleDoublechoice::generateMergeBinding
 * @param data
 * @param storageData
 * @param seed
 * @return
 */

QVariantList ModuleDoublechoice::generateMergeBinding(const QVariantMap &data, const QVariantMap &storageData, StorageSeed *seed) const
{
	bool isBindToRight = data.value(QStringLiteral("mode")).toString() == QStringLiteral("right");

	const QStringList usedSections = data.value(QStringLiteral("sections")).toStringList();
	const QVariantList &sections = storageData.value(QStringLiteral("sections")).toList();

	SeedDuplexHelper helper(seed, isBindToRight ? SEED_BINDING_RIGHT : SEED_BINDING_LEFT,
							isBindToRight ? SEED_BINDING_LEFT: SEED_BINDING_RIGHT);


	Doublechoice::DataList list;

	for (int i=0; i<sections.size(); ++i) {
		QVariantMap m = sections.at(i).toMap();
		const QString &key = m.value(QStringLiteral("key")).toString();

		if (!usedSections.contains(key))
			continue;

		const QVariantList &l = m.value(QStringLiteral("bindings")).toList();

		for (int j=0; const QVariant &v : l) {
			const QVariantMap m = v.toMap();

			list.append(Doublechoice::Data{.left = m.value(QStringLiteral("first")).toString().simplified(),
										   .right = m.value(QStringLiteral("second")).toString().simplified(),
										   .idx = (i+1)*1000+j+1
						});

			++j;
		}
	}

	const QStringList aList = list.toAnswerList(isBindToRight);
	list.toDuplexHelper(helper, data, isBindToRight, aList);


	return helper.getVariantList(true);
}



/**
 * @brief ModuleDoublechoice::generateBlock
 * @param data
 * @param storageData
 * @param seed
 * @return
 */

QVariantList ModuleDoublechoice::generateBlock(const bool &isMerge, const QVariantMap &data,
											   const QVariantMap &storageData, StorageSeed *seed) const
{
	const ModuleMergeblock::BlockUnion blocks = isMerge ?
													ModuleMergeblock::getUnion(
														storageData.value(QStringLiteral("sections")).toList(),
														data.value(QStringLiteral("sections")).toStringList()
														)
												  : ModuleMergeblock::getUnion(storageData.value(QStringLiteral("blocks")).toList());

	SeedDuplexHelper helper(seed, SEED_BLOCK_RIGHT, SEED_BLOCK_LEFT);


	Doublechoice::DataList dlist;

	for (const auto &[left, list] : blocks.asKeyValueRange()) {
		for (const auto &d : list) {
			const QStringList &right = d.content;

			for (int i=0; const QString &s : right) {
				dlist.append(Doublechoice::Data{.left = left,
												.right = s.simplified(),
												.idx = d.blockidx + i+1
							 });

				++i;
			}
		}
	}

	const QStringList aList = dlist.toAnswerList(true);
	dlist.toDuplexHelper(helper, data, true, aList);

	return helper.getVariantList(true);
}



/**
 * @brief ModuleDoublechoice::getOptionValue
 * @param options
 * @param isA
 * @return
 */

std::optional<quint8> ModuleDoublechoice::getOptionValue(const quint8 &value, const bool &isA)
{
	std::optional<quint8> r = std::nullopt;

	if (!value)
		return r;

	if (isA) {
		for (int idx=0; const quint8 v : m_optionsA) {
			if ((value & v) == v)
				r = idx;
			++idx;
		}
	} else {
		for (int idx=0; const quint8 v : m_optionsB) {
			if ((value & v) == v)
				r = idx;
			++idx;
		}
	}


	return r;
}


/**
 * @brief ModuleDoublechoice::getOptionString
 * @param value
 * @param isA
 * @return
 */

QString ModuleDoublechoice::getOptionString(const quint8 &value, const bool &isA)
{
	if (isA) {
		if (value >= m_optionsA.size())
			return {};

		return ExamPaper::getOptionString(m_optionsA.value(value));
	} else {
		if (value >= m_optionsB.size())
			return {};

		return ExamPaper::getOptionString(m_optionsB.value(value));
	}
}




/**
 * @brief ModuleDoublechoice::_generate
 * @param data
 * @param correctB
 * @param optionsListA
 * @param optionsListB
 * @param maxOptionsA
 * @param maxOptionsB
 * @return
 */

QVariantMap ModuleDoublechoice::_generate(const DoubleData &data, int maxOptionsA, int maxOptionsB)
{
	QVector<QPair<QString, bool>> optsA, optsB;

	optsA.append(qMakePair(data.correctA, true));
	optsB.append(qMakePair(data.correctB, true));

	QStringList oListA = data.oListA.values();
	QStringList oListB = data.oListB.values();

	oListA.removeAll(data.correctA);
	oListB.removeAll(data.correctB);

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(oListA.begin(), oListA.end(), g);
	std::shuffle(oListB.begin(), oListB.end(), g);

	for (const QString &o : std::as_const(oListA)) {
		if (optsA.size() >= (maxOptionsA > 1 ? maxOptionsA : 4))
			break;

		if (!o.isEmpty())
			optsA.append(qMakePair(o, false));
	}


	for (const QString &o : std::as_const(oListB)) {
		if (optsB.size() >= (maxOptionsB > 1 ? maxOptionsB : 4))
			break;

		if (!o.isEmpty())
			optsB.append(qMakePair(o, false));
	}


	int correctIdxA = -1;
	int correctIdxB = -1;

	QStringList optListA, optListB;

	std::shuffle(optsA.begin(), optsA.end(), g);
	std::shuffle(optsB.begin(), optsB.end(), g);

	for (const auto &p : optsA) {
		optListA.append(p.first);

		if (p.second)
			correctIdxA = optListA.size()-1;
	}

	for (const auto &p : optsB) {
		optListB.append(p.first);

		if (p.second)
			correctIdxB = optListB.size()-1;
	}

	QVariantMap m;

	m[QStringLiteral("answer")] = QVariantMap {
	{ QStringLiteral("first"), correctIdxA },
	{ QStringLiteral("second"), correctIdxB },
};

	m[QStringLiteral("optionsA")] = optListA;
	m[QStringLiteral("optionsB")] = optListB;

	return m;
}





ModuleDoublechoice::DoubleData::DoubleData(const QStringList &aList, const QString &correct, const QString &separator)
{
	QStringList cl = correct.split(separator, Qt::SkipEmptyParts);

	if (cl.size() > 1)
		correctB = cl.at(1).simplified();

	if (cl.size() > 0)
		correctA = cl.at(0).simplified();

	if (correctA.isEmpty())
		correctA = QStringLiteral(" ");

	if (correctB.isEmpty())
		correctB = QStringLiteral(" ");

	for (const QString &a : aList) {
		QStringList al = a.split(separator, Qt::SkipEmptyParts);

		if (al.size() > 1)
			oListB.insert(al.at(1).simplified());

		if (al.size() > 0)
			oListA.insert(al.at(0).simplified());
	}
}


/**
 * @brief ModuleDoublechoice::DataList::toAnswerList
 * @return
 */

QStringList Doublechoice::DataList::toAnswerList(const bool &isBindToRight) const
{
	QStringList aList;

	for (const Doublechoice::Data &d : *this) {
		if (isBindToRight && !d.left.isEmpty())
			aList.append(d.left);
		else if (!isBindToRight && !d.right.isEmpty())
			aList.append(d.right);
	}

	return aList;
}



/**
 * @brief Doublechoice::DataList::toDuplexHelper
 * @param helper
 */

void Doublechoice::DataList::toDuplexHelper(SeedDuplexHelper &helper, const QVariantMap &data, const bool &isBindToRight,
											const QStringList &aList) const
{
	const QString &question = data.value(QStringLiteral("question")).toString();
	const bool monospace = data.value(QStringLiteral("monospace")).toBool();
	const int maxOptionsA = std::max(2, data.value(QStringLiteral("maxOptionsA")).toInt());
	const int maxOptionsB = std::max(2, data.value(QStringLiteral("maxOptionsB")).toInt());
	const QString separator = data.value(QStringLiteral("separator")).toString();

	for (const Doublechoice::Data &d : *this) {
		if (d.left.isEmpty() || d.right.isEmpty())
			continue;

		const QString correct = isBindToRight ? d.left : d.right;

		QVariantMap retMap = ModuleDoublechoice::_generate(ModuleDoublechoice::DoubleData(aList, correct, separator),
														   maxOptionsA, maxOptionsB);

		if (question.isEmpty())
			retMap[QStringLiteral("question")] = (isBindToRight ? d.right : d.left);
		else if (question.contains(QStringLiteral("%1")))
			retMap[QStringLiteral("question")] = question.arg(isBindToRight ? d.right : d.left);
		else
			retMap[QStringLiteral("question")] = question;

		retMap[QStringLiteral("monospace")] = monospace;

		helper.append(retMap, d.idx, d.idx);
	}
}


