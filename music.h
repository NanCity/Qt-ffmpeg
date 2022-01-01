
#ifndef MUSIC_H
#define MUSIC_H
#include "tag.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <QMainWindow>
#include <QMutex>
#include <QTableWidget>
QT_BEGIN_NAMESPACE
namespace Ui {
class Music;
}
QT_END_NAMESPACE

enum status { STATE_VOLUME };

class skin;
class Base;
class Login;
class lyric;
class Mp3tag;
class Search;
class PersonForm;
class AudioDeCode;
class DesktopLyrics;
//class QNetworkReply;
class Local_and_Download;

class Music : public QMainWindow {
  Q_OBJECT
private:
  int sec{};
  Ui::Music *ui;
  QString SongName{};
  unsigned int CurrVolume{};
  bool sliderSeeking = false;
  int CurrentPlayerListIndex{0};
  QStringList playlist{};
  QMutex mutex{};
  Base *base;
  Login *login;
  lyric *lyr{};
  skin *Skin;
  Search *search;
//  DesktopLyrics *dest;
  //解码器
  AudioDeCode *Decode;
  M_Tag &tag = M_Tag::GetInstance();
  QTableWidget *tableWidget{};
  //检查鼠标是否移动
  bool mMoving = false;
  QPoint mLastMousePosition{};
  PersonForm *perform;
  Local_and_Download *localMusic;

  // stop按钮的状态
  enum class State {
    /*暂停*/
    pause,
    /*恢复播放*/
    resume,
    /*第一次启动*/
    none
  };
  State state;
  enum class PlayMode {
    /*顺序播放 */
    Order,
    /*单曲循环 */
    Single,
    /*随机播放 */
    Random
  };
  PlayMode Mode;
  //判断当前播放列表
  int curCoding{0};

public:
  explicit Music(QWidget *parent = nullptr);
  virtual ~Music();
  void init();
  void initWidget();
  void initTableWidget();
  //悬停提示
  void HoverTip();
  //播放歌曲
  // void SetBottonInformation(Mp3tag *tag);
  // void setBottomInformation();
  void PlayerMode();
  void Previous(QStringList &playerlist);
  void Next(QStringList &playerlist);
  void NetCodec();
  template <typename T> void SetBottonInformation(T &rhs = nullptr);
signals:
  void updateSongLrc(int sec);

protected:
  //重写键盘事件
  // void keyPressEvent(QKeyEvent *event);
  //重写鼠标操作,移动，按下，松开
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void Presskey(QKeyEvent *event);
  bool eventFilter(QObject *obj, QEvent *event);
  void closeEvent(QCloseEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);
  void dropEvent(QDropEvent *event);

private slots:
  void onSeekOk();
  void onDuration(int, int);
  void on_setPlatList(QStringList list);
  void setPlayMode(PlayMode &Mode);
  void on_btn_login_clicked();
  void on_btn_pictrue_clicked();
  void on_btn_stop_clicked();
  void on_btn_mode_clicked();
  void on_btn_next_clicked();
  void on_btn_lyric_clicked();
  void on_btn_max_clicked();
  void on_btn_prev_clicked();
  void on_btn_volum_clicked();
  void on_Sli_volum_valueChanged(int value);
  //进度条按压处理
  void on_playslider_sliderPressed();
  //进度条释放处理
  void on_playslider_sliderReleased();
  void on_btn_skin_clicked();
  void on_btn_personmessage_clicked();
  void on_lineEdit_search_returnPressed();
  /***************播放搜索到的歌曲*****************/
  void on_playSearchMusic(const int songid);
};
#endif // MUSIC_H

template <typename T> inline void Music::SetBottonInformation(T &rhs) {
  if (0 == curCoding) {
    Decode->DeCodeTag(playlist.at(CurrentPlayerListIndex).toLocal8Bit());
  }

  ui->btn_pictrue->setIcon(rhs.GetAblueArt());
  ui->btn_pictrue->setIconSize(ui->btn_pictrue->size());
  ui->lab_message->setText(
      QString("%1\n%2").arg(rhs.GetArtist()).arg(rhs.GetTitle()));
  ui->lab_time->setText(rhs.GetDuration());
  lyr->setMessage(rhs.GetAblueArt().toImage(), rhs.GetArtist(), rhs.GetTitle());
  SongName = rhs.GetTitle();
  lyr->setpic(rhs.GetAblueArt());
  //获取歌词
  lyr->GetTheLyricsName(SongName);
}
