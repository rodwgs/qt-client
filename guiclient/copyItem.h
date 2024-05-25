/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COPYITEM_H
#define COPYITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_copyItem.h"

class copyItem : public XDialog, public Ui::copyItem
{
    Q_OBJECT

public:
    copyItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~copyItem();

    Q_INVOKABLE inline  bool    captive() const { return _captive; }
    Q_INVOKABLE virtual bool    okToSave();
    Q_INVOKABLE virtual int     id()   const;

public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void clear();
    virtual void sUpdateItem();
    virtual void sCopyBom();
    virtual void sAddBomitem();
    virtual void sEditBomitem();
    virtual void sRevokeBomitem();
    virtual void sFillItem();
    virtual void sFillBomitem();
    virtual void sCopyItemsite();
    virtual void sAddItemsite();
    virtual void sEditItemsite();
    virtual void sRevokeItemsite();
    virtual void sFillItemsite();
    virtual void sNext();
    virtual void sCancel();
    virtual void sCopy();
    virtual void closeEvent( QCloseEvent * pEvent );

protected slots:
    virtual void languageChange();
    virtual void cancelCopy();
    virtual bool saveItem();

private:
    bool _captive;
    bool _inTransaction;
    bool _isActive;
    bool _committed;
    int _newitemid;

};

#endif // COPYITEM_H
