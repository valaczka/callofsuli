#ifndef EXAMGAME_H
#define EXAMGAME_H

#include <QJsonArray>
#include "abstractgame.h"
#include "question.h"
#include "exam.h"

/**
 * @brief The ExamGame class
 */

class ExamGame : public AbstractGame
{
	Q_OBJECT

	Q_PROPERTY(Exam::Mode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
	ExamGame(const Exam::Mode &mode, Client *client);
	virtual ~ExamGame();

	static QVector<Question> createQuestions(GameMapMissionLevel *missionLevel);
	static QJsonArray generatePaperQuestions(GameMapMissionLevel *missionLevel);
	static void clearQuestions(GameMapMissionLevel *missionLevel);

	const Exam::Mode &mode() const;
	void setMode(const Exam::Mode &newMode);

signals:
	void modeChanged();

protected:
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;
	virtual bool gameFinishEvent() override;

private:
	Exam::Mode m_mode = Exam::ExamPaper;
};


#endif // EXAMGAME_H
