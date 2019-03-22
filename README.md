# SerialPrinter

Serial Printer library allowing directing printout with macros and templates

## Example

**Create Printer and Load Macro Definitions**

You can create your own printer macros for your specific printer type.

```
ThermalPrinter thp(PRINTER_RX_PIN, PRINTER_TX_PIN, PRINTER_BAUDRATE);
thp.begin();
thp.loadMacros(&PRINTER_MACROS);
```

**Add Your Own Macros**

```
uint8_t m1[] = {27,45,1};
thp.addMacro("$u_on",m1,3);

uint8_t m2[] = {27,45,0};
thp.addMacro("$u_off",m2,3);
```

**Define Template**

Mix macros & text inside template.

```
String ticket = "$init$a_center";
ticket += "this is multi\nline ticket\n";
ticket += "millis=$millis\n";
ticket += "$u_onUnderlined text$u_off\n";
ticket += "$fs_1Normal font size\n";
ticket += "$fs_2Double size\n";
ticket += "$fs_3Tripple size\n"; // not all printers support 3x and higher sizes
ticket += "$fs_1\n\nAaaaand cut\n\n\n$cut_part";
```
**Load Template into Buffer**

All string manupulation is done in printer class buffer.

```
thp.load2Buffer(ticket.c_str());
```

**Replace Macros**

Replace loaded and manually added macros with their values.

```
thp.replaceMacros(); 
```

If you need explicitly replace macros with some calculated/dynamic value, e.g. time

```
char* m = new char[51];
snprintf(m,50,"%lu",millis());
thp.replaceMacro("$millis",(uint8_t*)m,strlen(m));
```

**Print**

Send buffer to printer

```
thp.writeBuffer();
```
