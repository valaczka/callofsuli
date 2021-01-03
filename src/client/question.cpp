/*
 * ---- Call of Suli ----
 *
 * question.cpp
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

#include "question.h"



Question::Question(const QString &module, const QVariantMap &data)
	: m_module(module)
	, m_data(data)
{


}




Question::Question(GameMap::Objective *objective)
{
	Q_ASSERT(objective);

	m_module = objective->module();
	m_data = objective->data();
}





/**
 * @brief Question::generate
 * @return
 */

QVariantMap Question::generate() const
{
	if (m_module == "truefalse")
		return generateTruefalse();
	else
		return QVariantMap();
}




/**
 * @brief Question::generateTruefalse
 * @return
 */

QVariantMap Question::generateTruefalse() const
{
	QVariantMap m = m_data;
	m["module"] = "truefalse";

	return m;
}
