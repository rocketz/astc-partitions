#include <QtGui>
#include <QApplication>
#include "app.h"

int main(int nArgc, char** ppzArgv) 
{
	QApplication app(nArgc, ppzArgv);
		
	QCoreApplication::setOrganizationName("Kimera");
	QCoreApplication::setOrganizationDomain("rocatis.dk");
	QCoreApplication::setApplicationName("ASTC");
		
	ZASTC window;
	
#if 0
	
	for (int p = 2; p <= 4; p++)
	{
		for (int s = 0; s < 14; s++)
		{
			window.mnNumPartitions = p;
			window.BlockSize(s);
		}
	}
	return 0;

#endif
	
	window.show();
	window.BlockSize(0);
	int nStatus = app.exec();
	return nStatus;
}

