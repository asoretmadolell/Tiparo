/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/* Tiparo.ino                                                                */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#include "Arduino.h"

#include <Wire.h>

/*****************************************************************************/
/*                                                                           */
/* Defines                                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Globals                                                                   */
/*                                                                           */
/*****************************************************************************/
const int MPU_addr = 0x68;  // I2C address of the MPU-6050
int16_t GyX, GyY, GyZ, AcX, AcY, AcZ, Tmp;

int xOffset = 0;
int yOffset = 0;
int zOffset = 0;

int i = 0;

long X = 0;
long Y = 0;
long Z = 0;

unsigned long previousMillis = 0;
unsigned long currentMillis = millis();
const long interval = 10;

unsigned long TIMERpreviousMillis = 0;
unsigned long TIMERcurrentMillis = millis();
const long TIMERinterval = 1;

int resetButtonPin = 12;

int horizontalPosition = 90;
int verticalPosition = 90;
int sideTiltPosition = 90;

int buttonCurrent = 0;
int buttonPrevious = 0;
unsigned char buttonHistory = 0;

int test = 0;

//Servo horizontalServo;
//Servo verticalServo;
//Servo sideTiltServo;

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/* setup()                                                                   */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
void setup()
{
    Serial.begin( 9600 );
    gyroSetup();
}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/* loop()                                                                    */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
void loop()
{
    headTracking();
}

void GetCalibrationData()
{
    int i = 0;
    long xCal = 0;
    long yCal = 0;
    long zCal = 0;

    Serial.println( "Calculating Gyro Offset Values..." );

    while( i < 1000 )
    {
        Wire.beginTransmission( MPU_addr );
        Wire.write( 0x3B );  // starting with register 0x3B (ACCEL_XOUT_H)
        Wire.endTransmission( false );
        Wire.requestFrom( MPU_addr, 14, true ); // request a total of 14 registers
        AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
        AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
        AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
        Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
        GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
        GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
        GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

        xCal = xCal + GyX;
        yCal = yCal + GyY;
        zCal = zCal + GyZ;

        i = i + 1;
    }

    xOffset = xCal / 1000;
    yOffset = yCal / 1000;
    zOffset = zCal / 1000;

    Serial.print( "X Offset: " );
    Serial.println( xOffset );

    Serial.print( "Y Offset: " );
    Serial.println( yOffset );

    Serial.print( "Z Offset: " );
    Serial.println( zOffset );

    Serial.println( "--------------------" );
    Serial.println();

    delay( 2500 );

    Serial.println( "Ready" );

    digitalWrite( 13, HIGH );

    return;
}

void gyroSetup()
{
    pinMode( resetButtonPin, INPUT );
    pinMode( 13, OUTPUT );
    digitalWrite( 13, LOW );

    //horizontalServo.attach( 7 );
    //horizontalServo.write( horizontalPosition );
    delay( 15 );
    //verticalServo.attach( 6 );
    //verticalServo.write( verticalPosition );
    delay( 15 );
    //sideTiltServo.attach( 5 );
    //sideTiltServo.write( sideTiltPosition );
    delay( 15 );

    Serial.println( "Initializing Gyroscope..." );

    Wire.begin();
    Wire.beginTransmission( MPU_addr );
    Wire.write( 0x6B );  // PWR_MGMT_1 register
    Wire.write( 0 );     // set to zero (wakes up the MPU-6050)
    Wire.endTransmission( true );

    Serial.println( "Initialization Complete" );

    GetCalibrationData();

    return;
}

void headTracking()
{
    TIMERcurrentMillis = millis();
    if( TIMERcurrentMillis - TIMERpreviousMillis >= TIMERinterval )
    {
        //reset timer
        TIMERpreviousMillis = TIMERcurrentMillis;

        buttonPrevious = buttonCurrent;
        if( digitalRead( resetButtonPin ) == HIGH )
        {
            buttonCurrent = 1;
        }
        else
        {
            buttonCurrent = 0;
        }
        buttonHistory = ( buttonHistory << 1 ) | buttonCurrent;

        if( ( buttonPrevious == 0 ) && ( buttonCurrent == 1 ) )
        {
            test = 0;
        }
    }

    if( test == 0 )
    {
        if( buttonHistory == 0x0 )
        {
            test = 1;

            sideTiltPosition = 90;
            //sideTiltServo.write( sideTiltPosition );
            verticalPosition = 90;
            //verticalServo.write( verticalPosition );
            horizontalPosition = 90;
            //horizontalServo.write( horizontalPosition );
        }
    }

    Wire.beginTransmission( MPU_addr );
    Wire.write( 0x43 );  // starting with register 0x43 (GYRO_XOUT_H)
    Wire.endTransmission( false );
    Wire.requestFrom( MPU_addr, 14, true );  // request a total of 6 registers

    GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

    GyX = ( GyX - xOffset ) / 131;
    GyY = ( GyY - yOffset ) / 131;
    GyZ = ( GyZ - zOffset ) / 131;

    X = X + GyX;
    Y = Y + GyY;
    Z = Z + GyZ;

    i = i + 1;

    currentMillis = millis();
    if( currentMillis - previousMillis >= interval )
    {
        //take average
        X = X / i;
        Y = Y / i;
        Z = Z / i;

        //multiply by 0.01 (ten milliseconds)
        X = X * 0.02;
        Y = Y * 0.02;
        Z = Z * 0.02;

        //reset i
        i = 0;

        //reset timer
        previousMillis = currentMillis;

        //-------------------------------------------------------------------------------------
        sideTiltPosition = sideTiltPosition + X;

        if( ( sideTiltPosition <= 180 ) && ( sideTiltPosition >= 0 ) )
        {
            //sideTiltServo.write( sideTiltPosition );
            Serial.print( sideTiltPosition );
        }
        else if( sideTiltPosition > 180 )
        {
            sideTiltPosition = 180;
        }
        else if( horizontalPosition < 0 )
        {
            sideTiltPosition = 0;
        }
        //-------------------------------------------------------------------------------------

        //-------------------------------------------------------------------------------------
        verticalPosition = verticalPosition + Y;

        if( ( verticalPosition <= 180 ) && ( verticalPosition >= 0 ) )
        {
            //verticalServo.write( verticalPosition );
            Serial.print( verticalPosition );
        }
        else if( verticalPosition > 180 )
        {
            verticalPosition = 180;
        }
        else if( verticalPosition < 0 )
        {
            verticalPosition = 0;
        }
        //-------------------------------------------------------------------------------------

        //-------------------------------------------------------------------------------------
        horizontalPosition = horizontalPosition + Z;

        if( ( horizontalPosition <= 180 ) && ( horizontalPosition >= 0 ) )
        {
            //horizontalServo.write( horizontalPosition );
            Serial.println( horizontalPosition );
        }
        else if( horizontalPosition > 180 )
        {
            horizontalPosition = 180;
        }
        else if( horizontalPosition < 0 )
        {
            horizontalPosition = 0;
        }
        //-------------------------------------------------------------------------------------

        //reset averages
        X = 0;
        Y = 0;
        Z = 0;
    }

    return;
}

