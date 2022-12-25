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
#include "gamequestioncomponent.h"
#include "qdatetime.h"
#include "question.h"

class AbstractGame;

class GameQuestion : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(AbstractGame *game READ game WRITE setGame NOTIFY gameChanged)
	Q_PROPERTY(QQuickItem *loader READ loader WRITE setLoader NOTIFY loaderChanged)
	Q_PROPERTY(int msecBeforeHide READ msecBeforeHide WRITE setMsecBeforeHide NOTIFY msecBeforeHideChanged)
	Q_PROPERTY(GameQuestionComponent *questionComponent READ questionComponent NOTIFY questionComponentChanged)
	Q_PROPERTY(QString objectiveUuid READ objectiveUuid WRITE setObjectiveUuid NOTIFY objectiveUuidChanged)
	Q_PROPERTY(QVariantMap questionData READ questionData WRITE setQuestionData NOTIFY questionDataChanged)

public:
	GameQuestion(QQuickItem *parent = nullptr);
	virtual ~GameQuestion();

	void loadQuestion(const QUrl &componentUrl, const QVariantMap &data, const QString &uuid = "");
	void loadQuestion(const Question &question);

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

public slots:
	void onSuccess(const QVariantMap &answer);
	void onFailed(const QVariantMap &answer);
	void onShowAnimationFinished();
	void onHideAnimationFinished();
	void onLoaderLoaded(QQuickItem *item);
	void answerReveal(const QVariantMap &answer);
	void finish();
	void forceDestroy();

signals:
	void success(QVariantMap answer);
	void failed(QVariantMap answer);
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


private:
	AbstractGame *m_game = nullptr;
	int m_msecBeforeHide = 0;
	QPointer<GameQuestionComponent> m_questionComponent = nullptr;
	QPointer<QQuickItem> m_loader = nullptr;
	QTime m_time;
	int m_elapsedMsec = -1;
	QString m_objectiveUuid;
	QVariantMap m_questionData;

};

#endif // GAMEQUESTION_H
