/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "applock.h"

#include "ui_scriptEditor.h"

class JSHighlighter;

class scriptEditor : public XWidget, public Ui::scriptEditor
{
    Q_OBJECT

public:
    scriptEditor(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::WindowFlags {});
    ~scriptEditor();
    static bool saveFile(const QString &source, QString &filename);

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void populate();
    virtual bool sSaveFile();
    virtual bool sSaveToDB();
    virtual void sFind();
    virtual void sGoto();
    virtual void sImport();
    virtual bool sSave();
    virtual void sExtractWidgets(const bool pInclude = false);
    virtual void sExtractWidgetsWithLabels();
    virtual void sExtractWidgetsWithoutLabels();

protected slots:
    virtual void languageChange();
    virtual void setMode(const int);
    virtual void sBlockCountChanged(const int);
    virtual void sPositionChanged();
    virtual void sFindSignal();
    virtual void sFindDo();
    virtual void sClose();

  protected:
    virtual void closeEvent(QCloseEvent *);

private:
    int _mode;
    int _pkgheadidOrig;
    int _scriptid;
    JSHighlighter *_highlighter;
    QTextDocument *_document;
    int _findCnt;
    AppLock _lock;

};

#endif // SCRIPT_H
