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
#include "gamemap.h"

class Question
{
public:
	explicit Question(const QString &module, const QVariantMap &data);
	explicit Question(GameMap::Objective *objective, const int &storageNum = 0);

	QVariantMap generate() const;

	static QVariantMap objectivesMap();
	static QVariantMap storagesMap();

	static QVariantMap objectiveInfo(const QString &module, const QVariantMap &data = QVariantMap(),
									 const QString &storageModule = "", const QVariantMap &storageData = QVariantMap());
	static QVariantMap objectiveInfo(const QString &module, const QString &dataString,
									 const QString &storageModule = "", const QString &storageDataString = "");

	static QVariantMap storageInfo(const QString &module);


	static QVariantMap objectiveData(const QString &module, const QVariantMap &data,
									 const QString &storageModule = "", const QVariantMap &storageData = QVariantMap());
	static QVariantMap objectiveData(const QString &module, const QString &dataString,
									 const QString &storageModule = "", const QString &storageDataString = "");
	static QVariantMap objectiveData(GameMap::Objective *objective, GameMap::Storage *storage = nullptr);

protected:
	QVariantMap generateTruefalse() const;
	static QVariantMap toMapTruefalse(const QVariantMap &data, const QString &storageModule = "",
									  const QVariantMap &storageData = QVariantMap());

	QVariantMap generateSimplechoice() const;
	static QVariantMap toMapSimplechoice(const QVariantMap &data, const QString &storageModule = "",
										 const QVariantMap &storageData = QVariantMap());

	QVariantMap generateCalculator() const;
	static QVariantMap toMapCalculator(const QVariantMap &data, const QString &storageModule = "",
									   const QVariantMap &storageData = QVariantMap());
	QVariantMap generateCalculatorPlusMinus(GameMap::Storage *storage) const;

	QString m_module;
	QVariantMap m_data;
	int m_storageNum;
	GameMap::Objective *m_objective;

};

#endif // QUESTION_H
