#if _MSC_VER >= 1600 // MSVC2015>1899,对于MSVC2010以上版本都可以使用
#pragma execution_character_set("utf-8")
#endif
#include "login.h"
#include "config.h"
#include "signup.h"
#include "ui_login.h"
#include <QDateTime>
#include <QEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QListWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QRegExpValidator>
#include <QTimer>
Login::Login(QWidget* parent) : QDialog(parent), ui(new Ui::Login) {
	ui->setupUi(this);
	ui->stackedWidget->setCurrentIndex(0);
	this->setWindowTitle("登录");
	config = new Config();
	signup = new Signup(this);
	time = new QTimer(this);
	request_time = new QTimer(this);
	countrieslist = new QListWidget(this);
	countrieslist->installEventFilter(this);
	countrieslist->hide();
	signup->hide();
	//处于输入状态的时候，是正常显示字符。 输入完毕之后,使用Password形式隐藏字符
	ui->line_word->setEchoMode(QLineEdit::PasswordEchoOnEdit);

	manger = new QNetworkAccessManager(this);

	NetRequest = new QNetworkRequest{};

	connect(signup, &Signup::back, this, [&]() {
		signup->hide();
		this->show();
		});

	connect(countrieslist, &QListWidget::itemClicked, this,
		&Login::countrieslist_itemClicked);

	connect(signup, &Signup::singin, this, &Login::show);

	connect(time, &QTimer::timeout, this, &Login::on_QRCexpired);

	AutoLogin();
}

Login::~Login() {
	delete ui;
	delete config;
	config = nullptr;
	delete NetRequest;
	NetRequest = nullptr;
}

bool Login::eventFilter(QObject* obj, QEvent* event) {
	if (obj == countrieslist) {
		if (event->type() == QEvent::Leave) {
			countrieslist->close();
		}
	}
	// return Login::eventFilter(obj, event);
	return false;
}

//解析json文件
bool Login::ParseJson(QJsonObject& rootobj) {
	//获取用户信息
	QJsonValue profileobj = rootobj.value("profile");
	//判断这个profile对象是否是object
	if (profileobj.isObject()) {
		//将其转换成object
		auto profile = profileobj.toObject();
		//将用户信息写入配置文件
		QString key{ "Userinfo" };
		config->SetBeginGroup(key);
		config->SetValue("userId", QVariant(profile.value("userId").toInt()));
		config->SetValue("nickname",
			QVariant(profile.value("nickname").toString()));
		config->SetValue("avatarUrl",
			QVariant(profile.value("avatarUrl").toString()));
		config->SetValue("backgroundUrl",
			QVariant(profile.value("backgroundUrl").toString()));
		config->SetValue("followeds", QVariant(profile.value("followeds").toInt()));
		config->SetValue("follows", QVariant(profile.value("follows").toInt()));
		config->SetValue("playlistCount",
			QVariant(profile.value("playlistCount").toInt()));
		config->endGroup();

		NetUserPic = manger->get(QNetworkRequest(profile.value("avatarUrl").toString()));
		connect(NetUserPic, &QNetworkReply::finished, this, &Login::on_FinshedPic);
		return true;
	}
	return false;
}

void Login::AutoLogin() {
	//自动登录，默认打勾
	ui->checkBox->setCheckState(Qt::Checked);
	if (config->GetValue("/Pwd/checkState") == "1") {
		phone = config->GetValue("/Pwd/phone");
		pwd = config->GetValue("/Pwd/pwd");
		ui->line_phoneID->setText(phone);
		ui->line_word->setText(pwd);
		if (phone.isEmpty() && pwd.isEmpty()) {
			return;
		}
		NetManager = manger->get(QNetworkRequest(
			QString(
				"http://localhost:3000/login/cellphone?phone=%1&password=%2")
			.arg(phone)
			.arg(pwd)));
		connect(NetManager, &QNetworkReply::finished, this, &Login::on_replyFinished);

		emit LoginSucces();
		fprintf(stdout, "***login succeed\n***");
	}
}


void Login::Grade() {
	QString url{ "http://localhost:3000/user/level" };
	QNetworkRequest* request = config->setCookies();
	request->setUrl(url);
	Netgrade = manger->get(*request);
	connect(Netgrade, &QNetworkReply::finished, this,
		&Login::on_finshedGrade);
}

