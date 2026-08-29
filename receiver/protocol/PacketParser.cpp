#include "PacketParser.h"

// =====================================================
// =====================================================
//              APPLICATION PACKET PARSER
// =====================================================
// =====================================================
//
// Parses:
//
// TS|v=1|id=1|seq=123|dist=43.2|bat=3.87|chg=0
//
// Important:
//
// 1. Unknown fields are ignored.
// 2. Field order doesn't matter.
// 3. Fields can be added later.
// 4. Optional fields may be missing.
// 5. Required fields must exist.
// 6. Invalid values reject the packet.
// =====================================================

bool parseApplicationPacket(
  const String &payload,
  int sourceAddress,
  ParsedPacket &result
) {

  result.versionFound =
    false;

  result.sequenceFound =
    false;

  result.distanceFound =
    false;

  result.batteryFound =
    false;

  result.chargingFound =
    false;

  result.version =
    0;

  result.sequence =
    0;

  result.distanceCm =
    0.0;

  result.batteryVoltage =
    0.0;

  result.charging =
    false;

  int start =
    0;

  bool firstField =
    true;

  while (
    start <
    payload.length()
  ) {

    int separator =
      payload.indexOf(
        '|',
        start
      );

    String field;

    if (
      separator < 0
    ) {

      field =
        payload.substring(
          start
        );

      start =
        payload.length();

    } else {

      field =
        payload.substring(
          start,
          separator
        );

      start =
        separator + 1;
    }

    field.trim();

    // -------------------------------------------------
    // Header
    // -------------------------------------------------

    if (
      firstField
    ) {

      firstField =
        false;

      if (
        packetConfig.requireHeader &&
        field != PACKET_HEADER
      ) {

        Serial.println(
          "Packet rejected: bad header"
        );

        return false;
      }

      continue;
    }

    // -------------------------------------------------
    // key=value
    // -------------------------------------------------

    int equals =
      field.indexOf('=');

    if (
      equals < 0
    ) {

      Serial.print(
        "Ignoring malformed field: "
      );

      Serial.println(
        field
      );

      continue;
    }

    String key =
      field.substring(
        0,
        equals
      );

    String value =
      field.substring(
        equals + 1
      );

    key.trim();
    value.trim();

    // =================================================
    // VERSION
    // =================================================

    if (
      key == "v"
    ) {

      result.versionFound =
        true;

      result.version =
        value.toInt();

      if (
        result.version !=
        SUPPORTED_PACKET_VERSION
      ) {

        Serial.print(
          "Packet rejected: unsupported version "
        );

        Serial.println(
          result.version
        );

        return false;
      }
    }

    // =================================================
    // ID (Optional field)
    // =================================================
    else if (
      key == "id"
    ) {
      // Ignored: The actual RYLR998 LoRa source address is the authoritative identifier
    }

    // =================================================
    // SEQUENCE
    // =================================================

    else if (
      key == "seq" &&
      packetConfig.sequence.enabled
    ) {

      result.sequenceFound =
        true;

      result.sequence =
        value.toInt();
    }

    // =================================================
    // DISTANCE
    // =================================================

    else if (
      key == "dist" &&
      packetConfig.distance.enabled
    ) {

      result.distanceFound =
        true;

      result.distanceCm =
        value.toFloat();
    }

    // =================================================
    // BATTERY
    // =================================================

    else if (
      key == "bat" &&
      packetConfig.batteryVoltage.enabled
    ) {

      result.batteryFound =
        true;

      result.batteryVoltage =
        value.toFloat();
    }

    // =================================================
    // CHARGING
    // =================================================

    else if (
      key == "chg" &&
      packetConfig.charging.enabled
    ) {

      result.chargingFound =
        true;

      result.charging =
        value.toInt() != 0;
    }

    // =================================================
    // UNKNOWN FIELD
    // =================================================

    else {

      Serial.print(
        "Ignoring unknown field: "
      );

      Serial.println(
        key
      );
    }
  }

  // ===================================================
  // Required field validation
  // ===================================================

  if (
    packetConfig.requireVersion &&
    !result.versionFound
  ) {

    Serial.println(
      "Packet rejected: version missing"
    );

    return false;
  }

  if (
    packetConfig.sequence.required &&
    !result.sequenceFound
  ) {

    Serial.println(
      "Packet rejected: sequence missing"
    );

    return false;
  }

  if (
    packetConfig.distance.required &&
    !result.distanceFound
  ) {

    Serial.println(
      "Packet rejected: distance missing"
    );

    return false;
  }

  if (
    packetConfig.batteryVoltage.required &&
    !result.batteryFound
  ) {

    Serial.println(
      "Packet rejected: battery voltage missing"
    );

    return false;
  }

  if (
    packetConfig.charging.required &&
    !result.chargingFound
  ) {

    Serial.println(
      "Packet rejected: charging state missing"
    );

    return false;
  }

  // ===================================================
  // Value validation
  // ===================================================

  if (
    result.distanceFound &&
    result.distanceCm < 0.0
  ) {

    Serial.println(
      "Packet rejected: invalid distance"
    );

    return false;
  }

  if (
    result.batteryFound &&
    result.batteryVoltage <= 0.0
  ) {

    Serial.println(
      "Packet rejected: invalid battery voltage"
    );

    return false;
  }

  return true;
}
