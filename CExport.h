//-----------------------------------------------------------------------------------------------
#ifndef CEXPORT_H
#define CEXPORT_H
//-----------------------------------------------------------------------------------------------
#include <QStringList>
#include "CVase.h"
//-----------------------------------------------------------------------------------------------
// Plan de découpe : colonnes partagées entre le tableau de l'appli, le CSV et le PDF.
class CExport
{
public:
    enum EColumn {
        COL_COUCHE = 0,
        COL_HAUTEUR,
        COL_DIAM_EXT,
        COL_DIAM_INT,
        COL_SEGMENTS,
        COL_ANGLE,
        COL_LONG_EXT,
        COL_LONG_INT,
        COL_LARG_PLANCHE,
        COL_LONG_PLANCHE,
        COL_COUNT
    };

    static QStringList headers(void);
    static QStringList row(const CCouche &couche);
    static QString number(double value, int decimals = 1);

    static bool toCsv(const CVase &vase, const QString &fileName, QString *error = nullptr);
    static bool toPdf(const CVase &vase, const QString &fileName, QString *error = nullptr);
};
//-----------------------------------------------------------------------------------------------
#endif // CEXPORT_H
//-----------------------------------------------------------------------------------------------
