/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createItemSitesByClassCode.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <QButtonGroup>
#include "guiErrorCheck.h"

createItemSitesByClassCode::createItemSitesByClassCode(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _locationGroupInt = new QButtonGroup(this);
  _locationGroupInt->addButton(_location);
  _locationGroupInt->addButton(_miscLocation);

  connect(_create, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(populateLocations()));
  connect(_controlMethod, SIGNAL(activated(int)), this, SLOT(sHandleControlMethod()));
  connect(_planningType, SIGNAL(activated(int)), this, SLOT(sHandlePlanningType()));
  connect(_poSupply, SIGNAL(toggled(bool)), this, SLOT(sHandlePOSupply(bool)));
  connect(_woSupply, SIGNAL(toggled(bool)), this, SLOT(sHandleWOSupply(bool)));
  _planningType->append(0, "None", "N");

  _mpsTimeFenceLit->hide();
  _mpsTimeFence->hide();
  _mpsTimeFenceDaysLit->hide();

  _orderGroup->setMinimum(1);

  if(_metrics->value("Application") == "PostBooks")
  {
    _planningType->setCurrentIndex(0);
    _planningType->hide();
    _planningTypeLit->hide();
    _createPlannedTransfers->hide();

    _orderGroupLit->hide();
    _orderGroup->hide();
    _orderGroupDaysLit->hide();
    _orderGroupFirst->hide();

    _create->setEnabled(true);
  }
  else
    _planningType->append(1, "MRP", "M");

  if (!_metrics->boolean("MultiWhs"))
    _createPlannedTransfers->hide();

  sHandlePlanningType();

  _reorderLevel->setValidator(omfgThis->qtyVal());
  _orderUpToQty->setValidator(omfgThis->qtyVal());
  _minimumOrder->setValidator(omfgThis->qtyVal());
  _maximumOrder->setValidator(omfgThis->qtyVal());
  _orderMultiple->setValidator(omfgThis->qtyVal());
  _safetyStock->setValidator(omfgThis->qtyVal());

  _classCode->setType(ParameterGroup::ClassCode);

  _plannerCode->setAllowNull(true);
  _plannerCode->setType(XComboBox::PlannerCodes);

  _costcat->setAllowNull(true);
  _costcat->setType(XComboBox::CostCategories);

  _controlMethod->append(0, "None",     "N");
  _controlMethod->append(1, "Regular",  "R");
  _controlMethod->append(2, "Lot #",    "L");
  _controlMethod->append(3, "Serial #", "S");
  _controlMethod->setCurrentIndex(-1);

  _reorderLevel->setDouble(0.0);
  _orderUpToQty->setDouble(0.0);
  _minimumOrder->setDouble(0.0);
  _maximumOrder->setDouble(0.0);
  _orderMultiple->setDouble(0.0);
  _safetyStock->setDouble(0.0);

  _cycleCountFreq->setValue(0);
  _leadTime->setValue(0);

  _eventFence->setValue(_metrics->value("DefaultEventFence").toInt());
//  _costcat->setEnabled(_metrics->boolean("InterfaceToGL"));

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
    _create->setEnabled(true);
  }
  else
  {
    _warehouse->setAllowNull(true);
    _warehouse->setNull();
  }

    //If not lot serial control, remove options
  if (!_metrics->boolean("LotSerialControl"))
  {
    _controlMethod->removeItem(3);
    _controlMethod->removeItem(2);
    _autoNumberGroup->hide();
    _tab->removeTab(_tab->indexOf(_expireTab));
  }

  //If routings disabled, hide options
  if (!_metrics->boolean("Routings"))
  {
    _disallowBlankWIP->hide();
  }

  _costAvg->setVisible(_metrics->boolean("AllowAvgCostMethod"));
  _costStd->setVisible(_metrics->boolean("AllowStdCostMethod"));

  adjustSize();

}

createItemSitesByClassCode::~createItemSitesByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void createItemSitesByClassCode::languageChange()
{
  retranslateUi(this);
}

