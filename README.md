

# Smart Gate Controller — QA Portfolio Project

## About the Project

## System Under Test

## Testing Approach

## Test Cases

## Test Results

## Defects Found

## Evidence

## Wokwi Simulation

## Skills Demonstrated


Project: Smart Gate Controller — Embedded QA Testing

Testing type:
Functional testing
Black-box testing
Negative testing
Regression testing
State transition testing

Environment:
Wokwi
Arduino Uno

Tested states:
CLOSED
   ↓ OPEN
OPENING
   ↓ LIMIT_OPEN
OPEN

OPEN
   ↓ CLOSE
CLOSING
   ↓ LIMIT_CLOSE
CLOSED

OPENING/CLOSING
       ↓ STOP
    STOPPED

Any state
   ↓ invalid condition
    ERROR
