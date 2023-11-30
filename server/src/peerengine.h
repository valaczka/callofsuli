#ifndef PEERENGINE_H
#define PEERENGINE_H

#include "abstractengine.h"
#include "qhostaddress.h"


/**
 * @brief The PeerUser class
 */

class PeerUser
{
public:
	PeerUser(const QString &username, const QHostAddress &host, const QString &agent)
		: m_username(username)
		, m_host(host)
		, m_timestamp(QDateTime::currentDateTime())
		, m_agent(agent)
	{}
	PeerUser(const QString &username, const QHostAddress &host)
		: PeerUser(username, host, QStringLiteral("")) {}
	PeerUser(const QString &username, const QString &agent)
		: PeerUser(username, QHostAddress::Any, agent) {}
	PeerUser(const QString &username)
		: PeerUser(username, QHostAddress::Any) {}
	PeerUser()
		: PeerUser(QString()) {}

	static QVector<PeerUser>::iterator find(QVector<PeerUser> *list, const PeerUser &user);
	static bool addOrUpdate(QVector<PeerUser> *list, const PeerUser &user);
	static bool remove(QVector<PeerUser> *list, const PeerUser &user);
	static bool get(QVector<PeerUser> *list, const QString &username, PeerUser *user);
	static bool clear(QVector<PeerUser> *list, const qint64 &sec = 120);
	static QJsonArray toJson(const QVector<PeerUser> &list);

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const QString &familyName() const;
	void setFamilyName(const QString &newFamilyName);

	const QString &givenName() const;
	void setGivenName(const QString &newGivenName);

	const QHostAddress &host() const;
	void setHost(const QHostAddress &newHost);

	const QDateTime &timestamp() const;
	void setTimestamp(const QDateTime &newTimestamp);

	const QString &agent() const;
	void setAgent(const QString &newAgent);

	friend bool operator==(const PeerUser &u1, const PeerUser &u2) {
		return (u1.m_username == u2.m_username && u1.m_host == u2.m_host && u1.m_agent == u2.m_agent);
	}

	friend QDebug operator<<(QDebug debug, const PeerUser &user) {
		QDebugStateSaver saver(debug);
		debug.nospace() << qPrintable(QByteArrayLiteral("PeerUser("))
						<< qPrintable(user.m_username)
						<< qPrintable(QByteArrayLiteral(", "))
						<< qPrintable(user.m_host.toString())
						<< qPrintable(QByteArrayLiteral(", "))
						<< qPrintable(user.m_agent)
						<< qPrintable(QByteArrayLiteral(" @"))
						<< qPrintable(user.m_timestamp.toString())
						<< ')';
		return debug;
	}


private:
	QString m_username;
	QString m_familyName;
	QString m_givenName;
	QHostAddress m_host;
	QDateTime m_timestamp;
	QString m_agent;

};




/**
 * @brief The PeerEngine class
 */

class PeerEngine : public AbstractEngine
{
	Q_OBJECT

public:
	explicit PeerEngine(EngineHandler *handler, QObject *parent = nullptr);
	virtual ~PeerEngine() {}

	bool logPeerUser(const PeerUser &user);
	const QVector<PeerUser> &peerUser() const { return m_peerUser; }

	virtual void timerMinuteTick() override;
	virtual void streamTriggerEvent(WebSocketStream *stream) override;

protected:
	virtual void streamLinkedEvent(WebSocketStream *stream) override;

private:
	QVector<PeerUser> m_peerUser;
};

#endif // PEERENGINE_H
