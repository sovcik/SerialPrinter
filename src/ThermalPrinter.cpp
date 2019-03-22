#include "ThermalPrinter.h"

//#define DEBUG_THERMAL_PRINTER

#ifdef DEBUG_THERMAL_PRINTER
#ifdef DEBUG_ESP_PORT
#define DEBUG_TH_PRINT(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#define DEBUG_TH_ARRAY(ARR,ARR_L) for (uint16_t _aidx =0; _aidx<ARR_L;_aidx++) {DEBUG_ESP_PORT.printf("%02X ",*(ARR+_aidx)); if (_aidx%20 == 19)DEBUG_ESP_PORT.printf("\n");}
#else
#define DEBUG_TH_PRINT(...) os_printf( __VA_ARGS__ )
#define DEBUG_TH_ARRAY(ARR,ARR_L) for (uint16_t _aidx =0; _aidx<ARR_L;_aidx++) {os_printf("%02X ",*(ARR+_aidx)); if (_aidx%20 == 19)os_printf("\n");}
#endif
#endif

#ifndef DEBUG_THERMAL_PRINTER
#define DEBUG_TH_PRINT(...)
#define DEBUG_TH_ARRAY(...)
#endif

ThermalPrinter::ThermalPrinter(int rxPin, int txPin, unsigned int baudRate)
        :SerialPrinter(rxPin, txPin, baudRate){
    DEBUG_TH_PRINT("[ThPrn] printer created\n");
}

bool ThermalPrinter::updatePrinterStatus(){
    uint8_t statusQuery[3] = {16,4,0};  //Real-time transmission status
    int _ps;

    ss->flush();
    DEBUG_TH_PRINT("[ThPrn:readStatus] Checking printer status\n");

    _status = 0;  // reset printer status

    statusQuery[2] = 1; // query printer status

    _ps = writeWaitRead(&statusQuery[0], 3, PRINTER_STATUS_TIMEOUT);
    
    if (_ps >= 0 && _ps & 0x12) { //0x12 are pre-set bits, if not set, then most likely reading garbage
        DEBUG_TH_PRINT("[ThPrn:readStatus] status1=%X ",_ps);
        if (_ps & 0x08){
            DEBUG_TH_PRINT("OFFLINE ");
            _status |= PRN_STAT_BIT_OFFLINE;
        }
        if (_ps & 0x80){
            DEBUG_TH_PRINT("PAPER_NOT_TORN ");
            _status |= PRN_STAT_OTHER_ERROR;
        }
        DEBUG_TH_PRINT("\n");
    } else {
        DEBUG_TH_PRINT("[ThPrn:readStatus] failed reading status1\n");
        _status |= PRN_STAT_BIT_OFFLINE;
        return false;
    }
    

    statusQuery[2] = 2; // query transit off-line status
    _ps = writeWaitRead(&statusQuery[0],3,PRINTER_STATUS_TIMEOUT);
    
    if (_ps >= 0 && _ps & 0x12) {
        DEBUG_TH_PRINT("[ThPrn:readStatus] status2=%X ",_ps);
        if (_ps & 0x04){
            DEBUG_TH_PRINT("COVER_OPEN/SHAFT_UNSET ");
            _status |= PRN_STAT_BIT_COVER_OPEN;
            _status |= PRN_STAT_BIT_SHAFT_UNSET;
        }
        if (_ps & 0x20){
            DEBUG_TH_PRINT("LOW_PAPER ");
            _status |= PRN_STAT_BIT_PAPER_OUT;
        }
        if (_ps & 0x40){
            DEBUG_TH_PRINT("ERROR ");
            _status |= PRN_STAT_OTHER_ERROR;
        }
        DEBUG_TH_PRINT("\n");

    } else {
        DEBUG_TH_PRINT("[ThPrn:readStatus] failed reading status2\n");
        return false;
    }

    statusQuery[2] = 3; // transmit error status
    _ps = writeWaitRead(&statusQuery[0],3,PRINTER_STATUS_TIMEOUT);
    
    if (_ps >= 0 && _ps & 0x12) {
        DEBUG_TH_PRINT("[ThPrn:readStatus] status3=%X ",_ps);
        if (_ps & 0x08){
            DEBUG_TH_PRINT("CUTTER_ERROR ");
            _status |= PRN_STAT_BIT_CUTTER_ERROR;
        }
        if (_ps & 0x20){
            DEBUG_TH_PRINT("UNRECOVERABLE_ERROR ");
            _status |= PRN_STAT_UNRECOVERABLE_ERROR;
        }
        if (_ps & 0x40){
            DEBUG_TH_PRINT("TEMP&VOLTAGE_ERROR ");
            _status |= PRN_STAT_OTHER_ERROR;
        }

        DEBUG_TH_PRINT("\n");        

    } else {
        DEBUG_TH_PRINT("[ThPrn:readStatus] failed reading status3\n");
        return false;
    }

    statusQuery[2] = 4; // paper sensor status
    _ps = writeWaitRead(&statusQuery[0],3,PRINTER_STATUS_TIMEOUT);
    
    if (_ps >= 0 && _ps & 0x12) {
       DEBUG_TH_PRINT("[ThPrn:readStatus] status4=%X ",_ps);
       if (_ps & 0x0C){
            DEBUG_TH_PRINT("PAPER_NEAR_END ");
            _status |= PRN_STAT_BIT_PAPER_ENDING;
        }
        if (_ps & 0x60){
            DEBUG_TH_PRINT("PAPER_END ");
            _status |= PRN_STAT_BIT_PAPER_OUT;
        }

        DEBUG_TH_PRINT("\n"); 

    } else {
        DEBUG_TH_PRINT("[ThPrn:readStatus] failed reading status4\n");
        return false;
    }

    DEBUG_TH_PRINT("[ThPrn:readStatus] printer status=%X\n",_status);

    return true;

}
