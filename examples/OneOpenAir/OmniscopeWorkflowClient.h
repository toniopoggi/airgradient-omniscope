#ifndef _OMNISCOPE_WORKFLOW_CLIENT_H_
#define _OMNISCOPE_WORKFLOW_CLIENT_H_

#include <Arduino.h>

class OmniscopeWorkflowClient {
public:
  bool postMeasurement(const String &endpoint, const String &block,
                       const String &parameterName, const String &username,
                       const String &password, const String &measurementJson);

private:
  String base64Encode(const String &value);
};

#endif /** _OMNISCOPE_WORKFLOW_CLIENT_H_ */
