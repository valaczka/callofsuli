/*
 * ---- Call of Suli ----
 *
 * abstractstorage.cpp
 *
 * Created on: 2020. 05. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractStorage
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

#include <QJsonDocument>
#include <QDebug>
#include <math.h>

#include "abstractstorage.h"


AbstractStorage::AbstractStorage(const QString &module)
	: m_module(module)
	, m_sumComputation(0, 0, 0, 0)
{

}


/**
 * @brief AbstractStorage::~AbstractStorage
 */

AbstractStorage::~AbstractStorage()
{
	/*qDeleteAll(m_objectives.begin(), m_objectives.end());
	m_objectives.clear();*/
}


/**
 * @brief AbstractStorage::fillContainers
 */

void AbstractStorage::fillContainers()
{
	fillContainerFavoriteIndices();
	fillContainerNoFavoriteIndices();
}


/**
 * @brief AbstractStorage::setObjectives
 * @param objectives
 */

void AbstractStorage::setObjectives(const QVariantList &objectives)
{
	/*foreach (QVariant v, objectives) {
		QVariantMap m = v.toMap();

		QString module = m.value("module").toString();

		QString data = m.value("data").toString();
		QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
		QJsonObject d = doc.object();

		qWarning() << "INVALID MODULE" << module;

	}*/
}





/**
 * @brief AbstractStorage::createTargets
 * @return
 */

QList<AbstractStorage::Target> AbstractStorage::createTargets()
{
	/*QVariantList origFavInd = m_containerFavoriteIndices;
	QVariantList origNoFavInd = m_containerNoFavoriteIndices;

	QVariantList reqFavInd;
	QVariantList reqNoFavInd;

	bool hasFavRest = true;
	bool hasNoFavRest = true;

	for (int i=0; i<m_sumComputation.favorite; ++i) {
		if (origFavInd.isEmpty()) {
			origFavInd = m_containerFavoriteIndices;
			hasFavRest = false;
		}

		int idx = random() % origFavInd.count();

		reqFavInd.append(origFavInd.takeAt(idx));
	}

	for (int i=0; i<m_sumComputation.nofavorite; ++i) {
		if (origNoFavInd.isEmpty()) {
			origNoFavInd = m_containerNoFavoriteIndices;
			hasNoFavRest = false;
		}

		int idx = random() % origNoFavInd.count();

		reqNoFavInd.append(origNoFavInd.takeAt(idx));
	}

	struct Index {
		QVariant idx;
		bool isFav;
	};

	QList<Index> origRestInd;
	QList<Index> reqUndInd;

	if (hasFavRest) {
		foreach (QVariant v, origFavInd) {
			Index i;
			i.idx = v;
			i.isFav = true;
			origRestInd << i;
		}
	}

	if (hasNoFavRest) {
		foreach (QVariant v, origNoFavInd) {
			Index i;
			i.idx = v;
			i.isFav = false;
			origRestInd << i;
		}
	}

	for (int i=0; i<m_sumComputation.undefined; ++i) {
		if (origRestInd.isEmpty()) {
			foreach (QVariant v, m_containerFavoriteIndices) {
				Index i;
				i.idx = v;
				i.isFav = true;
				origRestInd << i;
			}

			foreach (QVariant v, m_containerNoFavoriteIndices) {
				Index i;
				i.idx = v;
				i.isFav = true;
				origRestInd << i;
			}

			hasFavRest = false;
			hasNoFavRest = false;
		}

		int idx = random() % origRestInd.count();

		reqUndInd.append(origRestInd.takeAt(idx));
	}

	int restNum = m_sumComputation.rest;
	int restCount = 0;

	if (hasFavRest || hasNoFavRest) {
		int maxRest = origRestInd.count();

		restCount = floor((float) maxRest / (float) restNum);
	}
*/

	QList<AbstractStorage::Target> ret;

	/*foreach (AbstractObjective *AO, m_objectives) {
		QVariantList favIndices;
		QVariantList noFavIndices;

		for (int i=0; i<c.favorite && reqFavInd.count(); ++i) {
			int idx = random() % reqFavInd.count();
			favIndices.append(reqFavInd.takeAt(idx));
		}

		for (int i=0; i<c.nofavorite && reqNoFavInd.count(); ++i) {
			int idx = random() % reqNoFavInd.count();
			noFavIndices.append(reqNoFavInd.takeAt(idx));
		}

		for (int i=0; i<c.undefined && reqUndInd.count(); ++i) {
			int idx = random() % reqUndInd.count();
			Index q = reqUndInd.takeAt(idx);
			if (q.isFav)
				favIndices.append(q.idx);
			else
				noFavIndices.append(q.idx);
		}

		if (restCount) {
			for (int i=0; i<c.rest && restNum && origRestInd.count(); ++i) {
				int n;

				if (restNum > 1) {
					n = restCount;
				} else {
					n = origRestInd.count();
				}

				for (int j=0; j<n; ++j) {
					int idx = random() % origRestInd.count();
					Index q = origRestInd.takeAt(idx);
					if (q.isFav)
						favIndices.append(q.idx);
					else
						noFavIndices.append(q.idx);
				}

				restNum--;
			}
		}

		ret << AO->generateTargets(favIndices, noFavIndices);
	}*/

	return ret;

}



/**
 * @brief AbstractStorage::Target::Target
 * @param obj
 */
AbstractStorage::Target::Target(AbstractObjective *objective)
{
	solutionFunc = nullptr;
}
