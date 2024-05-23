/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "productCategories.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "productCategory.h"
#include "errorReporter.h"

productCategories::productCategories(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_prodcat, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deleteUnused, SIGNAL(clicked()), this, SLOT(sDeleteUnused()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_prodcat, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  _prodcat->addColumn(tr("Category"),    70, Qt::AlignLeft, true, "prodcat_code" );
  _prodcat->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "prodcat_descrip" );

  if (_privileges->check("MaintainProductCategories"))
  {
    connect(_prodcat, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_prodcat, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_prodcat, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_prodcat, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(false);
    _deleteUnused->setEnabled(false);
  }

  sFillList(-1);
}

productCategories::~productCategories()
{
    // no need to delete child widgets, Qt does it all for us
}

void productCategories::languageChange()
{
    retranslateUi(this);
}

void productCategories::sDelete()
{
  XSqlQuery productDelete;
  productDelete.prepare("SELECT deleteProductCategory(:prodcat_id) AS result;");
  productDelete.bindValue(":prodcat_id", _prodcat->id());
  productDelete.exec();
  if (productDelete.first())
  {
    switch (productDelete.value("result").toInt())
    {
      case -1:
        QMessageBox::warning( this, tr("Cannot Delete Product Category"),
                              tr( "You cannot delete the selected Product Category because there are currently items assigned to it.\n"
                                  "You must first re-assign these items before deleting the selected Product Category." ) );
        return;
    }

    sFillList(-1);
  }
  else
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Product Category"),
                       productDelete, __FILE__, __LINE__);
}

void productCategories::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  productCategory newdlg(this, "", true);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void productCategories::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("prodcat_id", _prodcat->id());

  productCategory newdlg(this, "", true);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void productCategories::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prodcat_id", _prodcat->id());

  productCategory newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void productCategories::sPopulateMenu( QMenu * menu )
{
  QAction *menuItem;

  menuItem = menu->addAction("Edit Product Cateogry...", this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainProductCategories"));

  menuItem = menu->addAction("Delete Product Category...", this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainProductCategories"));
}

void productCategories::sPrint()
{
  orReport report("ProductCategoriesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void productCategories::sDeleteUnused()
{
  XSqlQuery productDeleteUnused;
  if ( QMessageBox::warning( this, tr("Delete Unused Product Categories"),
                             tr("Are you sure that you wish to delete all unused Product Categories?"),
                             tr("&Yes"), tr("&No"), QString {}, 0, 1 ) == 0 )
  {
    productDeleteUnused.exec("SELECT deleteUnusedProductCategories() AS result;");
    sFillList(-1);
  }
}

void productCategories::sFillList(int pId)
{
  _prodcat->populate( "SELECT prodcat_id, prodcat_code, prodcat_descrip "
                      "FROM prodcat "
                      "ORDER BY prodcat_code;", pId );
}

