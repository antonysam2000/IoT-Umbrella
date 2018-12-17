#include "mbed.h"
#include "uLCD_4DGL.h"
#include "TMP102.h"
 
Serial pc(USBTX, USBRX);
Serial esp(p28, p27); // tx, rx
DigitalOut reset(p26);
DigitalOut led1(LED1);
DigitalOut led4(LED4);
Timer t;

TMP102 temperature(p9, p10, 0x90);

AnalogIn rain(p16);
DigitalOut dout(LED3);
PwmOut myled1(p25);
PwmOut myled2(p24);
float tempC, tempF = 0.0f;
float pwmVal = 0.0f;
float stopVal = 1.0f;
char temp_str[10];
char rain_str[10];
//uLCD_4DGL uLCD(p9,p10,p11); 
 
int  count,ended,timeout;
char buf[2024];
char snd[1024];
 
char ssid[32] = "Ayooooo";     // enter WiFi router ssid inside the quotes
char pwd [32] = "jasonmkrist"; // enter WiFi router password inside the quotes
 
void SendCMD(),getreply(),ESPconfig(),ESPsetbaudrate(), update();
 void dev_recv()
{
    led1 = !led1;
    while(esp.readable()) {
        pc.putc(esp.getc());
    }
}
 
void pc_recv()
{
    led4 = !led4;
    while(pc.readable()) {
        esp.putc(pc.getc());
    }
}
 
 
int main()
{
    reset=0; //hardware reset for 8266
    pc.baud(9600);  // set what you want here depending on your terminal program speed
    pc.printf("\f\n\r-------------ESP8266 Hardware Reset-------------\n\r");
    wait(0.5);
    reset=1;
    timeout=2;
    getreply();
 
    esp.baud(9600);   // change this to the new ESP8266 baudrate if it is changed at any time.
 
    //ESPsetbaudrate();   //******************  include this routine to set a different ESP8266 baudrate  ******************
 
    ESPconfig();        //******************  include Config to set the ESP8266 configuration  ***********************
 
 
 
    pc.attach(&pc_recv, Serial::RxIrq);
    esp.attach(&dev_recv, Serial::RxIrq);
    
    // continuosly get AP list and IP
    while(1) {
        
        wait(10);
        //update();
    }
 
}
 
// Sets new ESP8266 baurate, change the esp.baud(xxxxx) to match your new setting once this has been executed
void ESPsetbaudrate()
{
    strcpy(snd, "AT+CIOBAUD=115200\r\n");   // change the numeric value to the required baudrate
    SendCMD();
}
 
