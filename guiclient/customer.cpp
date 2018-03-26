/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customer.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <comment.h>
#include <metasql.h>
#include <openreports.h>

#include "addresscluster.h"
#include "characteristicAssignment.h"
#include "creditCard.h"
#include "crmaccount.h"
#include "custCharacteristicDelegate.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "mqlutil.h"
#include "shipTo.h"
#include "storedProcErrorLookup.h"
#include "taxRegistration.h"
#include "xcombobox.h"
#include "parameterwidget.h"

bool customer::userHasPriv(const int pMode, const int pId)
{
  Q_UNUSED(pId);
  bool priv = _privileges->check("MaintainCustomerMasters");
  if(pMode==cView)
    priv = priv || _privileges->check("ViewCustomerMasters");
  return priv;
}

customer::customer(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);
  _number->setShowInactive(true);

  _todoList = new todoList(this, "todoList", Qt::Widget);
  _todoListPage->layout()->addWidget(_todoList);
  _todoList->setCloseVisible(false);
  _todoList->setParameterWidgetVisible(false);
  _todoList->setQueryOnStartEnabled(false);
  _todoList->parameterWidget()->setDefault(tr("User"), QVariant(), true);
  _todoList->parameterWidget()->append("", "hasContext", ParameterWidget::Exists, true);
  _todoList->list()->hideColumn("crmacct_number");
  _todoList->list()->hideColumn("crmacct_name");
  _todoList->_projects->setForgetful(true);
  _todoList->_projects->setVisible(false);
  _todoList->_projects->setChecked(false);

  _contacts = new contacts(this, "contacts", Qt::Widget);
  _contactsPage->layout()->addWidget(_contacts);
  _contacts->setCloseVisible(false);
  _contacts->list()->hideColumn("crmacct_number");
  _contacts->list()->hideColumn("crmacct_name");
  _contacts->parameterWidget()->append("", "hasContext", ParameterWidget::Exists, true);
  _contacts->setParameterWidgetVisible(false);
  _contacts->setQueryOnStartEnabled(false);

  _quotes = new quotes(this, "quotes", Qt::Widget);
  _quotesPage->layout()->addWidget(_quotes);
  _quotes->setCloseVisible(false);
  _quotes->setParameterWidgetVisible(false);
  _quotes->setQueryOnStartEnabled(false);

  if (_metrics->boolean("ShowQuotesAfterSO"))
  {
    _quotes->findChild<XCheckBox*>("_convertedtoSo")->show();
    _quotes->findChild<XCheckBox*>("_convertedtoSo")->setForgetful(true);
    _quotes->findChild<XCheckBox*>("_convertedtoSo")->setChecked(false);
  }
  _quotes->list()->hideColumn("quhead_billtoname");

  _orders = new openSalesOrders(this, "openSalesOrders", Qt::Widget);
  _ordersPage->layout()->addWidget(_orders);
  _orders->setCloseVisible(false);
  _orders->setParameterWidgetVisible(false);
  _orders->setQueryOnStartEnabled(false);
  _orders->setAutoUpdateEnabled(false);
  _orders->optionsWidget()->show();
  _orders->list()->hideColumn("cust_number");
  _orders->list()->hideColumn("cohead_billtoname");

  _returns = new returnAuthorizationWorkbench(this, "returnAuthorizationWorkbench", Qt::Widget);
  _returnsPage->layout()->addWidget(_returns);
  _returns->findChild<QWidget*>("_close")->hide();
  _returns->findChild<QWidget*>("_customerSelector")->hide();
  _returns->findChild<XTreeWidget*>("_ra")->hideColumn("cust_name");
  _returns->findChild<XTreeWidget*>("_radue")->hideColumn("cust_name");

  _aritems = new dspAROpenItems(this, "dspAROpenItems", Qt::Widget);
  _aritems->setObjectName("dspAROpenItems");
  _aritemsPage->layout()->addWidget(_aritems);
  _aritems->setCloseVisible(false);
  _aritems->findChild<QWidget*>("_customerSelector")->hide();
//  _aritems->findChild<QWidget*>("_asofGroup")->hide();
  _aritems->findChild<DLineEdit*>("_asOf")->setDate(omfgThis->dbDate(), true);
  _aritems->findChild<XCheckBox*>("_closed")->show();
  _aritems->list()->hideColumn("cust_number");
  _aritems->list()->hideColumn("cust_name");

  _cashreceipts = new dspCashReceipts(this, "dspCashReceipts", Qt::Widget);
  _cashreceiptsPage->layout()->addWidget(_cashreceipts);
  _cashreceipts->setCloseVisible(false);
  _cashreceipts->findChild<QWidget*>("_customerSelector")->hide();
  _cashreceipts->findChild<DateCluster*>("_dates")->setStartDate(QDate().currentDate().addDays(-90));
  _cashreceipts->list()->hideColumn("cust_number");
  _cashreceipts->list()->hideColumn("cust_name");

  _cctrans = new dspCreditCardTransactions(this, "dspCreditCardTransactions", Qt::Widget);
  _cctransPage->layout()->addWidget(_cctrans);
  _cctrans->findChild<QWidget*>("_close")->hide();
  _cctrans->findChild<QWidget*>("_customerSelector")->hide();
  _cctrans->findChild<XTreeWidget*>("_preauth")->hideColumn("cust_number");
  _cctrans->findChild<XTreeWidget*>("_preauth")->hideColumn("cust_name");

  connect(this, SIGNAL(newId(int)), this, SLOT(sIdChanged(int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(sCancel()));
  connect(_crmacct, SIGNAL(clicked()), this, SLOT(sCrmAccount()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSaveClicked()));
  connect(_number, SIGNAL(newId(int)), this, SLOT(setId(int)));
  connect(_number, SIGNAL(editingFinished()), this, SLOT(sNumberEdited()));
  connect(_number, SIGNAL(editable(bool)), this, SLOT(sNumberEditable(bool)));
  connect(_salesrep, SIGNAL(newID(int)), this, SLOT(sPopulateCommission()));
  connect(_setSalesRepToShipTo, SIGNAL(clicked()), this, SLOT(sSetSalesRepToAllShipTo()));
  connect(_newShipto, SIGNAL(clicked()), this, SLOT(sNewShipto()));
  connect(_editShipto, SIGNAL(clicked()), this, SLOT(sEditShipto()));
  connect(_viewShipto, SIGNAL(clicked()), this, SLOT(sViewShipto()));
  connect(_deleteShipto, SIGNAL(clicked()), this, SLOT(sDeleteShipto()));
  connect(_shipto, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateShiptoMenu(QMenu*)));
  connect(_print,       SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_printShipto, SIGNAL(clicked()), this, SLOT(sPrintShipto()));
  connect(_downCC, SIGNAL(clicked()), this, SLOT(sMoveDown()));
  connect(_upCC, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_viewCC, SIGNAL(clicked()), this, SLOT(sViewCreditCard()));
  connect(_editCC, SIGNAL(clicked()), this, SLOT(sEditCreditCard()));
  connect(_deleteCC, SIGNAL(clicked()), this, SLOT(sDeleteCreditCard()));
  connect(_newCC, SIGNAL(clicked()), this, SLOT(sNewCreditCard()));
  connect(_deleteCharacteristic, SIGNAL(clicked()), this, SLOT(sDeleteCharacteristic()));
  connect(_editCharacteristic, SIGNAL(clicked()), this, SLOT(sEditCharacteristic()));
  connect(_newCharacteristic, SIGNAL(clicked()), this, SLOT(sNewCharacteristic()));
  connect(_deleteTaxreg, SIGNAL(clicked()), this, SLOT(sDeleteTaxreg()));
  connect(_editTaxreg,   SIGNAL(clicked()), this, SLOT(sEditTaxreg()));
  connect(_newTaxreg,    SIGNAL(clicked()), this, SLOT(sNewTaxreg()));
  connect(_viewTaxreg,   SIGNAL(clicked()), this, SLOT(sViewTaxreg()));
  connect(_custtype, SIGNAL(currentIndexChanged(int)), this, SLOT(sFillCharacteristicList()));
  connect(_billingButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_correspButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_shiptoButton,  SIGNAL(clicked()), this, SLOT(sHandleButtons()));

  connect(_generalButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_termsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_taxButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_creditcardsButton,  SIGNAL(clicked()), this, SLOT(sHandleButtons()));

  connect(_number,  SIGNAL(editingFinished()), this, SLOT(sCheckRequired()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheckRequired()));
  connect(_salesrep, SIGNAL(newID(int)), this, SLOT(sCheckRequired()));
  connect(_terms, SIGNAL(newID(int)), this, SLOT(sCheckRequired()));
  connect(_custtype, SIGNAL(newID(int)), this, SLOT(sCheckRequired()));

  connect(_contactsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_todoListButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_notesButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_commentsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_summaryButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_quotesButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_ordersButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_returnsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_aritemsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_cctransButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_cashreceiptsButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_tab, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

  _custid = -1;
  _crmacctid = -1;
  _NumberGen = -1;
  _autoSaved = false;
  _captive = false;
  _charfilled = false;
  _mode       = -1;

  _currency->setLabel(_currencyLit);

  _balanceMethod->append(0, tr("Balance Forward"), "B");
  _balanceMethod->append(1, tr("Open Items"),      "O");

  _taxreg->addColumn(tr("Tax Authority"), 100, Qt::AlignLeft, true, "taxauth_code");
  _taxreg->addColumn(tr("Registration #"), -1, Qt::AlignLeft, true, "taxreg_number");

  _shipto->addColumn(tr("Default"), _itemColumn, Qt::AlignLeft, true, "shipto_default");
  _shipto->addColumn(tr("Active"),  _itemColumn, Qt::AlignLeft, true, "shipto_active");  
  _shipto->addColumn(tr("Number"),  _itemColumn, Qt::AlignLeft, true, "shipto_num");
  _shipto->addColumn(tr("Name"),            150, Qt::AlignLeft, true, "shipto_name");
  _shipto->addColumn(tr("Address"),         150, Qt::AlignLeft, true, "addr_line1");
  _shipto->addColumn(tr("City, State, Zip"), -1, Qt::AlignLeft, true, "shipto_csz");

  _cc->addColumn(tr("Sequence"),_itemColumn, Qt::AlignLeft, true, "ccard_seq");
  _cc->addColumn(tr("Type"),    _itemColumn, Qt::AlignLeft, true, "type");
  _cc->addColumn(tr("Number"),          150, Qt::AlignRight,true, "f_number");
  _cc->addColumn(tr("Active"),           -1, Qt::AlignLeft, true, "ccard_active");

  _charass->addColumn(tr("Characteristic"), _itemColumn*2, Qt::AlignLeft, true, "char_name");
  _charass->addColumn(tr("Value"),          -1,            Qt::AlignLeft, true, "charass_value");

  _defaultCommissionPrcnt->setValidator(omfgThis->percentVal());
  _defaultDiscountPrcnt->setValidator(omfgThis->negPercentVal());

  _custchar = new QStandardItemModel(0, 2, this);
  _custchar->setHeaderData( 0, Qt::Horizontal, tr("Characteristic"), Qt::DisplayRole);
  _custchar->setHeaderData( 1, Qt::Horizontal, tr("Value"), Qt::DisplayRole);
  _chartempl->setModel(_custchar);
  CustCharacteristicDelegate * delegate = new CustCharacteristicDelegate(this);
  _chartempl->setItemDelegate(delegate);

  key = omfgThis->_key;
  if(!_metrics->boolean("CCAccept") || !_privileges->check("ProcessCreditCards"))
  {
    _creditcardsButton->hide();
    _cctransButton->hide();
  }

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _sellingWarehouseLit->hide();
    _sellingWarehouse->hide();
  }
  else
    _sellingWarehouse->setId(_preferences->value("PreferredWarehouse").toInt());

  if(!_metrics->boolean("AutoCreditWarnLateCustomers"))
    _warnLate->hide();
  else
    _graceDays->setValue(_metrics->value("DefaultAutoCreditWarnGraceDays").toInt());

  if (!_privileges->check("MaintainQuotes") && !_privileges->check("ViewQuotes"))
    _quotesButton->setEnabled(false);
  if (!_privileges->check("MaintainSalesOrders") && !_privileges->check("ViewSalesOrders"))
    _ordersButton->setEnabled(false);
  if (!_privileges->check("MaintainReturns") && !_privileges->check("ViewReturns"))
    _returnsButton->setEnabled(false);
  if (!_metrics->boolean("EnableReturnAuth"))
    _returnsButton->hide();

  setValid(false);

  _backlog->setPrecision(omfgThis->moneyVal());
  _lastYearSales->setPrecision(omfgThis->moneyVal());
  _lateBalance->setPrecision(omfgThis->moneyVal());
  _openBalance->setPrecision(omfgThis->moneyVal());
  _ytdSales->setPrecision(omfgThis->moneyVal());

  _chartempl->setAlternatingRowColors(true);
}

