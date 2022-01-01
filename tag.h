#ifndef _M_TAG_H
#define _M_TAG_H
//饿汉式单例
#include <QPixmap>
#include <QString>
class M_Tag {
public:
  // 将其拷贝构造和赋值构造成为私有函数, 禁止外部拷贝和赋值
  M_Tag(const M_Tag &signal) = delete;
  M_Tag &operator=(const M_Tag &signal) = delete;
  //获取单例实例,返回引用
  static M_Tag &GetInstance() {
    static M_Tag tag;
    return tag;
  }

  //打印实例地址
  void Print();
  QString GetArtist();
  QString GetTitle();
  QString GetAblue();
  QString GetSize();
  QString GetDuration();
  QPixmap GetAblueArt();

  void SetArtist(QString art) { Artist = art; }
  void SetTitle(QString titel) { Title = titel; }
  void SetAblue(QString ablue) { Ablue = ablue; }
  void SetSize(QString _size) { Size = _size; }
  void SetDuration(QString dur) { Duration = dur; }
  void SetAblueArt(QPixmap ico) { AblueArt = ico; }

private:
  //私有化默认构造函数
  M_Tag();
  ~M_Tag();

private:
  QString Artist{};
  QString Title{};
  QString Ablue{};
  QString Size{};
  QString Duration{};
  QPixmap AblueArt{};
};

// static M_Tag  &tag = M_Tag::GetInstance(); 会导致程序崩溃
//
static M_Tag &GetTag(M_Tag &tag) { return tag; }

#endif // _M_TAG_H
