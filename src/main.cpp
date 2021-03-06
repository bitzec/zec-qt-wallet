#include "mainwindow.h"
#include "settings.h"
#include "turnstile.h"

#include "precompiled.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);
    QIcon icon(":/icons/res/icon.ico");
    QApplication::setWindowIcon(icon);

    #ifdef Q_OS_LINUX
        QFontDatabase::addApplicationFont(":/fonts/res/Ubuntu-R.ttf");
        qApp->setFont(QFont("Ubuntu", 11, QFont::Normal, false));
    #endif

	std::srand(std::time(nullptr));

	QCoreApplication::setOrganizationName("zec-qt-wallet-org");
	QCoreApplication::setApplicationName("zec-qt-wallet");

    Settings::init();

    MainWindow w;
    w.setWindowTitle("zec-qt-wallet v" + QString(APP_VERSION));
    w.show();
    
    return QApplication::exec();
}
