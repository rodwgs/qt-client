/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FILEMOVESELECTOR_H
#define FILEMOVESELECTOR_H

#include "widgets.h"
#include "ui_filemoveselector.h"

class FileMoveSelector : public QWidget, public Ui::FileMoveSelector
{
    Q_OBJECT

  public:
    enum FileMoveOption { Nothing, Suffix, ChangeDir, Delete };
    Q_ENUM(FileMoveOption)

    FileMoveSelector(QWidget* parent = 0, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~FileMoveSelector();

    Q_INVOKABLE QString        code()           const;
    Q_INVOKABLE QString        codeForOption(FileMoveOption poption) const;
    Q_INVOKABLE QString        destdir()        const;
    Q_INVOKABLE FileMoveOption option()         const;
    Q_INVOKABLE QString        suffix()         const;

  public slots:
    bool setCode(QString    pcode);
    bool setDestdir(QString pdir);
    bool setOption(FileMoveOption poption);
    bool setSuffix(QString  psuffix);

  protected slots:
    virtual void languageChange();

  private:

};

void setupFileMoveSelector(QScriptEngine *engine);

#endif
