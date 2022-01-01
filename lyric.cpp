
#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif

#include "lyric.h"
#include "desktoplyrics.h"
#include "networkutil.h"
#include "ui_lyric.h"
#include <QAction>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QScrollBar>
#include <QVBoxLayout> //垂直布局管理器
lyric::lyric(QWidget *parent) : QWidget(parent), ui(new Ui::lyric) {
  ui->setupUi(this);
  this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  ui->scrollArea->setFrameShape(QFrame::NoFrame);
  destlyric = new DesktopLyrics();

  //方便设置垂直滑块
  vScrollbar = ui->scrollArea->verticalScrollBar();

  Netmangelyric = new QNetworkAccessManager(this);
  connect(Netmangelyric, &QNetworkAccessManager::finished, this,
          &lyric::on_ReplyFinished);

  Netmanger = new QNetworkAccessManager(this);

  connect(Netmanger, &QNetworkAccessManager::finished, this,
          [&](QNetworkReply *reply) {
            if (reply->error() == QNetworkReply::NoError) {
              QByteArray byte{reply->readAll()};
              QString filepath = QString("../lyric/%1.lrc").arg(name);
              QFile file(filepath);
              if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(byte);
                file.close();
                ReadLyric(filepath);
              } else {
                // ERROR(QString("Can't open %1").arg(filepath));
                //打印文件错误描述
                perror(filepath.toStdString().data());
                //                        ui->lab_lyric->setText("暂无歌词");
              }
            }
            reply->deleteLater();
          });
}

lyric::~lyric() {
  delete destlyric;
  destlyric = nullptr;
  delete ui;
}

void lyric::setMessage(QImage img, QString Art, QString title) {
  //  QPixmap pixmap{QPixmap::fromImage(img)};
  //  //设置自适应图片大小
  //  pixmap.scaled(ui->lab_AlbumArt->size(), Qt::KeepAspectRatio);
  ////
  /// setScaledContents：设置label的属性scaledContents，这个属性的作用是允许（禁止）label缩放它的内容充满整个可用的空间。
  //  ui->lab_AlbumArt->setScaledContents(true);
  //  ui->lab_AlbumArt->setPixmap(pixmap);
  //  ui->lab_AlbumArt->setStyleSheet("border: 0px solid ");
  //设置文字居中,且空间中垂直中心

  //  ui->lab_AlbumArt->setStyleSheet(
  //      "QLabel::#ui->lab_AlbumArt{ background-color: transparent;  "
  //      "border-radius: 10px}");

  //  ui->lab_lyric->setStyleSheet("background-color: transparent;");
  //  ui->lab_artist->setText(QString("%1\n%2").arg(title).arg(Art));
  //  ui->lab_artist->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

void lyric::on_ReplyFinished(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray byte{reply->readAll()};
    QJsonParseError eror_t{};
    QJsonDocument document = QJsonDocument::fromJson(byte, &eror_t);
    if (eror_t.error == QJsonParseError::NoError) {
      QJsonObject root = document.object();
      auto res = root.value("result");
      if (res.isObject()) {
        QJsonObject resobj = res.toObject();
        QJsonValue songs = resobj.value("songs");
        if (songs.isArray()) {
          auto songsarray = songs.toArray();
          QJsonValue idobj = songsarray.at(0).toObject();
          id = idobj.toObject().value("id").toInt();
          name = idobj.toObject().value("name").toString();
          Netmanger->get(QNetworkRequest(
              QString("http://cloud-music.pl-fe.cn/lyric?id=%1").arg(id)));
        }
      }
    }
  }
  ERROR("未找到歌词文件");
  reply->deleteLater();
}

void lyric::GetTheLyricsName(QString rhs) {
  QString filepath = QString("../lyric/%1.lrc").arg(rhs);
  QFileInfo info(filepath);
  //文件是否存在
  if (!info.isFile()) {
    QString url =
        QString("http://cloud-music.pl-fe.cn/search?keywords=%1").arg(rhs);
    Netmangelyric->get(QNetworkRequest(QString(url)));
  } else {
    //读取歌词文件
    ReadLyric(filepath);
  }
}

void lyric::ReadLyric(QString filepath) {
  QFile file(filepath);
  if (file.open(QIODevice::ReadOnly)) {
    name = filepath;
    lines.clear();
    process(file.readAll());
    file.close();
  } else {
    // ERROR(QString("The %1 lyrics file does not exist").arg(name));
    perror(filepath.toStdString().c_str());
    return;
  }
}

void lyric::setpic(QPixmap pix) {
  //保存图片到.exe目录,并设置歌词背景
  //    pix.save("pix.png");
  //    ui->lab_lyric->setStyleSheet("border-image:url(./pix.png);");
}

