/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SPLITRECEIPT_H
#define SPLITRECEIPT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_splitReceipt.h"

class splitReceipt : public XDialog, public Ui::splitReceipt
{
    Q_OBJECT

public:
    splitReceipt(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~splitReceipt();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sSplit();

protected slots:
    virtual void languageChange();

private:
    int		_recvid;

};

#endif // SPLITRECEIPT_H
