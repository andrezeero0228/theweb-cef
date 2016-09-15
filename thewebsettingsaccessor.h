#ifndef THEWEBSETTINGSACCESSOR_H
#define THEWEBSETTINGSACCESSOR_H

#include "cefheaders.h"
#include "cefengine.h"
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QThread>

class theWebSettingsAccessor : public CefV8Accessor, public CefRefCount
{
public:
    theWebSettingsAccessor(Browser browser);
    ~theWebSettingsAccessor();

    void AddRef() const;
    bool Release() const;
    bool HasOneRef() const;

    bool Get(const CefString &name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value> &retval, CefString &exception) override;
    bool Set(const CefString &name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString &exception) override;

    void sync();
private:
    QSettings settings;
    Browser associatedBrowser;
};

class V8Function : public CefV8Handler, public CefEngine
{
public:
    V8Function(void (*function)());

    bool Execute(const CefString &name, CefRefPtr<CefV8Value> object, const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception) override;

private:
    void (*functionToCall)();
};

#endif // THEWEBSETTINGSACCESSOR_H
