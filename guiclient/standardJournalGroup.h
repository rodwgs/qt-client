/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef STANDARDJOURNALGROUP_H
#define STANDARDJOURNALGROUP_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_standardJournalGroup.h"

class standardJournalGroup : public XDialog, public Ui::standardJournalGroup
{
    Q_OBJECT

public:
    standardJournalGroup(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~standardJournalGroup();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sCheck();
    virtual void sClose();
    virtual void sSave();
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sFillList();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _stdjrnlgrpid;

};

#endif // STANDARDJOURNALGROUP_H
