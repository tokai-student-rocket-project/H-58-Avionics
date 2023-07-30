# CANBUS

## パケット一覧

| Id   | Label                  | length | From | To          | Rate | Format                                                                                                                                                                        |
| ---- | ---------------------- | ------ | ---- | ----------- | ---- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0x00 | Orientation            | 5B     | SM   | ACM         | 20Hz | [0] Axis (uint8_t), [1:5] Value (float)                                                                                                                                       |
| 0x01 | Acceleration           | 5B     | SM   | ACM         | 20Hz | [0] Axis (uint8_t), [1:5] Value (float)                                                                                                                                       |
| 0x02 | Altitude               | 4B     | SM   | FM, ACM     | 20Hz | [0:4] Value (float)                                                                                                                                                           |
| 0x03 | Outside Temperature    | 4B     | SM   | ACM         | 20Hz | [0:4] Value (float)                                                                                                                                                           |
| 0x04 | Voltage                | 6B     | FM   | SCM         | 10Hz | [0:2] Supply Voltage (int16_t), Pool Voltage [2:4] (int16_t), [4:6] Battery Voltage (int16_t) Voltage                                                                         |
| 0x05 | SystemStatus           | 8B     | FM   | SM, MM, SCM | 10Hz | [0] FlightMode (uint8_t), [1] CameraState (bool), [2] SeparationState (bool), [3] DoLogging (bool), [4:8] FlightTime (uint32_t)                                               |
| 0x06 | Event                  | 6B     |      | SCM         |      | [0] Publisher (uint8_t), [1] EventCode (uint8_t), [2:6] Timestamp (uint32_t)                                                                                                  |
| 0x07 | Error                  | 7B     |      | SCM         |      | [0] Publisher (uint8_t), [1] ErrorCode (uint8_t), [2] ErrorReason (uint8_t), [3:7] Timestamp (uint32_t)                                                                       |
| 0x08 | Set Reference Pressure | 4B     | SCM  | SM          |      | [0:4] ReferencePressure (float)                                                                                                                                               |
| 0x09 | Trajectory Data        | 1B     | SM   | FM          |      | [0] IsFalling (bool)                                                                                                                                                          |
| 0x10 | Sensing Status         | 8B     | SM   | SCM         |      | [0:4] ReferencePressure (float), [4] IsSystemCalibrated (bool), [5] IsGyroscopeCalibrated (bool), [6] isAccelerometerCalibrated (float), [7] isMagnetometerCalibrated (float) |

SM ... SensingModule  
FM ... FlightModule  
MM ... MissionModule  
ACM ... AirDataCommunicationModule  
SCM ... SystemDataCommunicationModule  
