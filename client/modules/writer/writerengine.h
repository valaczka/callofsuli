/*
 * ---- Call of Suli ----
 *
 * writerengine.h
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

#ifndef WRITERENGINE_H
#define WRITERENGINE_H

#include <QObject>

class WriterEngine : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QList<QChar> characters READ characters NOTIFY charactersChanged)
	Q_PROPERTY(QString answer READ answer WRITE setAnswer NOTIFY answerChanged)
	Q_PROPERTY(QString text READ text NOTIFY textChanged)
	Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged)
	Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
	Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
	Q_PROPERTY(QString displayText READ displayText NOTIFY displayTextChanged)

public:
	explicit WriterEngine(QObject *parent = nullptr);

	const QList<QChar> &characters() const;

	const QString &answer() const;
	void setAnswer(const QString &newAnswer);

	const QString &text() const;

	int hp() const;
	void setHp(int newHp);

	int count() const;
	void setCount(int newCount);

	Q_INVOKABLE void write(const int &index);
	Q_INVOKABLE void pressKey(const QString &text);

	bool enabled() const;
	void setEnabled(bool newEnabled);

	QString displayText() const;

signals:
	void wrongKeyPressed(int buttonIndex);
	void succeed();
	void failed();

	void charactersChanged();
	void answerChanged();
	void textChanged();
	void hpChanged();
	void countChanged();
	void enabledChanged();
	void displayTextChanged();

private:
	void generate(const int &currentIndex = -1);
	void wrong(const int &index);

	QList<QChar> m_characters;
	QString m_answer;
	QString m_text;
	int m_hp = 3;
	int m_count = 6;
	bool m_enabled = true;
};

#endif // WRITERENGINE_H
