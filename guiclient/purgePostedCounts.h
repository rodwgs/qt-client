/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PURGEPOSTEDCOUNTS_H
#define PURGEPOSTEDCOUNTS_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_purgePostedCounts.h"

class purgePostedCounts : public XDialog, public Ui::purgePostedCounts
{
    Q_OBJECT

public:
    purgePostedCounts(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~purgePostedCounts();

public slots:
    virtual void sPurge();

protected slots:
    virtual void languageChange();

};

#endif // PURGEPOSTEDCOUNTS_H
