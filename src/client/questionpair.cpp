/*
 * ---- Call of Suli ----
 *
 * questionpair.cpp
 *
 * Created on: 2020. 05. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Questionpair
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

#include "questionpair.h"
#include <QDebug>

Questionpair::Questionpair(const QJsonObject &data)
	: AbstractStorage("questionpair")
{
	m_pairs = data.value("pairs").toArray();
	m_wrongs = data.value("wrongs").toArray();
}


/**
 * @brief Questionpair::fillContainerFavoriteIndices
 */

void Questionpair::fillContainerFavoriteIndices()
{
	for (int i=0; i<m_pairs.count(); ++i) {
		QJsonObject o = m_pairs.at(i).toObject();
		if (o.value("favorite").toBool())
			m_containerFavoriteIndices.append(i);
	}
}


/**
 * @brief Questionpair::fillContainerNoFavoriteIndices
 */

void Questionpair::fillContainerNoFavoriteIndices()
{
	for (int i=0; i<m_pairs.count(); ++i) {
		QJsonObject o = m_pairs.at(i).toObject();
		if (!o.value("favorite").toBool())
			m_containerNoFavoriteIndices.append(i);
	}
}


QJsonArray Questionpair::pairs() const
{
	return m_pairs;
}

QJsonArray Questionpair::wrongs() const
{
	return m_wrongs;
}

