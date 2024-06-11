/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ISSUEWOMATERIALBATCH_H
#define ISSUEWOMATERIALBATCH_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_issueWoMaterialBatch.h"

class issueWoMaterialBatch : public XDialog, public Ui::issueWoMaterialBatch
{
    Q_OBJECT

public:
    issueWoMaterialBatch(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~issueWoMaterialBatch();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sIssue();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    bool _hasPush;

};

#endif // ISSUEWOMATERIALBATCH_H
