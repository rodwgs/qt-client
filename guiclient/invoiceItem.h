/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef INVOICEITEM_H
#define INVOICEITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include <QStandardItemModel>

#include "ui_invoiceItem.h"

class invoiceItem : public XDialog, public Ui::invoiceItem
{
    Q_OBJECT

public:
    invoiceItem(QWidget* = 0, const char * = 0, Qt::WindowFlags = Qt::WindowFlags {});
    ~invoiceItem();

    Q_INVOKABLE virtual int id()   const;
    Q_INVOKABLE virtual int mode() const;

    virtual void populate();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sCalculateExtendedPrice();
    virtual void sPopulateItemInfo( int pItemid );
    virtual void sDeterminePrice();
    virtual void sListPrices();
    virtual void sLookupTax();
    virtual void sPriceGroup();
    virtual void sTaxDetail();
    virtual void sPopulateUOM();
    virtual void sQtyUOMChanged();
    virtual void sPriceUOMChanged();
    virtual void sMiscSelected(bool);
    virtual void sHandleUpdateInv();

protected slots:
    virtual void languageChange();

signals:
    void populated();
    void newId(int);
    void newMode(int);
    void saved(int);

private:
    int _mode;
    int _invcheadid;
    int _custid;
    int _shiptoid;
    int _invcitemid;
    double _priceRatioCache;
    int _taxzoneid;
    int	_invuomid;
    double _qtyinvuomratio;
    double _priceinvuomratio;
    double _listprice;
    bool _saved;
    bool _trackqoh;
    QStandardItemModel * _itemchar;
};

#endif // INVOICEITEM_H