void Login::Vip() {
	config->GetCookies();
	QString url{ "http://localhost:3000/vip/info" };
	QNetworkRequest* requests = config->setCookies();
	requests->setUrl(url);
	NetVip = manger->get(*requests);
	connect(NetVip, &QNetworkReply::finished, this,
		&Login::on_finshedVip);
}

//登陆
void Login::on_btn_signin_clicked() {
	//验证手机号码 1,2用于区分是密码错误还是手机号码错误
	static QRegExp regx_phone(
		"^1([358][0-9]|4[579]|66|7[0135678]|9[89])[0-9]{8}$");
	if (false == checkInput(ui->line_phoneID->text(), regx_phone, 1)) {
		return;
	}
	//验证密码 8-16位且必须为数字、大小写字母或符号中至少2种
	static QRegExp regx_pwd{
		"^(?![\\d]+$)(?![a-zA-Z]+$)(?![^\\da-zA-Z]+$).{8,16}$" };
	if (false == checkInput(ui->line_word->text(), regx_pwd, 2)) {
		return;
	}

	QString phone = ui->line_phoneID->text();
	QString psw = ui->line_word->text();
	//判断一下网络状态， 如果为NotAccessible重新设置一下
	//if (NetManager->networkAccessible() == QNetworkAccessManager::NotAccessible) {
	//	NetManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
	//}

	QString countrycode =
		ui->btn_countrieslist->text().split(QRegExp("[+]")).at(1);
	//以get方式发送一个网络请求
	/*
	 * phone 手机号
	 * countrycode 国家码
	 * password 密码
	 */
	NetManager = manger->get(
		QNetworkRequest(QString("http://localhost:3000/login/"
			"cellphone?phone=%1&countrycode=%2&password=%3")
			.arg(phone)
			.arg(countrycode)
			.arg(psw)));
	connect(NetManager, &QNetworkReply::finished, this, &Login::on_replyFinished);
}
//注册
void Login::on_btn_signup_clicked() {
	this->hide();
	signup->show();
}

void Login::on_btn_Qrcode_clicked()
{
	//跳转到扫码页面
	ui->stackedWidget->setCurrentIndex(1);
}

void Login::countrieslist_itemClicked(QListWidgetItem* item) {
	//截取右边5个字符
	QStringList str = item->data(0).toString().split(QRegExp("[+]"));
	ui->btn_countrieslist->setText("+" + str.at(1));
}

void Login::on_btn_countrieslist_clicked() {
	Config cfg("../config/countrieslist.ini");
	int index = cfg.SetBeginReadAry("countrieslist");
	if (index > 0) {
		for (int i = 0; i != index; ++i) {
			config->SetAryIndex(i);
			QString zh = cfg.GetValue("en");
			QString code = cfg.GetValue("code");
			qDebug() << zh << ' ' << code << '\n';
			countrieslist->addItem(
				new QListWidgetItem(QString("%1\t\t+%2").arg(zh).arg(code)));
		}
		cfg.endArray();

		countrieslist->resize(260, 220);
		countrieslist->move(35, 220);
		countrieslist->show();
	}
	else {
		//说明 : 调用此接口, 可获取国家编码列表
		QString url{ "http://localhost:3000/countries/code/list" };
		NetCountries = manger->get(QNetworkRequest(url));

		connect(NetCountries, &QNetworkReply::finished, this,
			&Login::on_finshedCountriesList);
	}
}

void Login::on_btn_backlogin_clicked() {
	time->stop();
	ui->stackedWidget->setCurrentIndex(0);
}