void createItemSitesByClassCode::sSave()
{
  XSqlQuery createSave;

  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(_warehouse->id() == -1, _warehouse,
                        tr("<p>You must select a Site for this Item Site "
                           "before creating it."))
       <<GuiErrorCheck(_costcat->id() == -1, _costcat,
                       tr("<p>You must select a Cost Category for these "
                          "Item Sites before you may create them."))
      <<GuiErrorCheck(_plannerCode->id() == -1, _plannerCode,
                      tr("<p>You must select a Planner Code for these "
                         "Item Sites before you may create them."))
      <<GuiErrorCheck(_controlMethod->currentIndex() == -1, _controlMethod,
                      tr("<p>You must select a Control Method for these "
                         "Item Sites before you may create them."))
      <<GuiErrorCheck(!_costNone->isChecked() && !_costAvg->isChecked()
                                        && !_costStd->isChecked(), _costNone,
                       tr("<p>You must select a Cost Method for this "
                          "Item Site before you may save it."))
      <<GuiErrorCheck(_stocked->isChecked() && _reorderLevel->toDouble() == 0, _reorderLevel,
                      tr("<p>You must set a reorder level "
                         "for a stocked item before you may save it."));

  if(GuiErrorCheck::reportErrors(this,tr("Cannot Save Item Site"),errors))
      return;

  if ( _locationControl->isChecked() )
  {
    XSqlQuery locationid;
    locationid.prepare( "SELECT location_id "
                        "FROM location "
                        "WHERE ((location_warehous_id=:warehous_id) "
                        " AND (location_active) ) "
                        "LIMIT 1;" );
    locationid.bindValue(":warehous_id", _warehouse->id());
    locationid.exec();
    if (!locationid.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Item Site"),
                             tr( "You must first create at least one valid "
				 "Location for this Site before items may be "
				 "multiply located." ) );
      return;
    }
  }

  QString sql( "INSERT INTO itemsite "
               "( itemsite_item_id,"
               "  itemsite_warehous_id, itemsite_qtyonhand, itemsite_value,"
               "  itemsite_useparams, itemsite_useparamsmanual,"
               "  itemsite_reorderlevel, itemsite_ordertoqty,"
               "  itemsite_minordqty, itemsite_maxordqty, itemsite_multordqty,"
               "  itemsite_safetystock, itemsite_cyclecountfreq,"
               "  itemsite_leadtime, itemsite_eventfence,"
               "  itemsite_plancode_id, itemsite_costcat_id,"
               "  itemsite_supply_itemsite_id,"
               "  itemsite_posupply, itemsite_wosupply,"
               "  itemsite_createpr, itemsite_createwo,"
               "  itemsite_sold, itemsite_soldranking,"
               "  itemsite_stocked, itemsite_disallowblankwip,"
               "  itemsite_controlmethod, itemsite_perishable, itemsite_active,"
               "  itemsite_loccntrl, itemsite_location_id, itemsite_location,"
               "  itemsite_location_comments, itemsite_notes,"
               "  itemsite_abcclass, itemsite_freeze, itemsite_datelastused,"
               "  itemsite_ordergroup, itemsite_ordergroup_first, itemsite_mps_timefence, "
               "  itemsite_autoabcclass, itemsite_planning_type, itemsite_costmethod, "
               "  itemsite_cosdefault, itemsite_lsseq_id ) "
               "SELECT item_id,"
               "       :warehous_id, 0.0, 0.0,"
               "       CASE WHEN item_type IN ('B', 'F', 'R', 'L', 'K') THEN false "
               "            ELSE :itemsite_useparams       END,"
               "       CASE WHEN item_type IN ('B', 'F', 'R', 'L', 'K') THEN false "
               "            ELSE :itemsite_useparamsmanual END,"
               "       CASE WHEN item_type IN ('B', 'F', 'R', 'L', 'K') THEN 0"
               "            ELSE :itemsite_reorderlevel END,"
               "       CASE WHEN item_type IN ('B', 'F', 'R', 'L', 'K') THEN 0"
               "            ELSE :itemsite_ordertoqty   END,"
               "       CASE WHEN item_type IN ('B', 'F', 'R', 'L', 'K') THEN 0"
               "            ELSE :itemsite_minordqty    END,"
               "       CASE WHEN item_type IN ('B', 'F', 'R', 'L', 'K') THEN 0"
               "            ELSE :itemsite_maxordqty    END,"
               "       CASE WHEN item_type IN ('B', 'F', 'R', 'L', 'K') THEN 0"
               "            ELSE :itemsite_multordqty   END,"
               "       :itemsite_safetystock, :itemsite_cyclecountfreq,"
               "       :itemsite_leadtime, :itemsite_eventfence,"
               "       :itemsite_plancode_id, :itemsite_costcat_id,"
               "       CASE WHEN COALESCE(:supplywhsid,-1) < 0  THEN NULL "
               "            ELSE (SELECT itemsite_id"
               "                    FROM itemsite"
               "                   WHERE ((itemsite_item_id=item_id)"
               "                      AND (itemsite_warehous_id=:supplywhsid)))"
               "       END,"
               "       :itemsite_posupply, :itemsite_wosupply,"
               "       :itemsite_createpr, :itemsite_createwo,"
               "       :itemsite_sold, :itemsite_soldranking,"
               "       :itemsite_stocked, :itemsite_disallowblankwip,"
               "       :itemsite_controlmethod, :itemsite_perishable, true,"
               "       CASE WHEN item_type IN ('B', 'F', 'R', 'L', 'K') "
               "                 THEN false "
               "            ELSE :itemsite_loccntrl END,"
               "       CASE WHEN item_type IN ('B', 'F', 'R', 'L', 'K') "
               "                 THEN -1 "
               "            ELSE :itemsite_location_id END,"
               "       CASE WHEN item_type IN ('B', 'F', 'R', 'L', 'K') "
               "                 THEN '' "
               "            ELSE :itemsite_location END,"
               "       :itemsite_location_comments, '',"
               "       :itemsite_abcclass, false, startOfTime(),"
               "       :itemsite_ordergroup, :itemsite_ordergroup_first, :itemsite_mps_timefence, "
               "       false, "
               "       CASE WHEN(item_type='L') THEN 'M' ELSE :itemsite_planning_type END, "
               "       CASE WHEN(item_type='R') THEN 'N' ELSE :itemsite_costmethod END, "
               "       :itemsite_cosdefault,  :itemsite_lsseq_id "
               "FROM item "
               "WHERE ( (item_id NOT IN ( SELECT itemsite_item_id"
               "                          FROM itemsite"
               "                          WHERE (itemsite_warehous_id=:warehous_id) ) )" );

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  sql += ");";

  createSave.prepare(sql);
  createSave.bindValue(":itemsite_reorderlevel", _reorderLevel->toDouble());
  createSave.bindValue(":itemsite_ordertoqty", _orderUpToQty->toDouble());
  createSave.bindValue(":itemsite_minordqty", _minimumOrder->toDouble());
  createSave.bindValue(":itemsite_maxordqty", _maximumOrder->toDouble());
  createSave.bindValue(":itemsite_multordqty", _orderMultiple->toDouble());
  createSave.bindValue(":itemsite_safetystock", _safetyStock->toDouble());
  createSave.bindValue(":itemsite_cyclecountfreq", _cycleCountFreq->value());
  createSave.bindValue(":itemsite_leadtime", _leadTime->value());
  createSave.bindValue(":itemsite_eventfence", _eventFence->value());
  createSave.bindValue(":itemsite_plancode_id", _plannerCode->id());
  createSave.bindValue(":itemsite_costcat_id", _costcat->id());

  if (_createPlannedTransfers->isChecked())
    createSave.bindValue(":supplywhsid", _suppliedFromSite->id());

  createSave.bindValue(":itemsite_useparams",     QVariant(_useParameters->isChecked()));
  createSave.bindValue(":itemsite_useparamsmanual", QVariant(_useParametersOnManual->isChecked()));
  createSave.bindValue(":itemsite_posupply",        QVariant(_poSupply->isChecked()));
  createSave.bindValue(":itemsite_wosupply",        QVariant(_woSupply->isChecked()));
