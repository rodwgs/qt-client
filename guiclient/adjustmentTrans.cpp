/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "adjustmentTrans.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <QButtonGroup>

#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#define DEBUG false

adjustmentTrans::adjustmentTrans(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _adjustmentTypeGroupInt = new QButtonGroup(this);

  connect(_absolute,                   SIGNAL(toggled(bool)), this, SLOT(sPopulateQty()));
  connect(_adjustmentTypeGroupInt,SIGNAL(buttonClicked(int)), this, SLOT(sPopulateQty()));
  connect(_post,                           SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_qty,          SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQty()));
  connect(_qty,          SIGNAL(textChanged(const QString&)), this, SLOT(sCostUpdated()));
  connect(_item,                          SIGNAL(newId(int)), this, SLOT(sPopulateQOH()));
  connect(_warehouse,                     SIGNAL(newID(int)), this, SLOT(sPopulateQOH()));
  connect(_cost, SIGNAL(textChanged(const QString&)), this, SLOT(sCostUpdated()));
  connect(_costManual, SIGNAL(toggled(bool)), this, SLOT(sPopulateQty()));

  _captive = false;

  _item->setType((ItemLineEdit::cGeneralInventory ^ ItemLineEdit::cBreeder) | ItemLineEdit::cActive);
  _item->addExtraClause( QString("(item_type NOT IN ('R', 'F'))") );
  _item->addExtraClause( QString("(itemsite_costmethod != 'J')") );
  _warehouse->setType(WComboBox::AllActiveInventory);
  _afterQty->setPrecision(omfgThis->qtyVal());
  _beforeQty->setPrecision(omfgThis->qtyVal());
  _cost->setValidator(omfgThis->costVal());
  _qty->setValidator(omfgThis->transQtyVal());
  _unitCost->setPrecision(omfgThis->costVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  if (!_metrics->boolean("AllowAvgCostMethod"))
    _tab->removeTab(0);

  _item->setFocus();
  _itemsiteId = 0;
  _controlledItem = false;
}

adjustmentTrans::~adjustmentTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

void adjustmentTrans::languageChange()
{
  retranslateUi(this);
}

enum SetResponse adjustmentTrans::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  int      invhistid = -1;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = true;

    _item->setItemsiteid(param.toInt());
    _item->setEnabled(false);
    _warehouse->setEnabled(false);
  }

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _captive = true;

    _qty->setDouble(param.toDouble());
    _qty->setEnabled(false);
    _afterQty->setDouble(param.toDouble());

    _absolute->setChecked(true);
    _adjustmentTypeGroup->setEnabled(false);
  }

  param = pParams.value("invhist_id", &valid);
  if (valid)
    invhistid = param.toInt();

  param = pParams.value("mode", &valid);
  XSqlQuery setAdjustment;
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _usernameLit->clear();
      _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
      _transDate->setDate(omfgThis->dbDate());
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _transDate->setEnabled(false);
      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
      _adjustmentTypeGroup->setEnabled(false);
      _qty->setEnabled(false);
      _documentNum->setEnabled(false);
      _notes->setEnabled(false);
      _post->hide();
      _close->setText(tr("&Close"));

      setAdjustment.prepare( "SELECT * "
                 "FROM invhist "
                 "WHERE (invhist_id=:invhist_id);" );
      setAdjustment.bindValue(":invhist_id", invhistid);
      setAdjustment.exec();
      if (setAdjustment.first())
      {
        _transDate->setDate(setAdjustment.value("invhist_transdate").toDate());
        _username->setText(setAdjustment.value("invhist_user").toString());
        _qty->setDouble(setAdjustment.value("invhist_invqty").toDouble());
        _beforeQty->setDouble(setAdjustment.value("invhist_qoh_before").toDouble());
        _afterQty->setDouble(setAdjustment.value("invhist_qoh_after").toDouble());
        _documentNum->setText(setAdjustment.value("invhist_ordnumber"));
        _notes->setText(setAdjustment.value("invhist_comments").toString());
        _item->setItemsiteid(setAdjustment.value("invhist_itemsite_id").toInt());
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Transaction"),
                                    setAdjustment, __FILE__, __LINE__))
      {
          return UndefinedError;
      }

    }
  }

  return NoError;
}