void Login::on_replyFinished() {
	if (NetManager->error() == QNetworkReply::NoError) {
		QString all = NetManager->readAll();
		QByteArray byte_array{};
		byte_array.append(all);
		QJsonParseError err_rpt{};
		// QJsonDocument封装了完整的json文档
		QJsonDocument document = QJsonDocument::fromJson(byte_array, &err_rpt);
		if (err_rpt.error == QJsonParseError::NoError) {
			QJsonObject rootobj = document.object();
			QString mes = rootobj.value("msg").toString();
			if (mes == "密码错误") {
				fprintf(stdout, mes.toStdString().data());
				ui->lab_error->setText(mes);
				return;
			}
			//获取等级
			Grade();
			//获取vip信息
			Vip();
			//获取登录信息
			for (auto it = rootobj.begin(); it != rootobj.end(); ++it) {
				if (ParseJson(rootobj)) {
					//获取请求头
					if (NetManager->hasRawHeader("Set-Cookie")) {
						QByteArray cookie = NetManager->rawHeader("Set-Cookie");
						//将cookie写入配置文件
						config->SetBeginGroup("cookie");
						config->SetValue("cookie", QVariant(cookie));
						config->endGroup();
					}
				}
			}

			//记录登录密码
			config->SetBeginGroup("Pwd");
			if (ui->checkBox->checkState() == Qt::Checked) {
				phone = ui->line_phoneID->text();
				pwd = ui->line_word->text();
				config->SetValue("phone", QVariant(phone));
				config->SetValue("pwd", QVariant(pwd));
				config->SetValue("checkState", QVariant(1));
				config->SetValue("loggingstatus", QVariant(1));
			}
			else {
				config->SetValue("checkState", QVariant(0));
			}
			config->endGroup();

			this->close();

			emit LoginSucces();
		}
		else {
			Error(tr("Json format error"));
			return;
		}
	}
}

void Login::on_FinshedPic() {
	if (NetUserPic->error() == QNetworkReply::NoError) {
		pix.loadFromData(NetUserPic->readAll());
		pix.save("../Userpix/user.png");
	}
}

bool Login::checkInput(QString str, const QRegExp regx, const int n) {
	QRegExpValidator pReg{ regx, 0 };
	int pos = 0;
	QValidator::State res = pReg.validate(str, pos);
	switch (n) {
	case 1:
		//输入的格式不对
		if (!(QValidator::Acceptable == res)) {
			ui->lab_error->setText("请输入11位手机号码");
			ui->line_phoneID->clear();
			ui->line_word->clear();
			return false;
		}
		break;
	case 2:
		if (!(QValidator::Acceptable == res)) {
			ui->lab_error->setText("8-16位数字、大小写字母或符号中至少2种");
			ui->line_word->clear();
			return false;
		}
		break;
	}
	return true;
}

void Login::on_finshedGrade() {
	if (Netgrade->error() == QNetworkReply::NoError) {
		QJsonParseError errt_t{};
		QJsonDocument document = QJsonDocument::fromJson(Netgrade->readAll(), &errt_t);
		if (errt_t.error == QJsonParseError::NoError) {
			QJsonObject root = document.object();
			auto data = root.value("data");
			if (data.isObject()) {
				QJsonObject datarot = data.toObject();
				level = datarot.value("level").toInt();
				config->SetBeginGroup("Userinfo");
				config->SetValue("level", QVariant(level));
				config->endGroup();
			}
		}
	}
}

void Login::on_finshedVip() {
	if (NetVip->error() == QNetworkReply::NoError) {
		QJsonParseError errt_t{};
		QJsonDocument document = QJsonDocument::fromJson(NetVip->readAll(), &errt_t);
		config->SetBeginGroup("Userinfo");
		if (errt_t.error == QJsonParseError::NoError) {
			QJsonObject root = document.object();
			QJsonValue data = root.value("data");
			if (data.isObject()) {
				QJsonObject musicrot = data.toObject();
				QJsonValue musicpack = musicrot.value("musicPackage");
				if (musicpack.isObject()) {
					QJsonObject packrot = musicpack.toObject();
					long long expiretime =
						packrot.value("expireTime").toVariant().toLongLong();
					QDateTime data = QDateTime::fromMSecsSinceEpoch(expiretime);
					QString time = data.toString("yyyy-MM-dd").toUtf8();
					qDebug() << time << "\n";
					config->SetValue("vip", QVariant(time));
				}
			}
		}
		else {
			config->SetValue("vip", QVariant("No VIP"));
		}
		config->endGroup();
	}
}


