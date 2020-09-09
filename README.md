# PM2100ModbusMaster
Wiring :

PM2100    <==>    Modul RS485

      A       <==>        A

      B       <==>        B
  
Modul RS485 <==>  ESP32 Board

    VCC     <==>    VIN

    GND     <==>    GND

    DI      <==>    TX

    RO      <==>    RX

    DE      <==>    18

    RE      <==>    19

Read data address

node.readHoldingRegisters(address, length);
