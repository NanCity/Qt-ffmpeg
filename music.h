#ifndef MUSIC_H
#define MUSIC_H
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
class Config;
class M_Tag;
class skin;
class Base;
class Login;
class lyric;
class Mp3tag;
class Search;
class PersonForm;
class AudioDeCode;
class DesktopLyrics;
class CloudMusic;
class QListWidgetItem;
class Local_and_Download;
class Dicovermusic;
class SoloAlbum;

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
  Config *config;
  Base *base;
  Login *login;
  lyric *lyr{};
  skin *Skin;
  Search *search;
  //云盘
  CloudMusic *cloudMusic;
  //  DesktopLyrics *dest;
  //解码器
  AudioDeCode *Decode;
  // M_Tag &tag = M_Tag::GetInstance();
  QTableWidget *tableWidget{};

  Dicovermusic *DicMusic;
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
  //窗口事件安装
  void InstallEventFilter();
  //初始化
  void init();
  //悬停提示
  void HoverTip();
  void PlayerMode();
  void CheckState();
  void Previous(QStringList &playerlist);
  void Next(QStringList &playerlist);
  void SetBottonInformation(M_Tag *);

  void AlbConnect();
  void DecodeConnect();
  void SearchConnect();
  void SocalMusicConnect();
  void CloudMusicConnect();
  void LocalMusicConnect();
  void PersonFormConnect();
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
  template <class T> void On_NetplayAll(T rhs);
  template <class T> void On_Netplay(T rhs, const int index);
  template <class T>
  void On_NetNextPlay(T rhs, const int index, const QString url);
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
  //登录成功
  void on_login_succes();

  //我的音乐这个listwidget被单击
  void on_down_listWidgetClicked(QListWidgetItem *item);

  //展开/隐藏歌单列表
  void on_btn_SongMenu_clicked();
  //创建歌单
  void on_AddSongMenu_clicked();
  //展开/隐藏收藏歌单
  void on_btn_collectMenu_clicked();

  /*搜索框输入内容*/
  void on_lineEdit_search_textChanged(QString str);
  void on_lineEdit_search_returnPressed();
  void on_Sli_volum_valueChanged(int value);
  //进度条按压处理
  void on_playslider_sliderPressed();
  //进度条释放处理
  void on_playslider_sliderReleased();
  void on_btn_skin_clicked();
  void on_btn_personmessage_clicked();
  /***************播放搜索到的歌曲*****************/
  //播放全部，先清空播放列表
  //  void on_NetplayAll();
  //  //播放点击的歌曲
  //  void on_Netplay(const int index);
  //  //下一首播放
  //  void on_NetNextPlay(const int index, const QString url);
};
#endif // MUSIC_H

template <class T> inline void Music::On_NetplayAll(T rhs) {
  playlist.clear();
  CurrentPlayerListIndex = 0;
  playlist = rhs->GetPlayList();
  Decode->tag->SetSongId(rhs->GetPlaylistID());
  ui->btn_stop->setStyleSheet(
      "border-image:url(:/images/bottom/btn_pause_h.png);");
  Decode->play(playlist.at(CurrentPlayerListIndex), 0);
  qDebug() << "进入次数\n";
}

template <class T> inline void Music::On_Netplay(T rhs, const int index) {
  QString url = QString("https://music.163.com/song/media/outer/url?id=%1.mp3")
                    .arg(rhs->GetPlaylistID().at(index));
  Decode->tag->SetSongId(rhs->GetPlaylistID());
  playlist.insert(CurrentPlayerListIndex, url);
  Decode->play(url, index);

  ui->btn_stop->setStyleSheet(
      "border-image:url(:/images/bottom/btn_pause_h.png);");
}

template <class T>
inline void Music::On_NetNextPlay(
    T rhs, const int index,
    const QString url) { //插入当前播放位置下一个位置，等待播放
  playlist.insert(CurrentPlayerListIndex + 1, url);
  Decode->tag->insertUrl(CurrentPlayerListIndex + 1,
                         rhs->GetPlaylistID().at(index));
}
