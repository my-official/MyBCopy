//#include "StartPage.h"
//#include "WizardPages.h"
#include "qtgui_pch.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "Utils_qtgui.h"


int main(int argc, char *argv[])
{
	try
	{
		QStringList libraryPaths;
		libraryPaths.append(QString("."));

		QApplication::setLibraryPaths(libraryPaths);

		QApplication a(argc, argv);

		QCoreApplication::setOrganizationName("MyBCopy Project");
		//QCoreApplication::setOrganizationDomain(QString());
		QCoreApplication::setApplicationName("MyBCopy");
		a.setWindowIcon(QIcon(":/Ariil-Alphabet-Letter-M.ico"));

		QTranslator qtTranslator;
		qtTranslator.load("qt_" + QLocale::system().name(),
			QLibraryInfo::location(QLibraryInfo::TranslationsPath));
		a.installTranslator(&qtTranslator);

		MainWindow::RecreateAndShowMainWindow();
		
		return a.exec();
	}
	catch (...)
	{
		return -1;
	}
}

