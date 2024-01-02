#ifndef EXAMGAME_H
#define EXAMGAME_H

#include "qlambdathreadworker.h"
#include "qprocess.h"
#include "qtemporarydir.h"
#include "qtextdocument.h"
#include "abstractgame.h"
#include "question.h"
#include "exam.h"
#include "SBarcodeDecoder.h"

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
		QString subject;
		int fontSize = 8;

		int pagePerUser = 1;				// Min. oldalszám diákonként (üres lapokkal kiegészíti, ha kell)
	};


	enum ScanFileState {
		ScanFileLoaded,
		ScanFileReadQR,
		ScanFileReadingOMR,
		ScanFileInvalid,
		ScanFileError,
		ScanFileFinished
	};

	Q_ENUM(ScanFileState);

	struct ScanFile {
		QString path;
		ScanFileState state = ScanFileLoaded;
		int examId = -1;
		int contentId = -1;
		QString username;
		QJsonArray result;
	};

	ExamGame(const Exam::Mode &mode, Client *client);
	virtual ~ExamGame();

	static QVector<Question> createQuestions(GameMapMissionLevel *missionLevel);
	static QJsonArray generatePaperQuestions(GameMapMissionLevel *missionLevel);

	void generatePdf(const QJsonArray &list, const PdfConfig &pdfConfig, TeacherGroup *group = nullptr);

	Q_INVOKABLE void scanImageDir(const QString &path);

	static PaperContent generateQuestions(const QVector<Question> &list);

	const Exam::Mode &mode() const;
	void setMode(const Exam::Mode &newMode);

signals:
	void modeChanged();
	void pdfFileGenerated(QString filename);
	void scanQRfinished();
	void scanOMRfinished();

protected:
	virtual QQuickItem *loadPage() override;
	virtual void connectGameQuestion() override;
	virtual bool gameFinishEvent() override;

private:
	static QString pdfTitle(const PdfConfig &pdfConfig, const QString &username, const int &contentId, const bool &isFirstPage, QTextDocument *document);
	static QString pdfSheet(const bool &addResource, const int &width, QTextDocument *document);
	static QString pdfQuestion(const QJsonArray &list);

	void scanImages();
	bool scanHasPendingQR();
	void scanUpdateQR(const QString &path, const QString &qr);
	void scanPreapareOMR();
	void runOMR();
	void onOmrFinished(int exitCode, QProcess::ExitStatus exitStatus);

	static const QString m_optionLetters;

	Exam::Mode m_mode = Exam::ExamPaper;
	QVector<ScanFile> m_scanFiles;
	std::unique_ptr<QTemporaryDir> m_scanTempDir;
	std::unique_ptr<QProcess> m_omrProcess = nullptr;
	QLambdaThreadWorker m_worker;
	QRecursiveMutex m_mutex;

};


/**
 * @brief The ExmSBarcodeDecoder class
 */

class ExamSBarcodeDecoder : public SBarcodeDecoder
{
	Q_OBJECT

public:
	ExamSBarcodeDecoder(QObject *parent = nullptr) : SBarcodeDecoder(parent) {}

	void setPath(const QString &p) { m_path = p; }
	const QString &path() const { return m_path; }

private:
	QString m_path;
};

#endif // EXAMGAME_H