void adjustmentTrans::sPost()
{
  XSqlQuery adjustmentPost;
  XSqlQuery cleanup;
  int itemlocSeries = 0;
  double qty = _qty->toDouble();
  double cost = _cost->toDouble();

  if (_absolute->isChecked())
  {
    qty = (_qty->toDouble() - _cachedQOH);
    cost = (_cost->toDouble() - _cachedValue);
  }

  QList<GuiErrorCheck>errors;
    errors<<GuiErrorCheck(!_item->isValid(), _item,
      tr("You must select an Item before posting this transaction."))
    <<GuiErrorCheck(_qty->text().length() == 0 || qty == 0, _qty,
      tr("<p>You must enter a Quantity before posting this Transaction."))
    <<GuiErrorCheck(_costAdjust->isEnabled() && _costAdjust->isChecked() && _costManual->isChecked()
      && (_cost->text().length() == 0 || cost == 0), _cost,
      tr("<p>You must enter a total cost value for the inventory to be transaction."))
    <<GuiErrorCheck( _costMethod == "A" && _afterQty->toDouble() < 0, _qty,
      tr("<p>Average cost adjustments may not result in a negative quantity on hand."));

  if (GuiErrorCheck::reportErrors(this,tr("Cannot Post Transaction"),errors))
    return;

  // Get parent series id
  XSqlQuery parentSeries;
  parentSeries.prepare("SELECT NEXTVAL('itemloc_series_seq') AS result;");
  parentSeries.exec();
  if (parentSeries.first())
    itemlocSeries = parentSeries.value("result").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Failed to Retrieve the Next itemloc_series_seq"),
                            parentSeries, __FILE__, __LINE__))
    return;

  if (DEBUG)
    qDebug() << "adjustmentTrans::sPost itemlocSeries: " << itemlocSeries;

  // Stage cleanup function to be called on error
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");
  cleanup.bindValue(":itemlocSeries", itemlocSeries);

  // If controlled item: create the parent itemlocdist record, call distributeInventory::seriesAdjust
  if (_controlledItem)
  {
    XSqlQuery parentItemlocdist;
    parentItemlocdist.prepare("SELECT createitemlocdistparent(:itemsite_id, :qty, 'AD'::TEXT, NULL, :itemlocSeries) AS result;");
    parentItemlocdist.bindValue(":itemsite_id", _itemsiteId);
    parentItemlocdist.bindValue(":qty", qty);
    parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
    parentItemlocdist.exec();
    if (parentItemlocdist.first())
    {
      if (distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(), true)
        == XDialog::Rejected)
      {
        cleanup.exec();
        QMessageBox::information(this, tr("Inventory Adjustment"),
                                 tr("Transaction Canceled") );
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating itemlocdist Records"),
                              parentItemlocdist, __FILE__, __LINE__))
    {
      return;
    }
  }

  // Post inventory transaction
  adjustmentPost.prepare( "SELECT invAdjustment(:itemsite_id, :qty, :docNumber, :comments, :date, "
                          " :cost, NULL, :itemlocSeries, true) AS result;");
  adjustmentPost.bindValue(":qty", qty);
  adjustmentPost.bindValue(":docNumber",   _documentNum->text());
  adjustmentPost.bindValue(":comments",    _notes->toPlainText());
  adjustmentPost.bindValue(":itemsite_id", _itemsiteId);
  adjustmentPost.bindValue(":date",        _transDate->date());
  adjustmentPost.bindValue(":itemlocSeries", itemlocSeries);
  if(!_costAdjust->isChecked())
    adjustmentPost.bindValue(":cost", 0.0);
  else if(_costManual->isChecked())
    adjustmentPost.bindValue(":cost", cost);
  adjustmentPost.exec();
  if (adjustmentPost.first())
  {
    // Continue to cleanup
    if (_captive)
      close();
    else
    {
      _item->setId(-1);
      _qty->clear();
      _documentNum->clear();
      _notes->clear();
      _beforeQty->clear();
      _afterQty->clear();
      _cost->clear();
      _transDate->setDate(omfgThis->dbDate());

      _item->setFocus();
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Transaction"),
                                adjustmentPost, __FILE__, __LINE__))
  {
    cleanup.exec();
    return;
  }
  else
  {
    cleanup.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Post Transaction Cancelled"),
                         tr("<p>No transaction was done because Item %1 "
                            "was not found at Site %2.")
                         .arg(_item->itemNumber()).arg(_warehouse->currentText()),
                         __FILE__, __LINE__);
  }
}

