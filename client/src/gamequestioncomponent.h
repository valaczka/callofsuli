/*
 * ---- Call of Suli ----
 *
 * gamequestioncomponent.h
 *
 * Created on: 2022. 12. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameQuestionComponent
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

#ifndef GAMEQUESTIONCOMPONENT_H
#define GAMEQUESTIONCOMPONENT_H

#include <QQuickItem>

class GameQuestion;

class GameQuestionComponent : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(QVariantMap questionData READ questionData NOTIFY questionDataChanged)
	Q_PROPERTY(GameQuestion *question READ question WRITE setQuestion NOTIFY questionChanged)

public:
	GameQuestionComponent(QQuickItem *parent = nullptr);
	virtual ~GameQuestionComponent();

	QVariantMap questionData() const;

	GameQuestion *question() const;
	void setQuestion(GameQuestion *newQuestion);

public slots:

signals:
	void questionDataChanged();
	void answerReveal(QVariantMap answer);
	void questionChanged();

private:
	GameQuestion *m_question = nullptr;

};

#endif // GAMEQUESTIONCOMPONENT_H
