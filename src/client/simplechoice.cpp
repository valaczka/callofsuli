/*
 * ---- Call of Suli ----
 *
 * simplechoice.cpp
 *
 * Created on: 2020. 05. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Simplechoice
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

#include <algorithm>

#include "questionpair.h"
#include "simplechoice.h"

Simplechoice::Simplechoice(AbstractStorage *storage, const QJsonObject &data)
	: AbstractObjective("simplechoice", storage)
{
	m_mode = ModeQuestionpair;

	int sFav = storage->containerFavoriteIndicesCount();
	int sNoFav = storage->containerNoFavoriteIndicesCount();

	if (storage->module() == "questionpair") {
		m_mode = ModeQuestionpair;

		QString mode = data.value("mode").toString();

		if (mode == "favorites")
			m_computation.favorite = sFav;
		else if (mode == "nofavorites")
			m_computation.nofavorite = sNoFav;
		else if (mode == "rest")
			m_computation.rest = 1;
		else if (mode == "number") {
			int num = data.value("number").toInt();

			if (data.value("favoritesFirst").toBool()) {
				int f = std::min(num, sFav);
				m_computation.favorite = f;
				m_computation.undefined = num-f;
			} else if (data.value("noFavoritesFirst").toBool()) {
				int f = std::min(num, sNoFav);
				m_computation.favorite = f;
				m_computation.undefined = num-f;
			} else {
				m_computation.undefined = num;
			}
		} else
			m_computation.undefined = storage->containerAllCount();

		m_prefix = data.value("prefix").toString();
		m_suffix = data.value("suffix").toString();
	} else if (storage->module() == "order") {
		int num = data.value("number").toInt();
		m_computation.undefined = num;
	}

}


/**
 * @brief Simplechoice::generateTargets
 * @param favIndices
 * @param noFavIndices
 * @return
 */

QList<QJsonObject> Simplechoice::generateTargets(const QVariantList &favIndices, const QVariantList &noFavIndices)
{
	QList<QJsonObject> ret;

	switch (m_mode) {
		case ModeQuestionpair:
			ret = generateTargetsQuestionpair(favIndices, noFavIndices);
			break;
		case ModeOrder:
			ret = generateTargetsOrder(favIndices, noFavIndices);
			break;
	}

	return ret;
}



/**
 * @brief Simplechoice::generateTargetsQuestionpair
 * @param favIndices
 * @param noFavIndices
 * @return
 */

QList<QJsonObject> Simplechoice::generateTargetsQuestionpair(const QVariantList &favIndices, const QVariantList &noFavIndices)
{
	QList<QJsonObject> list;

	foreach (QVariant v, favIndices)
		list << generateTargetQuestionpair(v.toInt());

	foreach (QVariant v, noFavIndices)
		list << generateTargetQuestionpair(v.toInt());

	return list;
}





/**
 * @brief Simplechoice::generateTargetsOrder
 * @param favIndices
 * @param noFavIndices
 * @return
 */

QList<QJsonObject> Simplechoice::generateTargetsOrder(const QVariantList &favIndices, const QVariantList &noFavIndices)
{

	return QList<QJsonObject>();
}



/**
 * @brief Simplechoice::generateTargetQuestionpair
 * @param pairIndex
 * @return
 */

QJsonObject Simplechoice::generateTargetQuestionpair(const int &pairIndex)
{
	Questionpair *q = (Questionpair *) m_storage;
	QJsonArray pairs = q->pairs();

	QJsonObject p = pairs.takeAt(pairIndex).toObject();

	QJsonObject target;
	target["question"] = m_prefix+p.value("question").toString()+m_suffix;

	QJsonArray answers;

	QJsonObject trueAnswer;
	trueAnswer["answer"] = p.value("answer").toString();
	trueAnswer["correct"] = true;
	answers << trueAnswer;


	QJsonArray wrongs = q->wrongs();
	for (int i=0; i<wrongs.count(); ++i) {
		QJsonObject o;
		o["answer"] = wrongs.at(i).toString();
		pairs.append(o);
	}

	for (int i=0; i<pairs.count() && i<3; ++i) {
		int idx = random() % pairs.count();
		QJsonObject p = pairs.takeAt(idx).toObject();
		QJsonObject falseAnswer;
		falseAnswer["answer"] = p.value("answer").toString();
		falseAnswer["correct"] = false;
		answers << falseAnswer;
	}

	QJsonArray ret;

	while (answers.count()) {
		int idx = random() % answers.count();
		ret << answers.takeAt(idx);
	}

	target["answers"] = ret;

	return target;
}

