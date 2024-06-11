/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RESERVESALESORDERITEM_H
#define RESERVESALESORDERITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_reserveSalesOrderItem.h"

class reserveSalesOrderItem : public XDialog, public Ui::reserveSalesOrderItem
{
    Q_OBJECT

public:
    reserveSalesOrderItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~reserveSalesOrderItem();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void reject();
    virtual void sFillList();
    virtual void sReserveLocation();
    virtual void sUnreserveLocation();
    virtual void sBcChanged(const QString);
    virtual void sBcReserve();
    virtual void sCatchLotSerialNumber(const QString);

protected slots:
    virtual void languageChange();
    virtual void populate();

private:
    QList<QVariant> _charidstext;
    QList<QVariant> _charidslist;
    QList<QVariant> _charidsdate;
  
    int _soitemid;
    int _itemsiteid;
    bool _locControl;
    QString _controlMethod;

};

#endif // RESERVESALESORDERITEM_H
