#ifndef LOADING_H
#define LOADING_H
#include <QLabel>
#include <QMovie>
class QTimer;
class QEvent;
class Loading : public QLabel {
private:
	size_t n{};
	QTimer* time;
	QMovie* movie;
public:
	Loading(QWidget* parent = nullptr);
	~Loading() {};
	void openfile(const QString& filename);
	void start();
	void Stop();
	void setposition(const int x, const int y);
public slots:
	void timeout();
};


#endif //