customer::~customer()
{
  // no need to delete child widgets, Qt does it all for us
}

void customer::languageChange()
{
  retranslateUi(this);
}

/* because the customer window is a workbench, which allows switching
   between customers and changing customer numbers and all that stuff,
   we can't handle crmaccount creation via set() and populate() the
   way we do with other crmaccount types.
 */
enum SetResponse customer::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _number->setEditMode(true);
    setId(param.toInt());
    _captive=true;
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _salesrep->setType(XComboBox::SalesRepsActive);

      sClear();

      connect(_shipto, SIGNAL(valid(bool)), _editShipto, SLOT(setEnabled(bool)));
      connect(_shipto, SIGNAL(valid(bool)), _deleteShipto, SLOT(setEnabled(bool)));
      connect(_shipto, SIGNAL(itemSelected(int)), _editShipto, SLOT(animateClick()));
      connect(_cc, SIGNAL(valid(bool)), _editCC, SLOT(setEnabled(bool)));
      connect(_cc, SIGNAL(valid(bool)), _deleteCC, SLOT(setEnabled(bool)));
      connect(_cc, SIGNAL(itemSelected(int)), _editCC, SLOT(animateClick()));
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      connect(_backorders, SIGNAL(toggled(bool)), _partialShipments, SLOT(setEnabled(bool)));
      connect(_backorders, SIGNAL(toggled(bool)), _partialShipments, SLOT(setChecked(bool)));

      emit newMode(_mode);
      emit newId(_custid);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_shipto, SIGNAL(valid(bool)), _editShipto, SLOT(setEnabled(bool)));
      connect(_shipto, SIGNAL(valid(bool)), _deleteShipto, SLOT(setEnabled(bool)));
      connect(_shipto, SIGNAL(itemSelected(int)), _editShipto, SLOT(animateClick()));
      connect(_cc, SIGNAL(valid(bool)), _editCC, SLOT(setEnabled(bool)));
      connect(_cc, SIGNAL(valid(bool)), _deleteCC, SLOT(setEnabled(bool)));
      connect(_cc, SIGNAL(itemSelected(int)), _editCC, SLOT(animateClick()));
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      connect(_backorders, SIGNAL(toggled(bool)), _partialShipments, SLOT(setEnabled(bool)));
      connect(_backorders, SIGNAL(toggled(bool)), _partialShipments, SLOT(setChecked(bool)));

      emit newMode(_mode);
    }
    else if (param.toString() == "view")
    {
      setViewMode();
    }
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _number->setEditMode(true);
    sLoadCrmAcct(param.toInt());
    _captive=true;
  }

  if (_mode == cEdit && _custid > 0)
  {
    if (!_lock.acquire("custinfo", _custid, AppLock::Interactive))
    {
      setViewMode();
    }
  }

  return NoError;
}

int customer::id() const
{
  return _custid;
}

/** \return one of cNew, cEdit, cView, ...
    \todo   change possible modes to an enum in guiclient.h (and add cUnknown?)
 */
int customer::mode() const
{
  return _mode;
}

void customer::setViewMode()
{
  _mode = cView;

  _number->setCanEdit(false);
  _name->setEnabled(false);
  _custtype->setEnabled(false);
  _active->setEnabled(false);
  _corrCntct->setEnabled(false);
  _billCntct->setEnabled(false);
  _terms->setEnabled(false);
  _balanceMethod->setEnabled(false);
  _defaultDiscountPrcnt->setEnabled(false);
  _creditLimit->setEnabled(false);
  _creditRating->setEnabled(false);
  _creditStatusGroup->setEnabled(false);
  _autoUpdateStatus->setEnabled(false);
  _autoHoldOrders->setEnabled(false);
  _taxzone->setEnabled(false);
  _sellingWarehouse->setEnabled(false);
  _salesrep->setEnabled(false);
  _defaultCommissionPrcnt->setEnabled(false);
  _shipvia->setEnabled(false);
  _shipform->setEnabled(false);
  _shipchrg->setEnabled(false);
  _backorders->setEnabled(false);
  _usesPOs->setEnabled(false);
  _blanketPos->setEnabled(false);
  _allowFFShipto->setEnabled(false);
  _allowFFBillto->setEnabled(false);
  _notes->setReadOnly(true);
  _comments->setReadOnly(true);
  _newShipto->setEnabled(false);
  _newCharacteristic->setEnabled(false);
  _newTaxreg->setEnabled(false);
  _currency->setEnabled(false);
  _partialShipments->setEnabled(false);
  _save->hide();
  _newCC->setEnabled(false);
  _editCC->setEnabled(false);
  _deleteCC->setEnabled(false);
  _upCC->setEnabled(false);
  _downCC->setEnabled(false);
  _warnLate->setEnabled(false);
  _charass->setEnabled(false);
  _chartempl->setEnabled(false);

  connect(_shipto, SIGNAL(itemSelected(int)), _viewShipto, SLOT(animateClick()));
  connect(_cc, SIGNAL(itemSelected(int)), _viewCC, SLOT(animateClick()));

  disconnect(_taxreg, SIGNAL(valid(bool)), _deleteTaxreg, SLOT(setEnabled(bool)));
  disconnect(_taxreg, SIGNAL(valid(bool)), _editTaxreg, SLOT(setEnabled(bool)));
  disconnect(_taxreg, SIGNAL(itemSelected(int)), _editTaxreg, SLOT(animateClick()));
  connect(_taxreg, SIGNAL(itemSelected(int)), _viewTaxreg, SLOT(animateClick()));

  ParameterList params;
  params.append("mode", "view");
  _contacts->set(params);

  emit newMode(_mode);
}

