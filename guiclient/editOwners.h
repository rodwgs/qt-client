/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EDITOWNERS_H
#define EDITOWNERS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_editOwners.h"

class editOwners : public XDialog, public Ui::editOwners
{
    Q_OBJECT

public:
    editOwners(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});

protected slots:
    virtual void sClose();
    virtual void sFillList();
    virtual void sModify();
    virtual void sModifyAll();
    virtual void sItemClicked();
    virtual bool modifyOne(XTreeWidgetItem*);

private:
    QString _queryString;
    bool _first;

};

#endif // EDITOWNERS_H
