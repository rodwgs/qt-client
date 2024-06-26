/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedOpenARItems.h"

#include <QAction>
#include <QMenu>
#include "guiErrorCheck.h"
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <datecluster.h>

#include "printStatementByCustomer.h"
#include "dspAROpenItems.h"
#include "errorReporter.h"

dspTimePhasedOpenARItems::dspTimePhasedOpenARItems(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspTimePhasedOpenARItems", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Receivables Aging"));
  setReportName("ARAging");

  connect(_custom, SIGNAL(toggled(bool)), this, SLOT(sToggleCustom()));
  connect(_detailedReport, SIGNAL(clicked()), this, SLOT(sToggleReport()));
  connect(_summaryReport, SIGNAL(clicked()), this, SLOT(sToggleReport()));
  
  list()->addColumn(tr("Cust. #"),  _orderColumn, Qt::AlignLeft, true, "araging_cust_number" );
  list()->addColumn(tr("Customer"), -1,          Qt::AlignLeft, true, "araging_cust_name" );
  
  _asOf->setDate(omfgThis->dbDate(), true);
  sToggleCustom();

  if(_preferences->value("ARAgingDefaultDate") == "doc")
    _useDocDate->setChecked(true);
  else
    _useDistDate->setChecked(true);
}

dspTimePhasedOpenARItems::~dspTimePhasedOpenARItems()
{
  // no need to delete child widgets, Qt does it all for us
  QString str("dist");
  if(_useDocDate->isChecked())
    str = "doc";
  _preferences->set("ARAgingDefaultDate", str);
}

bool dspTimePhasedOpenARItems::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  QList<GuiErrorCheck> errors;
  errors<< GuiErrorCheck((_custom->isChecked() && ! _periods->isPeriodSelected()) || (!_custom->isChecked() && !_asOf->isValid()), _custom,
                         tr("The criteria you specified are not complete. "
                             "Please make sure all fields are correctly filled "
                             "out before running the report."))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Incomplete criteria"), errors))
    return false;

  _customerSelector->appendValue(params);

  if(_custom->isChecked())
  {
    params.append("report_name", "TimePhasedOpenARItems");

    QList<XTreeWidgetItem*> selected = _periods->selectedItems();
    QList<QVariant> periodList;
    for (int i = 0; i < selected.size(); i++)
      periodList.append(((XTreeWidgetItem*)selected[i])->id());
    params.append("period_id_list", periodList);
  }
  else
  {
    params.append("report_name", "ARAging");
    params.append("relDate", _asOf->date());
  }

  // have both in case we add a third option
  params.append("useDocDate",  QVariant(_useDocDate->isChecked()));
  params.append("useDistDate", QVariant(_useDistDate->isChecked()));

  if (_excludeNegs->isChecked())
  {
    params.append("no_creditMemo", "");
    params.append("no_cashdeposit", "");
  }

  return true;
}