void customer::setValid(bool valid)
{
  _print->setEnabled(valid);
  _shiptoPage->setEnabled(valid);
  _taxPage->setEnabled(valid);
  _creditcardsPage->setEnabled(valid);
  _comments->setEnabled(valid);
  _tab->setTabEnabled(_tab->indexOf(_documentsTab),valid);
  _tab->setTabEnabled(_tab->indexOf(_crmTab),valid);
  _tab->setTabEnabled(_tab->indexOf(_salesTab),valid);
  _tab->setTabEnabled(_tab->indexOf(_accountingTab),valid);

  if (!_privileges->check("MaintainAllContacts") && !_privileges->check("ViewAllContacts") &&
      !_privileges->check("MaintainPersonalContacts") && !_privileges->check("ViewPersonalContacts"))
  {
    _contactsButton->setEnabled(false);
    _todoListButton->setChecked(true);
    sHandleButtons();
  }
  if (!_privileges->check("MaintainAllToDoItems") && !_privileges->check("ViewAllToDoItems") &&
      !_privileges->check("MaintainPersonalToDoItems") && !_privileges->check("ViewPersonalToDoItems"))
  {
    _todoListButton->setEnabled(false);
    if (_todoListButton->isChecked())
      _tab->setTabEnabled(_tab->indexOf(_crmTab),false);
  }

  if (!_privileges->check("EditAROpenItems") && !_privileges->check("ViewAROpenItems"))
  {
    if (_cctransButton->isHidden())
      _tab->setTabEnabled(_tab->indexOf(_accountingTab),false);
    else
    {
      _aritemsButton->setEnabled(false);
      _cashreceiptsButton->setEnabled(false);
      _cctransButton->setChecked(true);
      sHandleButtons();
    }
  }

  if (!valid)
  {
    _documents->setId(-1);
    _todoList->list()->clear();
    _contacts->list()->clear();
    _quotes->list()->clear();
    _orders->list()->clear();
    _returns->findChild<XTreeWidget*>("_ra")->clear();
    _returns->findChild<XTreeWidget*>("_radue")->clear();
    _aritems->list()->clear();
    _cashreceipts->list()->clear();
    _cctrans->findChild<XTreeWidget*>("_preauth")->clear();
  }
}

