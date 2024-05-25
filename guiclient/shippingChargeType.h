/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SHIPPINGCHARGETYPE_H
#define SHIPPINGCHARGETYPE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_shippingChargeType.h"

class shippingChargeType : public XDialog, public Ui::shippingChargeType
{
    Q_OBJECT

public:
    shippingChargeType(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~shippingChargeType();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sCheck();
    virtual void sSave();
    virtual void populate();

protected slots:
    virtual void languageChange();

private:
    int _shipchrgid;
    int _mode;

};

#endif // SHIPPINGCHARGETYPE_H
