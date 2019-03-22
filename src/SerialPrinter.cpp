#include <Arduino.h>
#include "SerialPrinter.h"

//#define DEBUG_SERIAL_PRINTER

#ifdef DEBUG_SERIAL_PRINTER
#ifdef DEBUG_ESP_PORT
#define DEBUG_SP_PRINT(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#define DEBUG_SP_ARRAY(ARR,ARR_L) for (uint16_t _aidx =0; _aidx<ARR_L;_aidx++) {DEBUG_ESP_PORT.printf("%02X ",*(ARR+_aidx)); if (_aidx%20 == 19)DEBUG_ESP_PORT.printf("\n");}
#else
#define DEBUG_SP_PRINT(...) os_printf( __VA_ARGS__ )
#define DEBUG_SP_ARRAY(ARR,ARR_L) for (uint16_t _aidx =0; _aidx<ARR_L;_aidx++) {os_printf("%02X ",*(ARR+_aidx)); if (_aidx%20 == 19)os_printf("\n");}
#endif
#endif

#ifndef DEBUG_SERIAL_PRINTER
#define DEBUG_SP_PRINT(...)
#define DEBUG_SP_ARRAY(...)
#endif


Printer::Printer(){   
    buff = new uint8_t[PRINTER_MAX_BUFF_SIZE];
    buffUsed = 0;
    updateInterval = 0;
    pc = NULL;

}

Printer::~Printer(){

    delete []buff;
    
    while(pc){
        TransTable *t = pc;
        pc = pc->next;
        delete t;
        free(t->macro);
        free(t->codes);
    }

}

void Printer::addMacro(const char* macro, const uint8_t* value, const uint16_t valueSize){
    DEBUG_SP_PRINT("[Prn:addMcr] adding macro=%s offset=%d length=%d\n",macro, *value, valueSize);
    TransTable* p = new TransTable();
    
    p->mSize = strlen(macro);
    p->macro = strdup(macro);

    p->cSize = valueSize;
    p->codes = (uint8_t*)malloc(p->cSize);
    if(p->codes){
        memcpy(p->codes, value, p->cSize);
        DEBUG_SP_PRINT("[Prn:addMcr] codes=");
        DEBUG_SP_ARRAY(value, valueSize);
        DEBUG_SP_PRINT("\n");
    
        p->next = pc;
        pc = p;

    } else {
        DEBUG_SP_PRINT("[Prn:addMcr] ERROR allocating memory for macro.\n")
    }
}

void Printer::loadMacros(const PrnMacros* macros){
    DEBUG_SP_PRINT("[Prn:loadMcr] loading %d macros from %X\n",macros->listSize, (unsigned int)macros);
    for(uint8_t i=0; i<macros->listSize; i++){
        addMacro(macros->list[i].macro, macros->map+macros->list[i].offset, macros->list[i].length);
    }
}

void Printer::clearBuffer(){
    buffUsed = 0;
}

void Printer::load2Buffer(const char* s){
    DEBUG_SP_PRINT("[Prn:L2b] %s\n",s);
    buffUsed = strlen(s);
    memcpy(buff,s,buffUsed);
}

bool Printer::write(uint8_t c){
    DEBUG_SP_PRINT("[Prn:write] c=%d\n",c);
    return true;
}

bool Printer::write(uint8_t* buff, uint16_t buffLen){
    DEBUG_SP_PRINT("[Prn:write] buffLen=%d\n",buffLen);
    return true;
}

bool Printer::writeBuffer(){
    DEBUG_SP_PRINT("[Prn:wrBuff] buffUsed=%d\n",buffUsed);
    return write(buff,buffUsed);
}


void Printer::replaceMacro(const char* s, uint8_t* value, uint16_t valueSize){
    DEBUG_SP_PRINT("[Prn:replMcr] macro=%s\n",s);
    buffUsed = replaceBytes(buff, buffUsed, PRINTER_MAX_BUFF_SIZE, (uint8_t*)s, strlen(s), value, valueSize);
}

void Printer::replaceMacros(){
    DEBUG_SP_PRINT("[Prn:replMacros]\n");

    TransTable* p = pc;
    while (p) {
        DEBUG_SP_PRINT("[Prn:replMcr] Replacing macro=%s with codes=",p->macro);
        DEBUG_SP_ARRAY(p->codes, p->cSize);
        DEBUG_SP_PRINT("\n");
        // replace macro with codes
        buffUsed = replaceBytes(buff, buffUsed, PRINTER_MAX_BUFF_SIZE, (uint8_t*)p->macro, p->mSize, p->codes, p->cSize);
        p = p->next;
    }
    
}

void Printer::startStatusUpdate(uint32_t interval){
    updateInterval = interval;
    lastUpdate = millis()-updateInterval;
}

void Printer::stopStatusUpdate(){
    updateInterval = 0;
}


void Printer::loop(){
    if (updateInterval && millis()-lastUpdate > updateInterval){
        DEBUG_SP_PRINT("[Prn:loop] Going to update printer status\n");
        updatePrinterStatus();
        lastUpdate = millis();
    }
}

bool Printer::printNumStr(const char *ns, int base){
    char* e = (char*)ns;
    
    bool success = true;

    while (*e != 0){
        long int i = strtol(e,&e,base);
        success &= write((uint8_t)i);
    }

    return success;
    
}

