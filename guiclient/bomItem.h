/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BOMITEM_H
#define BOMITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_bomItem.h"

class bomItem : public XDialog, public Ui::bomItem
{
    Q_OBJECT

public:
    bomItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~bomItem();

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sSave();
    virtual void sSaveClick();
    virtual void sClose();
    virtual void sItemTypeChanged( const QString & type );
    virtual void sPopulateUOM();
    virtual void sUOMChanged();
    virtual void populate();
    virtual void sNewSubstitute();
    virtual void sEditSubstitute();
    virtual void sDeleteSubstitute();
    virtual void sFillSubstituteList();
    virtual void sItemIdChanged();
    virtual void sCharIdChanged();
    virtual void sFillCostList();
    virtual void sHandleBomitemCost();
    virtual void sCostSelectionChanged();
    virtual void sNewCost();
    virtual void sEditCost();
    virtual void sDeleteCost();

signals:
   void saved(int);

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _invuomid;
    int _bomitemid;
    int _sourceBomitemid;
    int _parentitemid;
    int _bomheadid;
    QString _parentitemtype;
    int _revisionid;
    bool _saved;

};

#endif // BOMITEM_H
