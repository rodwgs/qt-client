/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "enterPoitemReceipt.h"

#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <openreports.h>

#include <metasql.h>

#include "xmessagebox.h"
#include "distributeInventory.h"
#include "itemSite.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

enterPoitemReceipt::enterPoitemReceipt(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sReceive()));
  connect(_toReceive, SIGNAL(editingFinished()), this, SLOT(sDetermineToReceiveInv()));
  connect(_toReceive, SIGNAL(editingFinished()), this, SLOT(sHandleExtendedCost()));
  connect(_purchCost,  SIGNAL(editingFinished()), this, SLOT(sHandleExtendedCost()));

  _invVendorUOMRatio->setPrecision(omfgThis->ratioVal());
  _ordered->setPrecision(omfgThis->qtyVal());
  _received->setPrecision(omfgThis->qtyVal());
  _returned->setPrecision(omfgThis->qtyVal());
  _toReceiveInv->setPrecision(omfgThis->qtyVal());

  _toReceive->setValidator(omfgThis->qtyVal());
  _receiptDate->setDate(QDate::currentDate());

  _mode		= cView;
  _orderitemid	= -1;
  _ordertype	= "";
  _receivable	= 0.0;
  _recvid	= -1;
  _snooze = false;
}

enterPoitemReceipt::~enterPoitemReceipt()
{
  // no need to delete child widgets, Qt does it all for us
}

void enterPoitemReceipt::languageChange()
{
  retranslateUi(this);
}

bool enterPoitemReceipt::correctReceipt(int pRecvid, QWidget *pParent)
{
  XSqlQuery entercorrectReceipt;
  //Validate - Drop Ship receipts may not be corrected
  entercorrectReceipt.prepare("SELECT (count(*) > 0) AS result "
            "FROM recv JOIN pohead ON ((recv_order_type='PO') AND (recv_order_number=pohead_number)) "
            "WHERE ((recv_id=:recvid) "
            "  AND  (COALESCE(pohead_dropship, false))); ");
  entercorrectReceipt.bindValue(":recvid", pRecvid);
  entercorrectReceipt.exec();
  if (entercorrectReceipt.first())
  {
    if (entercorrectReceipt.value("result").toBool())
    {
      QMessageBox::warning(pParent, tr("Cannot Correct"),
                            tr("<p>Receipt is a Drop Shipment.  The received quantity may not be changed.  "
                               "You must use Purchase Order Return to make corrections." ));
      return XDialog::Rejected;
    }
  }
  
  //Validate - Split receipts may not be corrected
  entercorrectReceipt.prepare("SELECT (count(*) > 0) AS result "
            "FROM recv "
            "WHERE (((recv_id=:recvid) "
            "  AND (recv_splitfrom_id IS NOT NULL)) "
            "  OR (recv_splitfrom_id=:recvid)); ");
  entercorrectReceipt.bindValue(":recvid", pRecvid);
  entercorrectReceipt.exec();
  if (entercorrectReceipt.first())
  {
    if (entercorrectReceipt.value("result").toBool())
    {
      QMessageBox::warning(pParent, tr("Cannot Correct"),
                           tr("<p>Receipt has been split.  The received quantity may not be changed."));
      return XDialog::Rejected;
    }
    else
    {
      ParameterList params;
      params.append("mode", "edit");
      params.append("recv_id", pRecvid);

      enterPoitemReceipt newdlg(pParent, "", true);
      newdlg.set(params);

      if (newdlg.exec() != XDialog::Rejected)
        return true;
    }
  }
  return false;
}

enum SetResponse enterPoitemReceipt::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _toReceiveLit->setText(tr("Correct Qty. to:"));
      _freightLit->setText(tr("Correct Freight to:"));
      _item->setEnabled(false);
      _expcat->setEnabled(false);
      setWindowTitle(tr("Correct Item Receipt"));
    }
  }

  param = pParams.value("order_type", &valid);
  if (valid)
    _ordertype = param.toString();

  param = pParams.value("lineitem_id", &valid);
  if (valid)
  {
    _orderitemid = param.toInt();
    populate();
  }

  param = pParams.value("recv_id", &valid);
  if (valid)
  {
    _recvid = param.toInt();
    populate();
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _toReceive->setDouble(param.toDouble());

  _snooze = pParams.inList("snooze");

  if(pParams.inList("receive"))
    sReceive();

  return NoError;
}

