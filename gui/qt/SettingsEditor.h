
namespace Ui {
class SettingsEditor;
}

class MainWindow;


class SettingsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsEditor(QWidget *parent = 0);
    ~SettingsEditor();
	
	void Set(const Settings& setting);
	Settings GetResult() const;
private slots:
	void on_LinkStoragePage_clicked();
	void on_LinkToolChainPage_clicked();
	void on_LinkSchemePage_clicked();
    void on_SettingsEditor_finished(int result);

private:
    Ui::SettingsEditor *ui;	

	Settings m_Settings;
};
