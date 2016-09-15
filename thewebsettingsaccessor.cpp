#include "thewebsettingsaccessor.h"

extern QVariantMap settingsData;

theWebSettingsAccessor::theWebSettingsAccessor(Browser browser)
{
    this->associatedBrowser = browser;

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings_get");
    browser.get()->SendProcessMessage(PID_BROWSER, message);
}

theWebSettingsAccessor::~theWebSettingsAccessor() {


}

void theWebSettingsAccessor::sync() {

}

bool theWebSettingsAccessor::Set(const CefString &name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString &exception) {
    if (name == "dnt") {
        if (value.get()->IsBool()) {
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("theWebSettings");
            CefRefPtr<CefListValue> args = message.get()->GetArgumentList();
            args.get()->SetString(0, "data/dnt");
            args.get()->SetString(1, "bool");
            args.get()->SetBool(2, value.get()->GetBoolValue());
            this->associatedBrowser.get()->SendProcessMessage(PID_BROWSER, message);

            settingsData.insert("data/dnt", value.get()->GetBoolValue());
        } else {
            exception = "Invalid Value Type";
        }
        return true;
    }
    return false;
}

bool theWebSettingsAccessor::Get(const CefString &name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value> &retval, CefString &exception) {
    if (name == "dnt") {
        retval = CefV8Value::CreateBool(settingsData.value("data/dnt", false).toBool());
        return true;
    }
    return false;
}

void theWebSettingsAccessor::AddRef() const {
    CefRefCount::AddRef();
}

bool theWebSettingsAccessor::Release() const {
    return CefRefCount::Release();
}

bool theWebSettingsAccessor::HasOneRef() const {
    return CefRefCount::HasOneRef();
}

V8Function::V8Function(void (*function)()) {
    this->functionToCall = function;
}

bool V8Function::Execute(const CefString &name, CefRefPtr<CefV8Value> object, const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception) {
    functionToCall();
    return true;
}