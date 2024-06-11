/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __LOGIN2_H__
#define __LOGIN2_H__

#include "tmp/ui_login2.h"

class QSplashScreen;
class QImage;
class QWidget;

#include <QDialog>
#include <QString>

#include "parameter.h"

class XAbstractMessageHandler;

class login2 : public QDialog, public Ui::login2
{
  Q_OBJECT

  public:
    login2(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~login2();

    QString _databaseURL;
    QString _user;

    virtual int set(const ParameterList & pParams, QSplashScreen * pSplash);
    virtual void populateDatabaseInfo();
    virtual QString username();
    virtual QString password();
    virtual QString company();
    virtual bool useCloud() const;

    QPushButton* _recent;
    QPushButton* _options;

  public slots:
    virtual int set(const ParameterList & pParams);
    virtual void setLogo( const QImage & );
    virtual void updateRecentOptions();
    virtual void updateRecentOptionsActions();
    virtual void selectRecentOptions();
    virtual void clearRecentOptions();

  protected slots:
    virtual void languageChange();

    virtual void sChangeURL();
    virtual void sHandleButton();
    virtual void sOpenHelp();
    virtual void sLogin();

  private:
    bool _captive;
    bool _nonxTupleDB;
    bool _multipleConnections;
    bool _saveSettings;
    bool _setSearchPath;
    QSplashScreen *_splash;
    QString _cUsername;
    QString _cPassword;
    QString _cServer;
    QString _cDatabase;
    QString _cPort;
    QString _cloudDatabaseURL;
    QString _cCompany;
    QString _connAppName;

    XAbstractMessageHandler* _handler;
};

#endif

