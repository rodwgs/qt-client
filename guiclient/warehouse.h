/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include "guiclient.h"
#include "xdialog.h"
#include "contactcluster.h"
#include "ui_warehouse.h"

class warehouse : public XDialog, public Ui::warehouse
{
    Q_OBJECT

public:
    warehouse(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~warehouse();

    Q_INVOKABLE virtual int id()   const;
    Q_INVOKABLE virtual int mode() const;

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void populate();
    virtual void sCheck();
    virtual void sDeleteZone();
    virtual void sEditZone();
    virtual void sFillList();
    virtual void sHandleWhsType();
    virtual void sNewZone();

signals:
    void newId(int);
    void newMode(int);
    void populated();
    void saved(int);

protected slots:
    virtual void languageChange();
    virtual void sSave();


private:
    int _mode;
    int _warehousid;

};

#endif // WAREHOUSE_H
