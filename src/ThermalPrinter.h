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

#ifndef __THERMALPRINTER_H__
#define __THERMALPRINTER_H__

#include "SerialPrinter.h"

const uint8_t binmap_ThermPrn[] = {
    // init
    27, 64,                             // printer reset
    27, 61, 1,                          // set printer ONLINE

    // align center
    27, 97, 1,                           // align center ESC a n

    // align left
    27, 97, 0,                           // align left ESC a n

    // align right
    27, 97, 2,                           // align right ESC a n

    // partial cut
    27, 109,                            // partial paper cut: ESC m

    // full cut
    27, 105,                            // full paper cut: ESC i

    // font size normal
    29, 33, 0x00,                       // GS ! n

    // font size double
    29, 33, 0x11,                       // GS !

    // font size x3
    29, 33, 0x22,                       // GS !
    
    // font size x4
    29, 33, 0x44,                       // GS !

    // font size x5
    29, 33, 0x55,                       // GS !

    // raster image
    29, 118, 48, 3,                      // GS v 0 m

    // codepage 437
    27, 116, 0,

    // codepage ISO-8859-1
    27, 116, 23,

    // codepage ISO-8859-2
    27, 116, 36

};

const PrnMacro macro_list_ThermPrn[] = {
    // macro, offset, length
    {(char*)"$init", 0,5},                 // init
    {(char*)"$a_center", 5,3},             // align center
    {(char*)"$a_left", 8,3},               // align left
    {(char*)"$a_right", 11,3},             // align right
    {(char*)"$cut_part",14,2},             // partial cut
    {(char*)"$cut_full",17,2},             // full cut
    {(char*)"$fs_1",18,3},                 // font size normal
    {(char*)"$fs_2",21,3},                 // font size x2
    {(char*)"$fs_3",24,3},                 // font size x3
    {(char*)"$fs_4",27,3},                 // font size x4
    {(char*)"$fs_5",30,3},                 // font size x5
    {(char*)"$image",33,4},                // raster image
    {(char*)"$cp473",37,3},                // codepage 473
    {(char*)"$cpISO-8859-1",40,3},         // codepage ISO-8859-1
    {(char*)"$cpISO-8859-2",43,3}          // codepage ISO-8859-2
};

const PrnMacros Macros_ThermPrn = {
    (uint8_t*) binmap_ThermPrn,               // pointer to map defined above 
    (PrnMacro*)macro_list_ThermPrn,           // pointer to list of macros defined above
    10                                        // count of macros defined above
};

class ThermalPrinter : public SerialPrinter {

    public:
        ThermalPrinter(int rxPin, int txPin, unsigned int baudRate);

        bool updatePrinterStatus() override;

};

#endif

