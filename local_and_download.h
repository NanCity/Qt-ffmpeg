
#ifndef LOCAL_AND_DOWNLOAD_H
#define LOCAL_AND_DOWNLOAD_H

#include "base.h"
#include <QWidget>
#include "Decode.h"
namespace Ui {
	class Local_and_Download;
}

class Local_and_Download : public QWidget {
	Q_OBJECT

public:
	explicit Local_and_Download(QWidget* parent = nullptr);
	virtual ~Local_and_Download();
	QStringList& PlayerList() { return lists; };
	// QStringList &getMusicPath();
	void InsertDataInfoTableWidget(const QStringList& value, const int index);
	QTableWidget* getTable();

private slots:
	void on_btn_openFile_clicked();
	void on_tableWidget_customContextMenuRequested(const QPoint& pos);
signals:
	void t_play(const int index);
	void t_nextplay(const int index);
private:

	Base* base{};
	//½âÂë
	AudioDeCode code{};
	QStringList lists{};
	Ui::Local_and_Download* ui;
};

#endif // LOCAL_AND_DOWNLOAD_H
