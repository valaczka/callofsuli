#ifndef EXAMGAME_H
#define EXAMGAME_H

#include "qtextdocument.h"
#include "abstractgame.h"
#include "question.h"
#include "exam.h"

class ExamGame : public AbstractGame
{
	Q_OBJECT

	Q_PROPERTY(Exam::Mode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
	struct PaperContent {
		QString questions;
		QString answers;
	};

	struct PdfConfig {
		int examId = 0;
		QString title;
		int fontSize = 10;

		int pagePerUser = 1;				// Min. oldalszám diákonként (üres lapokkal kiegészíti, ha kell)
	};

	ExamGame(const Exam::Mode &mode, Client *client);
	virtual ~ExamGame();

	static QVector<Question> createQuestions(GameMapMissionLevel *missionLevel);
	static QJsonArray generatePaperQuestions(GameMapMissionLevel *missionLevel);
	static void generatePdf(const QJsonArray &list, const PdfConfig &pdfConfig, TeacherGroup *group = nullptr);

	static PaperContent generateQuestions(const QVector<Question> &list);

	const Exam::Mode &mode() const;
	void setMode(const Exam::Mode &newMode);

signals:
	void modeChanged();

protected:
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;
	virtual bool gameFinishEvent() override;

private:
	static QString pdfTitle(const QString &title, const QString &username,
							const int &examId, const int &contentId, const bool &isFirstPage, QTextDocument *document);
	static QString pdfSheet(const bool &addResource, QTextDocument *document);

	Exam::Mode m_mode = Exam::ExamPaper;

};

#endif // EXAMGAME_H
