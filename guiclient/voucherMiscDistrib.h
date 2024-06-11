/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VOUCHERMISCDISTRIB_H
#define VOUCHERMISCDISTRIB_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_voucherMiscDistrib.h"

class voucherMiscDistrib : public XDialog, public Ui::voucherMiscDistrib
{
    Q_OBJECT

public:
    voucherMiscDistrib(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~voucherMiscDistrib();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sSave();
    virtual void sCheck();
    virtual void sHandleSelection();
    virtual void sPopulateVendorInfo(int);

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _voheadid;
    int _vodistid;
	bool _miscvoucher;

};

#endif // VOUCHERMISCDISTRIB_H