//  createSave.bindValue(":itemsite_createpr",      QVariant(_createPr->isChecked()));
  createSave.bindValue(":itemsite_createwo",      QVariant(_createWo->isChecked()));
  createSave.bindValue(":itemsite_sold",          QVariant(_sold->isChecked()));
  createSave.bindValue(":itemsite_stocked",       QVariant(_stocked->isChecked()));
  createSave.bindValue(":itemsite_loccntrl",      QVariant(_locationControl->isChecked()));
  createSave.bindValue(":itemsite_disallowblankwip", QVariant((_locationControl->isChecked() && _disallowBlankWIP->isChecked())));
  createSave.bindValue(":itemsite_perishable",    QVariant(_perishable->isChecked()));
  createSave.bindValue(":itemsite_soldranking", _soldRanking->value());
  createSave.bindValue(":itemsite_location_comments", _locationComments->text().trimmed());
  createSave.bindValue(":itemsite_abcclass", _abcClass->currentText());

  createSave.bindValue(":itemsite_planning_type", _planningType->code());
  createSave.bindValue(":itemsite_ordergroup", _orderGroup->value());
  createSave.bindValue(":itemsite_ordergroup_first", QVariant(_orderGroupFirst->isChecked()));
  createSave.bindValue(":itemsite_mps_timefence", _mpsTimeFence->value());

  if (_useDefaultLocation->isChecked())
  {
    if (_location->isChecked())
    {
      createSave.bindValue(":itemsite_location", "");
      createSave.bindValue(":itemsite_location_id", _locations->id());
    }
    else if (_miscLocation->isChecked())
    {
      createSave.bindValue(":itemsite_location", _miscLocationName->text().trimmed());
      createSave.bindValue(":itemsite_location_id", -1);
    }
  }
  else
  {
    createSave.bindValue(":itemsite_location", "");
    createSave.bindValue(":itemsite_location_id", -1);
  }

  createSave.bindValue(":itemsite_controlmethod", _controlMethod->code());

  if(_costNone->isChecked())
    createSave.bindValue(":itemsite_costmethod", "N");
  else if(_costAvg->isChecked())
    createSave.bindValue(":itemsite_costmethod", "A");
  else if(_costStd->isChecked())
    createSave.bindValue(":itemsite_costmethod", "S");

  if (_woCostGroup->isChecked())
  {
    if (_todate->isChecked())
	  createSave.bindValue(":itemsite_cosdefault", QString("D"));
    else
      createSave.bindValue(":itemsite_cosdefault", QString("P"));
  }

  if (_sequence->isValid())
    createSave.bindValue(":itemsite_lsseq_id", _sequence->id());

  createSave.bindValue(":warehous_id", _warehouse->id());
  _classCode->bindValue(createSave);
  createSave.exec();

  omfgThis->sItemsitesUpdated();

  accept();
}

