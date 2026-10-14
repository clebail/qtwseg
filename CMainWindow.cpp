//-----------------------------------------------------------------------------------------------
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QVBoxLayout>
#include "CMainWindow.h"
#include "CProfileEditor.h"
#include "C3dView.h"
#include "CExport.h"
#include "common.h"
//-----------------------------------------------------------------------------------------------
CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent) {
    vase = new CVase(this);
    editor = new CProfileEditor(vase);
    view3d = new C3dView(vase);

    table = new QTableWidget(0, CExport::COL_COUNT);
    table->setHorizontalHeaderLabels(CExport::headers());
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
    table->setToolTip(tr("Double-cliquer sur la colonne Segments pour changer le nombre de segments d'une couche"));

    QSplitter *views = new QSplitter(Qt::Horizontal);
    views->addWidget(editor);
    views->addWidget(view3d);

    QSplitter *right = new QSplitter(Qt::Vertical);
    right->addWidget(views);
    right->addWidget(table);
    right->setStretchFactor(0, 1);
    right->setSizes({620, 280});

    QSplitter *main = new QSplitter(Qt::Horizontal);
    main->addWidget(createParamsPanel());
    main->addWidget(right);
    main->setStretchFactor(1, 1);
    setCentralWidget(main);

    cursorLabel = new QLabel;
    statusBar()->addPermanentWidget(cursorLabel);
    statusBar()->showMessage(tr("Glisser un point pour modifier le profil, double-clic pour en ajouter, clic droit pour en supprimer"));

    createMenus();
    updateParamsWidgets();

    connect(editor, &CProfileEditor::cursorMoved, this, [this](const QPointF &w) {
        cursorLabel->setText(tr("r = %1 mm   y = %2 mm").arg(CExport::number(w.x()), CExport::number(w.y())));
    });
    // Différé : le tableau peut être reconstruit depuis son propre signal itemChanged
    connect(vase, &CVase::changed, this, &CMainWindow::onVaseChanged, Qt::QueuedConnection);
    connect(table, &QTableWidget::itemChanged, this, &CMainWindow::onTableItemChanged);

    setWindowTitle(tr("QtWSeg"));
    resize(1400, 900);
    onVaseChanged();
}
//-----------------------------------------------------------------------------------------------
CMainWindow::~CMainWindow() {
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::createMenus(void) {
    QMenu *file = menuBar()->addMenu(tr("&Fichier"));
    file->addAction(tr("&Nouveau"), QKeySequence::New, this, &CMainWindow::onNew);
    file->addAction(tr("&Ouvrir…"), QKeySequence::Open, this, &CMainWindow::onOpen);
    file->addAction(tr("&Enregistrer"), QKeySequence::Save, this, &CMainWindow::onSave);
    file->addAction(tr("Enregistrer &sous…"), QKeySequence::SaveAs, this, &CMainWindow::onSaveAs);
    file->addSeparator();
    file->addAction(tr("Exporter en &CSV…"), this, &CMainWindow::onExportCsv);
    file->addAction(tr("Exporter en &PDF…"), QKeySequence::Print, this, &CMainWindow::onExportPdf);
    file->addSeparator();
    file->addAction(tr("&Quitter"), QKeySequence::Quit, this, &QWidget::close);
}
//-----------------------------------------------------------------------------------------------
QDoubleSpinBox * CMainWindow::createSpin(double min, double max, double value) {
    QDoubleSpinBox *spin = new QDoubleSpinBox;
    spin->setRange(min, max);
    spin->setDecimals(1);
    spin->setSingleStep(0.5);
    spin->setSuffix(tr(" mm"));
    spin->setValue(value);
    connect(spin, &QDoubleSpinBox::valueChanged, this, &CMainWindow::onParamsEdited);
    return spin;
}
//-----------------------------------------------------------------------------------------------
QWidget * CMainWindow::createParamsPanel(void) {
    const SParams &p = vase->params();

    spinLayerHeight = createSpin(3.0, 100.0, p.layerHeight);
    spinLayerHeight->setToolTip(tr("Épaisseur des planches, donc hauteur de chaque couche"));
    spinWall = createSpin(1.0, 50.0, p.wallThickness);
    spinWall->setToolTip(tr("Épaisseur de la paroi une fois tournée"));
    spinMargin = createSpin(0.0, 20.0, p.turningMargin);
    spinMargin->setToolTip(tr("Matière ajoutée de chaque côté de la paroi pour le tournage"));
    spinBottom = createSpin(0.0, 100.0, p.bottomThickness);
    spinBottom->setToolTip(tr("Les couches qui touchent le fond sont des disques pleins"));

    spinSegments = new QSpinBox;
    spinSegments->setRange(MIN_SEGMENTS, MAX_SEGMENTS);
    spinSegments->setValue(p.defaultSegments);
    spinSegments->setToolTip(tr("Nombre de segments des couches sans réglage particulier"));
    connect(spinSegments, &QSpinBox::valueChanged, this, &CMainWindow::onParamsEdited);

    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Épaisseur de couche"), spinLayerHeight);
    form->addRow(tr("Épaisseur de paroi"), spinWall);
    form->addRow(tr("Surépaisseur de tournage"), spinMargin);
    form->addRow(tr("Épaisseur du fond"), spinBottom);
    form->addRow(tr("Segments par défaut"), spinSegments);

    summary = new QLabel;
    summary->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(form);
    layout->addSpacing(12);
    layout->addWidget(summary);
    layout->addStretch();

    QWidget *panel = new QWidget;
    panel->setLayout(layout);
    panel->setMinimumWidth(260);
    return panel;
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::updateParamsWidgets(void) {
    const SParams &p = vase->params();
    QSignalBlocker b1(spinLayerHeight), b2(spinWall), b3(spinMargin), b4(spinBottom), b5(spinSegments);

    spinLayerHeight->setValue(p.layerHeight);
    spinWall->setValue(p.wallThickness);
    spinMargin->setValue(p.turningMargin);
    spinBottom->setValue(p.bottomThickness);
    spinSegments->setValue(p.defaultSegments);
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::onParamsEdited(void) {
    SParams p;
    p.layerHeight = spinLayerHeight->value();
    p.wallThickness = spinWall->value();
    p.turningMargin = spinMargin->value();
    p.bottomThickness = spinBottom->value();
    p.defaultSegments = spinSegments->value();
    vase->setParams(p);
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::onVaseChanged(void) {
    const QVector<CCouche> &couches = vase->couches();
    QSignalBlocker blocker(table);

    table->setRowCount(couches.size());
    for(int r=0;r<couches.size();r++) {
        const CCouche &c = couches[r];
        QStringList cells = CExport::row(c);

        for(int col=0;col<CExport::COL_COUNT;col++) {
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            if(col == CExport::COL_SEGMENTS && !c.isDisc()) {
                item->setData(Qt::EditRole, c.nbSegments);
                item->setFlags(item->flags() | Qt::ItemIsEditable);
                if(c.nbSegments != vase->params().defaultSegments) {
                    QFont font = item->font();
                    font.setBold(true);
                    item->setFont(font);
                }
            } else {
                item->setText(cells.value(col));
            }
            table->setItem(r, col, item);
        }
    }

    double diamMax = 0.0;
    double lengthTotal = 0.0;
    for(const CCouche &c : couches) {
        diamMax = qMax(diamMax, 2.0 * c.rOut);
        lengthTotal += c.boardLength();
    }
    double height = couches.isEmpty() ? 0.0 : couches.last().y1 - couches.first().y0;

    summary->setText(tr("<b>Hauteur :</b> %1 mm<br><b>Ø max :</b> %2 mm<br><b>Couches :</b> %3<br>"
                        "<b>Longueur de planche cumulée :</b> %4 mm<br><i>(hors trait de scie)</i>")
                     .arg(CExport::number(height), CExport::number(diamMax))
                     .arg(couches.size())
                     .arg(CExport::number(lengthTotal, 0)));
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::onTableItemChanged(QTableWidgetItem *item) {
    if(item->column() != CExport::COL_SEGMENTS) {
        return;
    }

    int n = item->data(Qt::EditRole).toInt();
    if(n < MIN_SEGMENTS || n > MAX_SEGMENTS) {
        statusBar()->showMessage(tr("Le nombre de segments doit être entre %1 et %2").arg(MIN_SEGMENTS).arg(MAX_SEGMENTS), 4000);
        n = qBound(MIN_SEGMENTS, n, MAX_SEGMENTS);
    }
    vase->setLayerSegments(vase->couches().value(item->row()).index, n);
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::onNew(void) {
    vase->reset();
    currentFile.clear();
    updateParamsWidgets();
    setWindowFilePath(QString());
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::onOpen(void) {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Ouvrir un profil"), QString(), tr("Profil de vase (*.qws.json *.json)"));
    if(fileName.isEmpty()) {
        return;
    }

    QString error;
    if(!vase->load(fileName, &error)) {
        QMessageBox::warning(this, tr("Ouvrir"), tr("Impossible d'ouvrir %1 :\n%2").arg(fileName, error));
        return;
    }
    currentFile = fileName;
    updateParamsWidgets();
    setWindowFilePath(fileName);
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::onSave(void) {
    if(currentFile.isEmpty()) {
        onSaveAs();
        return;
    }

    QString error;
    if(!vase->save(currentFile, &error)) {
        QMessageBox::warning(this, tr("Enregistrer"), tr("Impossible d'enregistrer %1 :\n%2").arg(currentFile, error));
        return;
    }
    statusBar()->showMessage(tr("Profil enregistré : %1").arg(currentFile), 4000);
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::onSaveAs(void) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Enregistrer le profil"), currentFile.isEmpty() ? tr("vase.qws.json") : currentFile, tr("Profil de vase (*.qws.json)"));
    if(fileName.isEmpty()) {
        return;
    }
    currentFile = fileName;
    setWindowFilePath(fileName);
    onSave();
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::onExportCsv(void) {
    QString base = currentFile.isEmpty() ? tr("vase") : QFileInfo(currentFile).completeBaseName().section('.', 0, 0);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en CSV"), base + ".csv", tr("CSV (*.csv)"));
    if(fileName.isEmpty()) {
        return;
    }

    QString error;
    if(!CExport::toCsv(*vase, fileName, &error)) {
        QMessageBox::warning(this, tr("Exporter en CSV"), tr("Impossible d'écrire %1 :\n%2").arg(fileName, error));
        return;
    }
    statusBar()->showMessage(tr("CSV exporté : %1").arg(fileName), 4000);
}
//-----------------------------------------------------------------------------------------------
void CMainWindow::onExportPdf(void) {
    QString base = currentFile.isEmpty() ? tr("vase") : QFileInfo(currentFile).completeBaseName().section('.', 0, 0);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en PDF"), base + ".pdf", tr("PDF (*.pdf)"));
    if(fileName.isEmpty()) {
        return;
    }

    QString error;
    if(!CExport::toPdf(*vase, fileName, &error)) {
        QMessageBox::warning(this, tr("Exporter en PDF"), tr("Impossible d'écrire %1 :\n%2").arg(fileName, error));
        return;
    }
    statusBar()->showMessage(tr("PDF exporté : %1").arg(fileName), 4000);
}
//-----------------------------------------------------------------------------------------------
