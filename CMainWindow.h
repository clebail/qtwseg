//-----------------------------------------------------------------------------------------------
#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H
//-----------------------------------------------------------------------------------------------
#include <QMainWindow>
#include "CVase.h"
//-----------------------------------------------------------------------------------------------
class QDoubleSpinBox;
class QSpinBox;
class QLabel;
class QTableWidget;
class QTableWidgetItem;
class CProfileEditor;
class C3dView;
//-----------------------------------------------------------------------------------------------
class CMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CMainWindow(QWidget *parent = nullptr);
    ~CMainWindow();

private:
    CVase *vase;
    CProfileEditor *editor;
    C3dView *view3d;
    QTableWidget *table;
    QLabel *summary;
    QLabel *cursorLabel;
    QDoubleSpinBox *spinLayerHeight;
    QDoubleSpinBox *spinWall;
    QDoubleSpinBox *spinMargin;
    QDoubleSpinBox *spinBottom;
    QSpinBox *spinSegments;
    QString currentFile;

    void createMenus(void);
    QWidget * createParamsPanel(void);
    QDoubleSpinBox * createSpin(double min, double max, double value);
    void updateParamsWidgets(void);

private slots:
    void onParamsEdited(void);
    void onVaseChanged(void);
    void onTableItemChanged(QTableWidgetItem *item);
    void onNew(void);
    void onOpen(void);
    void onSave(void);
    void onSaveAs(void);
    void onExportCsv(void);
    void onExportPdf(void);
};
//-----------------------------------------------------------------------------------------------
#endif // CMAINWINDOW_H
//-----------------------------------------------------------------------------------------------