void createItemSitesByClassCode::sHandlePOSupply(bool /*pSupplied*/)
{
    /*
  if (pSupplied)
    _createPr->setEnabled(true);
  else
  {
    _createPr->setEnabled(false);
    _createPr->setChecked(false);
  }
  */
}

void createItemSitesByClassCode::sHandleWOSupply(bool pSupplied)
{
  if (pSupplied)
    _createWo->setEnabled(true);
  else
  {
    _createWo->setEnabled(false);
    _createWo->setChecked(false);
  }
}

void createItemSitesByClassCode::sHandleControlMethod()
{

  if (_controlMethod->currentIndex() == 0)
  {
    _costNone->setChecked(true);
    _costNone->setEnabled(true);
    _costAvg->setEnabled(false);
    _costStd->setEnabled(false);
  }
  else
  {
    if(_costStd->isVisibleTo(this) && !_costAvg->isChecked())
      _costStd->setChecked(true);
    else
      _costAvg->setChecked(true);
    _costNone->setEnabled(false);
    _costAvg->setEnabled(true);
    _costStd->setEnabled(true);
  }

  if ( (_controlMethod->currentIndex() == 2) ||
       (_controlMethod->currentIndex() == 3) )
    _perishable->setEnabled(true);
  else
  {
    _perishable->setChecked(false);
    _perishable->setEnabled(false);
  }
}

void createItemSitesByClassCode::sHandlePlanningType()
{
  if (_planningType->code() == "M" || _planningType->code() == "S")
  {
    _createPlannedTransfers->setEnabled(true);
    _orderGroup->setEnabled(true);
    _orderGroupFirst->setEnabled(true);

    if (_planningType->code() == "S")
      _mpsTimeFence->setEnabled(true);
    else
      _mpsTimeFence->setEnabled(false);
  }
  else
  {
    _createPlannedTransfers->setEnabled(false);
    _orderGroup->setEnabled(false);
    _orderGroupFirst->setEnabled(false);
    _mpsTimeFence->setEnabled(false);
  }

}

void createItemSitesByClassCode::populateLocations()
{
  XSqlQuery createpopulateLocations;
  createpopulateLocations.prepare( "SELECT location_id, formatLocationName(location_id) AS locationname "
             "FROM location "
             "WHERE ( (location_warehous_id=:warehous_id)"
             " AND (NOT location_restrict) "
             " AND (location_active) ) "
             "ORDER BY locationname;" );
  createpopulateLocations.bindValue(":warehous_id", _warehouse->id());
  createpopulateLocations.exec();
  _locations->populate(createpopulateLocations);

  if (_locations->count())
  {
    _location->setEnabled(true);
    _locations->setEnabled(true);
  }
  else
  {
    _location->setEnabled(false);
    _locations->setEnabled(false);
  }
}

void createItemSitesByClassCode::clear()
{
  _useParameters->setChecked(false);
  _useParametersOnManual->setChecked(false);
  _reorderLevel->setDouble(0.0);
  _orderUpToQty->setDouble(0.0);
  _minimumOrder->setDouble(0.0);
  _maximumOrder->setDouble(0.0);
  _orderMultiple->setDouble(0.0);
  _safetyStock->setDouble(0.0);

  _orderGroup->setValue(1);
  _orderGroupFirst->setChecked(false);

  _cycleCountFreq->setValue(0);
  _leadTime->setValue(0);
  _eventFence->setValue(_metrics->value("DefaultEventFence").toInt());

  _controlMethod->setCurrentIndex(1);
  sHandleControlMethod();
  _poSupply->setChecked(true);
  _woSupply->setChecked(true);
  _sold->setChecked(true);
  _stocked->setChecked(false);

  _locationControl->setChecked(false);
  _useDefaultLocation->setChecked(false);
  _miscLocationName->clear();
  _locationComments->clear();

  _costcat->setId(-1);

  _woCostGroup->setChecked(false);

  populateLocations();
}
