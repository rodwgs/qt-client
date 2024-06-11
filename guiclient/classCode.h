/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CLASSCODE_H
#define CLASSCODE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_classCode.h"

class classCode : public XDialog, public Ui::classCode
{
    Q_OBJECT

public:
    classCode(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~classCode();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sNewTax();
    virtual void sEditTax();
    virtual void sDeleteTax();
    virtual void sFillListTax();
    virtual bool sSave();

protected slots:
    virtual void languageChange();

    virtual void sSaveAndDone();
    virtual void sCheck();


private:
    int _classcodeid;
    int _mode;

};

#endif // CLASSCODE_H
