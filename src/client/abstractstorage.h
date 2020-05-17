/*
 * ---- Call of Suli ----
 *
 * abstractstorage.h
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

#ifndef ABSTRACTSTORAGE_H
#define ABSTRACTSTORAGE_H

#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QVariant>

class AbstractObjective;

class AbstractStorage
{
public:
	class Computation {
	public:
		int favorite;
		int nofavorite;
		int undefined;
		int rest;

		Computation(const int &f = 0, const int &nf = 0, const int &u = 0, const int &r = 0) {
			favorite = f;
			nofavorite = nf;
			undefined = u;
			rest = r;
		}

		Computation& operator=(const Computation &c) {
			favorite = c.favorite;
			nofavorite = c.nofavorite;
			undefined = c.undefined;
			rest = c.rest;
			return *this;
		}

		Computation& operator+=(const Computation &c) {
			favorite += c.favorite;
			nofavorite += c.nofavorite;
			undefined += c.undefined;
			rest += c.rest;
			return *this;
		}
	};

	class Target
	{
	public:

		Target(AbstractObjective *objective = nullptr);

		QString module;
		QJsonObject task;
		QJsonObject solution;
		bool (*solutionFunc)(const AbstractStorage::Target &target, const QJsonObject &solution);
	};


	AbstractStorage(const QString &module);
	virtual ~AbstractStorage();

	virtual void fillContainerFavoriteIndices() = 0;
	virtual void fillContainerNoFavoriteIndices() = 0;

	void fillContainers();
	int containerFavoriteIndicesCount() const { return m_containerFavoriteIndices.count(); }
	int containerNoFavoriteIndicesCount() const { return m_containerNoFavoriteIndices.count(); }
	int containerAllCount() const { return m_containerFavoriteIndices.count()+m_containerNoFavoriteIndices.count(); }

	void setObjectives(const QVariantList &objectives);

	QString module() const { return m_module; }

	QList<AbstractStorage::Target> createTargets();

protected:
	QString m_module;
	QVariantList m_containerFavoriteIndices;
	QVariantList m_containerNoFavoriteIndices;

	QList<AbstractObjective *>m_objectives;
	Computation m_sumComputation;

};



#endif // ABSTRACTSTORAGE_H
