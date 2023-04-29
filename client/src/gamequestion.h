/*
 * ---- Call of Suli ----
 *
 * gamequestion.h
 *
 * Created on: 2022. 12. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameQuestion
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

#ifndef GAMEQUESTION_H
#define GAMEQUESTION_H

#include <QQuickItem>
#include "abstractgame.h"
#include "gamequestioncomponent.h"
#include "qelapsedtimer.h"
#include "question.h"


class GameQuestion : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(AbstractGame *game READ game WRITE setGame NOTIFY gameChanged)
	Q_PROPERTY(QQuickItem *loader READ loader WRITE setLoader NOTIFY loaderChanged)
	Q_PROPERTY(int msecBeforeHide READ msecBeforeHide WRITE setMsecBeforeHide NOTIFY msecBeforeHideChanged)
	Q_PROPERTY(GameQuestionComponent *questionComponent READ questionComponent NOTIFY questionComponentChanged)
	Q_PROPERTY(QString objectiveUuid READ objectiveUuid WRITE setObjectiveUuid NOTIFY objectiveUuidChanged)
	Q_PROPERTY(QVariantMap questionData READ questionData WRITE setQuestionData NOTIFY questionDataChanged)
	Q_PROPERTY(bool postponeEnabled READ postponeEnabled WRITE setPostponeEnabled NOTIFY postponeEnabledChanged)
	Q_PROPERTY(GameMap::GameMode gameMode READ gameMode NOTIFY gameModeChanged)
	Q_PROPERTY(bool toggleMode READ toggleMode NOTIFY toggleModeChanged)
	Q_PROPERTY(QVariantMap storedAnswer READ storedAnswer WRITE setStoredAnswer NOTIFY storedAnswerChanged)

public:
	GameQuestion(QQuickItem *parent = nullptr);
	virtual ~GameQuestion();

	void loadQuestion(const QUrl &componentUrl, const QVariantMap &data, const QString &uuid = "", const QVariantMap &storedAnswer = QVariantMap());
	Q_INVOKABLE void loadQuestion(const Question &question, const QVariantMap &storedAnswer = QVariantMap());

	int msecBeforeHide() const;
	void setMsecBeforeHide(int newMsecBeforeHide);

	AbstractGame *game() const;
	void setGame(AbstractGame *newGame);

	GameQuestionComponent *questionComponent() const;
	void setQuestionComponent(GameQuestionComponent *newQuestionComponent);

	QQuickItem *loader() const;
	void setLoader(QQuickItem *newLoader);

	int elapsedMsec() const;

	const QString &objectiveUuid() const;
	void setObjectiveUuid(const QString &newObjectiveUuid);

	const QVariantMap &questionData() const;
	void setQuestionData(const QVariantMap &newQuestionData);

	bool postponeEnabled() const;
	void setPostponeEnabled(bool newPostponeEnabled);

	GameMap::GameMode gameMode() const;

	bool toggleMode() const;

	const QVariantMap &storedAnswer() const;
	void setStoredAnswer(const QVariantMap &newStoredAnswer);

public slots:
	void onSuccess(const QVariantMap &answer);
	void onFailed(const QVariantMap &answer);
	void onPostpone();
	void onShowAnimationFinished();
	void onHideAnimationFinished();
	void onLoaderLoaded(QQuickItem *item);
	void answerReveal(const QVariantMap &answer);
	void finish();
	void forceDestroy();

signals:
	void success(QVariantMap answer);
	void failed(QVariantMap answer);
	void postponed();
	void started();
	void finished();

	void msecBeforeHideChanged();
	void gameChanged();
	void questionComponentChanged();
	void questionLoaded(GameQuestionComponent *component);
	void questionLoadFailed();
	void loaderChanged();
	void objectiveUuidChanged();
	void questionDataChanged();
	void postponeEnabledChanged();
	void gameModeChanged();
	void toggleModeChanged();
	void storedAnswerChanged();

private:
	AbstractGame *m_game = nullptr;
	int m_msecBeforeHide = 0;
	QPointer<GameQuestionComponent> m_questionComponent = nullptr;
	QPointer<QQuickItem> m_loader = nullptr;
	QElapsedTimer m_elapsedTimer;
	int m_elapsedMsec = -1;
	QString m_objectiveUuid;
	QVariantMap m_questionData;
	bool m_postponeEnabled = false;
	QVariantMap m_storedAnswer;
};

#endif // GAMEQUESTION_H