bool customer::sSave()
{
  XSqlQuery customerSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_number->number().trimmed().isEmpty(), _number,
                          tr("You must enter a number for this Customer "
                             "before continuing"))
         << GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                          tr("You must enter a name for this Customer "
                             "before continuing"))
         << GuiErrorCheck(_custtype->id() == -1, _custtype,
                          tr("You must select a Customer Type code for this "
                             "Customer before continuing."))
         << GuiErrorCheck(_terms->id() == -1, _terms,
                          tr("You must select a Terms code for this "
                             "Customer before continuing."))
         << GuiErrorCheck(_salesrep->id() == -1, _salesrep,
                          tr("You must select a Sales Rep. for this "
                             "Customer before continuing."))
     ;

  if (_number->number().trimmed() != _cachedNumber)
  {
    XSqlQuery dupq;
    dupq.prepare("SELECT cust_name "
                 "FROM custinfo "
                 "WHERE (UPPER(cust_number)=UPPER(:cust_number)) "
                 "  AND (cust_id<>:cust_id);" );
    dupq.bindValue(":cust_name", _number->number().trimmed());
    dupq.bindValue(":cust_id", _custid);
    dupq.exec();
    if (dupq.first())
      errors << GuiErrorCheck(true, _number,
                              tr("<p>The newly entered Customer Number cannot "
                                 "be used as it is currently in use by the "
                                 "Customer '%1'.  Please correct or enter a "
                                 "new Customer Number." )
                                .arg(dupq.value("cust_name").toString()) );
  }
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Customer"), errors))
    return false;

  int custtype_id = -1;
  bool charProfile = false;

  if (_mode == cEdit)
  { 
    XSqlQuery customerType;

    customerType.prepare("SELECT cust_custtype_id, custtype_char"
                         "  FROM custinfo JOIN custtype ON (cust_custtype_id=custtype_id)"
                         " WHERE cust_id=:cust_id;");
    customerType.bindValue(":cust_id", _custid);
    customerType.exec();
    if (customerType.first())
    {
      custtype_id = customerType.value("cust_custtype_id").toInt();
      charProfile = customerType.value("custtype_char").toBool();
    }

    customerSave.prepare( "UPDATE custinfo SET "
               "       cust_number=:cust_number, cust_name=:cust_name,"
               "       cust_salesrep_id=:cust_salesrep_id,"
               "       cust_corrcntct_id=:cust_corrcntct_id, cust_cntct_id=:cust_cntct_id,"
               "       cust_custtype_id=:cust_custtype_id, cust_balmethod=:cust_balmethod,"
               "       cust_creditlmt=:cust_creditlmt, cust_creditlmt_curr_id=:cust_creditlmt_curr_id,"
               "       cust_creditrating=:cust_creditrating,"
               "       cust_autoupdatestatus=:cust_autoupdatestatus, cust_autoholdorders=:cust_autoholdorders,"
               "       cust_creditstatus=:cust_creditstatus,"
               "       cust_backorder=:cust_backorder, cust_ffshipto=:cust_ffshipto, cust_ffbillto=:cust_ffbillto,"
               "       cust_commprcnt=:cust_commprcnt,"
               "       cust_partialship=:cust_partialship, cust_shipvia=:cust_shipvia,"
               "       cust_shipchrg_id=:cust_shipchrg_id, cust_shipform_id=:cust_shipform_id,"
               "       cust_terms_id=:cust_terms_id,"
               "       cust_discntprcnt=:cust_discntprcnt,"
               "       cust_taxzone_id=:cust_taxzone_id, "
               "       cust_active=:cust_active, cust_usespos=:cust_usespos,"
               "       cust_blanketpos=:cust_blanketpos, cust_comments=:cust_comments,"
               "       cust_preferred_warehous_id=:cust_preferred_warehous_id, "
               "       cust_gracedays=:cust_gracedays,"
               "       cust_curr_id=:cust_curr_id,"
               "       cust_financecharge=:cust_financecharge "
               "WHERE (cust_id=:cust_id);" );
  }
  else
    customerSave.prepare( "INSERT INTO custinfo "
               "( cust_id, cust_number,"
               "  cust_salesrep_id, cust_name,"
               "  cust_corrcntct_id, cust_cntct_id,"
               "  cust_custtype_id, cust_balmethod,"
               "  cust_creditlmt, cust_creditlmt_curr_id,"
               "  cust_creditrating, cust_creditstatus,"
               "  cust_autoupdatestatus, cust_autoholdorders,"
               "  cust_backorder, cust_ffshipto, cust_ffbillto,"
               "  cust_commprcnt, cust_partialship,"
               "  cust_shipvia,"
               "  cust_shipchrg_id, cust_shipform_id, cust_terms_id,"
               "  cust_discntprcnt, cust_taxzone_id, "
               "  cust_active, cust_usespos, cust_blanketpos, cust_comments,"
               "  cust_preferred_warehous_id, "
               "  cust_gracedays, cust_curr_id, cust_financecharge ) "
               "VALUES "
               "( :cust_id, :cust_number,"
               "  :cust_salesrep_id, :cust_name,"
               "  :cust_corrcntct_id, :cust_cntct_id,"
               "  :cust_custtype_id, :cust_balmethod,"
               "  :cust_creditlmt, :cust_creditlmt_curr_id,"
               "  :cust_creditrating, :cust_creditstatus,"
               "  :cust_autoupdatestatus, :cust_autoholdorders,"
               "  :cust_backorder, :cust_ffshipto, :cust_ffbillto,"
               "  :cust_commprcnt, :cust_partialship,"
               "  :cust_shipvia,"
               "  :cust_shipchrg_id, :cust_shipform_id, :cust_terms_id,"
               "  :cust_discntprcnt, :cust_taxzone_id,"
               "  :cust_active, :cust_usespos, :cust_blanketpos, :cust_comments,"
               "  :cust_preferred_warehous_id, "
               "  :cust_gracedays, :cust_curr_id, :cust_financecharge ) " );

  customerSave.bindValue(":cust_id", _custid);
  customerSave.bindValue(":cust_number", _number->number().trimmed());
  customerSave.bindValue(":cust_name", _name->text().trimmed());
  customerSave.bindValue(":cust_salesrep_id", _salesrep->id());
  if (_corrCntct->id() > 0)
    customerSave.bindValue(":cust_corrcntct_id", _corrCntct->id());        // else NULL
  if (_billCntct->id() > 0)
    customerSave.bindValue(":cust_cntct_id", _billCntct->id());            // else NULL
  customerSave.bindValue(":cust_custtype_id", _custtype->id());

  customerSave.bindValue(":cust_balmethod", _balanceMethod->code());

  if (_inGoodStanding->isChecked())
    customerSave.bindValue(":cust_creditstatus", "G");
  else if (_onCreditWarning->isChecked())
    customerSave.bindValue(":cust_creditstatus", "W");
  else if (_onCreditHold->isChecked())
    customerSave.bindValue(":cust_creditstatus", "H");
  else
    customerSave.bindValue(":cust_creditstatus", "U");

  customerSave.bindValue(":cust_creditlmt_curr_id", _creditLimit->id());
  customerSave.bindValue(":cust_creditlmt", _creditLimit->localValue());
  customerSave.bindValue(":cust_creditrating", _creditRating->text());
  customerSave.bindValue(":cust_autoupdatestatus", QVariant(_autoUpdateStatus->isChecked()));
  customerSave.bindValue(":cust_autoholdorders", QVariant(_autoHoldOrders->isChecked()));
  customerSave.bindValue(":cust_commprcnt", (_defaultCommissionPrcnt->toDouble() / 100.0));
  customerSave.bindValue(":cust_terms_id", _terms->id());
  customerSave.bindValue(":cust_discntprcnt", (_defaultDiscountPrcnt->toDouble() / 100.0));

  if (_taxzone->isValid())
    customerSave.bindValue(":cust_taxzone_id", _taxzone->id());

  customerSave.bindValue(":cust_shipvia", _shipvia->currentText());
  customerSave.bindValue(":cust_shipchrg_id", _shipchrg->id());
  if(_shipform->id() > 0)
    customerSave.bindValue(":cust_shipform_id", _shipform->id());

  customerSave.bindValue(":cust_active",     QVariant(_active->isChecked()));
  customerSave.bindValue(":cust_usespos",    QVariant(_usesPOs->isChecked()));
  customerSave.bindValue(":cust_blanketpos", QVariant(_blanketPos->isChecked()));
  customerSave.bindValue(":cust_partialship",QVariant(_partialShipments->isChecked()));
  customerSave.bindValue(":cust_backorder",  QVariant(_backorders->isChecked()));
  customerSave.bindValue(":cust_ffshipto",   QVariant(_allowFFShipto->isChecked()));
  customerSave.bindValue(":cust_ffbillto",   QVariant(_allowFFBillto->isChecked()));

  customerSave.bindValue(":cust_comments", _notes->toPlainText());

  customerSave.bindValue(":cust_preferred_warehous_id", _sellingWarehouse->id());
  customerSave.bindValue(":cust_curr_id", _currency->id());

  if(_warnLate->isChecked())
    customerSave.bindValue(":cust_gracedays", _graceDays->value());

  if(_exemptFinanceCharge->isChecked())
    customerSave.bindValue(":cust_financecharge", false);
  else
    customerSave.bindValue(":cust_financecharge", true);
    
  customerSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving"),
                           customerSave, __FILE__, __LINE__))
    return false;

  if (_mode == cNew)
  {
    _mode = cEdit;
    emit newMode(_mode);
    emit newId(_custid); // custcluster listeners couldn't handle set()'s emit
  }

  //Save characteristics
  if (charProfile && _custtype->id() != custtype_id &&
      QMessageBox::question(this, tr("Delete?"),
                            tr("Do you want to DELETE all characteristics related to the old Customer Type?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::Yes)
  {
    XSqlQuery deleteChar;

    deleteChar.prepare("DELETE FROM charass "
                       "USING "
                       " (SELECT DISTINCT b.charass_id AS id "
                       "  FROM custtype "
                       "  JOIN charass a ON (a.charass_target_type='CT') "
                       "   AND (a.charass_target_id=custtype_id) "
                       "  JOIN charass b ON (b.charass_target_type='C') "
                       "   AND (b.charass_target_id=:cust_id) "
                       "   AND (a.charass_char_id=b.charass_char_id) "
                       "  WHERE (custtype_id=:custtype_id)) qry "
                       "WHERE charass_id=qry.id;");
    deleteChar.bindValue(":cust_id", _custid);
    deleteChar.bindValue(":custtype_id", custtype_id);
    deleteChar.exec();
  }

  if (_widgetStack->currentIndex() == 1)
  {
    customerSave.prepare("SELECT updateCharAssignment('C', :target_id, :char_id, :char_value);");

    QModelIndex idx1, idx2;
    for(int i = 0; i < _custchar->rowCount(); i++)
    {
      idx1 = _custchar->index(i, 0);
      idx2 = _custchar->index(i, 1);
      customerSave.bindValue(":target_id", _custid);
      customerSave.bindValue(":char_id", _custchar->data(idx1, Qt::UserRole));
      customerSave.bindValue(":char_value", _custchar->data(idx2, Qt::DisplayRole));
      customerSave.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Saving Characteristic"),
                           customerSave, __FILE__, __LINE__);
    }
  }

  setValid(true);
  omfgThis->sCustomersUpdated(_custid, true);
  _autoSaved = true;

  return true;
}

void customer::sSaveClicked()
{
  _save->setFocus();

  if (!sSave())
    return;

  _autoSaved=false;
  _NumberGen = -1;
  emit saved(_custid);
  if (_captive || isModal())
    close();
  else
    sClear();
}

void customer::sCheck()
{
  XSqlQuery customerCheck;
  _number->setNumber(_number->number().trimmed().toUpper());

  if (_cachedNumber == _number->number())
    return;

  if(cNew == _mode && -1 != _NumberGen && _number->number().toInt() != _NumberGen)
  {
    XSqlQuery query;
    query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
    query.bindValue(":Number", _NumberGen);
    query.exec();
    _NumberGen = -1;
  }

  customerCheck.prepare( "SELECT cust_id, 1 AS type "
             "FROM custinfo "
             "WHERE (cust_number=:cust_number) "
             "UNION "
             "SELECT prospect_id, 2 AS type "
             "FROM prospect "
             "WHERE (prospect_number=:cust_number) "
             "UNION "
             "SELECT crmacct_id, 3 AS type "
             "FROM crmacct "
             "WHERE (crmacct_number=:cust_number) "
             "ORDER BY type; ");
  customerCheck.bindValue(":cust_number", _number->number());
  customerCheck.exec();
  if (customerCheck.first())
  {
    if ((customerCheck.value("type").toInt() == 1) && (customerCheck.value("cust_id").toInt() != _custid) && (_notice))
    {
      if (QMessageBox::question(this, tr("Customer Exists"),
              tr("<p>This number is currently used by an existing Customer. "
                   "Do you want to edit that Customer?"),
              QMessageBox::Yes,
              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
        _number->setNumber(_cachedNumber);
        _number->setFocus();
        return;
      }

      if (! _lock.release())
        ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                             _lock.lastError(), __FILE__, __LINE__);

      _number->setId(customerCheck.value("cust_id").toInt());

      if (_mode == cEdit && !_lock.acquire("custinfo", customerCheck.value("cust_id").toInt(), 
                                          AppLock::Interactive))
      {
        setViewMode();
      }

      _mode = cEdit;
      _name->setFocus();
      emit newMode(_mode);
    }
    else if ( (_mode == cEdit) &&
              ((customerCheck.value("type").toInt() == 2) ||
              (customerCheck.value("type").toInt() == 3)) &&
              (_notice))
    {
      if (QMessageBox::critical(this, tr("Invalid Number"),
                                tr("<p>This number is currently "
                                   "assigned to another Account.")))
      {
        _number->setNumber(_cachedNumber);
        _number->setFocus();
        _notice = false;
        return;
      }
    }
    else if ((customerCheck.value("type").toInt() == 2) && (_notice))
    {
      int quotecount = 0;
      if (_privileges->check("ConvertQuotes"))
      {
        XSqlQuery quoteq;
        quoteq.prepare("SELECT COUNT(*) AS quotecount"
                       "  FROM quhead"
                       " WHERE (quhead_cust_id=:id);");
        quoteq.bindValue(":id", customerCheck.value("cust_id"));
        quoteq.exec();
        if (quoteq.first())
          quotecount = quoteq.value("quotecount").toInt();
        // ignore errors
      }

      if (QMessageBox::question(this, tr("Convert"),
                                tr("<p>This number is currently assigned to "
                                   "a Prospect. Do you want to convert the "
                                   "Prospect to a Customer?"),
              QMessageBox::Yes,
              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
        _number->setId(-1);
        _number->setFocus();
        return;
      }

      bool convertquotes = quotecount != 0 &&
                           (QMessageBox::question(this, tr("Convert"),
                                    tr("<p>Do you want to convert all of the "
                                       "Quotes for the Prospect to "
                                       "Sales Orders?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No) == QMessageBox::Yes);

      XSqlQuery convertq;
      convertq.prepare("SELECT convertProspectToCustomer(:id, :convertquotes) AS result;");
      convertq.bindValue(":id", customerCheck.value("cust_id"));
      convertq.bindValue(":convertquotes", convertquotes);
      convertq.exec();
      if (convertq.first())
      {
        int result = convertq.value("result").toInt();
        _name->setFocus();
        setId(result);
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Convert Error"),
                                    convertq, __FILE__, __LINE__))
        return;
    } // number in use by prospect
    else if ((customerCheck.value("type").toInt() == 3) && (_notice))
    {
      if (QMessageBox::question(this, tr("Convert"),
                  tr("<p>This number is currently assigned to Account. "
                     "Do you want to convert the Account to a Customer?"),
              QMessageBox::Yes,
              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
        _number->setId(-1);
        _number->setFocus();
        return;
      }
      sLoadCrmAcct(customerCheck.value("cust_id").toInt());
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Checking Number"),
                                customerCheck, __FILE__, __LINE__))
    return;
}

bool customer::sCheckRequired()
{
    if ( ( _number->number().trimmed().length() == 0) ||
         (_name->text().trimmed().length() == 0) ||
         (_custtype->id() == -1) ||
         (_terms->id() == -1) ||
         (_salesrep->id() == -1) ||
         (_custid == -1) )
    {
      setValid(false);
      return false;
    }
    setValid(true);
    return true;
}

void customer::sCrmAccount()
{
  ParameterList params;
  params.append("crmacct_id", _crmacctid);
  if ((cView == _mode && _privileges->check("ViewAllCRMAccounts")) ||
      (cView == _mode && _privileges->check("ViewPersonalCRMAccounts")
                      && omfgThis->username() == _crmowner) ||
      (cEdit == _mode && _privileges->check("ViewAllCRMAccounts")
                      && ! _privileges->check("MaintainAllCRMAccounts")) ||
      (cEdit == _mode && _privileges->check("ViewPersonalCRMAccounts")
                      && ! _privileges->check("MaintainPersonalCRMAccounts")
                      && omfgThis->username() == _crmowner))
    params.append("mode", "view");
  else if ((cEdit == _mode && _privileges->check("MaintainAllCRMAccounts")) ||
           (cEdit == _mode && _privileges->check("MaintainPersonalCRMAccounts")
                           && omfgThis->username() == _crmowner))
    params.append("mode", "edit");
  else if ((cNew == _mode && _privileges->check("MaintainAllCRMAccounts")) ||
           (cNew == _mode && _privileges->check("MaintainPersonalCRMAccounts")
                          && omfgThis->username() == _crmowner))
    params.append("mode", "edit");
  else
  {
    qWarning("tried to open Account window without privilege");
    return;
  }

  crmaccount *newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void customer::sPrintShipto()
{
  ParameterList params;
  params.append("cust_id", _custid);

  orReport report("ShipToMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void customer::sNewShipto()
{
  if (_mode == cNew)
  {
    if (!sSave())
      return;
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _custid);

  shipTo newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillShiptoList();
}

void customer::sEditShipto()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("shipto_id", _shipto->id());

  shipTo newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillShiptoList();
}

void customer::sViewShipto()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("shipto_id", _shipto->id());

  shipTo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void customer::sDeleteShipto()
{
  if (QMessageBox::question(this, tr("Delete Ship To?"),
                            tr("Are you sure that you want to delete this Ship To?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  XSqlQuery delq;
  delq.prepare("SELECT deleteShipTo(:shipto_id) AS result;");
  delq.bindValue(":shipto_id", _shipto->id());
  delq.exec();
  if (delq.first())
  {
    int result = delq.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Ship To"),
                             storedProcErrorLookup("deleteShipTo", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Ship To"),
                                delq, __FILE__, __LINE__))
    return;

  sFillShiptoList();
}

void customer::sNewCharacteristic()
{
  if (_mode == cNew)
  {
    if (!sSave())
      return;
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _custid);

  characteristicAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharacteristicList();
}

void customer::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharacteristicList();
}

void customer::sDeleteCharacteristic()
{
  XSqlQuery delq;
  delq.prepare( "DELETE FROM charass WHERE (charass_id=:charass_id);" );
  delq.bindValue(":charass_id", _charass->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Characteristic"),
                           delq, __FILE__, __LINE__))
    return;

  sFillCharacteristicList();
}

void customer::sFillCharacteristicList()
{
  XSqlQuery customerFillCharacteristicList;
  customerFillCharacteristicList.prepare( "SELECT custtype_char "
             "FROM custtype "
             "WHERE (custtype_id=:custtype_id);");
  customerFillCharacteristicList.bindValue(":custtype_id",_custtype->id());
  customerFillCharacteristicList.exec();

  customerFillCharacteristicList.first();
  if (customerFillCharacteristicList.value("custtype_char").toBool())
  {
    if (_charfilled)
      return;
    _widgetStack->setCurrentIndex(1);
    _custchar->removeRows(0, _custchar->rowCount());
    customerFillCharacteristicList.prepare( "SELECT char_id, char_name, "
               " CASE WHEN char_type < 2 THEN "
               "   charass_value "
               " ELSE "
               "   formatDate(charass_value::date) "
               " END AS f_charass_value, "
               "charass_value "
               "FROM ( "
               "SELECT DISTINCT char_id, char_name, char_type, char_order, "
               "       COALESCE(b.charass_value, (SELECT c.charass_value FROM charass c WHERE ((c.charass_target_type='CT') AND (c.charass_target_id=:custtype_id) AND (c.charass_default) AND (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type='C'"
               "      AND b.charass_target_id=:cust_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='CT')"
               "   AND   (a.charass_target_id=:custtype_id) ) "
               " ORDER BY char_order, char_name) data;" );
    customerFillCharacteristicList.bindValue(":custtype_id", _custtype->id());
    customerFillCharacteristicList.bindValue(":cust_id", _custid);
    customerFillCharacteristicList.exec();

    int row = 0;
    QModelIndex idx;
    while(customerFillCharacteristicList.next())
    {
      _custchar->insertRow(_custchar->rowCount());
      idx = _custchar->index(row, 0);
      _custchar->setData(idx, customerFillCharacteristicList.value("char_name"), Qt::DisplayRole);
      _custchar->setData(idx, customerFillCharacteristicList.value("char_id"), Qt::UserRole);
      idx = _custchar->index(row, 1);
      _custchar->setData(idx, customerFillCharacteristicList.value("f_charass_value"), Qt::DisplayRole);
      _custchar->setData(idx, customerFillCharacteristicList.value("charass_value"), Qt::UserRole);
      _custchar->setData(idx, _custtype->id(), Xt::IdRole);
      row++;
    }
    _charfilled=true;
  }
  else
  {
    _widgetStack->setCurrentIndex(0);
    XSqlQuery r;
    r.prepare( "SELECT charass_id, char_name, "
               " CASE WHEN char_type = 2 THEN "
               "   formatDate(charass_value::date) "
               " ELSE "
               "   charass_value "
               "END AS charass_value "
               "FROM charass, char "
               "WHERE ( (charass_target_type='C')"
               " AND (charass_char_id=char_id)"
               " AND (charass_target_id=:cust_id) ) "
               "ORDER BY char_order, char_name;" );
    r.bindValue(":custtype_id", _custtype->id());
    r.bindValue(":cust_id", _custid);
    r.exec();
    _charass->populate(r);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Characteristics"),
                             r, __FILE__, __LINE__))
      return;
    _charfilled=false;
  }
}

void customer::sPopulateShiptoMenu(QMenu *menuThis)
{
  if (_mode != cView)
    menuThis->addAction(tr("Edit..."),   this, SLOT(sEditShipto()));
  menuThis->addAction(tr("View..."),   this, SLOT(sViewShipto()));
  if (_mode != cView)
    menuThis->addAction(tr("Delete..."), this, SLOT(sDeleteShipto()));
}

void customer::sFillShiptoList()
{
  XSqlQuery r;
  r.prepare( "SELECT shipto_id, shipto_default, shipto_active, "
             "       shipto_num, shipto_name, addr_line1,"
             "       (addr_city || ', ' || addr_state || '  ' || addr_postalcode) AS shipto_csz "
             "  FROM shiptoinfo LEFT OUTER JOIN addr ON shipto_addr_id=addr_id"
             " WHERE (shipto_cust_id=:cust_id) "
             "ORDER BY shipto_num;" );
  r.bindValue(":cust_id", _custid);
  r.exec();
  _shipto->populate(r);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Ship Tos"),
                           r, __FILE__, __LINE__))
    return;
}

void customer::sNewTaxreg()
{
  if (_mode == cNew)
  {
    if (!sSave())
      return;
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("taxreg_rel_id", _custid);
  params.append("taxreg_rel_type", "C");

  taxRegistration newdlg(this, "", true);
  if (newdlg.set(params) == NoError && newdlg.exec() != XDialog::Rejected)
    sFillTaxregList();
}

void customer::sEditTaxreg()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxreg_id", _taxreg->id());

  taxRegistration newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.set(params) == NoError && newdlg.exec() != XDialog::Rejected)
    sFillTaxregList();
}

void customer::sViewTaxreg()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxreg_id", _taxreg->id());

  taxRegistration newdlg(this, "", true);
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void customer::sDeleteTaxreg()
{
  XSqlQuery delq;
  delq.prepare("DELETE FROM taxreg WHERE (taxreg_id=:taxreg_id);");
  delq.bindValue(":taxreg_id", _taxreg->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Tax Registrations"),
                           delq, __FILE__, __LINE__))
    return;
  sFillTaxregList();
}

void customer::sFillTaxregList()
{
  XSqlQuery taxreg;
  taxreg.prepare("SELECT taxreg_id, taxreg_taxauth_id, "
                 "       taxauth_code, taxreg_number "
                 "FROM taxreg, taxauth "
                 "WHERE ((taxreg_rel_type='C') "
                 "  AND  (taxreg_rel_id=:cust_id) "
                 "  AND  (taxreg_taxauth_id=taxauth_id));");
  taxreg.bindValue(":cust_id", _custid);
  taxreg.exec();
  _taxreg->populate(taxreg, true);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Tax Registrations"),
                           taxreg, __FILE__, __LINE__))
    return;
}

