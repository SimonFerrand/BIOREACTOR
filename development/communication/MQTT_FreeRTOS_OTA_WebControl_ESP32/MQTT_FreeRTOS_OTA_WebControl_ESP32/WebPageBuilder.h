// WebPageBuilder.h - Génération des pages HTML
#ifndef WEB_PAGE_BUILDER_H
#define WEB_PAGE_BUILDER_H

#include <Arduino.h>
#include "MessageFormatter.h"

class WebPageBuilder {
public:
    static String buildIndexPage(const String& deviceInfo);
    static String buildDataPage(const SensorData& data);
    static String buildConfigPage();
    
private:
    static String getHeader(const String& title);
    static String getFooter();
    static String getStyle();
    static String getScript();
};

#endif