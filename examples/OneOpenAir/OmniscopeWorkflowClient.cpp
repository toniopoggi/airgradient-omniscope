#include "OmniscopeWorkflowClient.h"

#include "Libraries/Arduino_JSON/src/Arduino_JSON.h"
#include <HTTPClient.h>

namespace {

// Sectigo Public Server Authentication Root E46.
// This is the public trust anchor currently used by *.omniscope.me.
static const char *const OMNISCOPE_ROOT_CA =
    "-----BEGIN CERTIFICATE-----\n"
    "MIICOjCCAcGgAwIBAgIQQvLM2htpN0RfFf51KBC49DAKBggqhkjOPQQDAzBfMQsw\n"
    "CQYDVQQGEwJHQjEYMBYGA1UEChMPU2VjdGlnbyBMaW1pdGVkMTYwNAYDVQQDEy1T\n"
    "ZWN0aWdvIFB1YmxpYyBTZXJ2ZXIgQXV0aGVudGljYXRpb24gUm9vdCBFNDYwHhcN\n"
    "MjEwMzIyMDAwMDAwWhcNNDYwMzIxMjM1OTU5WjBfMQswCQYDVQQGEwJHQjEYMBYG\n"
    "A1UEChMPU2VjdGlnbyBMaW1pdGVkMTYwNAYDVQQDEy1TZWN0aWdvIFB1YmxpYyBT\n"
    "ZXJ2ZXIgQXV0aGVudGljYXRpb24gUm9vdCBFNDYwdjAQBgcqhkjOPQIBBgUrgQQA\n"
    "IgNiAAR2+pmpbiDt+dd34wc7qNs9Xzjoq1WmVk/WSOrsfy2qw7LFeeyZYX8QeccC\n"
    "WvkEN/U0NSt3zn8gj1KjAIns1aeibVvjS5KToID1AZTc8GgHHs3u/iVStSBDHBv+\n"
    "6xnOQ6OjQjBAMB0GA1UdDgQWBBTRItpMWfFLXyY4qp3W7usNw/upYTAOBgNVHQ8B\n"
    "Af8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAwNnADBkAjAn7qRa\n"
    "qCG76UeXlImldCBteU/IvZNeWBj7LRoAasm4PdCkT0RHlAFWovgzJQxC36oCMB3q\n"
    "4S6ILuH5px0CMk7yn2xVdOOurvulGu7t0vzCAxHrRVxgED1cf5kDW21USAGKcw==\n"
    "-----END CERTIFICATE-----\n";

} // namespace

String OmniscopeWorkflowClient::base64Encode(const String &value) {
  static const char alphabet[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String encoded;
  encoded.reserve(((value.length() + 2) / 3) * 4);

  for (size_t i = 0; i < value.length(); i += 3) {
    const uint32_t octetA = static_cast<uint8_t>(value[i]);
    const bool hasB = (i + 1) < value.length();
    const bool hasC = (i + 2) < value.length();
    const uint32_t octetB = hasB ? static_cast<uint8_t>(value[i + 1]) : 0;
    const uint32_t octetC = hasC ? static_cast<uint8_t>(value[i + 2]) : 0;
    const uint32_t triple = (octetA << 16) | (octetB << 8) | octetC;

    encoded += alphabet[(triple >> 18) & 0x3F];
    encoded += alphabet[(triple >> 12) & 0x3F];
    encoded += hasB ? alphabet[(triple >> 6) & 0x3F] : '=';
    encoded += hasC ? alphabet[triple & 0x3F] : '=';
  }

  return encoded;
}

bool OmniscopeWorkflowClient::postMeasurement(
    const String &endpoint, const String &block, const String &parameterName,
    const String &username, const String &password,
    const String &measurementJson) {
  if (!endpoint.startsWith("https://")) {
    Serial.println("Omniscope: endpoint must use HTTPS");
    return false;
  }

  JSONVar blocks;
  blocks[0] = block;

  JSONVar update;
  update["name"] = parameterName;
  update["value"] = measurementJson;

  JSONVar updates;
  updates[0] = update;

  JSONVar params;
  params["updates"] = updates;
  params["waitForIdle"] = true;

  JSONVar request;
  request["blocks"] = blocks;
  request["refreshFromSource"] = true;
  request["cancelExisting"] = false;
  request["waitForIdle"] = true;
  request["params"] = params;

  const String body = JSON.stringify(request);
  const String authorization =
      "Basic " + base64Encode(username + ":" + password);

  HTTPClient client;
  client.setConnectTimeout(15000);
  client.setTimeout(15000);
  if (!client.begin(endpoint, OMNISCOPE_ROOT_CA)) {
    Serial.println("Omniscope: failed to initialise HTTPS client");
    return false;
  }

  client.addHeader("Content-Type", "application/json");
  client.addHeader("Authorization", authorization);
  Serial.printf("Omniscope: submitting workflow (%u-byte request)\n",
                static_cast<unsigned int>(body.length()));
  const int statusCode = client.POST(body);
  client.end();

  if (statusCode >= 200 && statusCode < 300) {
    Serial.printf("Omniscope: workflow submitted (HTTP %d)\n", statusCode);
    return true;
  }

  Serial.printf("Omniscope: workflow submission failed (HTTP %d)\n",
                statusCode);
  return false;
}
