/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POSTPORETURNCREDITMEMO_H
#define POSTPORETURNCREDITMEMO_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_postPoReturnCreditMemo.h"

class postPoReturnCreditMemo : public XDialog, public Ui::postPoReturnCreditMemo
{
    Q_OBJECT

public:
    postPoReturnCreditMemo(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~postPoReturnCreditMemo();

public slots:
    virtual enum SetResponse set(const ParameterList &);
    virtual void sPost();

protected slots:
    virtual void languageChange();

private:
    int _porejectid;
};

#endif // POSTPORETURNCREDITMEMO_H
