#include "Loading.h"
#include <QMovie>
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
	movie->start();
}

void Loading::Stop()
{
	movie->setPaused(true);
}

void Loading::setposition(const int x, const int y)
{
	this->move(x, y);
}
