/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PURCHASEORDERITEM_H
#define PURCHASEORDERITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <QStandardItemModel>
#include <parameter.h>
#include "ui_purchaseOrderItem.h"

class purchaseOrderItem : public XDialog, public Ui::purchaseOrderItem
{
    Q_OBJECT

public:
    purchaseOrderItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~purchaseOrderItem();

  
    virtual void  prepare();
    virtual void  clear();
    Q_INVOKABLE virtual int id() { return _poitemid; }

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sDeterminePrice();
    virtual void sCalculateTax();        // new function added
    virtual void sTaxDetail();           // new function added

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sPopulateExtPrice();
    virtual void sFindWarehouseItemsites( int pId );
    virtual void sPopulateItemInfo( int pItemid );
    virtual void sPopulateItemChar();
    virtual void sPopulateItemsiteInfo();
    virtual void sPopulateItemSourceInfo( int pItemsrcid );
    virtual void sInventoryItemToggled( bool yes );
    virtual void sVendorItemNumberSearch();
    virtual void sVendorItemNumberList();
    virtual void sHandleButtons();
    virtual void sVendorListPrices();


private:
    int _itemsrcid;
    int _mode;
    int _poheadid;
    int _poitemid;
    int _parentwo;
    int _parentso;
    int _vendid;
    int _preferredWarehouseid;
    int _taxzoneid;
    double _invVendUOMRatio;
    double _minimumOrder;
    double _orderMultiple;
    double _orderQtyCache;
    double _maxCost;
    bool _overriddenUnitPrice;
    bool _dropship;
    bool _captive;
    QString	_poStatus;
    QString	_costmethod;
    QStandardItemModel * _itemchar;

};

#endif // PURCHASEORDERITEM_H
