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
	const QStringList &options = data.value(QStringLiteral("options")).toStringList();


	QString html;

	if (data.value(QStringLiteral("monospace")).toBool())
		html += Question::monspaceTagStart();

	html += QStringLiteral("<p class=\"options\">");
	html += options.join(QStringLiteral(" • "));
	html += QStringLiteral("</p>");

	html += QStringLiteral("<p>");

	if (answer.contains(QStringLiteral("list"))) {
		const QVariantList &list = answer.value(QStringLiteral("list")).toList();

		QStringList a;

		foreach (const QVariant &v, list) {
			bool ok = false;
			const int &idx = v.toInt(&ok);

			if (ok && idx >= 0 && idx < options.size())
				a.append(options.at(idx));
			else
				a.append(QStringLiteral("???"));
		}

		if (success)
			html += QStringLiteral("<span class=\"answer\">");
		else
			html += QStringLiteral("<span class=\"answerFail\">");

		html += a.join(QStringLiteral(", ")) + QStringLiteral("</span>");

	}

	if (!success) {
		html += QStringLiteral(" <span class=\"answerCorrect\">");
		const QVariantList &aList = data.value(QStringLiteral("answer")).toList();
		QStringList cList;
		for (int i=0; i<options.size(); ++i) {
			if (aList.contains(i))
				cList.append(options.at(i));
		}
		html += cList.join(QStringLiteral(", ")) + QStringLiteral("</span>");
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

	/*if (storage->name() == QStringLiteral("binding"))
		return generateBinding(data, storageData, seed);

	if (storage->name() == QStringLiteral("block"))
		return generateBlock(data, storageData, seed);

	if (storage->name() == QStringLiteral("mergeblock"))
		return generateMergeBlock(data, storageData, seed);

	if (storage->name() == QStringLiteral("mergebinding"))
		return generateMergeBinding(data, storageData, seed);*/



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

		s.append(QStringLiteral("**")+m.value(QStringLiteral("question")).toString()+QStringLiteral("**\n"));


		{
			s.append(QStringLiteral("- (1)\n"));
			int correct = m.value(QStringLiteral("answerA"), -1).toInt();
			QStringList l = m.value(QStringLiteral("optionsA")).toStringList();

			for (int i=0; i<l.size(); ++i) {
				if (i==correct)
					s.append(QStringLiteral("  - **")+l.at(i)+QStringLiteral("**\n"));
				else
					s.append(QStringLiteral("  - ")+l.at(i)+QStringLiteral("\n"));
			}
		}

		{
			s.append(QStringLiteral("- (2)\n"));
			int correct = m.value(QStringLiteral("answerB"), -1).toInt();
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

	QString correctA, correctB;

	QStringList cl = correct.split(separator, Qt::SkipEmptyParts);

	if (cl.size() > 1)
		correctB = cl.at(1).simplified();

	if (cl.size() > 0)
		correctA = cl.at(0).simplified();

	if (correctA.isEmpty())
		correctA = QStringLiteral(" ");

	if (correctB.isEmpty())
		correctB = QStringLiteral(" ");


	QSet<QString> oListA, oListB;

	for (const QString &a : aList) {
		QStringList al = a.split(separator, Qt::SkipEmptyParts);

		if (al.size() > 1)
			oListB.insert(al.at(1).simplified());

		if (al.size() > 0)
			oListA.insert(al.at(0).simplified());
	}

	QVariantMap m = _generate(correctA, correctB, oListA, oListB, maxOptionsA, maxOptionsB);

	m[QStringLiteral("question")] = question;
	m[QStringLiteral("monospace")] = monospace;

	return m;
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
		for (const quint8 v : m_optionsA) {
			if ((value & v) == v)
				r = v;
		}
	} else {
		for (const quint8 v : m_optionsB) {
			if ((value & v) == v)
				r = v;
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
 * @param correctA
 * @param correctB
 * @param optionsListA
 * @param optionsListB
 * @param maxOptionsA
 * @param maxOptionsB
 * @return
 */

QVariantMap ModuleDoublechoice::_generate(const QString &correctA, const QString &correctB,
										  const QSet<QString> &optionsListA, const QSet<QString> &optionsListB,
										  int maxOptionsA, int maxOptionsB) const
{
	QVector<QPair<QString, bool>> optsA, optsB;

	optsA.append(qMakePair(correctA, true));
	optsB.append(qMakePair(correctB, true));

	QStringList oListA = optionsListA.values();
	QStringList oListB = optionsListB.values();

	oListA.removeAll(correctA);
	oListB.removeAll(correctB);

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

