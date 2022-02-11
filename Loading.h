#ifndef LOADING_H
#define LOADING_H
#include <QLabel>
#include <QMovie>
class Loading : public QLabel {
private:
	QMovie* movie;
public:
	Loading(QWidget* parent = nullptr);
	~Loading() {};
	void openfile(const QString& filename);
	void start();
	void Stop();
	void setposition(const int x, const int y);

};


#endif //