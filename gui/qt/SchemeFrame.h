#pragma once

namespace Ui {
class SchemeFrame;
}

class SchemeFrame : public QWidget
{
    Q_OBJECT
public:
    explicit SchemeFrame(QWidget *parent = 0);
    ~SchemeFrame();

	void Set(const Settings& settings);
	void GetResult(Settings& settings);
private slots:
    void on_DefaultTemporaryPathAction_triggered();
    void on_BrowseTemporaryDirButton_clicked();    

private:
    Ui::SchemeFrame *ui;
	QString m_DefaultTempPath;	
	
	void SetDefaults();
};

