#ifndef SEARCH_H
#define SEARCH_H

#include <QMap>
#include <QWidget>
class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class Base;
#define ERROR(string)                                                          \
  qDebug() << __FILE__ << ' ' << __LINE__ << ' ' << string << '\n';

namespace Ui {
class Search;
}

//保存解析json得到的数据
struct SearchResults {
  int mvid;
  int song_id;
  int singer_id;
  int album_id;
  int duration;
  QString song_name;
  QString singer_name;
  QString album;
  SearchResults()
      : song_id{}, singer_id{}, mvid{}, duration{}, song_name{}, album{},
        singer_name{}, album_id{} {};
  ~SearchResults(){};
};

//歌曲是否可以播放
struct StateOfSong {
  bool success;
  QString message;
  StateOfSong() : success{false}, message{} {};
};

//由于不止一次用于请求数据，所以用一个枚举类型来区分
enum class RequestType { songType, stateType, Song_Details, play_all };
class Search : public QWidget {
  Q_OBJECT

public:
  explicit Search(QWidget *parent = nullptr);
  virtual ~Search();
  int CurInex() { return curindex; }
  void InitPlayListTabWiget();
  void GetSearchText(QString &str);
  void Parsejson(QJsonObject &root);
  void InitTableHeader();
  void ParseSongState(QJsonObject &root);
  void NetWorkState(QNetworkReply *reply);
  void ParseSongDetails(QJsonObject &root);
  void getAlbumPic(const int n);
  void ParseAlbumPic(const int n);

  QList<SearchResults> getSearchResults() { return SearchResults; }
  QPixmap getAlbumArt() { return AlbumArt; };
private slots:
  void on_btn_playall_clicked();
  void on_replyFinished(QNetworkReply *);
  void on_table_playlist_cellDoubleClicked(int row, int column);
  void on_btn_downloadall_clicked();
  void on_parsepci(QNetworkReply *reply);
signals:
  void play(QString str);

private:
  Ui::Search *ui;
  int curindex{};
  QPixmap AlbumArt{};
  Base *base;
  QString picUrl{};
  SearchResults s_results{};
  StateOfSong state;
  QStringList playlistID{};
  QList<SearchResults> SearchResults{};
  QNetworkRequest *NetRequest;
  QNetworkAccessManager *NetManager;

  //区分每次请求的类型
  QMap<QNetworkReply *, RequestType> typeMap;
};

#endif // SEARCH_H
