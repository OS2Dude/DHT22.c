/**
 * DHT22.c
 * Read data (Temperature & Relative Humidity) from DHT22 sensor.
 * Data is transmitted bit by bit.  Start of bit is signaled by line going LOW,
 * then value of bit is determined by how long the line stays HIGH.
 * @author Chris Wolcott.  Based on code found at http://www.uugear.com/portfolio/read-dht1122-temperature-humidity-sensor-from-raspberry-pi/
 * Compile with: cc -Wall DHT22.c -o DHT22 -lwiringPi
 * Run with: ./DHT22
 */

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_TIMINGS 85                  // Takes 84 state changes to transmit data 
#define DHT_PIN     27                  // BCM16  GPIO-16  Phy 36 
#define BAD_VALUE   999.9f

int     data[5] = {0,0,0,0,0};
float   cachedH = BAD_VALUE;
float   cachedC = BAD_VALUE;

/**
 * read_DHT_Data
 * Signals DHT22 Sensor to send data.  Attemps to read data sensor sends.  If data passes checks, display it  
 * otherwise display cached values if valid else an error msg.
 * @param None
 * @return None
 */

void    read_DHT_Data()  {
    uint8_t lastState     = HIGH;
    uint8_t stateDuration = 0;
    uint8_t stateChanges  = 0;
    uint8_t bitsRead      = 0;
    float   h             = 0.0;
    float   c             = 0.0;
    float   f             = 0.0;
    char *  dataType;

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    /* Signal Sensor we're ready to read by pulling pin UP for 10 milliseconds,
       pulling pin down for 18 milliseconds and then back up for 40 microseconds. */

    pinMode( DHT_PIN, OUTPUT );
    digitalWrite( DHT_PIN, HIGH );
    delay(10);
    digitalWrite( DHT_PIN, LOW );
    delay(18);
    digitalWrite( DHT_PIN, HIGH );
    delayMicroseconds(40);

    /* Read data from pin.  Look for a change in state. */

    pinMode( DHT_PIN, INPUT );

    for ( (stateChanges=0), (stateDuration=0); (stateChanges < MAX_TIMINGS) && (stateDuration < 255) ; stateChanges++) {
        stateDuration  = 0;
        while( (digitalRead( DHT_PIN ) == lastState) && (stateDuration < 255) ) {
            stateDuration++;
            delayMicroseconds( 1 );
        };

        lastState = digitalRead( DHT_PIN );

        // First 2 state changes are sensor signaling ready to send, ignore them.
        // Each bit is preceeded by a state change to mark its beginning, ignore it too.
        if ( (stateChanges > 2) && (stateChanges % 2 == 0) ) {
            data[ bitsRead / 8 ] <<= 1; // Each array element has 8 bits.  Shift Left 1 bit.
            if ( stateDuration > 16 )   // A State Change > 16 microseconds is a '1'.
                data[ bitsRead / 8 ] |= 0x00000001;
            bitsRead++;
        }
    }

    /**
     * Read 40 bits. (Five elements of 8 bits each)  Last element is a checksum.
     */

    if ( (bitsRead >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
        h = (float)((data[0] << 8) + data[1]) / 10.0;
        c = (float)((data[2] << 8) + data[3]) / 10.0;
        if ( data[2] & 0x80 )           // Negative Sign Bit on.
            c *= -1;
        cachedH  = h;
        cachedC  = c;
        dataType = "Temperature";
    }
    else {                              // Data Bad, use cached values.
        h = cachedH;
        c = cachedC;
        dataType = "Cached Temp";
    }

    if ( (h == BAD_VALUE) || (c == BAD_VALUE) )
        printf("Data not good, Skipped\n");
    else {
        f = c * 1.8f + 32.0f;
        printf("%s: %-3.1f *C  (%-3.1f*F)  Humidity: %-3.1f%%\n",dataType,c,f,h);
    }
}


/**
 * main()
 * Program Entry Point.  Provides a way to test the above method.
 * @param None
 * @return int  Program Exist Status
 */

int main( void ) {

    if ( wiringPiSetup() == -1 )
        exit( 1 );

    for (int i=0; i<5000; i++) {
        read_DHT_Data();
        delay( 10000 );  /* Wait 10 seconds between readings. */
    }

    return(0);
}