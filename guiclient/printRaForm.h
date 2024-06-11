/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTRAFORM_H
#define PRINTRAFORM_H

#include "xdialog.h"
#include <parameter.h>
#include "ui_printRaForm.h"
#include "guiclient.h"

class printRaForm : public XDialog, public Ui::printRaForm
{
    Q_OBJECT

public:
    printRaForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~printRaForm();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sPrint();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // PRINTRAFORM_H
