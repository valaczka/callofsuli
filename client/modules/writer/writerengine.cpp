/*
 * ---- Call of Suli ----
 *
 * writerengine.cpp
 *
 * Created on: 2023. 07. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * WriterEngine
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

#include "writerengine.h"
#include "qdebug.h"
#include "qmath.h"
#include <QRandomGenerator>

WriterEngine::WriterEngine(QObject *parent)
	: QObject{parent}
{
	generate();
}


/**
 * @brief WriterEngine::characters
 * @return
 */

const QList<QChar> &WriterEngine::characters() const
{
	return m_characters;
}


/**
 * @brief WriterEngine::answer
 * @return
 */

const QString &WriterEngine::answer() const
{
	return m_answer;
}

void WriterEngine::setAnswer(const QString &newAnswer)
{
	if (m_answer == newAnswer)
		return;
	m_answer = newAnswer;
	emit answerChanged();
	m_text = QLatin1String("");
	emit textChanged();
	emit displayTextChanged();
	m_characters.clear();
	generate();
}


/**
 * @brief WriterEngine::text
 * @return
 */

const QString &WriterEngine::text() const
{
	return m_text;
}


/**
 * @brief WriterEngine::hp
 * @return
 */

int WriterEngine::hp() const
{
	return m_hp;
}

void WriterEngine::setHp(int newHp)
{
	if (m_hp == newHp)
		return;
	m_hp = newHp;
	emit hpChanged();
}


/**
 * @brief WriterEngine::count
 * @return
 */

int WriterEngine::count() const
{
	return m_count;
}

void WriterEngine::setCount(int newCount)
{
	if (m_count == newCount)
		return;
	m_count = newCount;
	m_characters.clear();
	generate();
	emit countChanged();
}



/**
 * @brief WriterEngine::write
 * @param index
 */

void WriterEngine::write(const int &index)
{
	if (index < 0 || index >= m_characters.size()) {
		qWarning() << "Invalid index:" << index;
		return wrong(index);
	}

	const QChar &ch = m_characters.at(index);

	int pos = m_text.size();

	if (pos >= m_answer.size()) {
		qWarning() << "Overflow";
		setEnabled(false);
		return wrong(index);
	}

	if (m_answer.at(pos).toUpper() == ch) {
		int nextPos = pos+1;

		while (nextPos < m_answer.length() && m_answer.at(nextPos).isSpace())
			++nextPos;

		m_text = m_answer.left(nextPos);
		emit textChanged();
		emit displayTextChanged();

		if (nextPos >= m_answer.length()) {
			emit succeed();
			setEnabled(false);
			return;
		}

		generate(index);
		return;
	}

	wrong(index);
}



/**
 * @brief WriterEngine::pressKey
 * @param text
 */

void WriterEngine::pressKey(const QString &text)
{
	if (!text.isEmpty()) {
		const QChar &ch = text.at(0).toUpper();

		for (int i=0; i<m_characters.size(); ++i) {
			if (m_characters.at(i) == ch) {
				write(i);
				return;
			}
		}
	}

	wrong(-1);
}



/**
 * @brief WriterEngine::generate
 */

void WriterEngine::generate(const int &currentIndex)
{
	QList<QChar> list;

	static const QString &defaultCharacters = QStringLiteral("AÁBCDEÉFGHIÍJKLMNOÖŐPQRSTUÚÜŰVWXYZ2345789");

	for (int i=0; i<defaultCharacters.size(); ++i)
		list.append(defaultCharacters.at(i));

	for (int i=0; i<m_answer.length(); ++i) {
		const QChar &ch = m_answer.at(i).toUpper();

		if (!ch.isSpace() && !list.contains(ch))
			list.append(ch);
	}

	while (m_characters.size() > m_count)
		m_characters.removeLast();

	QVector<int> indicesToReplace;

	for (int i=0; i<m_count; ++i)
		indicesToReplace.append(i);

	if (!m_characters.isEmpty()) {
		if (!m_answer.isEmpty() && m_text.size() < m_answer.size()) {
			QVector<QChar> nextChars;

			nextChars << m_answer.at(m_text.size()).toUpper();

			for (int i=m_text.size()+1; i < m_answer.size(); ++i) {
				const QChar &ch = m_answer.at(i);
				if (ch.isSpace())
					continue;

				nextChars << m_answer.at(i).toUpper();
				break;
			}

			for (int i=0; i<m_characters.size(); ++i) {
				if (nextChars.contains(m_characters.at(i)))
					indicesToReplace.removeAll(i);
			}
		}

		if (currentIndex != -1)
			indicesToReplace.removeAll(currentIndex);

		while (indicesToReplace.size() > qMax(2, qCeil(m_count*0.66)))
			indicesToReplace.removeAt(QRandomGenerator::global()->bounded(indicesToReplace.size()));

	}

	for (int i=0; i<m_characters.size(); ++i)
		list.removeAll(m_characters.at(i));


	// Fill characters

	while (m_characters.size() < m_count)
		m_characters.append(QChar(' '));

	int pos = m_text.size();

	if (pos < m_answer.size() && !m_characters.contains(m_answer.at(pos).toUpper()) && !indicesToReplace.isEmpty()) {
		int idx = indicesToReplace.takeAt(QRandomGenerator::global()->bounded(indicesToReplace.size()));
		const QChar &ch = m_answer.at(pos).toUpper();
		m_characters[idx] = ch;
		list.removeAll(ch);
	}

	for (++pos; pos < m_answer.size() && !indicesToReplace.isEmpty(); ++pos) {
		 const QChar &ch = m_answer.at(pos).toUpper();
		 if (ch.isSpace() || m_characters.contains(ch))
			 continue;

		 int idx = indicesToReplace.takeAt(QRandomGenerator::global()->bounded(indicesToReplace.size()));
		 m_characters[idx] = ch;
		 list.removeAll(ch);

		 break;
	}


	while (!indicesToReplace.isEmpty() && !list.isEmpty()) {
		int idx = indicesToReplace.takeFirst();
		const QChar &ch = list.takeAt(QRandomGenerator::global()->bounded(list.size()));
		m_characters[idx] = ch;
	}

	// Last check

	if (m_text.size() < m_answer.size() && !m_characters.contains(m_answer.at(m_text.size()).toUpper())) {
		qWarning() << "Last check error";
		int idx = QRandomGenerator::global()->bounded(m_characters.size());
		m_characters[idx] = m_answer.at(m_text.size()).toUpper();
	}

	emit charactersChanged();
}



/**
 * @brief WriterEngine::wrong
 * @param index
 */

void WriterEngine::wrong(const int &index)
{
	emit wrongKeyPressed(index);
	int newHP = qMax(0, m_hp-1);
	setHp(newHP);

	if (newHP == 0) {
		emit failed();
		setEnabled(false);
	}
}



/**
 * @brief WriterEngine::displayText
 * @return
 */

QString WriterEngine::displayText() const
{
	return m_text + m_answer.mid(m_text.size()).replace(QRegExp("\\S"), "•");
}




/**
 * @brief WriterEngine::enabled
 * @return
 */

bool WriterEngine::enabled() const
{
	return m_enabled;
}

void WriterEngine::setEnabled(bool newEnabled)
{
	if (m_enabled == newEnabled)
		return;
	m_enabled = newEnabled;
	emit enabledChanged();
}
