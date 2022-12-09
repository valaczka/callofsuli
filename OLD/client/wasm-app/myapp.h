#ifndef MYAPP_H
#define MYAPP_H

#include <QObject>

class MyApp : public QObject
{
	Q_OBJECT
public:
	explicit MyApp(QObject *parent = nullptr);

public slots:
	void testUrl();

signals:

};

#endif // MYAPP_H
