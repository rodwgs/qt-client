/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef GETLOTINFO_H
#define GETLOTINFO_H

#include "guiclient.h"
#include "xdialog.h"

#include "ui_getLotInfo.h"
#include "lotSerialUtils.h"

class getLotInfo : public XDialog, public Ui::getLotInfo
{
    Q_OBJECT

public:
    getLotInfo(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~getLotInfo();

public slots:
    virtual QString lot();
    virtual QDate expiration();
    virtual QDate warranty();
    virtual void enableExpiration( bool yes );
    virtual void enableWarranty( bool yes );
    virtual void sAssign();

protected slots:
    virtual void languageChange();

private:
    LotSerialUtils _lschars;
    QList<QWidget *> _charWidgets;

};

#endif // GETLOTINFO_H