bool SerialPrinter::write(uint8_t c){
    DEBUG_SP_PRINT("[SerialPrn:write] data=\n",c);
    if (_status & PRN_STAT_BIT_OFFLINE){
        DEBUG_SP_PRINT("[SerialPrn:write] printer is offline\n");
        return false;
    }
    int w = ss->write(c); 
    DEBUG_SP_PRINT("\n==data written=%d\n",w);
    return w == 1;
}


bool SerialPrinter::write(uint8_t* buff, uint16_t buffLen){
    DEBUG_SP_PRINT("[SerialPrn:write] buffLen=%d data=\n",buffLen);
    if (_status & PRN_STAT_BIT_OFFLINE){
        DEBUG_SP_PRINT("[SerialPrn:write] printer is offline\n");
        return false;
    }
    DEBUG_SP_ARRAY(buff,buffLen);
    int w = ss->write(buff,buffLen); 
    DEBUG_SP_PRINT("\n==data written=%d\n",w);
    return w == buffLen;
}


size_t SerialPrinter::print(const char* s){
    DEBUG_SP_PRINT("[SerialPrn:print] %s\n",s);
    if (_status & PRN_STAT_BIT_OFFLINE){
        DEBUG_SP_PRINT("[SerialPrn:print] printer is offline\n");
        return 0;
    }
    return ss->print(s);
}

size_t SerialPrinter::print(int i){
    DEBUG_SP_PRINT("[SerialPrn:print] %d\n",i);
    if (_status & PRN_STAT_BIT_OFFLINE){
        DEBUG_SP_PRINT("[SerialPrn:print] printer is offline\n");
        return 0;
    }

    return ss->print(i);
}

size_t SerialPrinter::println(const char* s){
    DEBUG_SP_PRINT("[SerialPrn:println] %s\n",s);
    if (_status & PRN_STAT_BIT_OFFLINE){
        DEBUG_SP_PRINT("[SerialPrn:println] printer is offline\n");
        return 0;
    }

    return ss->println(s);
}

size_t SerialPrinter::println(int i){
    DEBUG_SP_PRINT("[SerialPrn:println] %d\n",i);
    if (_status & PRN_STAT_BIT_OFFLINE){
        DEBUG_SP_PRINT("[SerialPrn:println] printer is offline\n");
        return 0;
    }

    return ss->println(i);
}

SerialPrinter::SerialPrinter(int rxPin, int txPin, unsigned int baudRate)
        :Printer(){
    DEBUG_SP_PRINT("[SerialPrn] rx=%d, tx=%d, baudrate=%d\n", rxPin, txPin, baudRate);
    this->rxPin = rxPin;
    this->txPin = txPin;
    this->baudRate = baudRate;
    ss = new SoftwareSerial(rxPin, txPin);
    DEBUG_SP_PRINT("[SerialPrn] created\n");
}

SerialPrinter::~SerialPrinter(){
    ss->end();
    delete ss;
}

void SerialPrinter::begin(){
    DEBUG_SP_PRINT("[SerialPrn:begin] \n");
    //----- start printer
    ss->begin(baudRate);
}

int SerialPrinter::writeWaitRead(uint8_t* buff, size_t size, uint32_t wait){
    int _ps;
    uint32_t rxtimer = millis();

    ss->write(buff,size);

    while(!ss->available() && (millis()-rxtimer < wait)) {
        delay(0);
    };

    if (ss->available()){
        _ps = ss->read();
    } else {
        _ps = -1;
    }

    return _ps;

}

uint16_t Printer::replaceBytes(
    uint8_t* buff, const uint16_t buffUsed, const uint16_t buffSize, 
    const uint8_t* find, const uint16_t findSize, 
    const uint8_t* replace, const uint16_t replaceSize){

    uint16_t bu = buffUsed;
    uint16_t i = 0;
    uint8_t* bp = buff;
    uint16_t j;

    DEBUG_SP_PRINT("[utils:replB] start buffused=%d findSize=%d replSize=%d\n", bu, findSize, replaceSize);
    DEBUG_SP_PRINT("[before]%s\n\n",buff);
    
    while (i+findSize <= bu                                                      // if $find can fit into remaining buffer
           && (bu + replaceSize - findSize) < buffSize){                        // and if buffer will not overflow after replace
        j=0;
        while(j < findSize && *(bp+j)==*(find+j)) j++;                          // check if $find can be found on current position
        if (j == findSize) {                                                    // found -> replace
            DEBUG_SP_PRINT("[utils:replB] found at %d\n",(bp-buff));
            memmove(bp+replaceSize,bp+findSize,bu-(i+findSize));                 // move the rest of buffer to create space for $replace
            DEBUG_SP_PRINT("[after move]%s\n\n",buff);
            memcpy(bp,replace,replaceSize);                                     // copy $replace to created space
            DEBUG_SP_PRINT("[after replace]%s\n\n",buff);
            bu += (replaceSize - findSize);                                     // adjust used buffer size
            i += replaceSize;                                                   // set new buffer position to be after $replace
            bp += replaceSize;
        } else {                                                                // if not found
            i++;                                                                // move one position in buffer
            bp++;
        }
    }
    DEBUG_SP_PRINT("[utils:replB] after replacing buff used=%d\n",bu);
    DEBUG_SP_PRINT("[after]%s\n\n",buff);
    return bu;                                                                  // return used buffer size
}
