#include <Arduino.h>
#include <ThermalPrinter.h>

#define PRINTER_RX_PIN              2
#define PRINTER_TX_PIN              5
#define PRINTER_BAUDRATE            9600

#define PRINTER_MACROS              Macros_ThermPrn  // see ThermalPrinter.h

ThermalPrinter thp(PRINTER_RX_PIN, PRINTER_TX_PIN, PRINTER_BAUDRATE);
int thp_status = 0;

void serial_printer(){

    thp.begin();
    thp.loadMacros(&PRINTER_MACROS);   

    // it is possible to prepare printout template
    // containing predefined (see ThermapPrinter.h) 
    // and your own macros
    String ticket = "$init$a_center";
    ticket += "this is multi\nline ticket\n";
    ticket += "millis=$millis\n";
    ticket += "$u_onUnderlined text$u_off\n";
    ticket += "$fs_1Normal font size\n";
    ticket += "$fs_2Double size\n";
    ticket += "$fs_3Tripple size\n"; // not all printers support 3x and higher sizes
    ticket += "$fs_1\n\nAaaaand cut\n\n\n$cut_part";

    Serial.printf("Loading ticket to buffer\n");
    thp.load2Buffer(ticket.c_str());

    // add your own macro constants
    uint8_t m1[] = {27,45,1};
    thp.addMacro("$u_on",m1,3);

    uint8_t m2[] = {27,45,0};
    thp.addMacro("$u_off",m2,3);

    Serial.printf("Replacing previously loaded macros found in ticket\n");
    thp.replaceMacros(); 

    // Manually replace user defined macro
    char* m = new char[51];
    snprintf(m,50,"%lu",millis());
    thp.replaceMacro("$millis",(uint8_t*)m,strlen(m));
    delete []m;

    Serial.printf("Printing...\n");
    // send buffer to printer
    thp.writeBuffer();

    // and some ad-hoc printing
    thp.printNumStr("1b 21 30",16);   // sending printer codes directly
    thp.println("Double size");

    thp.startStatusUpdate(1000); // check interval in milliseconds
    thp.updatePrinterStatus();   // ontime request
    thp_status = thp.status();
    Serial.printf("Printer status=%x\n",thp_status);

}

void setup_serial_printer() {
    Serial.begin(115200);

    // wait few seconds
    for(int i=0;i<5;i++){
        Serial.printf("waiting %d\n",i);
        delay(1000);
    }

    serial_printer();
}

void loop_serial_printer(){
    thp.loop();
    if (thp.status() != thp_status){
        thp_status = thp.status();
        Serial.printf("Printer status changed=%x\n",thp_status);
    }

}