void customer::sPopulateCommission()
{
  if (_mode != cView)
  {
    XSqlQuery s;
    s.prepare( "SELECT salesrep_commission "
               "FROM salesrep "
               "WHERE (salesrep_id=:salesrep_id);" );
    s.bindValue(":salesrep_id", _salesrep->id());
    s.exec();
    if (s.first())
      _defaultCommissionPrcnt->setDouble(s.value("salesrep_commission").toDouble() * 100);
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Commission"),
                                  s, __FILE__, __LINE__))
    return;
  }
}

void customer::populate()
{
  XSqlQuery cust;
  XSqlQuery salesRep;

  salesRep.prepare("SELECT salesrep_id, (salesrep_number || '-' || salesrep_name), salesrep_number "
                   "FROM salesrep "
                   "WHERE (salesrep_active) "
                   "UNION "
                   "SELECT salesrep_id, (salesrep_number || '-' || salesrep_name), salesrep_number "
                   "FROM salesrep "
                   "WHERE (salesrep_id=(SELECT cust_salesrep_id FROM custinfo WHERE cust_id=:cust)) "
                   "ORDER by salesrep_number;");
  salesRep.bindValue(":cust", _custid);
  salesRep.exec();
  _salesrep->populate(salesRep);

  _notice = false;
  cust.prepare( "SELECT custinfo.*, "
                "       cust_commprcnt, cust_discntprcnt,"
                "       (cust_gracedays IS NOT NULL) AS hasGraceDays,"
                "       COALESCE(cust_financecharge, true) AS financecharge,"
                "       crmacct_id, crmacct_owner_username "
                "FROM custinfo LEFT OUTER JOIN "
                "     crmacct ON (cust_id=crmacct_cust_id) "
                "WHERE (cust_id=:cust_id);" );
  cust.bindValue(":cust_id", _custid);
  cust.exec();
  if (cust.first())
  {
    if (_mode == cNew)
    {
      _mode = cEdit;
      emit newMode(_mode);
    }
    setValid(true);

    _crmacctid = cust.value("crmacct_id").toInt();
    _crmowner = cust.value("crmacct_owner_username").toString();
    _crmacct->setEnabled(_crmacctid > 0 &&
                         (_privileges->check("MaintainAllCRMAccounts") ||
                          _privileges->check("ViewAllCRMAccounts") ||
                          (omfgThis->username() == _crmowner && _privileges->check("MaintainPersonalCRMAccounts")) ||
                          (omfgThis->username() == _crmowner && _privileges->check("ViewPersonalCRMAccounts"))));

    _number->setNumber(cust.value("cust_number").toString());
    _cachedNumber = cust.value("cust_number").toString();
    _name->setText(cust.value("cust_name"));
    _corrCntct->setId(cust.value("cust_corrcntct_id").toInt());
    _corrCntct->setSearchAcct(cust.value("crmacct_id").toInt());
    _billCntct->setId(cust.value("cust_cntct_id").toInt());
    _billCntct->setSearchAcct(cust.value("crmacct_id").toInt());
    _creditLimit->set(cust.value("cust_creditlmt").toDouble(),
                      cust.value("cust_creditlmt_curr_id").toInt(),
                      QDate::currentDate(),
                      false);
    _creditRating->setText(cust.value("cust_creditrating"));
    _autoUpdateStatus->setChecked(cust.value("cust_autoupdatestatus").toBool());
    _autoHoldOrders->setChecked(cust.value("cust_autoholdorders").toBool());
    _defaultDiscountPrcnt->setDouble(cust.value("cust_discntprcnt").toDouble() * 100);

    if(cust.value("hasGraceDays").toBool())
    {
      _warnLate->setChecked(true);
      _graceDays->setValue(cust.value("cust_gracedays").toInt());
    }
    else
    {
      _warnLate->setChecked(false);
      _graceDays->setValue(_metrics->value("DefaultAutoCreditWarnGraceDays").toInt());
    }

    _notes->setText(cust.value("cust_comments").toString());

    _custtype->setId(cust.value("cust_custtype_id").toInt());
    if((!_privileges->check("MaintainCustomerMastersCustomerType")) && (_custtype->id() != -1))
      _custtype->setEnabled(false);

    _salesrep->setId(cust.value("cust_salesrep_id").toInt());
    _defaultCommissionPrcnt->setDouble(cust.value("cust_commprcnt").toDouble() * 100);
    _terms->setId(cust.value("cust_terms_id").toInt());
    _taxzone->setId(cust.value("cust_taxzone_id").toInt());
    _shipform->setId(cust.value("cust_shipform_id").toInt());
    _shipchrg->setId(cust.value("cust_shipchrg_id").toInt());
    _shipvia->setText(cust.value("cust_shipvia").toString());
    if (!cust.value("financecharge").toBool())
      _exemptFinanceCharge->setChecked(true);

    _sellingWarehouse->setId(cust.value("cust_preferred_warehous_id").toInt());

    _balanceMethod->setCode(cust.value("cust_balmethod").toString());

    _active->setChecked(cust.value("cust_active").toBool());
    _backorders->setChecked(cust.value("cust_backorder").toBool());
    _partialShipments->setChecked(cust.value("cust_partialship").toBool());
    _partialShipments->setEnabled(cust.value("cust_backorder").toBool());
    _allowFFShipto->setChecked(cust.value("cust_ffshipto").toBool());
    _allowFFBillto->setChecked(cust.value("cust_ffbillto").toBool());
    _usesPOs->setChecked(cust.value("cust_usespos").toBool());
    _blanketPos->setEnabled(cView != _mode && _usesPOs->isChecked());
    _blanketPos->setChecked(cust.value("cust_blanketpos").toBool());
    _currency->setId(cust.value("cust_curr_id").toInt());

    if (cust.value("cust_creditstatus").toString() == "G")
      _inGoodStanding->setChecked(true);
    else if (cust.value("cust_creditstatus").toString() == "W")
      _onCreditWarning->setChecked(true);
    else
      _onCreditHold->setChecked(true);
    
    _comments->setId(_crmacctid);
    _documents->setId(_crmacctid);

    _todoList->parameterWidget()->setDefault(tr("Account"), _crmacctid, true);
    _contacts->setCrmacctid(_crmacctid);

    _quotes->parameterWidget()->setDefault(tr("Customer"), _custid, true);
    _orders->setCustId(_custid);
    _returns->findChild<CustomerSelector*>("_customerSelector")->setCustId(_custid);
    _aritems->findChild<CustomerSelector*>("_customerSelector")->setCustId(_custid);
    _cashreceipts->findChild<CustomerSelector*>("_customerSelector")->setCustId(_custid);
    _cctrans->findChild<CustomerSelector*>("_customerSelector")->setCustId(_custid);

    sFillList();

    emit populated();
    _autoSaved=false;
    return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Customer"),
                                cust, __FILE__, __LINE__))
    return;

  sClear();

}

