/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef APOPENITEM_H
#define APOPENITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_apOpenItem.h"

class apOpenItem : public XDialog, public Ui::apOpenItem
{
    Q_OBJECT

public:
    apOpenItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~apOpenItem();

    virtual void populate();
	virtual void populateStatus();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sClose();
    virtual void sDetermineTaxAmount();
    virtual bool sInitializeMemo();
    virtual void sPopulateVendInfo( int vend_id );
    virtual void sPopulateDueDate();
    virtual void sTaxDetail();
    virtual void sPrintOnPost(int temp_id);
    virtual void sReleaseNumber();
    virtual void sToggleAccount();
    virtual void sCalcBalance();
    virtual void sViewMode();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _apopenid;
    double _cAmount;
    int _seqiss;

};

#endif // APOPENITEM_H