//获取国家编码列表
void Login::on_finshedCountriesList() {
	if (NetCountries->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument docment = QJsonDocument::fromJson(NetCountries->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject root = docment.object();
			QJsonValue data = root.value("data");
			if (data.isArray()) {
				QJsonArray dataAry = data.toArray();
				Config cfg("../config/countrieslist.ini");
				cfg.SetBeginArray("countrieslist");
				int index = 0;
				foreach(const QJsonValue & rhs, dataAry) {
					if (rhs.isObject()) {
						QJsonObject rot = rhs.toObject();
						QString lab = rot.value("label").toString();
						QJsonValue countryList = rot.value("countryList");
						if (countryList.isArray()) {
							QJsonArray countryListArt = countryList.toArray();
							foreach(const QJsonValue & lhs, countryListArt) {
								//只有一个对象，不需要用到循环
								QJsonObject lhsrot = lhs.toObject();
								QString zh = lhsrot.value("zh").toString().toUtf8();
								QString en = lhsrot.value("en").toString();
								QString locale = lhsrot.value("locale").toString();
								QString code = lhsrot.value("code").toString();
								cfg.SetAryIndex(index);
								cfg.SetValue("label", QVariant(lab));
								cfg.SetValue("zh", QVariant(zh));
								cfg.SetValue("en", QVariant(en));
								cfg.SetValue("locale", QVariant(locale));
								cfg.SetValue("code", QVariant(code));
								++index;
							}
						}
					}
				}
				cfg.endArray();
			}
		}
		else {
			fprintf(stdout, "Json format error\n");
		}
	}
}

/*
 * 二维码登录
 * key 请求对象
 * time  时间戳
 */
void Login::on_btn_QRC_clicked() {
	//获取当前时间
	// QDateTime _time = QDateTime::currentDateTime();
	//将当前时间转为时间戳
	// uint timeT = _time.toTime_t();
	qint64 _time = QDateTime::currentMSecsSinceEpoch();
	QString Qrc_Key{
		QString("http://localhost:3000/login/qr/key?time=%1").arg(_time) };
	NetQRC_key = manger->get(QNetworkRequest(Qrc_Key));

	connect(NetQRC_key, &QNetworkReply::finished, this,
		&Login::on_finshedQRC_key);
	ui->stackedWidget->setCurrentIndex(1);
}

void Login::on_finshedQRC_key() {
	if (NetQRC_key->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument docment = QJsonDocument::fromJson(NetQRC_key->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject rot = docment.object();
			QJsonValue data = rot.value("data");
			if (data.isObject()) {
				QJsonObject datarot = data.toObject();
				QRC_KEY = datarot.value("unikey").toString();
				qDebug() << "Key = " << QRC_KEY << "\n";
				//创建二维码
				QString QRC_CreateUrl{
					QString(
						"http://localhost:3000/login/qr/create?key=%1&qrimg=true")
						.arg(QRC_KEY) };
				NetQRC_create = manger->get(QNetworkRequest(QRC_CreateUrl));
				connect(NetQRC_create, &QNetworkReply::finished, this,
					&Login::on_finshedQRC_create);
			}
		}
		else {
			fprintf(stdout, "json解析失败\n");
		}
	}
}

//获取二维码
void Login::on_finshedQRC_create() {
	if (NetQRC_create->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument docment = QJsonDocument::fromJson(NetQRC_create->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject rot = docment.object();
			QJsonValue data = rot.value("data");
			if (data.isObject()) {
				QJsonObject datarot = data.toObject();
				//网页登录
				// QString imgUrl = datarot.value("qrurl").toString();
				QString byt = datarot.value("qrimg").toVariant().toByteArray();
				QPixmap pix;
				// Base64 转成图片
				pix.loadFromData(
					QByteArray::fromBase64(byt.section(",", 1).toLocal8Bit()));
				ui->lab_QRC->setPixmap(pix);
				//保存到本地
				// pix.save("../Userpix/QRC.png");
				//开始计时，一分钟有效时间
				time->start(1000);
				//QString checkUrl{
				//    QString("http://localhost:3000/login/qr/check?key=%1")
				//        .arg(QRC_KEY)};
				//NetQRC_check->get(QNetworkRequest(checkUrl));
			}
		}
		else {
			fprintf(stdout, "json解析失败\n");
		}
	}
}

/*
 * 检测是否扫码,轮询此接口可获取二维码扫码状态
 * 800 为二维码过期
 * 801 为等待扫码
 * 802 为待确认
 * 803 为授权登录成功(803 状态码下会返回 cookies)
 */

