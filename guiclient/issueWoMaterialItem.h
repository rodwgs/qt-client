/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ISSUEWOMATERIALITEM_H
#define ISSUEWOMATERIALITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_issueWoMaterialItem.h"

class issueWoMaterialItem : public XDialog, public Ui::issueWoMaterialItem
{
    Q_OBJECT

public:
    issueWoMaterialItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~issueWoMaterialItem();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sCatchItemid( int pItemid );
    virtual void sCatchItemsiteid( int pItemsiteid );
    virtual void sIssue();
    virtual void sSetQOH(int pWomatlid);
    virtual void sPopulateQOH();

protected slots:
    virtual void languageChange();

private:
    bool   _captive;
    double _cachedQOH;

};

#endif // ISSUEWOMATERIALITEM_H
