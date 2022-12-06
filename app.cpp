/*

Shows ASTC block partitions

Article from ARM here with 8x8 partitions http://community.arm.com/groups/arm-mali-graphics/blog/2013/10/22/astc-does-it

Their images are flipped on Y compared to mine!

14 sizes
3 partition modes each -> 42

*/

#include <QtGui>
#include <QMenuBar>
#include <QFontMetrics>
#include <QtGui>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <QComboBox>
#include <QMenuBar>
#include <QAction>
#include <QCheckBox>
#include <QGridLayout>
#include <QPushButton>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include "app.h"

#define TITLE "ASTC Partition Viewer v1.0"

ZASTC::ZASTC(QWidget* parent) : QMainWindow(parent)
{
	resize(700,700);
	mpPixmap = NULL;
	mpInfo = NULL;
	mnNumPartitions = 2;

	QStringList list;
	list << "4x4" << "5x4" << "5x5" << "6x5" << "6x6" << "8x5" << "8x6" << "10x5" << "10x6" << "8x8" << "10x8" << "10x10" << "12x10" << "12x12";
	mpBlockSize = new QComboBox;
	mpBlockSize->addItems(list);

	QStringList partList;
	partList << "2" << "3" << "4";
	mpPartitions = new QComboBox;
	mpPartitions->addItems(partList);

	mpDupes = new QCheckBox;
	mpDupes->setChecked(true);
	mpDupeStats = new QLabel;
	QLabel* pDupeLabel = new QLabel;
	pDupeLabel->setText("Show Duplicates");

	QLabel* pCrossDupeLabel = new QLabel;
	pCrossDupeLabel->setText("Highlight Cross-Set Duplicates");
	mpCrossDupes = new QCheckBox;
	mpCrossDupes->setChecked(true);

	mpImageLabel = new QLabel;

	QPushButton* pSave = new QPushButton("Save...");

	mpScrollArea = new QScrollArea;
	mpScrollArea->setBackgroundRole(QPalette::Dark);
	mpScrollArea->setWidget(mpImageLabel);
	mpScrollArea->setWidgetResizable(true);
	mpScrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	QWidget* pWidget = new QWidget;
	QGridLayout* pMainLayout = new QGridLayout;
	pMainLayout->addWidget(mpBlockSize, 0,0);
	pMainLayout->addWidget(mpPartitions, 0,1);
	pMainLayout->addWidget(mpDupeStats, 0,2, Qt::AlignLeft);
	pMainLayout->addWidget(pSave, 0,5);
	
	QGridLayout* pLayout = new QGridLayout;
	pLayout->addWidget(mpDupes, 0,0);
	pLayout->addWidget(pDupeLabel, 0,1);
	pLayout->addWidget(mpCrossDupes, 0,3);
	pLayout->addWidget(pCrossDupeLabel, 0,4);
	pLayout->setColumnStretch(2, 5);
	pLayout->setColumnStretch(5, 80);

	pMainLayout->addLayout(pLayout, 1, 0, 1, 5);
	
	pMainLayout->addWidget(mpScrollArea, 2,0,1,6);
	pMainLayout->setColumnStretch(4, 80);
	pWidget->setLayout(pMainLayout);

//	BlockSize(0);

	connect(mpBlockSize, SIGNAL(activated(int)), this, SLOT(BlockSize(int)));
	connect(mpPartitions, SIGNAL(activated(int)), this, SLOT(Partitions(int)));
	connect(mpDupes, SIGNAL(stateChanged(int)), this, SLOT(Dupes(int)));
	connect(mpCrossDupes, SIGNAL(stateChanged(int)), this, SLOT(Dupes(int)));
	connect(pSave, SIGNAL(clicked()), this, SLOT(Save()));

	setCentralWidget(pWidget);
	setWindowTitle("ASTC Partition Viewer");

	///

	mActionAbout = new QAction("&About...", this);
	mActionAbout->setMenuRole(QAction::AboutRole);
	//	mActionAbout->setShortcut(tr("F1"));
	connect(mActionAbout, SIGNAL(triggered()), this, SLOT(about()));

	mActionAboutQt = new QAction("About &Qt...", this);
	mActionAboutQt->setMenuRole(QAction::AboutQtRole);
	connect(mActionAboutQt, SIGNAL(triggered()), this, SLOT(aboutQt()));

	QMenuBar* pBar = menuBar();
	QMenu* pHelp = pBar->addMenu("&Help");
	pHelp->addAction(mActionAbout);
	pHelp->addAction(mActionAboutQt);
}