void enterPoitemReceipt::populate()
{
  XSqlQuery enterpopulate;
  ParameterList params;

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  if (_metrics->boolean("EnableReturnAuth"))
    params.append("EnableReturnAuth");

  // NOTE: this crashes if popm is defined and toQuery() is called outside the blocks
  if (_mode == cNew)
  {
    MetaSQLQuery popm = mqlLoad("itemReceipt", "populateNew");

    params.append("ordertype",    _ordertype);
    params.append("orderitem_id", _orderitemid);

    enterpopulate = popm.toQuery(params);
  }
  else if (_mode == cEdit)
  {
    MetaSQLQuery popm = mqlLoad("itemReceipt", "populateEdit");
    params.append("recv_id", _recvid);
    enterpopulate = popm.toQuery(params);
  }
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Incomplete Parameter List"),
                         tr("%1:  <p>_orderitem_id=%2, _ordertype=%3, _mode=%4")
                         .arg(windowTitle())
                         .arg(_orderitemid)
                         .arg(_ordertype)
                         .arg(_mode),
                         __FILE__,__LINE__);
    return;
  }
  if (enterpopulate.first())
  {
    _orderNumber->setText(enterpopulate.value("order_number").toString());
    _lineNumber->setText(enterpopulate.value("orderitem_linenumber").toString());
    _vendorItemNumber->setText(enterpopulate.value("vend_item_number").toString());
    _vendorDescrip->setText(enterpopulate.value("vend_item_descrip").toString());
    _vendorUOM->setText(enterpopulate.value("vend_uom").toString());
    _invVendorUOMRatio->setDouble(enterpopulate.value("orderitem_qty_invuomratio").toDouble());
    _dueDate->setDate(enterpopulate.value("duedate").toDate());
    _ordered->setDouble(enterpopulate.value("orderitem_qty_ordered").toDouble());
    _received->setDouble(enterpopulate.value("qtyreceived").toDouble());
    _returned->setDouble(enterpopulate.value("qtyreturned").toDouble());
    _receivable = enterpopulate.value("receivable").toDouble();
    _notes->setText(enterpopulate.value("notes").toString());
    _receiptDate->setDate(enterpopulate.value("effective").toDate());
    _freight->setId(enterpopulate.value("curr_id").toInt());
    _freight->setLocalValue(enterpopulate.value("recv_freight").toDouble());

    if (_ordertype.isEmpty())
      _ordertype = enterpopulate.value("recv_order_type").toString();
    if (_ordertype == "PO")
      _orderType->setText(tr("P/O"));
    else if (_ordertype == "TO")
    {
      _returnedLit->setText(tr("Qty. Shipped:"));
      _orderType->setText(tr("T/O"));
    }
    else if (_ordertype == "RA")
      _orderType->setText(tr("R/A"));

    // Set class vars
    _itemsiteId = enterpopulate.value("itemsiteid").toInt();
    if (_mode == cEdit)
      _recvPosted = enterpopulate.value("recv_posted").toBool();

    if (enterpopulate.value("inventoryitem").toBool())   
    {
      if (_itemsiteId > 0)
        _item->setItemsiteid(_itemsiteId);
      _item->setEnabled(false);
      _itemLitStack->setCurrentIndex(0);
      _itemStack->setCurrentIndex(0);
    }
    else
    {
      int expcatid = enterpopulate.value("expcatid").toInt();
      if (expcatid > 0)
        _expcat->setId(expcatid);
      _expcat->setEnabled(false);
      _itemLitStack->setCurrentIndex(1);
      _itemStack->setCurrentIndex(1);

    }

    _purchCost->setId(enterpopulate.value("recv_purchcost_curr_id").toInt());
    _purchCost->setLocalValue(enterpopulate.value("recv_purchcost").toDouble());
    _purchCost->setEnabled(enterpopulate.value("costmethod_average").toBool() && _metrics->boolean("AllowReceiptCostOverride"));

    _extendedCost->setId(enterpopulate.value("recv_purchcost_curr_id").toInt());

    if (enterpopulate.value("inventoryitem").toBool() && _itemsiteId <= 0)
    {
      MetaSQLQuery ism = mqlLoad("itemReceipt", "sourceItemSite");
      XSqlQuery isq = ism.toQuery(params);
      if (isq.first())
      {
        _itemsiteId = itemSite::createItemSite(this,
                      isq.value("itemsite_id").toInt(),
                      isq.value("warehous_id").toInt(),
                      true);
        if (_itemsiteId < 0)
          return;
        _item->setItemsiteid(_itemsiteId);
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving P/O Receipt Information"),
                                    isq, __FILE__, __LINE__))
      {
        return;
      }
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving P/O Receipt Information"),
                                enterpopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void enterPoitemReceipt::sReceive()
{   
  if(_metrics->boolean("DisallowReceiptExcessQty") && _receivable < _toReceive->toDouble())
  {
    XMessageBox::message( (isVisible() ? this : parentWidget()), QMessageBox::Warning, tr("Cannot Receive"),
                          tr(  "<p>Cannot receive more quantity than ordered." ),
                          QString {}, QString {}, _snooze );
    return;
  }

  if(_ordertype == "RA" && _receivable < _toReceive->toDouble())
  {
    XMessageBox::message( (isVisible() ? this : parentWidget()), QMessageBox::Warning, tr("Cannot Receive"),
                          tr(  "<p>Cannot receive more quantity than authorized." ),
                          QString {}, QString {}, _snooze );
    return;
  }

  double tolerance = _metrics->value("ReceiptQtyTolerancePct").toDouble() / 100.0;
  if(_metrics->boolean("WarnIfReceiptQtyDiffers") &&
      (_receivable < _toReceive->toDouble() * (1.0 - tolerance) ||
       _receivable > _toReceive->toDouble() * (1.0 + tolerance)))
  {
    if(XMessageBox::message( (isVisible() ? this : parentWidget()) , QMessageBox::Question, tr("Receipt Qty. Differs"),
        tr("<p>The Qty entered does not match the receivable Qty for this order. "
		   "Do you wish to continue anyway?"),
        tr("Yes"), tr("No"), _snooze, 0, 1) == 1)
      return;
  }

  XSqlQuery enterReceive;
  XSqlQuery updateNotes;
  int itemlocSeries = 0;
  QString storedProc;

  // Stage cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE); ");

  if (_mode == cNew)
  {
    enterReceive.prepare("SELECT enterReceipt(:ordertype, :poitem_id, :qty, :freight, :notes, "
              ":curr_id, :effective, :purchcost) AS result;");
    enterReceive.bindValue(":poitem_id",	_orderitemid);
    enterReceive.bindValue(":ordertype",	_ordertype);
    storedProc = "enterReceipt";
  }
  else if (_mode == cEdit)
  {
    // Get parent series id
    XSqlQuery parentSeries;
    parentSeries.prepare("SELECT NEXTVAL('itemloc_series_seq') AS result, "
                         "  isControlledItemsite(:itemsite_id) AS controlled;");
    parentSeries.bindValue(":itemsite_id", _itemsiteId);
    parentSeries.exec();
    if (parentSeries.first() && parentSeries.value("result").toInt() > 0)
    {
      itemlocSeries = parentSeries.value("result").toInt();
      cleanup.bindValue(":itemlocSeries", itemlocSeries);
    }
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Failed to Retrieve the Next itemloc_series_seq"),
                              parentSeries, __FILE__, __LINE__);
      return;
    }

    // If controlled item, unposted, qty >= qty already received: 
    // create the parent itemlocdist record, call distributeInventory::seriesAdjust
    if ((_recvPosted && (_toReceive->toDouble() != _received->toDouble()) && _itemsiteId != -1)
      && parentSeries.value("controlled").toBool())
    {
      XSqlQuery parentItemlocdist;
      parentItemlocdist.prepare("SELECT createitemlocdistparent(:itemsite_id, :qty, :orderType, "
                                " :orderitemId, :itemlocSeries, NULL, NULL, 'RP');");
      parentItemlocdist.bindValue(":itemsite_id", _itemsiteId);
      parentItemlocdist.bindValue(":qty", 
        (_toReceive->toDouble() - _received->toDouble()) * _invVendorUOMRatio->toDouble());
      parentItemlocdist.bindValue(":orderType", _ordertype);
      parentItemlocdist.bindValue(":orderitemId", _orderitemid);
      parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
      parentItemlocdist.exec();
      if (parentItemlocdist.first())
      {
        if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
          QDate(), true) == XDialog::Rejected)
        {
          cleanup.exec();
          XMessageBox::message( (isVisible() ? this : parentWidget()), QMessageBox::Warning, tr("Enter PO Receipt"),
                            tr(  "<p>Transaction Cancelled." ),
                            QString {}, QString {}, _snooze );
          return;
        }
      }
      else
      {
        cleanup.exec();
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),\
          parentItemlocdist, __FILE__, __LINE__);
        return;
      }
    }

    // Update notes, bind the old to the cleanup query in case of correctReceipt error
    XSqlQuery updateNotes;
    updateNotes.prepare("UPDATE recv n "
                         "SET recv_notes = :notes "
                         "FROM recv o "
                         "WHERE n.recv_id = :recv_id "
                         "  AND n.recv_id = o.recv_id "
                         "RETURNING o.recv_notes;");
    updateNotes.bindValue(":notes",	_notes->toPlainText());
    updateNotes.bindValue(":recv_id",	_recvid);
    updateNotes.exec();
    if (updateNotes.lastError().type() != QSqlError::NoError)
    {
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving P/O Receipt Information"),
                           updateNotes, __FILE__, __LINE__);
      return;
    }

    // Stage old notes value to call updateNotes again if correctReceipt fails
    updateNotes.bindValue(":recv_id",  _recvid);
    updateNotes.bindValue(":notes", updateNotes.value("recv_notes").toString());
    
    // Proceed to post inventory transaction with detail
    enterReceive.prepare("SELECT correctReceipt(:recv_id, :qty, :freight, :itemlocSeries, "
              ":curr_id, :effective, :purchcost, TRUE) AS result;");
    enterReceive.bindValue(":recv_id", _recvid);
    enterReceive.bindValue(":itemlocSeries", itemlocSeries);
    storedProc = "correctReceipt";
  }

  enterReceive.bindValue(":qty",		_toReceive->toDouble());
  enterReceive.bindValue(":freight",	_freight->localValue());
  enterReceive.bindValue(":notes",		_notes->toPlainText());
  enterReceive.bindValue(":curr_id",	_freight->id());
  enterReceive.bindValue(":effective",	_receiptDate->date());
  enterReceive.bindValue(":purchcost",     _purchCost->localValue());
  enterReceive.exec();
  if (enterReceive.first())
  {
    int result = enterReceive.value("result").toInt();
    // For cEdit (correctReceipt), make sure the returned series matches the series passed
    if ((result < 0) || (_mode == cEdit && (itemlocSeries != result)))
    {
      updateNotes.exec();
      cleanup.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving PO Receipt Information"),
        storedProcErrorLookup(storedProc, result), __FILE__, __LINE__);
      return;
    }
  }
  else if (enterReceive.lastError().type() != QSqlError::NoError)
  {
      if (_mode == cEdit) // Only cEdit created itemlocdist records
      {
        updateNotes.exec();
        cleanup.exec();
      }
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving P/O Receipt Information"),
                           enterReceive, __FILE__, __LINE__);
      return;
  }

  omfgThis->sPurchaseOrderReceiptsUpdated();
  accept();
  
  if (_printLabel->isChecked())
    sPrintItemLabel();
}

void enterPoitemReceipt::sDetermineToReceiveInv()
{
    _toReceiveInv->setDouble(_invVendorUOMRatio->toDouble() * _toReceive->toDouble());
}

void enterPoitemReceipt::sPrintItemLabel()
{
    ParameterList params;
    params.append("vendorItemLit", tr("Vendor Item#:"));
    params.append("ordertype", _ordertype);
    params.append("orderitemid", _orderitemid);
    orReport report("ReceivingLabel", params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
    }
}

void enterPoitemReceipt::sHandleExtendedCost()
{
  _extendedCost->setLocalValue(_toReceive->toDouble() * _purchCost->localValue());
}
