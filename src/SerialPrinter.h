/*MIT License

Copyright (c) 2019 Jozef Sovcik

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#ifndef __SERIALPRINTER_H__
#define __SERIALPRINTER_H__

#include <Arduino.h>
#include <SoftwareSerial.h>

#define PRINTER_MAX_BUFF_SIZE   (uint16_t)1024  // space allocated in heap

#define PRN_STAT_BIT_OFFLINE            0x01   // 
#define PRN_STAT_BIT_PAPER_OUT          0x02   // paper out
#define PRN_STAT_BIT_PAPER_ENDING       0x04
#define PRN_STAT_BIT_SHAFT_UNSET        0x08
#define PRN_STAT_BIT_COVER_OPEN         0x10
#define PRN_STAT_BIT_CUTTER_ERROR       0x20
#define PRN_STAT_OTHER_ERROR            0x40
#define PRN_STAT_UNRECOVERABLE_ERROR    0x80

#define PRINTER_STATUS_TIMEOUT      1000        // expecting status reply from printer within this time

enum Printer_Status {
    PRN_OFF,
    PRN_READY,
    PRN_ERROR,
    PRN_OFFLINE
};

struct PrnMacro {
    char* macro;
    uint16_t offset;
    uint16_t length;
};

struct PrnMacros {
    uint8_t*    map;
    PrnMacro*   list;
    uint8_t     listSize;
};

class Printer {
    public:
        // translation table string -> array of printer codes
        struct TransTable {
            char* macro;
            uint16_t mSize;
            uint8_t* codes;
            uint16_t cSize;
            TransTable* next;
        };

    protected:
        TransTable* pc;
        uint8_t* buff;
        uint16_t buffUsed;
        
        int _status;

        uint16_t replaceBytes(
            uint8_t* buff, const uint16_t buffUsed, const uint16_t buffSize, 
            const uint8_t* find, const uint16_t findSize, 
            const uint8_t* replace, const uint16_t replaceSize);

        uint32_t lastUpdate;
        uint32_t updateInterval;            

    public:
        Printer();
        virtual ~Printer();
        
        virtual void begin() =0;

        virtual bool write(uint8_t* buff, uint16_t buffLen);
        virtual bool write(uint8_t c);
        
        virtual size_t print(const char* s) =0;
        virtual size_t print(int i) =0;
        virtual size_t println(const char* s) =0;
        virtual size_t println(int i) =0;
        
        virtual bool printNumStr(const char* ns, int base=10);
        
        void clearBuffer();
        void load2Buffer(const char* s);
        bool writeBuffer();

        // load printer macros
        void loadMacros(const PrnMacros* macros);
        void addMacro(const char* macro, const uint8_t* value, const uint16_t valueSize);
        void replaceMacros();
        
        void replaceMacro(const char* s, uint8_t* value, uint16_t valueSize);

        // will start reqular printer status query
        // requires loop() to be called regularly
        void startStatusUpdate(uint32_t interval); 
        void stopStatusUpdate();
        virtual bool updatePrinterStatus()=0;
        virtual inline int status(){return _status;};

        virtual void loop();  // loop handles regular status queries

};

class SerialPrinter : public Printer {
    protected:
        SoftwareSerial* ss;
        int rxPin, txPin;
        unsigned int baudRate;

        int writeWaitRead(uint8_t* buff, size_t size, uint32_t wait=100);

    public:
        SerialPrinter(int rxPin, int txPin, unsigned int baudRate);
        virtual ~SerialPrinter();
        virtual void begin() override;
        virtual bool write(uint8_t* buff, uint16_t buffLen) override;
        virtual bool write(uint8_t c) override;
        virtual size_t print(const char* s) override;
        virtual size_t print(int i) override;
        virtual size_t println(const char* s) override;
        virtual size_t println(int i) override;
    
};

#endif