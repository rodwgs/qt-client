/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ACCOUNTINGYEARPERIOD_H
#define ACCOUNTINGYEARPERIOD_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_accountingYearPeriod.h"

class accountingYearPeriod : public XDialog, public Ui::accountingYearPeriod
{
    Q_OBJECT

public:
    accountingYearPeriod(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~accountingYearPeriod();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sSave();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    bool _cachedFrozen;
    bool _cachedClosed;
    QDate _cachedEndDate;
    QDate _cachedStartDate;
    int _periodid;
    int _mode;

};

#endif // ACCOUNTINGYEARPERIOD_H
