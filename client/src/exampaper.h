/*
 * ---- Call of Suli ----
 *
 * exampaper.h
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

#ifndef EXAMPAPER_H
#define EXAMPAPER_H


#include "qtypes.h"
#include <QString>
#include <optional>
#include <QJsonObject>
#include <QJsonArray>


/**
 * @brief The ExamPaper class
 */

class ExamPaper
{
public:
	ExamPaper() = default;

	enum OptionLetter {
		LetterInvalid = 0,
		LetterA = 0b100000,
		LetterB = 0b010000,
		LetterC = 0b001000,
		LetterD = 0b000100,
		LetterE = 0b000010,
		LetterF = 0b000001,
	};


	static qsizetype maxLetterValue() { return m_letterValue.size(); }

	static quint8 getOptionValue(const QString &options);
	static QString getOptionString(const quint8 &value);

	static QString toOptionLetters(const quint8 &value);
	static QSet<OptionLetter> getOptionLetterList(const QString &options);

	static std::optional<quint8> toOptionValue(const QString &options);

	static std::optional<quint8> toOptionValue(const QJsonObject &data, const int &number) {
		return toOptionValue(data.value(QStringLiteral("q")+QString::number(number)).toString());
	}


	static QSet<OptionLetter> getOptionLetterList(const QJsonObject &data, const int &number) {
		return getOptionLetterList(data.value(QStringLiteral("q")+QString::number(number)).toString());
	}

	static const QHash<QChar, OptionLetter> m_letterHash;
	static const QList<quint8> m_letterValue;
};

#endif // EXAMPAPER_H