void customer::sPopulateSummary()
{
  XSqlQuery query;
  query.prepare( "SELECT MIN(cohist_invcdate) AS firstdate,"
                 "       MAX(cohist_invcdate) AS lastdate "
                 "FROM cohist "
                 "WHERE (cohist_cust_id=:cust_id) "
                 "AND (cohist_doctype='I');" );
  query.bindValue(":cust_id", _custid);
  query.exec();
  if (query.first())
  {
    _firstSaleDate->setDate(query.value("firstdate").toDate());
    _lastSaleDate->setDate(query.value("lastdate").toDate());
  }

  ParameterList params;
  params.append("cust_id", _custid);
  MetaSQLQuery lySales = mqlLoad("customer", "lastYearSales");
  query = lySales.toQuery(params);
  if (query.first())
    _lastYearSales->setDouble(query.value("lysales").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Sales History"),
                                  query, __FILE__, __LINE__))
      return;

  MetaSQLQuery ytdSales = mqlLoad("customer", "ytdSales");
  query = ytdSales.toQuery(params);
  if (query.first())
    _ytdSales->setDouble(query.value("ytdsales").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Sales History"),
                                  query, __FILE__, __LINE__))
      return;

  query.prepare( "SELECT COALESCE( SUM( (noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) * coitem_qty_invuomratio) *"
                 "                                   (coitem_price / coitem_price_invuomratio) ), 0 ) AS backlog "
                 "FROM cohead, coitem, itemsite, item "
                 "WHERE ( (coitem_cohead_id=cohead_id)"
                 " AND (coitem_itemsite_id=itemsite_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (coitem_status='O')"
                 " AND (cohead_cust_id=:cust_id) );" );
  query.bindValue(":cust_id", _custid);
  query.exec();
  if (query.first())
    _backlog->setDouble(query.value("backlog").toDouble());

  query.prepare( "SELECT COALESCE( SUM( CASE WHEN (aropen_doctype IN ('I', 'D')) THEN (aropen_amount - aropen_paid)"
                 "                                       ELSE ((aropen_amount - aropen_paid) * -1)"
                 "                                   END ), 0 ) AS balance "
                 "FROM aropen "
                 "WHERE ( (aropen_open)"
                 " AND (aropen_cust_id=:cust_id) );" );
  query.bindValue(":cust_id", _custid);
  query.exec();
  if (query.first())
    _openBalance->setDouble(query.value("balance").toDouble());

  query.prepare( "SELECT noNeg( COALESCE( SUM( CASE WHEN (aropen_doctype IN ('I', 'D')) THEN (aropen_amount - aropen_paid)"
                 "                                      ELSE ((aropen_amount - aropen_paid) * -1)"
                 "                                   END ), 0 ) ) AS balance "
                 "FROM aropen "
                 "WHERE ( (aropen_open)"
                 " AND (aropen_duedate < CURRENT_DATE)"
                 " AND (aropen_cust_id=:cust_id) );" );
  query.bindValue(":cust_id", _custid);
  query.exec();
  if (query.first())
    _lateBalance->setDouble(query.value("balance").toDouble());
}

