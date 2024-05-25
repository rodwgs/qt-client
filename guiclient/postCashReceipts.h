/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POSTCASHRECEIPTS_H
#define POSTCASHRECEIPTS_H

#include "xdialog.h"
#include "ui_postCashReceipts.h"

class postCashReceipts : public XDialog, public Ui::postCashReceipts
{
    Q_OBJECT

public:
    postCashReceipts(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~postCashReceipts();

protected slots:
    virtual void languageChange();

    virtual void sPost();


};

#endif // POSTCASHRECEIPTS_H
