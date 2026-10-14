//-----------------------------------------------------------------------------------------------
#include <QCoreApplication>
#include <QDate>
#include <QFile>
#include <QLocale>
#include <QPainter>
#include <QPdfWriter>
#include <QTextStream>
#include "CExport.h"
#include "CProfileEditor.h"
//-----------------------------------------------------------------------------------------------
#define PDF_RESOLUTION          300
#define PDF_MARGIN_MM           12.0
#define PDF_PROFILE_HEIGHT_MM   125.0
#define PDF_ROW_HEIGHT_MM       6.0
#define PDF_HEADER_HEIGHT_MM    11.0
//-----------------------------------------------------------------------------------------------
static QString tr(const char *text) {
    return QCoreApplication::translate("CExport", text);
}
//-----------------------------------------------------------------------------------------------
QStringList CExport::headers(void) {
    return {
        tr("Couche"),
        tr("Hauteur (mm)"),
        tr("Ø ext (mm)"),
        tr("Ø int (mm)"),
        tr("Segments"),
        tr("Angle onglet (°)"),
        tr("L ext (mm)"),
        tr("L int (mm)"),
        tr("Largeur planche (mm)"),
        tr("Longueur planche (mm)")
    };
}
//-----------------------------------------------------------------------------------------------
QString CExport::number(double value, int decimals) {
    return QLocale().toString(value, 'f', decimals);
}
//-----------------------------------------------------------------------------------------------
QStringList CExport::row(const CCouche &couche) {
    QStringList r;
    QString none = QStringLiteral("—");

    r << QString::number(couche.index + 1)
      << number(couche.y0) + QStringLiteral(" – ") + number(couche.y1)
      << number(2.0 * couche.rOut);

    if(couche.isDisc()) {
        r << none << tr("Disque") << none << none << none;
    } else {
        r << number(2.0 * couche.rIn)
          << QString::number(couche.nbSegments)
          << number(couche.miterAngle(), 2)
          << number(couche.outerLength())
          << number(couche.innerLength());
    }

    r << number(couche.boardWidth())
      << number(couche.boardLength());

    return r;
}
//-----------------------------------------------------------------------------------------------
bool CExport::toCsv(const CVase &vase, const QString &fileName, QString *error) {
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if(error) {
            *error = file.errorString();
        }
        return false;
    }

    // Point-virgule et BOM UTF-8 : ouverture directe dans un tableur français
    QTextStream out(&file);
    out.setGenerateByteOrderMark(true);
    out << headers().join(';') << '\n';
    for(const CCouche &c : vase.couches()) {
        out << row(c).join(';') << '\n';
    }

    return true;
}
//-----------------------------------------------------------------------------------------------
bool CExport::toPdf(const CVase &vase, const QString &fileName, QString *error) {
    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(PDF_MARGIN_MM, PDF_MARGIN_MM, PDF_MARGIN_MM, PDF_MARGIN_MM), QPageLayout::Millimeter);
    writer.setResolution(PDF_RESOLUTION);
    writer.setTitle(tr("Plan de découpe"));
    writer.setCreator(QCoreApplication::applicationName());

    QPainter painter;
    if(!painter.begin(&writer)) {
        if(error) {
            *error = tr("Impossible d'écrire le fichier PDF.");
        }
        return false;
    }

    const double mm = PDF_RESOLUTION / 25.4;
    const double unit = PDF_RESOLUTION / 96.0;
    const double pageW = writer.width();
    const double pageH = writer.height();
    const SParams &p = vase.params();
    const QVector<CCouche> &couches = vase.couches();

    double y = 0.0;

    // En-tête
    QFont font = painter.font();
    font.setPointSizeF(16);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRectF(0, y, pageW, 9 * mm), Qt::AlignLeft | Qt::AlignVCenter, tr("Plan de découpe — vase segmenté"));
    y += 10 * mm;

    double height = couches.isEmpty() ? 0.0 : couches.last().y1 - couches.first().y0;
    double diamMax = 0.0;
    double lengthTotal = 0.0;
    for(const CCouche &c : couches) {
        diamMax = qMax(diamMax, 2.0 * c.rOut);
        lengthTotal += c.boardLength();
    }

    font.setPointSizeF(9);
    font.setBold(false);
    painter.setFont(font);
    QString info = tr("%1 · Épaisseur de couche %2 mm · Paroi %3 mm · Surépaisseur %4 mm · Fond %5 mm\n"
                      "Hauteur %6 mm · Ø max %7 mm · %8 couches · Longueur de planche cumulée %9 mm (hors trait de scie)")
            .arg(QLocale().toString(QDate::currentDate(), QLocale::ShortFormat))
            .arg(number(p.layerHeight), number(p.wallThickness), number(p.turningMargin), number(p.bottomThickness))
            .arg(number(height), number(diamMax))
            .arg(couches.size())
            .arg(number(lengthTotal, 0));
    painter.drawText(QRectF(0, y, pageW, 10 * mm), Qt::AlignLeft | Qt::TextWordWrap, info);
    y += 12 * mm;

    // Profil
    QRectF profileRect(0, y, pageW, PDF_PROFILE_HEIGHT_MM * mm);
    painter.setPen(QPen(Qt::lightGray, unit));
    painter.drawRect(profileRect);
    QTransform t = CProfileEditor::fitTransform(&vase, profileRect.adjusted(4 * mm, 4 * mm, -4 * mm, -4 * mm));
    CProfileEditor::render(painter, profileRect, t, &vase, unit, -1, false);
    y += profileRect.height() + 6 * mm;

    // Tableau
    static const double weights[COL_COUNT] = { 0.8, 1.6, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.1, 1.2 };
    double weightSum = 0.0;
    for(double w : weights) {
        weightSum += w;
    }

    auto drawRow = [&](const QStringList &cells, double rowH, bool header) {
        double x = 0.0;
        for(int i=0;i<COL_COUNT;i++) {
            QRectF cell(x, y, pageW * weights[i] / weightSum, rowH);
            painter.setPen(QPen(Qt::gray, unit));
            painter.setBrush(header ? QBrush(QColor(235, 225, 210)) : QBrush(Qt::NoBrush));
            painter.drawRect(cell);
            painter.setPen(Qt::black);
            painter.drawText(cell.adjusted(1 * mm, 0, -1 * mm, 0), Qt::AlignCenter | Qt::TextWordWrap, cells.value(i));
            x += cell.width();
        }
        y += rowH;
    };

    auto drawHeader = [&]() {
        font.setBold(true);
        painter.setFont(font);
        drawRow(headers(), PDF_HEADER_HEIGHT_MM * mm, true);
        font.setBold(false);
        painter.setFont(font);
    };

    font.setPointSizeF(8);
    drawHeader();
    for(const CCouche &c : couches) {
        if(y + PDF_ROW_HEIGHT_MM * mm > pageH) {
            writer.newPage();
            y = 0.0;
            drawHeader();
        }
        drawRow(row(c), PDF_ROW_HEIGHT_MM * mm, false);
    }

    painter.end();
    return true;
}
//-----------------------------------------------------------------------------------------------