void customer::sPrint()
{
  ParameterList params;
  params.append("cust_id", _custid);

  orReport report("CustomerInformation", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void customer::sNewCreditCard()
{
  if (_mode == cNew)
  {
    if (!sSave())
      return;
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _custid);

  creditCard newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCcardList();
}

void customer::sEditCreditCard()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", _custid);
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCcardList();
}

void customer::sViewCreditCard()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _custid);
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void customer::sDeleteCreditCard()
{
  if (QMessageBox::question(this, tr("Delete Credit Card?"),
                            tr("Are you sure that you want to delete this Credit Card?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  XSqlQuery m;

  m.prepare("SELECT date_part('month', expire) AS expire_month, "
            "       date_part('year', expire) AS expire_year "
            "FROM ( "
            " SELECT CASE WHEN ccpay_transaction_datetime > "
            "                  date_trunc('month', now()) THEN "
            "                  now() "
            "             ELSE "
            "                  now() - '1 month'::INTERVAL "
            "            END AS expire "
            " FROM ccpay "
            " WHERE ccpay_ccard_id=:ccard_id "
            " ORDER BY ccpay_transaction_datetime DESC "
            " LIMIT 1) qry;");
  m.bindValue(":ccard_id", _cc->id());
  m.exec();

  if (m.first())
  {
    QString month = m.value("expire_month").toString();
    QString year = m.value("expire_year").toString();

    m.prepare("UPDATE ccard "
              "SET ccard_active=FALSE, "
              "    ccard_month_expired=encrypt(setbytea(lpad(:month, 2, '0')), setbytea(:key), 'bf'), "
              "    ccard_year_expired=encrypt(setbytea(:year), setbytea(:key), 'bf') "
              "WHERE ccard_id=:ccard_id;");
    m.bindValue(":ccard_id", _cc->id());
    m.bindValue(":month", month);
    m.bindValue(":year", year);
    m.bindValue(":key", omfgThis->_key);
    m.exec();
    if (m.lastError().type() != QSqlError::NoError)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Credit Card"),
                           m.lastError(), __FILE__, __LINE__);
      return;
    }
  }
  else if (m.lastError().type() != QSqlError::NoError)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Credit Card"),
                         m.lastError(), __FILE__, __LINE__);
    return;
  }
  else
  {
    m.prepare("DELETE FROM ccard WHERE ccard_id=:ccard_id;");
    m.bindValue(":ccard_id", _cc->id());
    m.exec();
    if (m.lastError().type() != QSqlError::NoError)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Credit Card"),
                           m.lastError(), __FILE__, __LINE__);
      return;
    }
  }

  sFillCcardList();
}

void customer::sMoveUp()
{
  XSqlQuery m;
  m.prepare("SELECT moveCcardUp(:ccard_id) AS result;");
  m.bindValue(":ccard_id", _cc->id());
  m.exec();

  sFillCcardList();
}

void customer::sMoveDown()
{
  XSqlQuery m;
  m.prepare("SELECT moveCcardDown(:ccard_id) AS result;");
  m.bindValue(":ccard_id", _cc->id());
  m.exec();

  sFillCcardList();
}

void customer::sFillList()
{
  if (_tab->currentIndex() == _tab->indexOf(_addressTab))
  {
    if (_shiptoButton->isChecked())
      sFillShiptoList();
  }
  else if (_tab->currentIndex() == _tab->indexOf(_settingsTab))
  {
    if (_taxButton->isChecked())
      sFillTaxregList();
    else if (_creditcardsButton->isChecked())
      sFillCcardList();
  }
  else if (_tab->currentIndex() == _tab->indexOf(_characteristicsTab))
     sFillCharacteristicList();
  else if (_tab->currentIndex() == _tab->indexOf(_crmTab))
  {
    if (_contactsButton->isChecked())
      _contacts->sFillList();
    else if (_todoListButton->isChecked())
      _todoList->sFillList();
  }
  else if (_tab->currentIndex() == _tab->indexOf(_salesTab))
  {
    if (_summaryButton->isChecked())
      sPopulateSummary();
    else if (_quotesButton->isChecked())
      _quotes->sFillList();
    else if (_ordersButton->isChecked())
      _orders->sFillList();
    else if (_returnsButton->isChecked())
      _returns->sFillLists();
  }
  else if (_tab->currentIndex() == _tab->indexOf(_accountingTab))
  {
    if (_aritemsButton->isChecked())
      _aritems->sFillList();
    else if (_cashreceiptsButton->isChecked())
      _cashreceipts->sFillList();
    else if (_cctransButton->isChecked())
      _cctrans->sFillList();
  }
}

void customer::sFillCcardList()
{
  key = omfgThis->_key;

  XSqlQuery r;
  r.prepare( "SELECT expireCreditCard(:cust_id, setbytea(:key));");
  r.bindValue(":cust_id", _custid);
  r.bindValue(":key", key);
  r.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Expiring Credit Card"),
                           r, __FILE__, __LINE__))
    return;

  MetaSQLQuery mql = mqlLoad("creditCards", "detail");
  ParameterList params;
  params.append("cust_id",         _custid);
  params.append("masterCard",      tr("MasterCard"));
  params.append("visa",            tr("VISA"));
  params.append("americanExpress", tr("American Express"));
  params.append("discover",        tr("Discover"));
  params.append("other",           tr("Other"));
  params.append("key",             key);
  r = mql.toQuery(params);
  _cc->populate(r);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Credit Cards"),
                           r, __FILE__, __LINE__))
    return;
}

void customer::sLoadCrmAcct(int crmacctId)
{
  _notice = false;
  _crmacctid = crmacctId;
  _billCntct->setSearchAcct(_crmacctid);
  _corrCntct->setSearchAcct(_crmacctid);

  XSqlQuery getq;
  getq.prepare("SELECT * FROM crmacct WHERE (crmacct_id=:crmacct_id);");
  getq.bindValue(":crmacct_id", crmacctId);
  getq.exec();
  if (getq.first())
  {
    _crmowner = getq.value("crmacct_owner_username").toString();
    _number->setCanEdit(true);
    _number->setEditMode(true);
    _number->setNumber(getq.value("crmacct_number").toString());
    _cachedNumber=_number->number().trimmed().toUpper();
//    _number->setEditMode(false);
//    _number->setCanEdit(false);
    _name->setText(getq.value("crmacct_name").toString());
    _active->setChecked(getq.value("crmacct_active").toBool());
    _billCntct->setId(getq.value("crmacct_cntct_id_1").toInt());
    _corrCntct->setId(getq.value("crmacct_cntct_id_1").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Account"),
                           getq, __FILE__, __LINE__))
    return;

  _crmacct->setEnabled(_crmacctid > 0 &&
                       (_privileges->check("MaintainAllCRMAccounts") ||
                        _privileges->check("ViewAllCRMAccounts") ||
                        (omfgThis->username() == _crmowner && _privileges->check("MaintainPersonalCRMAccounts")) ||
                        (omfgThis->username() == _crmowner && _privileges->check("ViewPersonalCRMAccounts"))));

  _name->setFocus();
}

void customer::sNumberEdited()
{
  _notice = true;
  sCheck();
}

// Set selected sales rep to all ship to for the current customer
void customer::sSetSalesRepToAllShipTo()
{
  XSqlQuery updateShipTo;
  updateShipTo.prepare( "UPDATE shiptoinfo "
                        "SET shipto_commission=:shipto_commission,"
                        "    shipto_salesrep_id=:shipto_salesrep_id "
                        "WHERE (shipto_cust_id=:cust_id);" );

  updateShipTo.bindValue(":shipto_commission", (_defaultCommissionPrcnt->toDouble() / 100));
  if (_salesrep->id() != -1)
    updateShipTo.bindValue(":shipto_salesrep_id", _salesrep->id());
  updateShipTo.bindValue(":cust_id", _custid);

  updateShipTo.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Ship To info"),
                           updateShipTo, __FILE__, __LINE__))
    return;
}

void customer::closeEvent(QCloseEvent *pEvent)
{
  if(cNew == _mode && -1 != _NumberGen)
  {
    XSqlQuery query;
    query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
    query.bindValue(":Number", _NumberGen);
    query.exec();
    _NumberGen = -1;
  }
  XWidget::closeEvent(pEvent);
}

