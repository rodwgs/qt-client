/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UPDATECREDITSTATUSBYCUSTOMER_H
#define UPDATECREDITSTATUSBYCUSTOMER_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_updateCreditStatusByCustomer.h"

class updateCreditStatusByCustomer : public XDialog, public Ui::updateCreditStatusByCustomer
{
    Q_OBJECT

public:
    updateCreditStatusByCustomer(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~updateCreditStatusByCustomer();

public slots:
    virtual void sUpdate();
    virtual void sPopulate( int pCustid );

protected slots:
    virtual void languageChange();

};

#endif // UPDATECREDITSTATUSBYCUSTOMER_H