void adjustmentTrans::sPopulateQOH()
{
  XSqlQuery populateAdjustment;
  if (_mode != cView)
  {
    populateAdjustment.prepare("SELECT itemsite_id, itemsite_freeze, itemsite_qtyonhand, "
               "  itemsite_costmethod, itemsite_value, "
               "  itemsite_loccntrl, "
               "  (itemsite_controlmethod IN ('L', 'S') ) AS lscntrl "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id));" );
    populateAdjustment.bindValue(":item_id", _item->id());
    populateAdjustment.bindValue(":warehous_id", _warehouse->id());
    populateAdjustment.exec();

    _absolute->setStyleSheet("");
    if (populateAdjustment.first())
    {
      _itemsiteId = populateAdjustment.value("itemsite_id").toInt();
      _cachedValue = populateAdjustment.value("itemsite_value").toDouble();
      _cachedQOH = populateAdjustment.value("itemsite_qtyonhand").toDouble();
      _controlledItem = populateAdjustment.value("lscntrl").toBool() ||
        populateAdjustment.value("itemsite_loccntrl").toBool();
      if(_cachedQOH == 0.0)
        _costManual->setChecked(true);
      _beforeQty->setDouble(populateAdjustment.value("itemsite_qtyonhand").toDouble());
      _costAdjust->setChecked(true);
      _costAdjust->setEnabled(populateAdjustment.value("itemsite_costmethod").toString() == "A");
      _costMethod = populateAdjustment.value("itemsite_costmethod").toString();

      if (populateAdjustment.value("itemsite_freeze").toBool())
        _absolute->setStyleSheet(QString("* { color: %1; }")
                                 .arg(namedColor("error").name()));

      if (_item->isFractional())
        _qty->setValidator(omfgThis->transQtyVal());
      else
        _qty->setValidator(new QIntValidator(this));
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Information"),
                                  populateAdjustment, __FILE__, __LINE__))
    {
        return;
    }

    sPopulateQty();
  }
}

void adjustmentTrans::sPopulateQty()
{
  if (_mode == cNew)
  {

    if (_qty->text().trimmed().length())
    {
      if (_absolute->isChecked())
        _afterQty->setDouble(_qty->toDouble());

      else if (_relative->isChecked())
        _afterQty->setDouble(_cachedQOH + _qty->toDouble());
    }
    else
      _afterQty->clear();

    bool neg = ((_afterQty->toDouble() < _cachedQOH) || (_afterQty->toDouble() == 0));
    if(neg)
      _costCalculated->setChecked(true);
    _costManual->setEnabled(!neg);
    _cost->setEnabled(!neg && _costManual->isChecked());
    _lblCost->setEnabled(!neg);
    _unitCost->setEnabled(!neg);
    _unitCostLit->setEnabled(!neg);

    if (_afterQty->toDouble() == 0)
    {
      _costAdjust->setChecked(true);
      _costAdjust->setEnabled(false);
    }
    else
      _costAdjust->setEnabled(_costMethod == "A");
  }
}

void adjustmentTrans::sCostUpdated()
{
  if(_cost->toDouble() == 0.0 || _qty->toDouble() == 0.0)
    _unitCost->setText(tr("N/A"));
  else
    _unitCost->setDouble(_cost->toDouble() / _qty->toDouble());
}
