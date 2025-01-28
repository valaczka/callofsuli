/*
 * ---- Call of Suli ----
 *
 * question.h
 *
 * Created on: 2021. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Question
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef QUESTION_H
#define QUESTION_H

#include <QString>
#include <QVariantMap>

class GameMapObjective;

class Question
{
public:
	explicit Question(GameMapObjective *objective = nullptr);

	bool isValid() const;
	QString module() const;
	QString uuid() const;

	QVariantMap generate() const;

	QVariantMap commonData() const;

	static QVariantMap objectiveInfo(const QString &module, const QVariantMap &data,
									 const QString &storageModule = "", const QVariantMap &storageData = QVariantMap());

	static QVariantMap storageInfo(const QString &module, const QVariantMap &data);

	QString qml() const;

	static QString convertToMonospace(const QString &text);
	static QString monspaceTagStart();
	static QString monspaceTagEnd();

	friend bool operator==(const Question &l, const Question &r)
	{
		return l.m_objective == r.m_objective;
	}


private:
	GameMapObjective *m_objective = nullptr;
};

Q_DECLARE_METATYPE(Question)

#endif // QUESTION_H