void dspTimePhasedOpenARItems::sViewOpenItems()
{
  ParameterList params;
  params.append("cust_id", list()->id());
  if (_custom->isChecked())
  {
    params.append("startDate", _periods->getSelected(_column - 1)->startDate());
    params.append("endDate", _periods->getSelected(_column - 1)->endDate());
  }
  else
  {
    QDate asOfDate;
    asOfDate = _asOf->date();
    if (_column == 3)
      params.append("startDate", asOfDate );
    else if (_column == 4)
    {
      params.append("startDate", asOfDate.addDays(-30) );
      params.append("endDate", asOfDate.addDays(-1));
    }
    else if (_column == 5)
    {
      params.append("startDate",asOfDate.addDays(-60) );
      params.append("endDate", asOfDate.addDays(-31));
    }
    else if (_column == 6)
    {
      params.append("startDate",asOfDate.addDays(-90) );
      params.append("endDate", asOfDate.addDays(-61));
    }
    else if (_column == 7)
      params.append("endDate",asOfDate.addDays(-91) );
  }
  params.append("byDueDate");
  params.append("run");
  params.append("asofDate", _asOf->date());

  dspAROpenItems *newdlg = new dspAROpenItems();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedOpenARItems::sPrintStatement()
{
  ParameterList params;
  params.append("cust_id", list()->id());
  params.append("asofDate", _asOf->date());
  params.append("print");

  printStatementByCustomer newdlg(this, "", true);
  newdlg.set(params);
}

void dspTimePhasedOpenARItems::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
{
  //QAction *menuItem;
  _column = pColumn;
  
  if ((_custom->isChecked()) && (_column == list()->column("linetotal")))
    return;

  if ((_column > 1) && (list()->id() > 0))
  {
    (void)pMenu->addAction(tr("View Open Items..."), this, SLOT(sViewOpenItems()));

    pMenu->addSeparator();

    (void)pMenu->addAction(tr("Print Statement..."), this, SLOT(sPrintStatement()));
  }
}

void dspTimePhasedOpenARItems::sFillList()
{
  if (_custom->isChecked())
    sFillCustom();
  else
    sFillStd();
}

void dspTimePhasedOpenARItems::sFillCustom()
{
  XSqlQuery dspFillCustom;
  if (!_periods->isPeriodSelected())
  {
    QList<GuiErrorCheck> errors;
    errors<< GuiErrorCheck(isVisible(), _periods,
                         tr("Please select one or more Calendar Periods"))
    ;
    if (GuiErrorCheck::reportErrors(this, tr("Select Calendar Periods"), errors))
     return ;
  }

  _columnDates.clear();
  list()->setColumnCount(2);

  QString sql("SELECT cust_id, cust_number, cust_name");
  QStringList linetotal;

  int columns = 1;
  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket%1").arg(columns++);
    sql += QString(", openARItemsValue(cust_id, %2) AS %1,"
                   " 'curr' AS %3_xtnumericrole, 0 AS %4_xttotalrole")
	   .arg(bucketname)
	   .arg(cursor->id())
           .arg(bucketname)
           .arg(bucketname);

    list()->addColumn(formatDate(cursor->startDate()), _bigMoneyColumn, Qt::AlignRight, true, bucketname);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
    linetotal << QString("openARItemsValue(cust_id, %2)").arg(cursor->id());
  }

  list()->addColumn(tr("Total"), _bigMoneyColumn, Qt::AlignRight, true, "linetotal");

  sql += ", " + linetotal.join("+") + " AS linetotal,"
         " 'curr' AS linetotal_xtnumericrole,"
         " 0 AS linetotal_xttotalrole,"
         " (" + linetotal.join("+") + ") = 0.0 AS xthiddenrole "
         "FROM custinfo LEFT OUTER JOIN custgrpitem ON (cust_id = custgrpitem_cust_id) "
         "<? if exists('cust_id') ?>"
         "WHERE (cust_id=<? value('cust_id') ?>)"
         "<? elseif exists('custtype_id') ?>"
         "WHERE (cust_custtype_id=<? value('custtype_id') ?>)"
         "<? elseif exists('custgrp_id') ?>"
		 "WHERE (custgrpitem_custgrp_id=<? value('custgrp_id') ?>)"
         "<? elseif exists('custtype_pattern') ?>"
         "WHERE (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ <? value('custtype_pattern') ?>))) "
         "<? endif ?>"
         "ORDER BY cust_number;";

  MetaSQLQuery mql(sql);
  ParameterList params;
  if (! setParams(params))
    return;
  dspFillCustom = mql.toQuery(params);
  list()->populate(dspFillCustom);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Information"),
                                dspFillCustom, __FILE__, __LINE__))
  {
    return;
  }
}

void dspTimePhasedOpenARItems::sFillStd()
{
  XSqlQuery dspFillStd;
  MetaSQLQuery mql = mqlLoad("arAging", "detail");
  ParameterList params;
  if (! setParams(params))
    return;

  dspFillStd = mql.toQuery(params);
  list()->populate(dspFillStd);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Information"),
                                dspFillStd, __FILE__, __LINE__))
  {
    return;
  }
}

void dspTimePhasedOpenARItems::sToggleCustom()
{
  list()->clear();
  if (_custom->isChecked())
  {
    setReportName("TimePhasedOpenARItems");
    _calendarLit->setHidden(false);
    _calendar->setHidden(false);
    _periods->setHidden(false);
    _asOf->setDate(omfgThis->dbDate(), true);
    _asOf->setEnabled(false);
    _useGroup->setHidden(true);
    _reptGroup->setHidden(true);

    list()->setColumnCount(0);
    list()->addColumn(tr("Cust. #"), _orderColumn, Qt::AlignLeft, true, "cust_number");
    list()->addColumn(tr("Customer"),         180, Qt::AlignLeft, true, "cust_name");
  }
  else
  {
    sToggleReport();
    _calendarLit->setHidden(true);
    _calendar->setHidden(true);
    _periods->setHidden(true);
    _asOf->setEnabled(true);
    _useGroup->setHidden(false);
    _reptGroup->setHidden(false);

    list()->setColumnCount(0);
    list()->addColumn(tr("Cust. #"),       _orderColumn, Qt::AlignLeft,  true, "araging_cust_number");
    list()->addColumn(tr("Customer"),               180, Qt::AlignLeft,  true, "araging_cust_name");
    list()->addColumn(tr("Total Open"), _bigMoneyColumn, Qt::AlignRight, true, "araging_total_val_sum");
    list()->addColumn(tr("0+ Days"),    _bigMoneyColumn, Qt::AlignRight, true, "araging_cur_val_sum");
    list()->addColumn(tr("0-30 Days"),  _bigMoneyColumn, Qt::AlignRight, true, "araging_thirty_val_sum");
    list()->addColumn(tr("31-60 Days"), _bigMoneyColumn, Qt::AlignRight, true, "araging_sixty_val_sum");
    list()->addColumn(tr("61-90 Days"), _bigMoneyColumn, Qt::AlignRight, true, "araging_ninety_val_sum");
    list()->addColumn(tr("90+ Days"),   _bigMoneyColumn, Qt::AlignRight, true, "araging_plus_val_sum");
  }
}

void dspTimePhasedOpenARItems::sToggleReport()
{
  if (_detailedReport->isChecked())
    setReportName("ARAging");
  else
    setReportName("ARAgingSummary");
}
