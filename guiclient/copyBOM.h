/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COPYBOM_H
#define COPYBOM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_copyBOM.h"

class copyBOM : public XDialog, public Ui::copyBOM
{
    Q_OBJECT

public:
    copyBOM(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~copyBOM();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sCopy();
    virtual void sHandleButtons();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // COPYBOM_H
