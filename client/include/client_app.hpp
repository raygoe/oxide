#ifndef OXIDE_CLIENT_APP_HPP
#define OXIDE_CLIENT_APP_HPP

#include "include/cef_app.h"

class OxideApp : public CefApp,
                  public CefBrowserProcessHandler {
 public:
  OxideApp();

  // CefApp methods:
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
      OVERRIDE { return this; }

  // CefBrowserProcessHandler methods:
  virtual void OnContextInitialized() OVERRIDE;

 private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(OxideApp);
};

#endif // OXIDE_CLIENT_APP_HPP