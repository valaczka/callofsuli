#ifndef EXAMGAME_H
#define EXAMGAME_H

#include "abstractgame.h"
#include "question.h"

class ExamGame : public AbstractGame
{
	Q_OBJECT

	Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
	enum Mode {
		ExamPaper,
		ExamOnline
	};

	Q_ENUM(Mode);

	struct PaperContent {
		QString questions;
		QString answers;
	};

	ExamGame(const Mode &mode, Client *client);
	virtual ~ExamGame();

	static QVector<Question> createQuestions(GameMapMissionLevel *missionLevel);
	static PaperContent generateQuestions(const QVector<Question> &list);

	const Mode &mode() const;
	void setMode(const Mode &newMode);

signals:
	void modeChanged();

protected:
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;
	virtual bool gameFinishEvent() override;

private:
	Mode m_mode = ExamPaper;

};

#endif // EXAMGAME_H