ZASTC::~ZASTC()
{
	if (mpInfo)
	{
		delete mpInfo;
	}
}

void ZASTC::about()
{
	QMessageBox::about(this, TITLE, "<p>This program shows ASTC partitions with some stats... Twitter: @jonrocatis</p>"
		"<p>There is an article from ARM with some more info <a href='http://community.arm.com/groups/arm-mali-graphics/blog/2013/10/22/astc-does-it'>here</a></p>"
		"<p>The partitioning algorithm is described in some more detail <a href='http://www.cs.cmu.edu/afs/cs/academic/class/15869-f11/www/readings/nystad12_astc.pdf'>here</a></p>");
}

void ZASTC::aboutQt()
{
	QMessageBox::aboutQt(this, TITLE);
}


void ZASTC::BlockSize(int size)
{
	if (mpPixmap)
	{
		delete mpPixmap;
		mpPixmap = NULL;
	}

	if (mpInfo)
	{
		delete mpInfo;
	}

	const int sizes[14][2] = {{4,4}, {5,4}, {5,5}, {6,5}, {6,6}, {8,5}, {8,6}, {10,5}, {10,6}, {8,8}, {10,8}, {10,10}, {12,10}, {12,12}};
#if 1
	const int psizes[14] = {8,8,7,7,7,6,6,5,5,5,5,5,4,4};
#else
	int psizes[14];
	for (int i = 0; i < 14; i++) psizes[i] = 2;
#endif

	const ZPartitionInfo* apExtra[2] = {nullptr, nullptr};
	for (int iExtra = 0; iExtra < (mnNumPartitions - 2); iExtra++)
	{
		apExtra[iExtra] = new ZPartitionInfo(sizes[size][0], sizes[size][1], iExtra + 2);
	}

	mpInfo = new ZPartitionInfo(sizes[size][0], sizes[size][1], mnNumPartitions, apExtra[0], apExtra[1]);
	const int hideDupes = !mpDupes->isChecked();
	const int showCrossDupes = mpCrossDupes->isChecked();

	mpPixmap = mpInfo->CreatePixmap(mpScrollArea->width(), psizes[size], hideDupes, showCrossDupes);
	mpImageLabel->setPixmap(*mpPixmap);
	
	QString sPartition = QString("Partition Usage: %1 %2 %3 %4 (%5% %6% %7% %8%)").arg(mpInfo->manPartitionUsage[0]).arg(mpInfo->manPartitionUsage[1]).arg(mpInfo->manPartitionUsage[2]).arg(mpInfo->manPartitionUsage[3]).arg(mpInfo->manPartitionUsage[0] * 100.0f / 1024.0f, 0, 'f', 1).arg(mpInfo->manPartitionUsage[1] * 100.0f / 1024.0f, 0, 'f', 1).arg(mpInfo->manPartitionUsage[2] * 100.0f / 1024.0f, 0, 'f', 1).arg(mpInfo->manPartitionUsage[3] * 100.0f / 1024.0f, 0, 'f', 1);
	
	float bpp = 128.0 / (sizes[size][0] * sizes[size][1]);

	int nTotalDupes = mpInfo->mnNumDupes + mpInfo->mnNumCrossDupes;
	QString sPatterns = QString("%1/1024 (%2%). Unique: %3. %4. bpp: %5. Cross-Set Dupes: %6").arg(nTotalDupes).arg(nTotalDupes * 100 / 1024.0, 0, 'f', 1).arg(1024 - nTotalDupes).arg(sPartition).arg(bpp, 0, 'f', 2).arg(mpInfo->mnNumCrossDupes);
	mpDupeStats->setText(sPatterns);
	
#if 0
	qDebug() << sizes[size][0] << "x" << sizes[size][1] << "P:" << mnNumPartitions;
	qDebug() << sPartition;
	qDebug() << sPatterns;
	qDebug() << "";
#endif

	delete apExtra[0];
	delete apExtra[1];

	update();
}

#if REALTIME_UPDATE
void ZASTC::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
	BlockSize(mpBlockSize->currentIndex());
}
#endif


void ZASTC::Partitions(int idx)
{
	mnNumPartitions = idx + 2;
	BlockSize(mpBlockSize->currentIndex());
}


void ZASTC::Dupes(int show)
{
	Q_UNUSED(show);
	BlockSize(mpBlockSize->currentIndex());
}

void ZASTC::Save()
{
	QString filename = QFileDialog::getSaveFileName(this, "Save Partition", QString(), "Images (*.png)");
	if (!filename.isEmpty())
	{
		mpPixmap->save(filename, "PNG");
	}
}

