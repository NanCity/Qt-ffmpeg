#include "Loading.h"
#include <QMovie>
#include <QEvent>
#include <QTimer>
Loading::Loading(QWidget* parent) {
	this->setParent(parent);
	this->setGeometry(405, 40, 150, 150);
	movie = new QMovie(this);
	//默认打开的gif加载动画
	movie->setFileName(":/gif/gif/loading4.gif");
	if (!movie->isValid()) {
		fprintf(stdout, "GIF不可用\n");
	}
	else
	{
		//适应label大小
		this->setMovie(movie);
		movie->setScaledSize(this->size());
	}
	time = new QTimer(this);
	//超时
	connect(time, &QTimer::timeout, this, &Loading::timeout);
}

void Loading::openfile(const QString& filename)
{
	movie->setFileName(filename);
	if (!movie->isValid()) {
		fprintf(stdout, "GIF不可用\n");
		return;
	}
	else
	{
		this->setMovie(movie);
		movie->setScaledSize(this->size());
	}

}

void Loading::start()
{
	time->start(1000);
	movie->start();
}

void Loading::Stop()
{
	time->stop();
	movie->setPaused(true);
}

void Loading::setposition(const int x, const int y)
{
	this->move(x, y);
}

void Loading::timeout()
{
	//60秒,网络超时，自动关闭加载动画
	if (n == 60) {
		time->stop();
		movie->setPaused(true);
		this->hide();
		n = 0;
	}
	++n;
}