//  +++++++++++++++++++++++++++++++++ This is for ESP8266 config only, run this once to set up the ESP8266 +++++++++++++++
void ESPconfig()
{

    wait(5);
    pc.printf("\f---------- Starting ESP Config ----------\r\n\n");
        strcpy(snd,".\r\n.\r\n");
    SendCMD();
        wait(1);
    pc.printf("---------- Reset & get Firmware ----------\r\n");
    strcpy(snd,"node.restart()\r\n");
    SendCMD();
    timeout=5;
    getreply();
    pc.printf(buf);
 
    wait(2);
 
    pc.printf("\n---------- Get Version ----------\r\n");
    strcpy(snd,"print(node.info())\r\n");
    SendCMD();
    timeout=4;
    getreply();
    pc.printf(buf);
 
    wait(3);
 
    // set CWMODE to 1=Station,2=AP,3=BOTH, default mode 1 (Station)
    pc.printf("\n---------- Setting Mode ----------\r\n");
    strcpy(snd, "wifi.setmode(wifi.STATION)\r\n");
    SendCMD();
    timeout=4;
    getreply();
    pc.printf(buf);
 
    wait(2);
 
   
 
    pc.printf("\n---------- Listing Access Points ----------\r\n");
    strcpy(snd, "function listap(t)\r\n");
        SendCMD();
        wait(1);
        strcpy(snd, "for k,v in pairs(t) do\r\n");
        SendCMD();
        wait(1);
        strcpy(snd, "print(k..\" : \"..v)\r\n");
        SendCMD();
        wait(1);
        strcpy(snd, "end\r\n");
        SendCMD();
        wait(1);
        strcpy(snd, "end\r\n");
        SendCMD();
        wait(1);
        strcpy(snd, "wifi.sta.getap(listap)\r\n");
    SendCMD();
    wait(1);
        timeout=15;
    getreply();
    pc.printf(buf);
 
    wait(2);
 
    pc.printf("\n---------- Connecting to AP ----------\r\n");
    pc.printf("ssid = %s   pwd = %s\r\n",ssid,pwd);
    strcpy(snd, "wifi.sta.config(\"");
    strcat(snd, ssid);
    strcat(snd, "\",\"");
    strcat(snd, pwd);
    strcat(snd, "\")\r\n");
    SendCMD();
    timeout=10;
    getreply();
    pc.printf(buf);
 
    wait(5);
 
    pc.printf("\n---------- Get IP's ----------\r\n");
    strcpy(snd, "print(wifi.sta.getip())\r\n");
    SendCMD();
    timeout=3;
    getreply();
    pc.printf(buf);
 
    wait(1);
 
    pc.printf("\n---------- Get Connection Status ----------\r\n");
    strcpy(snd, "print(wifi.sta.status())\r\n");
    SendCMD();
    timeout=5;
    getreply();
    pc.printf(buf);
 
    pc.printf("\n\n\n  If you get a valid (non zero) IP, ESP8266 has been set up.\r\n");
    pc.printf("  Run this if you want to reconfig the ESP8266 at any time.\r\n");
    pc.printf("  It saves the SSID and password settings internally\r\n");
    wait(10);
        
        
         tempC = temperature.read();
        tempF = (9.0*tempC)/5.0 + 32.0;      
        pwmVal = (tempF - 55)/40;
        if(pwmVal > 1) 
            pwmVal = 1;
        if(pwmVal < 0) 
            pwmVal = 0;
                
        myled1 = pwmVal;        //blue LED
        myled2 = 1 - pwmVal;    //red  LED
            
        wait(.15);
        if(rain > 0.07) 
        {
            myled1 = stopVal;
            myled2 = stopVal;
            wait(.15);
            strcpy(rain_str, "Raining");
        }
        else
        {
            strcpy(rain_str, "Dry");
        }
        myled1 = pwmVal;        //blue LED
        myled2 = 1 - pwmVal;    //red  LED
            
        //print current temp
        printf("%5.2F C %5.2F F %F \n\r", tempC, tempF, rain.read());
        sprintf(temp_str,"%2.3f",tempF);
        
          
        
                    pc.printf("\n---------- Setting up http server ----------\r\n");
    strcpy(snd, "srv=net.createServer(net.TCP)\r\n");
        SendCMD();
        wait(1);
        strcpy(snd, "srv:listen(80,function(conn)\r\n");
        SendCMD();
        wait(1);
        strcpy(snd, "conn:on(\"receive\",function(conn,payload)\r\n");
        SendCMD();
        wait(1);
        strcpy(snd, "print(payload)\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "conn:send(\"<!DOCTYPE html>\")\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "conn:send(\"<html>\")\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "conn:send(\"<h1> Hi James, NodeMcu.</h1>\")\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "conn:send(\"<h3>Temperature (F): ");
        strcat(snd, temp_str);
        strcat(snd, "</h3>\")\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "conn:send(\"<h3>Rain Status: ");
        strcat(snd, rain_str);
        strcat(snd, "</h3>\")\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "conn:send(\"</html>\")\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "end)\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "conn:on(\"sent\",function(conn) conn:close() end)\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "end)\r\n");
        SendCMD();
        wait(1);
        
        timeout=17;
        getreply();
        pc.printf(buf);
        pc.printf("\r\nDONE");
        
 
}
 
void SendCMD()
{
    esp.printf("%s", snd);
}
 
void getreply()
{
    memset(buf, '\0', sizeof(buf));
    t.start();
    ended=0;
    count=0;
    while(!ended) {
        if(esp.readable()) {
            buf[count] = esp.getc();
            count++;
        }
        if(t.read() > timeout) {
            ended = 1;
            t.stop();
            t.reset();
        }
    }
}
 
void update()
{
        tempC = temperature.read();
        tempF = (9.0*tempC)/5.0 + 32.0;      
        pwmVal = (tempF - 55)/40;
        if(pwmVal > 1) 
            pwmVal = 1;
        if(pwmVal < 0) 
            pwmVal = 0;
                
        myled1 = pwmVal;        //blue LED
        myled2 = 1 - pwmVal;    //red  LED
            
        wait(.15);
        if(rain > 0.07) 
        {
            myled1 = stopVal;
            myled2 = stopVal;
            wait(.15);
        }
        myled1 = pwmVal;        //blue LED
        myled2 = 1 - pwmVal;    //red  LED
            
        //print current temp
        printf("%5.2F C %5.2F F %F \n\r", tempC, tempF, rain.read());
        sprintf(temp_str,"%2.3f",tempF);
        
                    pc.printf("\n---------- Setting up http server ----------\r\n");

        strcpy(snd, "conn:on(\"receive\",function(conn,payload)\r\n");
        SendCMD();
        wait(1);
        strcpy(snd, "print(payload)\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "conn:send(\"<!DOCTYPE html>\")\r\n");
        SendCMD();
      wait(1);
        
        strcpy(snd, "conn:send(\"<html>\")\r\n");
        SendCMD();
      wait(1);
        
        strcpy(snd, "conn:send(\"<h1> Hi James, NodeMcu.</h1>\")\r\n");
      SendCMD();
        wait(1);
        
        strcpy(snd, "conn:send(\"<h3>Temperature (F): ");
        strcat(snd, temp_str);
        strcat(snd, "</h3>\")\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "conn:send(\"</html>\")\r\n");
        SendCMD();
        wait(1);
        
        strcpy(snd, "end)\r\n");
        SendCMD();
        wait(1);
            
        strcpy(snd, "conn:on(\"sent\",function(conn) conn:close() end)\r\n");
        SendCMD();
        wait(1);
        strcpy(snd, "end)\r\n");
        SendCMD();
        wait(1);
        
        timeout=17;
        getreply();
        pc.printf(buf);
        pc.printf("\r\nDONE");
        
}
    