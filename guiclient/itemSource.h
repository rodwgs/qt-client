/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMSOURCE_H
#define ITEMSOURCE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_itemSource.h"

class itemSource : public XDialog, public Ui::itemSource
{
    Q_OBJECT

public:
    itemSource(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~itemSource();

    Q_INVOKABLE virtual int id()   const;
    Q_INVOKABLE virtual int mode() const;
  
public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual bool sSave();
    virtual void sSaveClicked();
    virtual void sAdd();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sPopulateMenu( QMenu * pMenu );
    virtual void sFillPriceList();
    virtual void populate();
    virtual void sRejected();
    virtual void sVendorChanged( int pId );
    virtual void sContractChanged( int pId );

protected slots:
    virtual void languageChange();

private:
    int  _mode;
    int  _itemsrcid;
    bool _captive;
    bool _new;
 
};

#endif // ITEMSOURCE_H
