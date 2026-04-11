# Charles

Charles is an ESP-32 C6 based project with two servos. One controls the left/right movement of an eyeball. The other controls the open/closing of an eyelid

We will eventually use ESP-NOW to control this board from another ESP 32 that will have a set of buttons. For now we are building a basic demo of the servos.

Board power will come over USB and eventually over battery

Serveo battery will be external with common ground

## Remote control behavior (ESP-NOW)
- Encoder left/right moves the eyeball pan servo on D0.
- Boot mode uses FAST encoder gain: 13 deg per encoder step.
- Button 1 (bit 0) snaps the pan servo to 0 deg.
- Button 2 (bit 1) triggers a blink on the eyelid servo on D1.
- Button 3 (bit 2) snaps the pan servo to 90 deg.
- Button 4 (bit 3) snaps the pan servo to 135 deg.
- Button 5 (bit 4) toggles encoder gain mode between FAST (13) and SLOW (5).
- Encoder press recenters the pan servo to default when present in the payload.

## Wiring
 - left/right movement servo on pin D0
 - eye lid servo on pin D1

## Servos

Using Servo Motor Micro SG90
Product page: https://protosupplies.com/product/servo-motor-micro-sg90/

### Specs
Technical Specifications
Motor Model		Generic SG90 (China)
Drive Type		Analog
Degree Rotation		180° (±15°)
 Operating Ratings		
Voltage	4.8-6VDC  (5V Typical)
Current (idle)	10mA (typical)
Current (typical during movement)	100-250mA
Current (stall)	360mA (measured)
Stall Torque	1.7 kg-cm (measured)
Speed	0.12s / 60 degree (varies with VDC)
 Dimensions		
 Cable Length	 24cm  (9.5″)
 Motor Housing L x W x H	23 x 12 x 26mm (0.9 x0 .5 x  1″)
 Motor Height (w/ shaft)	32mm (1.26″)
Motor Housing Width with Mounting Ears	32mm (1.26″)