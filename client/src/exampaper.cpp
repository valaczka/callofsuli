/*
 * ---- Call of Suli ----
 *
 * exampaper.cpp
 *
 * Created on: 2026. 01. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ExamPaper
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

#include "exampaper.h"
#include "Logger.h"




const QHash<QChar, ExamPaper::OptionLetter> ExamPaper::m_letterHash = {
	{ 'A', LetterA },
	{ 'B', LetterB },
	{ 'C', LetterC },
	{ 'D', LetterD },
	{ 'E', LetterE },
	{ 'F', LetterF },
};



const QList<quint8> ExamPaper::m_letterValue = {
	LetterA,
	LetterB,
	LetterC,
	LetterD,
	LetterE,
	LetterF,

	LetterA | LetterB,
	LetterA | LetterC,
	LetterA | LetterD,
	LetterA | LetterE,
	LetterA | LetterF,

	LetterB | LetterC,
	LetterB | LetterD,
	LetterB | LetterE,
	LetterB | LetterF,

	LetterC | LetterD,
	LetterC | LetterE,
	LetterC | LetterF,

	LetterD | LetterE,
	LetterD | LetterF,

	LetterE | LetterF,
};





/**
 * @brief ExamPaper::getOptionValue
 * @param options
 * @return
 */

quint8 ExamPaper::getOptionValue(const QString &options)
{
	quint8 value = 0x0;

	for (const QChar &ch : options) {
		value |= m_letterHash.value(ch, LetterInvalid);
	}

	return value;
}



/**
 * @brief ExamPaper::getOptionString
 * @param value
 * @return
 */

QString ExamPaper::getOptionString(const quint8 &value)
{
	QString s;

	QList<QChar> keys = m_letterHash.keys();
	std::sort(keys.begin(), keys.end());

	for (const QChar &ch : keys) {
		if (value & m_letterHash.value(ch))
			s.append(ch);
	}

	return s;
}



/**
 * @brief ExamPaper::toOptionLetters
 * @param value
 * @return
 */

QString ExamPaper::toOptionLetters(const quint8 &value)
{
	QString s;

	if (value >= m_letterValue.size()) {
		LOG_CERROR("utils") << "Letter size exceeded" << value;
		return s;
	}

	const quint8 realvalue = m_letterValue.at(value);

	QList<QChar> keys = m_letterHash.keys();
	std::sort(keys.begin(), keys.end());

	for (const QChar &ch : keys) {
		if (realvalue & m_letterHash.value(ch))
			s.append(ch);
	}

	return s;
}


/**
 * @brief ExamPaper::toOptionValue
 * @param options
 * @return
 */

std::optional<quint8> ExamPaper::toOptionValue(const QString &options)
{
	const quint8 value = getOptionValue(options);

	if (value == 0x0)
		return std::nullopt;


	const int idx = m_letterValue.indexOf(value);

	if (idx < 0)
		return std::nullopt;

	return static_cast<quint8>(idx);
}




/**
 * @brief ExamPaper::getOptionLetterList
 * @param options
 * @return
 */

QSet<ExamPaper::OptionLetter> ExamPaper::getOptionLetterList(const QString &options)
{
	QSet<OptionLetter> list;

	for (const QChar &ch : options) {
		OptionLetter l = m_letterHash.value(ch, LetterInvalid);
		if (l != LetterInvalid)
			list.insert(l);
	}

	return list;
}