bool lyric::process(QString content) {
  const QRegExp rx(
      "\\[(\\d+)?:(\\d+\\.\\d+)?\\]"); //用来查找时间标签的正则表达式
  // const QRegExp rx("\\[(\\d{1,2}):(\\d{1,2}).(\\d{1,2})\\]");
  // 步骤1
  int pos =
      rx.indexIn(content); //返回第一个匹配项的位置，如果没有匹配项则返回-1。
  if (pos == -1) {
    return false;
  } else {
    int lastPos;           //
    QList<int> timeLabels; //时间标签
    do {
      // 步骤2
      timeLabels << (rx.cap(1).toInt() * 60 + rx.cap(2).toDouble()) * 1000;
      lastPos =
          pos +
          rx.matchedLength(); //返回最后一个匹配字符串的长度，如果没有匹配则返回-1
      pos = rx.indexIn(content, lastPos);
      if (pos == -1) {
        QString text = content.mid(lastPos).trimmed();
        foreach (const int &time, timeLabels) {
          //删除多余的字符串
          static QString str =
              "\\n\"},\"klyric\":{\"version\":0,\"lyric\":\"\"},"
              "\"tlyric\":{"
              "\"version\":0,\"lyric\":\"\"},\"code\":200}";
          text.remove(str);
          lines.push_back(LyricLine(time, text));
        }
        break;
      }
      // 步骤3
      QString text = content.mid(lastPos, pos - lastPos);
      if (!text.isEmpty()) {
        foreach (const int &time, timeLabels) {
          lines.push_back(LyricLine(time, text.remove("\\n")));
        }
        timeLabels.clear();
      }
    } while (true);
    // 步骤4
    // qStableSort(lines.begin(), lines.end(),
    //            [=](const LyricLine &A1, const LyricLine &A2) {
    //              return A1.time < A2.time;
    //            }); //排序
    std::stable_sort(lines.begin(), lines.end(),
                     [=](const LyricLine &rhs, const LyricLine &lhs) {
                       return rhs.time < lhs.time;
                     });
  }
  //删除最后一行不必要的字符串
  //    QString str{};
  //    for (int x = 0; x != lines.length(); ++x) {
  //        str.append(lines.at(x).text + "\n");
  //    }
  //删除多余的字符串
  //    str.remove(QRegExp("[,a-zA-Z:2{}""\"\"]|(.n)"));
  //          str.remove("\\n\"},\"klyric\":{\"version\":0,\"lyric\":\"\"},\"tlyric\":{"
  //                  "\"version\":0,\"lyric\":\"\"},\"code\":200}");

  //    ui->lab_lyric->setText(str);
  creatlab();
  if (lines.size()) {
    return true;
  }
  return false;
}
int lyric::getIndex(qint64 position) {
  if (!lines.size()) {
    return -1; //如果没歌词
  } else {
    if (lines[0].time >= position) {
      return 0;
    }
  }
  int i = 1;
  for (i = 1; i < lines.size(); i++) {
    if (lines[i - 1].time < position && lines[i].time >= position) {
      return i - 1;
    }
  }
  return lines.size() - 1;
}

void lyric::showcontent(qint64 position) {
  int index = getIndex(position);
  if (index == -1) {
    return;
    //        ui->lab_lyric_1->setText("暂无歌词");
  } else {
    // qDebug() << "输出歌词：" << getLyricText(index);
    //自动向下滑动
    listLab.at(index)->setStyleSheet("color:red");
    slidingDown(index);

    // slidingDown(index);
    //设置桌面歌词
    destlyric->SetLastlyric(getLyricText(index));
    destlyric->SetNextlyric(getLyricText(index - 1));
  }
}

QString lyric::getLyricText(int index) {
  if (index < 0 || index >= lines.size()) {
    return "";
  } else {

    return lines[index].text;
  }
}

void lyric::clearlabel() {
  for (int x = 0; x != 27; x++) {
    listLab.at(x)->clear();
  }
}

void lyric::creatlab() {
  QVBoxLayout *box = new QVBoxLayout(ui->scrollArea->widget());
  int len = lines.length();
  if (len == 0) {
    QLabel *lab = new QLabel("暂无歌词", ui->scrollArea->widget());
    lab->setStyleSheet("color:red");
    return;
  }

  for (int x = 0; x != len; ++x) {
    QLabel *bel = new QLabel(ui->scrollArea->widget());
    //设置居中
    bel->setAlignment(Qt::AlignCenter);
    bel->setStyleSheet("font-family:Microsoft YaHei;font-size:20px;");
    listLab.push_back(bel);
    //加入布局管理器
    box->addWidget(listLab.at(x));
  }
  showlyric();
}

void lyric::showlyric() {
  int n = 0;
  for (auto it = listLab.begin(); it != listLab.end(); ++it) {
    it.i->t()->setText(getLyricText(n));
    it.i->t()->setStyleSheet("color:#666666");
    ++n;
  }
}

void lyric::slidingDown(int index) {
  if (index - 1 < 0)
    return;
  //恢复颜色
  listLab.at(index - 1)->setStyleSheet("color:#666666");
  int height = listLab.at(1)->height();
  vScrollbar->setSliderPosition(index * height);
}