void Login::on_finshedQRC_check() {
	if (NetQRC_check->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument docment = QJsonDocument::fromJson(NetQRC_check->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject rot = docment.object();
			qDebug() << rot << '\n';
			int code = rot.take("code").toVariant().toInt();
			qDebug() << "code =" << code << "\n";
			// 800 为二维码过期
			if (800 == code) {
				time->stop();
				int rect = QMessageBox::information(this, "tip",
					"二维码不存在或已过期,是否刷新",
					QMessageBox::Yes, QMessageBox::No);
				if (rect == QMessageBox::Yes) {
					//从新请求登录二维码
					QDateTime _time = QDateTime::currentDateTime();
					QString Qrc_Key{
						QString("http://localhost:3000/login/qr/key?time=%1")
							.arg(_time.toTime_t()) };
					NetQRC_key = manger->get(QNetworkRequest(Qrc_Key));
					return;
				}
			}

			if (801 == code) {
				// fprintf(stdout, "Waiting for sweeping code\n");

				return;
			}

			if (802 == code) {
				// fprintf(stdout, "扫码待确认\n");

				return;
			}

			if (803 == code) {
				this->close();
				//拿到cookies
				QString cookie = rot.value("cookie").toString();
				//将cookie放入配置文件
				config->SetBeginGroup("cookie");
				config->SetValue("cookie", QVariant(cookie));
				config->endGroup();

				//获取账号信息,带上cookie
				NetRequest->setUrl(QUrl("http://localhost:3000/user/account?cookie=" + cookie));
				NetUserMsg = manger->get(*NetRequest);
				connect(NetUserMsg, &QNetworkReply::finished, this,
					&Login::on_finshedUserMsg);
				time->stop();
				fprintf(stdout, "login ok\n");
				ui->stackedWidget->setCurrentIndex(0);

			}
		}
		else {
			fprintf(stdout, "json解析失败\n");
		}
	}
}


void Login::on_pushButton_clicked() {
	QString checkUrl{ QString("http://localhost:3000/login/qr/check?key=%1")
						 .arg(QRC_KEY) };
	NetQRC_check = manger->get(QNetworkRequest(checkUrl));

	connect(NetQRC_check, &QNetworkReply::finished, this,
		&Login::on_finshedQRC_check);
}

void Login::on_QRCexpired() {
	static int n = 0;
	if (60 == n) {
		n = 0;
		time->stop();
		int rect =
			QMessageBox::warning(this, "warning", "二维码已过期,是否刷新二维码",
				QMessageBox::Yes, QMessageBox::No);
		if (rect == QMessageBox::Yes) {
			//从新请求登录二维码
			//将当前时间转为时间戳
			// uint timeT = _time.toTime_t();
			//获取当前时间
			qint64 time = QDateTime::currentMSecsSinceEpoch();
			QString Qrc_Key{
				QString("http://localhost:3000/login/qr/key?time=%1")
					.arg(time) };
			NetQRC_key = manger->get(QNetworkRequest(Qrc_Key));
		}
		else {
			//不处理
		}
	}
	else {
		//还没有扫码，持续检测
		qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
		QString checkUrl{
			QString("http://localhost:3000/login/qr/check?key=%1&timerstamp=%2")
				.arg(QRC_KEY)
				.arg(timestamp) };
		qDebug() << timestamp << '\n';
		// 1643116209085  13位
		NetQRC_check = manger->get(QNetworkRequest(checkUrl));
		connect(NetQRC_check, &QNetworkReply::finished, this,
			&Login::on_finshedQRC_check);
		++n;
		fprintf(stdout, "coming cheach = %d\n", n);
	}
}


void Login::on_finshedUserMsg() {
	if (NetUserMsg->error() == QNetworkReply::NoError) {
		QByteArray byt{ NetUserMsg->readAll() };
		QJsonParseError err_t{};
		QJsonDocument deocument = QJsonDocument::fromJson(byt, &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject rot = deocument.object();
			//拿到用户ID
			int userId = rot.value("account").toObject().value("id").toInt();
			GetUserInfo(userId);
		}
	}
}



void Login::GetUserInfo(const int& UserId)
{
	QString Url{ QString("http://localhost:3000/user/detail?uid=%1").arg( + UserId)};
	NetUserInfo = manger->get(QNetworkRequest(Url));
	connect(NetUserInfo, &QNetworkReply::finished, this,
		&Login::on_finshedNetUserInfo);
}

void Login::on_finshedNetUserInfo()
{
	if (NetUserInfo->error() == QNetworkReply::NoError) {
		QJsonParseError err_t{};
		QJsonDocument deocument = QJsonDocument::fromJson(NetUserInfo->readAll(), &err_t);
		if (err_t.error == QJsonParseError::NoError) {
			QJsonObject rot = deocument.object();
			ParseJson(rot);
			Vip();
			Grade();
			emit LoginSucces();
		}
	};
}
