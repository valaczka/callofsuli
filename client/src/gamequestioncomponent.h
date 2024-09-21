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

#ifndef OPAQUE_PTR_GameQuestion
#define OPAQUE_PTR_GameQuestion
  Q_DECLARE_OPAQUE_POINTER(GameQuestion*)
#endif


/**
 * @brief The GameQuestionComponent class
 */

class GameQuestionComponent : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(QVariantMap questionData READ questionData NOTIFY questionDataChanged)
	Q_PROPERTY(GameQuestion *question READ question WRITE setQuestion NOTIFY questionChanged)
	Q_PROPERTY(ToggleMode toggleMode READ toggleMode NOTIFY toggleModeChanged)
	Q_PROPERTY(QVariantMap storedAnswer READ storedAnswer NOTIFY storedAnswerChanged)

public:
	GameQuestionComponent(QQuickItem *parent = nullptr);
	virtual ~GameQuestionComponent();

	enum ToggleMode {
		ToggleNone,					// nem mutatja a választ (pl. feladatmegoldás)
		ToggleSelect,				// későbbi beküldéshez mutatja a választ (pl. teszt)
		ToggleFeedback				// beküldi+mutatja a választ (pl. multiplayer)
	};

	Q_ENUM(ToggleMode)

	QVariantMap questionData() const;

	GameQuestion *question() const;
	void setQuestion(GameQuestion *newQuestion);
	ToggleMode toggleMode() const;

	QVariantMap storedAnswer() const;

public slots:

signals:
	void vibrateRequest();
	void questionDataChanged();
	void answerReveal(QVariantMap answer);
	void questionChanged();
	void toggleModeChanged();
	void storedAnswerChanged();

private:
	GameQuestion *m_question = nullptr;
};

#endif // GAMEQUESTIONCOMPONENT_H
