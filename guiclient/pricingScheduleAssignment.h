/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRICINGSCHEDULEASSIGNMENT_H
#define PRICINGSCHEDULEASSIGNMENT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_pricingScheduleAssignment.h"

class pricingScheduleAssignment : public XDialog, public Ui::pricingScheduleAssignment
{
    Q_OBJECT

public:
    pricingScheduleAssignment(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~pricingScheduleAssignment();

    virtual SetResponse set( const ParameterList & pParams );

public slots:
    virtual void sAssign();
    virtual void populate();

protected slots:
    virtual void languageChange();

    virtual void sCustomerSelected();


private:
    int _mode;
    int _ipsassid;
    bool _listpricesched;

};

#endif // PRICINGSCHEDULEASSIGNMENT_H
