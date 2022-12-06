#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "common.h"
#include <QMainWindow>
#include <QScrollArea>
#include <QPixmap>
#include <QComboBox>
#include <QCheckBox>

class QLabel;
class QAction;

#define REALTIME_UPDATE 0

class ZPartitionInfo
{
  public:
  	ZPartitionInfo(int w, int h, int partitions, const ZPartitionInfo* pExtra0 = nullptr, const ZPartitionInfo* pExtra1 = nullptr);
   ~ZPartitionInfo();
	
	static const int patterns = 1024;

	struct SSimilar
	{
		void Init()
		{
			set = -1;
			pattern = -1;
		}
		int set;
		int pattern;
	};

	void CalcPatterns();

	QPixmap*	CreatePixmap(uint nGUIWidth, uint size, int hideDupes, int showCrossDupes);
	int			CheckUnique(int pattern);

	ZPartitionInfo::SSimilar CheckCrossDupes(int pattern, const ZPartitionInfo* pExtra0, const ZPartitionInfo* pExtra1) const;
	bool Test(const uint8* curblock, uint& idx) const;

	int NumUnique() const { return 1024 - mnNumDupes; }
	
	int mnWidth;
	int mnHeight;
	int mnNumPartitions;
	uint8* mpPartitions;
	uint8 maNumActualPartitions[1024];
	int maDupes[1024];								// Refers to the unique id. -1 if unique
	SSimilar maSimilar[1024];
	int mnNumDupes;
	int mnNumCrossDupes;
	int manPartitionUsage[4];
};


class ZASTC : public QMainWindow
{
	Q_OBJECT

  public:
	ZASTC(QWidget* parent = NULL);
	virtual ~ZASTC();

#if REALTIME_UPDATE
	void resizeEvent(QResizeEvent *event);
#endif
	
	void CreatePixmap(uint size, int hideDupes);

  public slots:
  	void BlockSize(int size);
  	void Partitions(int size);
  	void Dupes(int show);
  	void Save();
	void about();
	void aboutQt();

  public:
	ZPartitionInfo* mpInfo;
	QScrollArea* mpScrollArea;
	QPixmap* mpPixmap;
	QLabel* mpImageLabel;
	QComboBox* mpBlockSize;
	QComboBox* mpPartitions;
	QCheckBox* mpDupes;
	QCheckBox* mpCrossDupes;
	QLabel* mpDupeStats;
	int mnNumPartitions;

	QAction* mActionAbout;
	QAction* mActionAboutQt;
};

#endif
