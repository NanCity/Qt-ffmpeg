#ifndef MUSIC_H
#define MUSIC_H
#include <QAudioFormat>
#include <QAudioOutput>
#include <QList>
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
class QLineEdit;
class PersonForm;
class AudioDeCode;
class DesktopLyrics;
class CloudMusic;
class QListWidgetItem;
class Local_and_Download;
class Dicovermusic;
class SoloAlbum;
class Singer;
class SingerDetails;
class SongMenu;
class QListWidget;
class Music : public QMainWindow {
	Q_OBJECT
private:
	int sec{};
	Ui::Music* ui;
	QLineEdit* edit;
	QString SongName{};
	unsigned int CurrVolume{};
	bool sliderSeeking = false;
	int CurrentPlayerListIndex{ 0 };
	QStringList playlist{};
	QMutex mutex{};
	Config* config;
	SingerDetails* singetdeatils;
	Base* base;
	Login* login;
	lyric* lyr{};
	skin* Skin;
	Search* search;
	//云盘
	CloudMusic* cloudMusic;
	//  DesktopLyrics *dest;
	//解码器
	AudioDeCode* Decode;
	//歌单
	SongMenu* songmenu;
	QTableWidget* tableWidget{};
	Dicovermusic* DicMusic;
	QPoint mLastMousePosition{};
	PersonForm* perform;
	Local_and_Download* localMusic;
	//QListWidget鼠标右键操作
	QMenu* menu;
	QAction* Play;
	QAction* NextPlay;
	QAction* Down;
	QAction* Editmenuinfo;
	QAction* DelSongMenu;

	//检查鼠标是否移动
	bool mMoving = false;
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
	int curCoding{ 0 };

public:
	explicit Music(QWidget* parent = nullptr);
	virtual ~Music();
	//窗口事件安装
	void InstallEventFilter();
	//初始化
	void init();
	//初始化菜单
	void initMenu(QListWidget *listwidget);
	//悬停提示
	void HoverTip();
	void PlayerMode();
	void CheckState();
	void Previous(QStringList& playerlist);
	void Next(QStringList& playerlist);
	void SetBottonInformation(M_Tag*);

	//各个模块的connect
	void AlbConnect();
	void DecodeConnect();
	void SearchConnect();
	void SocalMusicConnect();
	void CloudMusicConnect();
	void LocalMusicConnect();
	void PersonFormConnect();
	void RecommendedDailyConnect();
	void SingerDetailsConnect();
	void SongMenuConnect();

signals:
	void updateSongLrc(int sec);

protected:
	//重写键盘事件
	// void keyPressEvent(QKeyEvent *event);
	//重写鼠标操作,移动，按下，松开
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void Presskey(QKeyEvent* event);
	bool eventFilter(QObject* obj, QEvent* event);
	void closeEvent(QCloseEvent* event);
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent(QDropEvent* event);
	template <class T> void On_NetplayAll(T rhs);
	template <class T> void On_Netplay(T rhs, const int index);
	template <class T>
	void On_NetNextPlay(T rhs, const int index, const QString _id);
private slots:
	//返回首页
	void on_title_logo_clicked();
	void onSeekOk();
	void onDuration(int, int);
	void on_setPlatList(QStringList list);
	void setPlayMode(PlayMode& Mode);
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

	//本地与下载listwidget被单击
	void on_down_listWidgetClicked(QListWidgetItem* item);

	//展开/隐藏歌单列表
	void on_btn_SongMenu_clicked();
	//创建歌单
	void on_AddSongMenu_clicked();
	//展开/隐藏收藏歌单
	void on_btn_collectMenu_clicked();

	//用户歌单被点击
	void on_SongMenuList_itemClicked(QListWidgetItem* item);
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
};
#endif // MUSIC_H

template <class T> inline void Music::On_NetplayAll(T rhs) {
	playlist.clear();
	state = State::resume;
	CurrentPlayerListIndex = 0;
	playlist = rhs->GetPlayList();
	if (playlist.isEmpty()) {
		return;
	}
	ui->btn_stop->setStyleSheet(
		"border-image:url(:/images/bottom/btn_pause_h.png);");
	Decode->play(playlist.at(0));
}

template <class T> inline void Music::On_Netplay(T rhs, const int index) {
	state = State::resume;
	QString ID = rhs->GetPlayList().at(index);
	playlist.insert(CurrentPlayerListIndex + 1, ID);
	Decode->play(playlist.at(CurrentPlayerListIndex + 1));
	ui->btn_stop->setStyleSheet(
		"border-image:url(:/images/bottom/btn_pause_h.png);");
}

template <class T>
inline void Music::On_NetNextPlay(T rhs, const int index, const QString _id) {
	//插入当前播放位置下一个位置，等待播放
	state = State::resume;
	playlist.insert(CurrentPlayerListIndex + 1, _id);
}