void customer::currentTabChanged(int index)
{
  if ( (index == _tab->indexOf(_crmTab) ||
        index == _tab->indexOf(_salesTab) ||
        index == _tab->indexOf(_accountingTab)) &&
        (_mode == cNew) )
    sSave();
  else
    sFillList();
}

void customer::sHandleButtons()
{
  if (_billingButton->isChecked())
    _addressStack->setCurrentIndex(0);
  else if (_correspButton->isChecked())
    _addressStack->setCurrentIndex(1);
  else
    _addressStack->setCurrentIndex(2);

  if (_notesButton->isChecked())
    _remarksStack->setCurrentIndex(0);
  else
    _remarksStack->setCurrentIndex(1);

  if (_generalButton->isChecked())
    _settingsStack->setCurrentIndex(0);
  else if (_termsButton->isChecked())
    _settingsStack->setCurrentIndex(1);
  else if (_taxButton->isChecked())
    _settingsStack->setCurrentIndex(2);
  else if (_creditcardsButton->isVisible())
    _settingsStack->setCurrentIndex(3);

  if (_contactsButton->isChecked())
    _crmStack->setCurrentIndex(0);
  else if (_todoListButton->isChecked())
    _crmStack->setCurrentIndex(1);
  else
    _crmStack->setCurrentIndex(2);

  if (_summaryButton->isChecked())
    _salesStack->setCurrentIndex(0);
  else if (_quotesButton->isChecked())
    _salesStack->setCurrentIndex(1);
  else if (_ordersButton->isChecked())
    _salesStack->setCurrentIndex(2);
  else
    _salesStack->setCurrentIndex(3);

  if (_aritemsButton->isChecked())
    _receivablesStack->setCurrentIndex(0);
  else if (_cashreceiptsButton->isChecked())
    _receivablesStack->setCurrentIndex(1);
  else if (_cctransButton->isVisible())
    _receivablesStack->setCurrentIndex(2);

  sFillList();
}

void customer::sCancel()
{
  if (_autoSaved)
      QMessageBox::information( this, tr("Customer Saved"),
                           tr("The customer record was automatically "
                           "saved to the database. The committed changes "
                           "will not be cancelled.") );
  close();
}

void customer::setId(int p)
{
  if (_custid==p)
    return;

  if (! _lock.release())
    ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                         _lock.lastError(), __FILE__, __LINE__);

  if (_mode == cEdit && !_lock.acquire("custinfo", p, AppLock::Interactive))
    setViewMode();

  sClear();
  _custid=p;
  populate();
  emit newId(_custid);
}

void customer::sIdChanged(int id)
{
  _orders->parameterWidget()->setDefault(tr("Customer"), id, true);
  _quotes->parameterWidget()->setDefault(tr("Customer"), id, true);
  _returns->findChild<CustomerSelector*>("_customerSelector")->setCustId(id);
  _aritems->findChild<CustomerSelector*>("_customerSelector")->setCustId(id);
  _cashreceipts->findChild<CustomerSelector*>("_customerSelector")->setCustId(id);
  _cctrans->findChild<CustomerSelector*>("_customerSelector")->setCustId(id);

  XSqlQuery qry;
  qry.prepare("SELECT crmacct_id "
                    "FROM crmacct "
                    "WHERE crmacct_cust_id=:cust_id;" );
  qry.bindValue(":cust_id", id);
  qry.exec();

  if(qry.first())
  {
    _contacts->parameterWidget()->setDefault(tr("Account"), qry.value("crmacct_id").toInt(), true);
    _todoList->parameterWidget()->setDefault(tr("Account"), qry.value("crmacct_id").toInt(), true);
    _billCntct->setSearchAcct(qry.value("crmacct_id").toInt());
    _corrCntct->setSearchAcct(qry.value("crmacct_id").toInt());
  }
  else if(qry.lastError().type() != QSqlError::NoError)
    QMessageBox::warning(this, tr("Database Error"),
                         qry.lastError().text());
}

void customer::sClear()
{
    _custid = -1;
    emit newId(_custid);
    _crmacctid = -1;
    _crmacct->setEnabled(false);

    disconnect(_number, SIGNAL(newId(int)), this, SLOT(setId(int)));
    _number->clear();
    connect(_number, SIGNAL(newId(int)), this, SLOT(setId(int)));

    _cachedNumber="";
    _name->clear();
    _corrCntct->setId(-1);
    _billCntct->setId(-1);
    _creditLimit->clear();
    _creditRating->clear();
    _autoUpdateStatus->setChecked(false);
    _autoHoldOrders->setChecked(false);
    _defaultDiscountPrcnt->clear();
    _warnLate->setChecked(false);
    _graceDays->setValue(_metrics->value("DefaultAutoCreditWarnGraceDays").toInt());

    _notes->clear();

    _salesrep->setId(_metrics->value("DefaultSalesRep").toInt());
    _terms->setId(_metrics->value("DefaultTerms").toInt());
    _exemptFinanceCharge->setChecked(false);
    _taxzone->setCurrentIndex(-1);
    _shipform->setId(_metrics->value("DefaultShipFormId").toInt());
    _shipvia->setId(_metrics->value("DefaultShipViaId").toInt());
    _shipchrg->setId(_metrics->value("DefaultShipChrgId").toInt());
    _sellingWarehouse->setId(_metrics->value("DefaultSellingWarehouseId").toInt());
    _custtype->setId(_metrics->value("DefaultCustType").toInt());
    _backorders->setChecked(_metrics->boolean("DefaultBackOrders"));
    _partialShipments->setEnabled(_metrics->boolean("DefaultBackOrders"));
    _partialShipments->setChecked(_metrics->boolean("DefaultPartialShipments"));
    _allowFFShipto->setChecked(_metrics->boolean("DefaultFreeFormShiptos"));
    _creditLimit->setId(CurrCluster::baseId());
    _creditLimit->setBaseValue(_metrics->value("SOCreditLimit").toDouble());
    _creditRating->setText(_metrics->value("SOCreditRate"));

    _balanceMethod->setCode(_metrics->value("DefaultBalanceMethod"));

    if(!_privileges->check("MaintainCustomerMastersCustomerType")
       && !_privileges->check("MaintainCustomerMastersCustomerTypeOnCreate")
       && (_custtype->id() != -1))
      _custtype->setEnabled(false);

    _defaultCommissionPrcnt->setDouble(0);
    _active->setChecked(true);
    _allowFFBillto->setChecked(false);
    _usesPOs->setChecked(false);
    _blanketPos->setEnabled(cView != _mode && _usesPOs->isChecked());
    _blanketPos->setChecked(false);
    _currency->setId(CurrCluster::baseId());
    if (_metrics->value("SOCreditStatus") == "H")
      _onCreditHold->setChecked(true);
    else if (_metrics->value("SOCreditStatus") == "W")
      _onCreditWarning->setChecked(true);
    else
      _inGoodStanding->setChecked(true);

    _shipto->clear();
    _custchar->removeRows(0, _custchar->rowCount());
    _charass->clear();
    _widgetStack->setCurrentIndex(0);

    _todoList->parameterWidget()->setDefault(tr("Account"), -1, true);
    _contacts->setCrmacctid(_crmacctid);

    _quotes->parameterWidget()->setDefault(tr("Customer"), -1, true);
    _orders->setCustId(-1);
    _returns->findChild<CustomerSelector*>("_customerSelector")->setCustId(-1);
    _aritems->findChild<CustomerSelector*>("_customerSelector")->setCustId(-1);
    _cashreceipts->findChild<CustomerSelector*>("_customerSelector")->setCustId(-1);
    _cctrans->findChild<CustomerSelector*>("_customerSelector")->setCustId(-1);

    _comments->setId(_crmacctid);
    _documents->setId(_crmacctid);

    _print->setEnabled(false);

    sFillList();
    _charfilled = false;
    setValid(false);

    if (_number->editMode() || _mode == cNew)
      sPrepare();
}

void customer::sNumberEditable(bool p)
{
  if (p && _number->id() == -1)
    sClear();
}

void customer::sPrepare()
{
  if (_mode == cEdit)
  {
    _mode = cNew;
    emit newMode(_mode);
  }

  XSqlQuery idq;
  idq.exec("SELECT NEXTVAL('cust_cust_id_seq') AS cust_id");
  if (idq.first())
  {
    _custid = idq.value("cust_id").toInt();
    emit newId(_custid);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting new id"),
                                idq, __FILE__, __LINE__))
    return;

  disconnect(_number, SIGNAL(editable(bool)), this, SLOT(sNumberEditable(bool)));
  _number->clear();
  _number->setCanEdit(true);
  _number->setEditMode(true);
  connect(_number, SIGNAL(editable(bool)), this, SLOT(sNumberEditable(bool)));

  // Handle Auto numbering
  if(((_x_metrics &&
       _x_metrics->value("CRMAccountNumberGeneration") == "A") ||
      (_x_metrics->value("CRMAccountNumberGeneration") == "O"))
    && _number->number().isEmpty() )
  {
    XSqlQuery num;
    num.exec("SELECT fetchCRMAccountNumber() AS number;");
    if (num.first())
      _number->setNumber(num.value("number").toString());
    _custtype->setFocus();
  }

  _NumberGen = _number->number().toInt();
}

