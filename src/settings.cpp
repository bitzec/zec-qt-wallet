#include "precompiled.h"

#include "utils.h"
#include "settings.h"

Settings* Settings::instance = nullptr;

Settings::~Settings() {
	delete defaults;
	delete zcashconf;
	delete uisettings;
}

bool Settings::getSaveZtxs() {
	// Load from the QT Settings. 
	return QSettings().value("options/savesenttx", true).toBool();
}

void Settings::setSaveZtxs(bool save) {
	QSettings().setValue("options/savesenttx", save);
}

Settings* Settings::init() {	
	if (instance == nullptr) 
		instance = new Settings();

	// There are 3 possible configurations
	// 1. The defaults
	instance->defaults = new Config{ "127.0.0.1", "8232", "", "" };

	// 2. From the UI settings
	auto settingsFound = instance->loadFromSettings();

	// 3. From the zcash.conf file
	auto confFound = instance->loadFromFile();

	// zcash.conf (#3) is first priority if it exists
	if (confFound) {
		instance->currentConfig = instance->zcashconf;
	}
	else if (settingsFound) {
		instance->currentConfig = instance->uisettings;
	}
	else {
		instance->currentConfig = instance->defaults;
	}

	return instance;
}

Settings* Settings::getInstance() {
	return instance;
}


QString Settings::getHost() {
	return currentConfig->host;
}

QString Settings::getPort() {
	return currentConfig->port;
}


QString Settings::getUsernamePassword() {
    return currentConfig->rpcuser % ":" % currentConfig->rpcpassword;
}

bool Settings::loadFromSettings() {
	delete uisettings;

	// Load from the QT Settings. 
	QSettings s;
	
	auto host		= s.value("connection/host").toString();
	auto port		= s.value("connection/port").toString();
	auto username	= s.value("connection/rpcuser").toString();
	auto password	= s.value("connection/rpcpassword").toString();	

	uisettings = new Config{host, port, username, password};

    return !username.isEmpty();
}

void Settings::saveSettings(const QString& host, const QString& port, const QString& username, const QString& password) {
	QSettings s;

	s.setValue("connection/host", host);
	s.setValue("connection/port", port);
	s.setValue("connection/rpcuser", username);
	s.setValue("connection/rpcpassword", password);

	s.sync();

	// re-init to load correct settings
	init();
}

bool Settings::loadFromFile() {
	delete zcashconf;

#ifdef Q_OS_LINUX
	confLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, ".zcash/zcash.conf");
#elif defined(Q_OS_DARWIN)
	confLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, "/Library/Application Support/Zcash/zcash.conf");
#else
	confLocation = QStandardPaths::locate(QStandardPaths::AppDataLocation, "../../Zcash/zcash.conf");
#endif

	confLocation = QDir::cleanPath(confLocation);

	if (confLocation.isNull()) {
		// No zcash file, just return with nothing
		return false;
	}

	QFile file(confLocation);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << file.errorString();
		return false;
	}

	QTextStream in(&file);

	zcashconf = new Config();
	zcashconf->host = defaults->host;

	while (!in.atEnd()) {
		QString line = in.readLine();
		auto s = line.indexOf("=");
		QString name  = line.left(s).trimmed().toLower();
		QString value = line.right(line.length() - s - 1).trimmed();

		if (name == "rpcuser") {
			zcashconf->rpcuser = value;
		}
		if (name == "rpcpassword") {
			zcashconf->rpcpassword = value;
		}
		if (name == "rpcport") {
			zcashconf->port = value;
		}
		if (name == "testnet" &&
			value == "1"  &&
			zcashconf->port.isEmpty()) {
				zcashconf->port = "18232";
		}
	}

	// If rpcport is not in the file, and it was not set by the testnet=1 flag, then go to default
	if (zcashconf->port.isEmpty()) zcashconf->port = defaults->port;

	file.close();

	return true;
}

bool Settings::isTestnet() {
	return _isTestnet;
}

void Settings::setTestnet(bool isTestnet) {
	this->_isTestnet = isTestnet;
}

bool Settings::isSaplingAddress(QString addr) {
	return ( isTestnet() && addr.startsWith("ztestsapling")) ||
	       (!isTestnet() && addr.startsWith("zs"));
}

bool Settings::isSproutAddress(QString addr) {
	return isZAddress(addr) && !isSaplingAddress(addr);
}

bool Settings::isZAddress(QString addr) {
	return addr.startsWith("z");
}

bool Settings::isSyncing() {
	return _isSyncing;
}

void Settings::setSyncing(bool syncing) {
	this->_isSyncing = syncing;
}

int Settings::getBlockNumber() {
	return this->_blockNumber;
}

void Settings::setBlockNumber(int number) {
	this->_blockNumber = number;
}

bool Settings::isSaplingActive() {
	return  (isTestnet() && getBlockNumber() > 280000) ||
	       (!isTestnet() && getBlockNumber() > 419200);
}

double Settings::getZECPrice() { 
	return zecPrice; 
}

QString Settings::getUSDFormat(double bal) {
	if (!isTestnet() && getZECPrice() > 0) 
		return "$" + QLocale(QLocale::English).toString(bal * getZECPrice(), 'f', 2); 
	else 
		return QString();
}

QString Settings::getZECDisplayFormat(double bal) {
	return QString::number(bal, 'g', 8) % " " % Utils::getTokenName();
}

QString Settings::getZECUSDDisplayFormat(double bal) {
	auto usdFormat = getUSDFormat(bal);
	if (!usdFormat.isEmpty())
		return getZECDisplayFormat(bal) % " (" % getUSDFormat(bal) % ")";
	else
		return getZECDisplayFormat(bal);
}